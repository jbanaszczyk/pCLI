@echo off

if [%2] EQU [] goto :Usage

echo %1 =^> %2..\..\
copy %1 %2..\..\ >nul
goto :Finish

:Usage
Echo Usage: $TargetPath $TargetDir $PlatformName $Configuration
echo $1: %1
echo $2: %2
echo $3: %3
echo $4: %4
goto :Finish

:Finish