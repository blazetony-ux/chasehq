param(
    [string]$Root = ".",
    [switch]$Fix,
    [switch]$IncludeEvidence
)

$ErrorActionPreference = "Stop"

$rootPath = (Resolve-Path -LiteralPath $Root).Path

$extensions = @(
    ".ps1",".psm1",".psd1",".md",".txt",".json",".chqscript",
    ".html",".htm",".css",".js",".csv",".xml",".yml",".yaml",
    ".cmake",".c",".cc",".cpp",".h",".hh",".hpp",".ini",".cfg"
)

$skipDirNames = @(".git",".vs","out","build","roms")
if (-not $IncludeEvidence) {
    $skipDirNames += "evidence"
}

$utf8Strict = [System.Text.UTF8Encoding]::new($false, $true)
$utf8NoBom  = [System.Text.UTF8Encoding]::new($false)

$cp1252Strict = [System.Text.Encoding]::GetEncoding(
    1252,
    [System.Text.EncoderExceptionFallback]::new(),
    [System.Text.DecoderExceptionFallback]::new()
)

function Get-TextFromBytes {
    param([byte[]]$Bytes)

    if ($Bytes.Length -ge 3 -and
        $Bytes[0] -eq 0xEF -and
        $Bytes[1] -eq 0xBB -and
        $Bytes[2] -eq 0xBF) {
        return [pscustomobject]@{
            Text = [System.Text.Encoding]::UTF8.GetString($Bytes, 3, $Bytes.Length - 3)
            Encoding = "utf8-bom"
        }
    }

    if ($Bytes.Length -ge 2 -and
        $Bytes[0] -eq 0xFF -and
        $Bytes[1] -eq 0xFE) {
        return [pscustomobject]@{
            Text = [System.Text.Encoding]::Unicode.GetString($Bytes, 2, $Bytes.Length - 2)
            Encoding = "utf16-le"
        }
    }

    if ($Bytes.Length -ge 2 -and
        $Bytes[0] -eq 0xFE -and
        $Bytes[1] -eq 0xFF) {
        return [pscustomobject]@{
            Text = [System.Text.Encoding]::BigEndianUnicode.GetString($Bytes, 2, $Bytes.Length - 2)
            Encoding = "utf16-be"
        }
    }

    try {
        return [pscustomobject]@{
            Text = $utf8Strict.GetString($Bytes)
            Encoding = "utf8"
        }
    }
    catch {
        return [pscustomobject]@{
            Text = $cp1252Strict.GetString($Bytes)
            Encoding = "cp1252"
        }
    }
}

function Get-MojibakeScore {
    param([string]$Text)

    if ($null -eq $Text) {
        return 0
    }

    # ASCII-only source. These code points commonly begin UTF-8 text
    # that has been decoded once as Windows-1252/Latin-1.
    $pattern = "\u00C2|\u00C3|\u00E2|\u00F0"
    return ([regex]::Matches($Text, $pattern)).Count
}

function Repair-Mojibake {
    param([string]$Text)

    $current = $Text

    for ($pass = 0; $pass -lt 2; $pass++) {
        $beforeScore = Get-MojibakeScore $current
        if ($beforeScore -eq 0) {
            break
        }

        try {
            $bytes = $cp1252Strict.GetBytes($current)
            $candidate = $utf8Strict.GetString($bytes)
            $afterScore = Get-MojibakeScore $candidate

            if ($afterScore -lt $beforeScore -and
                $candidate.IndexOf([char]0xFFFD) -lt 0) {
                $current = $candidate
            }
            else {
                break
            }
        }
        catch {
            break
        }
    }

    return $current
}

$files = Get-ChildItem -LiteralPath $rootPath -Recurse -File | Where-Object {
    $ext = $_.Extension.ToLowerInvariant()

    if ($extensions -notcontains $ext) {
        return $false
    }

    $relative = $_.FullName.Substring($rootPath.Length).TrimStart('\','/')
    $parts = $relative -split '[\\/]'

    foreach ($dirName in $skipDirNames) {
        if ($parts -contains $dirName) {
            return $false
        }
    }

    return $true
}

$results = New-Object System.Collections.Generic.List[object]

foreach ($file in $files) {
    try {
        $bytes = [System.IO.File]::ReadAllBytes($file.FullName)

        # Skip obvious binary content.
        $sampleLength = [Math]::Min($bytes.Length, 4096)
        $nulCount = 0
        for ($i = 0; $i -lt $sampleLength; $i++) {
            if ($bytes[$i] -eq 0) {
                $nulCount++
            }
        }
        if ($nulCount -gt 4) {
            continue
        }

        $decoded = Get-TextFromBytes $bytes
        $before = $decoded.Text
        $after = Repair-Mojibake $before

        $beforeScore = Get-MojibakeScore $before
        $afterScore = Get-MojibakeScore $after

        $needsEncodingNormalise = $decoded.Encoding -ne "utf8"
        $needsMojibakeRepair = $after -cne $before
        $changed = $needsEncodingNormalise -or $needsMojibakeRepair

        if ($changed -and $Fix) {
            [System.IO.File]::WriteAllText($file.FullName, $after, $utf8NoBom)
        }

        if ($changed -or $beforeScore -gt 0) {
            $results.Add([pscustomobject]@{
                File = $file.FullName.Substring($rootPath.Length).TrimStart('\','/')
                OriginalEncoding = $decoded.Encoding
                MojibakeBefore = $beforeScore
                MojibakeAfter = $afterScore
                Action = if ($Fix -and $changed) {
                    "FIXED"
                }
                elseif ($changed) {
                    "WOULD FIX"
                }
                else {
                    "REVIEW"
                }
            })
        }
    }
    catch {
        $results.Add([pscustomobject]@{
            File = $file.FullName.Substring($rootPath.Length).TrimStart('\','/')
            OriginalEncoding = "ERROR"
            MojibakeBefore = ""
            MojibakeAfter = ""
            Action = $_.Exception.Message
        })
    }
}

if ($results.Count -eq 0) {
    Write-Host "No encoding or mojibake issues found in scanned project text files."
}
else {
    $results | Sort-Object File | Format-Table -AutoSize
}

Write-Host ""

if ($Fix) {
    Write-Host "Finished. Affected text files were normalised to UTF-8 without BOM."
}
else {
    Write-Host "Dry run only. Re-run with -Fix to apply safe repairs and UTF-8 normalisation."
}
