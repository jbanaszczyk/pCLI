@echo off

if [%2] EQU [] goto :Usage

if [%3] EQU [Win32] goto :Win32

:x64

echo %1 =^> %2..\..\
copy %1 %2..\..\ >nul
goto :Finish

:Win32
echo %1 =^> %2..\..\
copy %1 %2..\..\ >nul

echo %1 =^> %2..\..\..\
copy %1 %2..\..\..\ >nul

echo %1 =^> %2..\..\..\..\
copy %1 %2..\..\..\..\ >nul


if exist %2..\..\..\..\..\Start.exe echo %1 =^> %2..\..\..\..\..\
if exist %2..\..\..\..\..\Start.exe copy %1 %2..\..\..\..\..\ >nul

goto :Finish

:Usage
Echo Usage: $TargetPath $TargetDir $PlatformName $Configuration
echo $1: %1
echo $2: %2
echo $3: %3
echo $4: %4
goto :Finish

:Finish