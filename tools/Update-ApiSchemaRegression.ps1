param([string]$Root=(Split-Path -Parent $PSScriptRoot))
$web=Join-Path $Root 'Start-ChaseHQWeb.ps1'
$text=Get-Content $web -Raw
$m=[regex]::Match($text,'\$scriptApiActions=@\((.*?)\)\r?\n\$scriptActionSchemas=','Singleline')
if(-not$m.Success){throw 'Could not locate $scriptApiActions'}
$actions=@([regex]::Matches($m.Groups[1].Value,"'([^']+)'")|ForEach-Object{$_.Groups[1].Value})
$out=@('# name: Regression - API schema contracts','# purpose: Assert every currently scriptable API action exposes a real schema contract.','# generated-from: Start-ChaseHQWeb.ps1 $scriptApiActions','','echo === API SCHEMA CONTRACT REGRESSION ===')
$out += @($actions|ForEach-Object{"api schema action=$_"})
$out += @('echo === API SCHEMA CONTRACT REGRESSION COMPLETE ===','')
$path=Join-Path $Root 'research\scripts\regression\regression-api-schema-contracts.chqscript'
Set-Content $path $out -Encoding UTF8
Write-Host "Updated $path with $($actions.Count) API schema checks."
