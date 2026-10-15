[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]$LanAddress,

    [ValidateRange(1,65534)]
    [int]$HttpPort=37680,

    [switch]$Install,

    [string]$UserName=''
)

$ErrorActionPreference='Stop'
Set-StrictMode -Version 2.0

function Quote-Arg([string]$Value){
    if($null -eq $Value){return '""'}
    if($Value -notmatch '[\s"]'){return $Value}
    return '"'+($Value -replace '(\\*)"','$1$1\\"' -replace '(\\+)$','$1$1')+'"'
}

function Test-Admin {
    $identity=[Security.Principal.WindowsIdentity]::GetCurrent()
    $principal=New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

$exactPrefix="http://$LanAddress`:$HttpPort/"
$loopbackIpPrefix="http://127.0.0.1`:$HttpPort/"
$localhostPrefix="http://localhost`:$HttpPort/"
$frontendPrefixes=@($loopbackIpPrefix,$localhostPrefix,$exactPrefix)

function Test-FrontendPrefixPermission {
    $probe=[Net.HttpListener]::new()
    try{
        foreach($prefix in $frontendPrefixes){$probe.Prefixes.Add($prefix)}
        $probe.Start()
        return [pscustomobject]@{ok=$true;code=0;message=''}
    }catch [System.Net.HttpListenerException]{
        return [pscustomobject]@{ok=$false;code=[int]$_.Exception.NativeErrorCode;message=$_.Exception.Message}
    }finally{
        try{if($probe.IsListening){$probe.Stop()}}catch{}
        try{$probe.Close()}catch{}
    }
}

function Get-UrlAclState {
    param([string]$Prefix,[string]$ExpectedUser)
    $lines=@(& netsh.exe http show urlacl "url=$Prefix" 2>&1)
    $exit=[int]$LASTEXITCODE
    $text=($lines|Out-String)
    $exists=($exit -eq 0 -and $text -match [regex]::Escape($Prefix))
    $userMatch=$false
    if($exists -and -not[string]::IsNullOrWhiteSpace($ExpectedUser)){
        $userMatch=($text -match [regex]::Escape($ExpectedUser))
    }
    return [pscustomobject]@{exists=$exists;userMatch=$userMatch;exitCode=$exit;text=$text}
}

function Ensure-ExactUrlAcl {
    param([string]$Prefix,[string]$ExpectedUser)
    $state=Get-UrlAclState -Prefix $Prefix -ExpectedUser $ExpectedUser
    if($state.exists){
        if($state.userMatch){
            Write-Host "LAN URL ACL already present for ${ExpectedUser}: $Prefix" -ForegroundColor DarkGreen
            return
        }
        throw "HTTP URL reservation $Prefix already exists but is not assigned to $ExpectedUser. Refusing to replace another reservation automatically. Inspect it with: netsh http show urlacl url=$Prefix"
    }

    Write-Host "Creating LAN URL ACL for ${ExpectedUser}: $Prefix" -ForegroundColor Yellow
    & netsh.exe http add urlacl "url=$Prefix" "user=$ExpectedUser" listen=yes delegate=no | Out-Host
    if($LASTEXITCODE -ne 0){throw "Could not reserve HTTP namespace $Prefix for $ExpectedUser. Inspect reservations with: netsh http show urlacl"}

    $verify=Get-UrlAclState -Prefix $Prefix -ExpectedUser $ExpectedUser
    if(-not $verify.exists -or -not $verify.userMatch){throw "HTTP URL reservation verification failed for $Prefix / $ExpectedUser after netsh reported success."}
}

$ruleName="ChaseHQ Workbench LAN $HttpPort"
function Test-LanFirewallRule {
    try{
        $cmd=Get-Command Get-NetFirewallRule -ErrorAction SilentlyContinue
        if($null -eq $cmd){return $false}
        $rules=@(Get-NetFirewallRule -DisplayName $ruleName -ErrorAction SilentlyContinue | Where-Object{$_.Enabled -eq 'True'})
        return ($rules.Count -gt 0)
    }catch{return $false}
}

if([string]::IsNullOrWhiteSpace($UserName)){
    $UserName=((& whoami.exe) | Out-String).Trim()
}
if([string]::IsNullOrWhiteSpace($UserName)){throw 'Could not determine the current Windows user for the HTTP URL reservation.'}

if($Install){
    if(-not(Test-Admin)){throw 'LAN prerequisite installation requires an elevated PowerShell process.'}

    # IMPORTANT: do not use an elevated HttpListener bind test to decide whether
    # the normal user needs a URL ACL. Administrators can often bind regardless,
    # which previously caused a false setup success and a later error 5 when the
    # non-elevated launcher resumed. The elevated phase must ensure the ACL for
    # the ORIGINAL user explicitly.
    foreach($prefix in $frontendPrefixes){Ensure-ExactUrlAcl -Prefix $prefix -ExpectedUser $UserName}

    if(-not(Test-LanFirewallRule)){
        $fw=Get-Command New-NetFirewallRule -ErrorAction SilentlyContinue
        if($null -eq $fw){throw 'New-NetFirewallRule is unavailable; cannot create the Private-profile LAN firewall rule automatically.'}
        New-NetFirewallRule -DisplayName $ruleName -Direction Inbound -Action Allow -Protocol TCP -LocalPort $HttpPort -Profile Private | Out-Null
    }
    exit 0
}

$prefixState=Test-FrontendPrefixPermission
$firewallReady=Test-LanFirewallRule
if($prefixState.ok -and $firewallReady){
    Write-Host "LAN HTTP namespaces: PASS (3 frontend prefixes; LAN $LanAddress`:$HttpPort)" -ForegroundColor Green
    Write-Host "LAN firewall rule:  PASS ($ruleName)" -ForegroundColor Green
    exit 0
}

$needs=@()
if(-not $prefixState.ok){
    if($prefixState.code -ne 5){throw "LAN listener preflight failed for $exactPrefix (error $($prefixState.code)): $($prefixState.message)"}
    $needs+='HTTP URL reservation'
}
if(-not $firewallReady){$needs+='Private-profile firewall rule'}

if(Test-Admin){
    & $PSCommandPath -LanAddress $LanAddress -HttpPort $HttpPort -Install -UserName $UserName
    if($LASTEXITCODE -ne 0){throw 'Elevated LAN prerequisite installation failed.'}
}else{
    Write-Host ('First-use LAN setup requires elevation: '+($needs -join ' + ')) -ForegroundColor Yellow
    Write-Host 'A one-time Windows UAC prompt will follow. Use -LocalOnly to opt out of LAN/mobile access.' -ForegroundColor Yellow
    $parts=@('-NoLogo','-NoProfile','-ExecutionPolicy','Bypass','-File',$PSCommandPath,'-LanAddress',$LanAddress,'-HttpPort',[string]$HttpPort,'-Install','-UserName',$UserName)
    $argLine=(($parts|ForEach-Object{Quote-Arg $_}) -join ' ')
    try{$proc=Start-Process -FilePath 'powershell.exe' -Verb RunAs -ArgumentList $argLine -Wait -PassThru}catch{throw "LAN setup was not completed. Re-run and approve the UAC prompt, or launch with -LocalOnly. $($_.Exception.Message)"}
    if($proc.ExitCode -ne 0){throw "Elevated LAN prerequisite setup failed with exit code $($proc.ExitCode). Use -LocalOnly for loopback-only operation."}
}

# Final proof MUST be performed back in the original non-elevated process.
$aclFailures=@()
foreach($prefix in $frontendPrefixes){
    $aclVerify=Get-UrlAclState -Prefix $prefix -ExpectedUser $UserName
    if(-not $aclVerify.exists -or -not $aclVerify.userMatch){$aclFailures += $prefix}
}
if($aclFailures.Count -gt 0){throw ("Frontend URL reservation(s) are not present for $UserName after setup: "+($aclFailures -join ', '))}
$verify=Test-FrontendPrefixPermission
if(-not $verify.ok){throw "Frontend HTTP namespaces are still unavailable to $UserName after setup (error $($verify.code)): $($verify.message)"}
if(-not(Test-LanFirewallRule)){throw "LAN firewall rule '$ruleName' is still unavailable after setup."}
Write-Host "Frontend URL reservations: PASS ($UserName -> 127.0.0.1 + localhost + $LanAddress)" -ForegroundColor Green
Write-Host "Frontend HTTP namespaces: PASS (3-prefix HttpListener)" -ForegroundColor Green
Write-Host "LAN firewall rule:         PASS ($ruleName)" -ForegroundColor Green
