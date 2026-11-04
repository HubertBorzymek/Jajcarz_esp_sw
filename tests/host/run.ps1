# Kompiluje i uruchamia testy logiki kompilatorem MSVC z Visual Studio.
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path "$PSScriptRoot\..\..").Path
$out = Join-Path $env:TEMP 'jajcarz_host_tests'
New-Item -ItemType Directory -Force $out | Out-Null

$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

$batch = Join-Path $out 'build.cmd'
@"
@echo off
call "$vs\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 || exit /b 1
cd /d "$out"
cl /nologo /utf-8 /std:c11 /W4 /WX ^
  /I"$root\components\panel\include" ^
  /I"$root\components\presence\include" ^
  /I"$root\components\jajcarz_types\include" ^
  "$root\tests\host\test_panel.c" ^
  "$root\components\panel\src\panel.c" ^
  "$root\components\panel\src\debounce.c" ^
  "$root\components\presence\src\presence.c" ^
  /Fe:test_panel.exe || exit /b 1
"$out\test_panel.exe"
"@ | Set-Content -Encoding ascii $batch

cmd /c $batch
exit $LASTEXITCODE
