[CmdletBinding()]
param([Parameter(Mandatory)][string]$Experiment,[string]$HostName='127.0.0.1',[int]$HttpPort=37680,[string]$OutputRoot='.\evidence\experiments')
$ErrorActionPreference='Stop'
Import-Module (Join-Path $PSScriptRoot 'ChaseHQResearch.psm1') -Force
$spec=Get-Content $Experiment -Raw|ConvertFrom-Json
if($spec.schema -ne 'chq-experiment-v1'){throw 'Unsupported experiment schema'}
$conn=Connect-ChaseHQ -HostName $HostName -Port $HttpPort
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss';$root=Join-Path $OutputRoot ("$($spec.name)-$stamp");New-Item -ItemType Directory -Force -Path $root|Out-Null
$manifest=[ordered]@{schema='chq-experiment-result-v1';name=$spec.name;started=(Get-Date).ToString('o');api=$conn.BaseUri;source=(Resolve-Path $Experiment).Path;runs=@()}
$runs=if($spec.runs){[int]$spec.runs}else{1}
for($run=1;$run-le$runs;$run++){$rd=Join-Path $root ("run-{0:D2}"-f$run);New-Item -ItemType Directory -Force -Path $rd|Out-Null
  $rr=[ordered]@{run=$run;started=(Get-Date).ToString('o');steps=@()}
  if($spec.checkpoint){$cp=[string]$spec.checkpoint;$r=Invoke-ChaseHQCommand $conn "checkpoint load $cp";$rr.steps+=@{op='checkpoint-load';result=$r}}
  foreach($s in $spec.steps){$op=[string]$s.op;$result=$null
    switch($op){
      'step-frame' {$result=Step-ChaseHQFrame $conn ([int]$s.frames)}
      'ioc-pulse' {$result=Invoke-ChaseHQIocPulse $conn ([int]$s.port) ([string]$s.mask) ([int]$s.frames)}
      'capture-evidence' {$result=Save-ChaseHQEvidence $conn ("$($spec.name)-run$run-$($s.name)")}
      'capture-memory' {$p=Join-Path $rd ("$($s.name)-$($s.cpu)-$($s.start)-$($s.end).bin");$result=(Export-ChaseHQMemory $conn ([string]$s.cpu) ([string]$s.start) ([string]$s.end) $p).FullName}
      'command' {$result=Invoke-ChaseHQCommand $conn ([string]$s.command)}
      'pause' {$result=Pause-ChaseHQ $conn}
      'resume' {$result=Resume-ChaseHQ $conn}
      default {throw "Unknown experiment operation: $op"}
    }
    $rr.steps+=@{op=$op;result=$result}
  }
  $rr.ended=(Get-Date).ToString('o');$manifest.runs+=$rr
}
$manifest.ended=(Get-Date).ToString('o');$manifest|ConvertTo-Json -Depth 30|Set-Content (Join-Path $root 'manifest.json') -Encoding UTF8
$manifest
