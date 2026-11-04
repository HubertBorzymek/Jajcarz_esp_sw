# Kompiluje i uruchamia testy logiki kompilatorem MSVC z Visual Studio.
# Wymaga managed_components, czyli choc jednego wczesniejszego idf.py build.
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path "$PSScriptRoot\..\..").Path
$out = Join-Path $env:TEMP 'jajcarz_host_tests'
New-Item -ItemType Directory -Force $out | Out-Null

$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$cjson = "$root\managed_components\espressif__cjson\cJSON"

$batch = Join-Path $out 'build.cmd'
@"
@echo off
call "$vs\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 || exit /b 1
cd /d "$out"
cl /nologo /utf-8 /std:c11 /W4 /WX /c /D_CRT_SECURE_NO_WARNINGS ^
  /I"$root\tests\host" ^
  /I"$root\components\panel\include" ^
  /I"$root\components\presence\include" ^
  /I"$root\components\protocol\include" ^
  /I"$root\components\sync\include" ^
  /I"$root\components\jajcarz_types\include" ^
  /I"$cjson" ^
  "$root\tests\host\test_main.c" ^
  "$root\tests\host\test_panel.c" ^
  "$root\tests\host\test_protocol.c" ^
  "$root\tests\host\test_sync.c" ^
  "$root\components\panel\src\panel.c" ^
  "$root\components\panel\src\debounce.c" ^
  "$root\components\presence\src\presence.c" ^
  "$root\components\protocol\src\protocol_parse.c" ^
  "$root\components\protocol\src\protocol_encode.c" ^
  "$root\components\sync\src\sync.c" >nul || exit /b 1
cl /nologo /utf-8 /W0 /c /D_CRT_SECURE_NO_WARNINGS "$cjson\cJSON.c" >nul || exit /b 1
cl /nologo *.obj /Fe:host_tests.exe >nul || exit /b 1
"$out\host_tests.exe"
"@ | Set-Content -Encoding ascii $batch

cmd /c $batch
exit $LASTEXITCODE
