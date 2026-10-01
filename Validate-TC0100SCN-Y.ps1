$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $root
try {
    $exe = @('.\out\build\x64-Debug\ChaseHQNative.exe', '.\out\build\x64-Debug\Debug\ChaseHQNative.exe') | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $exe) { throw 'ChaseHQNative.exe not found - run .\Build-Debug.bat first' }
    $roms = '.\roms\chasehq'
    $checkpoint = '.\checkpoints\stage1-gameplay-2064.chqstate'
    $expected = '.\docs\evidence\tc0100scn-y-scroll-rc23\expected-frame-2065.png'
    if (-not (Test-Path $checkpoint)) { throw "Checkpoint missing: $checkpoint" }
    if (-not (Test-Path $expected)) { throw "Expected reference missing: $expected" }
    $started = Get-Date
    & $exe $roms --load-checkpoint $checkpoint --screenshot-at 2065 --exit-at-frame 2066 --evidence-bundle --evidence-name 'tc0100-y-rc24-default'
    if ($LASTEXITCODE -ne 0) { throw "ChaseHQNative exited with code $LASTEXITCODE" }
    $shot = Get-ChildItem '.\evidence\tc0100-y-rc24-default_*\screenshots\frame_00002065.png' -File | Where-Object { $_.LastWriteTime -ge $started } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $shot) { throw 'RC2.4 validation screenshot not found' }
    $actualHash = (Get-FileHash $shot.FullName -Algorithm SHA256).Hash
    $expectedHash = (Get-FileHash $expected -Algorithm SHA256).Hash
    if ($actualHash -ne $expectedHash) {
        throw "TC0100SCN Y-scroll visual mismatch. expected=$expectedHash actual=$actualHash screenshot=$($shot.FullName)"
    }
    $bundle = Get-ChildItem '.\evidence\tc0100-y-rc24-default_*.zip' -File | Where-Object { $_.LastWriteTime -ge $started } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    Write-Host "PASS: RC2.4 default frame 2065 exactly matches the RC2.3 both-sign proxy."
    Write-Host "SHA256: $actualHash"
    if ($bundle) { Write-Host "EVIDENCE: $($bundle.FullName)" }
}
finally {
    Pop-Location
}
