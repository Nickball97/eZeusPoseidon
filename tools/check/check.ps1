# The safety net: runs cities with no window and nobody playing, and
# compares what happened with the results kept in tools/check/baseline.
# See mod-docs/safety-net.md.
#
#   check.ps1            run every adventure in maps.txt and compare
#   check.ps1 -Twice     also run each one a second time; both must match
#   check.ps1 -Update    keep the new results as the baseline
#
# Build first (tools/build.cmd). The game may be running: the check
# uses its own copy of the program, Bin\eZeus-check.exe.
param(
    [switch]$Update,
    [switch]$Twice,
    [int]$Years = 10,
    [int]$Seed = 1,
    [string]$Build = "C:\Users\LENOVO\source\repos\eZeus-build",
    [string]$Game = "D:\Steam\steamapps\common\Zeus + Poseidon\eZeus-0.8.2-beta"
)

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$repo = Split-Path -Parent (Split-Path -Parent $here)
$baseline = Join-Path $here "baseline"
$out = Join-Path $here "out"
New-Item -ItemType Directory -Force $baseline | Out-Null
New-Item -ItemType Directory -Force $out | Out-Null

$built = Get-ChildItem $Build -Recurse -Filter eZeus.exe |
         Where-Object { $_.FullName -notmatch "CMakeFiles" } |
         Select-Object -First 1
if (-not $built) { Write-Host "eZeus.exe not found in $Build. Build first."; exit 2 }
$exe = Join-Path $Game "Bin\eZeus-check.exe"
Copy-Item $built.FullName $exe -Force

# The check reads the game folder's text and sanctuary files, which
# deploy.ps1 copies from the repo. Say so if they are not the same.
function Test-SameFile($a, $b) {
    if (-not (Test-Path $a) -or -not (Test-Path $b)) { return $false }
    return (Get-FileHash $a).Hash -eq (Get-FileHash $b).Hash
}
$stale = @()
if (-not (Test-SameFile "$repo\text\language.txt" "$Game\Text\language.txt")) { $stale += "text\language.txt" }
Get-ChildItem "$repo\sanctuaries\*.txt" | ForEach-Object {
    if (-not (Test-SameFile $_.FullName "$Game\Sanctuaries\$($_.Name)")) { $stale += "sanctuaries\$($_.Name)" }
}
if ($stale.Count -gt 0) {
    Write-Host "Note: the game folder has older copies of: $($stale -join ', ')."
    Write-Host "      The check uses the game folder's copies. Install the build (deploy.ps1) to refresh them."
}

function Invoke-Check($map, $file) {
    $log = [System.IO.Path]::ChangeExtension($file, ".log")
    if (Test-Path $file) { Remove-Item $file -Confirm:$false }
    $arguments = "--headless-check `"$map`" $Years --seed $Seed --out `"$file`""
    $p = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory (Split-Path $exe) `
            -NoNewWindow -Wait -PassThru `
            -RedirectStandardOutput $log -RedirectStandardError "$log.err"
    return ($p.ExitCode -eq 0) -and (Test-Path $file)
}

$maps = Get-Content (Join-Path $here "maps.txt") |
        ForEach-Object { $_.Trim() } |
        Where-Object { $_ -and -not $_.StartsWith(";") }

$bad = 0

# The table of building kinds (buildings/ebuildingregistry.cpp) must say the
# truth about every building in it: its type and its footprint.
$tableLog = Join-Path $out "building-table.log"
$tp = Start-Process -FilePath $exe -ArgumentList "--headless-check `"$($maps[0])`" 0 --building-table" `
        -WorkingDirectory (Split-Path $exe) -NoNewWindow -Wait -PassThru `
        -RedirectStandardOutput $tableLog -RedirectStandardError "$tableLog.err"
if ($tp.ExitCode -eq 0) {
    Write-Host "TABLE OK the table of building kinds"
} else {
    Write-Host "TABLE WRONG  the table of building kinds (see $tableLog)"
    $bad++
}
foreach ($map in $maps) {
    $name = ($map -replace '[^A-Za-z0-9]+', '_').Trim('_')
    $file = Join-Path $out "$name.txt"
    $base = Join-Path $baseline "$name.txt"
    $started = Get-Date
    $ok = Invoke-Check $map $file
    $secs = [int]((Get-Date) - $started).TotalSeconds
    if (-not $ok) {
        Write-Host "CRASHED  $map  (see $([System.IO.Path]::ChangeExtension($file, '.log')))"
        $bad++
        continue
    }
    if ($Twice) {
        $file2 = Join-Path $out "$name.second.txt"
        $ok2 = Invoke-Check $map $file2
        if (-not $ok2 -or -not (Test-SameFile $file $file2)) {
            Write-Host "UNSTEADY $map  (two runs of the same build differ: compare $file and $file2)"
            $bad++
            continue
        }
    }
    if ($Update) {
        Copy-Item $file $base -Force
        Write-Host "SAVED    $map  (${secs}s)"
    } elseif (-not (Test-Path $base)) {
        Write-Host "NEW      $map  (no baseline yet; run with -Update to keep this result)"
        $bad++
    } elseif (Test-SameFile $file $base) {
        Write-Host "SAME     $map  (${secs}s)"
    } else {
        Write-Host "CHANGED  $map"
        $diff = Compare-Object (Get-Content $base) (Get-Content $file) | Select-Object -First 12
        foreach ($d in $diff) {
            $side = "now:   "
            if ($d.SideIndicator -eq "<=") { $side = "before:" }
            Write-Host "           $side $($d.InputObject)"
        }
        Write-Host "           (whole result: $file)"
        $bad++
    }
}

if ($bad -gt 0) {
    Write-Host ""
    Write-Host "$bad check(s) failed."
    exit 1
}
Write-Host ""
if ($Update) { Write-Host "Baseline updated for $($maps.Count) adventures." }
else { Write-Host "All $($maps.Count) adventures match the baseline." }
exit 0
