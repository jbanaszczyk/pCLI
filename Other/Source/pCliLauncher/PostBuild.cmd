@echo off

if [%2] EQU [] goto :Usage

if [%3] EQU [Win32] (set localCopy=32) ELSE (set localCopy=64)
if [%4] EQU [Debug] set localCopy=%localCopy%_d
set localCopy=%~n1%localCopy%%~x1

echo %1 =^> %2..\..\..\%localCopy%
copy %1 %2..\..\..\%localCopy%

if [%3_%4] NEQ [x64_Release] goto :EOF

if exist %2..\..\..\..\..\Start.exe echo %1 =^> %2..\..\..\
if exist %2..\..\..\..\..\Start.exe copy %1 %2..\..\..\ >nul

if exist %2..\..\..\..\..\Start.exe echo %1 =^> %2..\..\..\..\..\
if exist %2..\..\..\..\..\Start.exe copy %1 %2..\..\..\..\..\ >nul

goto :EOF

:Usage
Echo Usage: cmd /c PostBuild.cmd $TargetPath $TargetDir $PlatformName $Configuration
echo %%1: %1
echo %%2: %2
echo %%3: %3
echo %%4: %4
