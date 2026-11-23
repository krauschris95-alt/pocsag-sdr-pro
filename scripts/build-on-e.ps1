[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    [string]$Toolchain = 'E:\pocsag avec sdr\.tools',
    [int]$Parallel = 4,
    [switch]$Test
)

$ErrorActionPreference = 'Stop'
$root = 'E:\POCSAG SDR Pro'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$official = Join-Path $root 'sources\pocsag-sdr-pro'
if ($repo -ne $official) { throw "Utiliser le checkout de référence $official" }
if ($Parallel -lt 1) { throw 'Parallel doit être positif.' }
$build = Join-Path $root ("build\" + $Configuration.ToLowerInvariant())
$tempDir = Join-Path $root 'build\temp'
$qt = Join-Path $Toolchain 'Qt\6.8.3\mingw_64'
$mingw = Join-Path $Toolchain 'Qt\Tools\mingw1310_64\bin'
$cmakeDir = Join-Path $Toolchain 'cmake\data\bin'
$ninja = Join-Path $Toolchain 'bin\ninja.exe'
$engine = Join-Path $root 'build\multimon\multimon-ng.exe'
$logs = Join-Path $root 'logs'
$reports = Join-Path $root 'tests'
foreach ($dir in $build,$tempDir,$logs,$reports) { [void][IO.Directory]::CreateDirectory($dir) }
$previousPath,$previousTemp,$previousTmp = $env:PATH,$env:TEMP,$env:TMP
try {
    $env:PATH = "$qt\bin;$mingw;$cmakeDir;$env:PATH"
    $env:TEMP = $tempDir
    $env:TMP = $tempDir
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    & "$cmakeDir\cmake.exe" -S $repo -B $build -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" "-DCMAKE_CXX_COMPILER=$mingw\g++.exe" "-DCMAKE_PREFIX_PATH=$qt" "-DCMAKE_BUILD_TYPE=$Configuration"
    if ($LASTEXITCODE) { throw 'Configuration CMake en échec.' }
    & "$cmakeDir\cmake.exe" --build $build --parallel $Parallel 2>&1 | Tee-Object (Join-Path $logs "build-$Configuration-$stamp.log")
    if ($LASTEXITCODE) { throw 'Compilation en échec.' }
    if (!(Test-Path -LiteralPath $engine)) { throw "Moteur FLEX épinglé attendu dans $engine. Voir docs/FLEX-THIRD-PARTY.md." }
    Copy-Item -LiteralPath $engine -Destination (Join-Path $build 'multimon-ng.exe')
    if ($Test) {
        & "$cmakeDir\ctest.exe" --test-dir $build -C $Configuration --output-on-failure --output-log (Join-Path $reports "ctest-$Configuration-$stamp.log")
        if ($LASTEXITCODE) { throw 'CTest a signalé un échec.' }
    }
} finally {
    $env:PATH,$env:TEMP,$env:TMP = $previousPath,$previousTemp,$previousTmp
}