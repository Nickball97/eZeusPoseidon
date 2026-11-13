# Installs the build made by tools\build.cmd into the game folder as
# Bin\eZeus-dev.exe, with the repository's text and sanctuary layout files.
# Only run it while eZeus-dev.exe is not running.
#
#   powershell -ExecutionPolicy Bypass -File tools\deploy.ps1
param(
    # the eZeus data folder inside the game's folder
    [string]$Game = "D:\Steam\steamapps\common\Zeus + Poseidon\eZeus-0.8.2-beta",
    # where tools\build.cmd builds: "eZeus-build" next to the repository,
    # or the folder named in EZEUS_BUILD
    [string]$Build = ""
)
$src = Split-Path -Parent $PSScriptRoot
if (-not $Build) { $Build = $env:EZEUS_BUILD }
if (-not $Build) { $Build = Join-Path (Split-Path -Parent $src) "eZeus-build" }
$bld = $Build
$dst = $Game

$exe = Get-ChildItem $bld -Recurse -Filter eZeus.exe | Select-Object -First 1
if (-not $exe) { throw "eZeus.exe not found in $bld" }
Copy-Item $exe.FullName "$dst\Bin\eZeus-dev.exe" -Force
$pdb = Join-Path $bld "eZeus.pdb"; if (Test-Path $pdb) { Copy-Item $pdb "$dst\Bin\eZeus.pdb" -Force }
Get-ChildItem $bld -Recurse -Filter "SDL2*.dll" | Where-Object { $_.FullName -notmatch "CMakeFiles" } | ForEach-Object {
    Copy-Item $_.FullName "$dst\Bin\" -Force
}
Copy-Item "$src\text\language.txt" "$dst\Text\language.txt" -Force
Copy-Item "$src\sanctuaries\*.txt" "$dst\Sanctuaries\" -Force
Get-ChildItem "$dst\Bin" | Select-Object Name, Length, LastWriteTime
# numbers.txt overrides the code's defaults; drop the keys our code now sets
# from verified sources (one-time backup kept as numbers.txt.orig).
$num = "$dst\numbers.txt"
if (-not (Test-Path "$num.orig")) { Copy-Item $num "$num.orig" }
$drop = 'olive_press_processing_period_i|winery_processing_period_i|armory_processing_period_i|sculpture_studio_processing_period_i|horse_ranch_horse_spawn_period_i|chariot_building_time_i|marble_collect_period_i|bronze_collect_period_i|wood_collect_period_i|horse_ranch_wheat_use_period_i|silver_collect_period_i|army_strength_horseman_mult_d|trireme_wharf_build_time_i|trireme_wharf_build_stages_i|horse_vendor_max_resource_take_distance_i|cattle_mature_period_i|boar_spawn_period_i|deer_spawn_period_i|olive_tree_ripe_period_i|vine_ripe_period_i|fish_collect_period_i|urchin_collect_period_i|fishery_unpack_time_i|house_culture_decrement_period_i|house_hygiene_decrement_period_i|house_water_decrement_period_i|corral_processing_period_i|corral_kill_period_i|very_low_tax_coverage_d'
$lines = Get-Content $num | Where-Object { $_ -notmatch "^($drop) " }
Set-Content -Path $num -Value $lines -Encoding ascii
# Sprite sheets cut from the game's own files (tools/sgextract, sheets.txt)
$sgx = "$bld\sgextract"
$sgxExe = "$sgx\sgextract.exe"
$sgxSrc = "$src\tools\sgextract\sgextract.cpp"
if (-not (Test-Path $sgxExe) -or (Get-Item $sgxSrc).LastWriteTime -gt (Get-Item $sgxExe).LastWriteTime) {
    New-Item -ItemType Directory -Force $sgx | Out-Null
    cmd /c "`"$src\tools\sgextract\build.cmd`" `"$sgx`"" | Out-Null
}
$data = Join-Path (Split-Path $dst -Parent) "DATA"
Get-Content "$src\tools\sgextract\sheets.txt" | Where-Object { $_ -and -not $_.StartsWith('#') } | ForEach-Object {
    $p = $_ -split '\s+'
    if (-not (Test-Path "$dst\Textures\60\$($p[3])_0.png")) {
        & $sgxExe "$data\$($p[0])" $p[1] $p[2] $p[3] "$dst\Textures"
    }
}
