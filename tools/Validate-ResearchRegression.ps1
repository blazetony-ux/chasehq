param([string]$Root=(Split-Path -Parent $PSScriptRoot))
& (Join-Path $PSScriptRoot 'Update-ApiSchemaRegression.ps1') -Root $Root
$web=Get-Content (Join-Path $Root 'Start-ChaseHQWeb.ps1') -Raw
if($web -match 'See action implementation'){throw 'Generic API schema placeholder found'}
$required=@('regression-api-schema-contracts.chqscript','regression-script-query-functions.chqscript','regression-checkpoint-render-determinism.chqscript','regression-live-sprite-api.chqscript','regression-profile-metadata.chqscript','quick-smoke.chqscript','full-regression.chqscript')
foreach($n in $required){$p=Join-Path $Root ('research\scripts\regression\'+$n);if(-not(Test-Path $p)){throw "Missing required regression: $n"}}
Write-Host 'Research API/script regression surface validation PASS.'
