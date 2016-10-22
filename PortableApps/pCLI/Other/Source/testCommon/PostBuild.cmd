@echo off

if [%2] EQU [] goto :Usage

xcopy /YF %1 %2..\..\
goto :Finish

:Usage
Echo Usage: $TargetPath $TargetDir $PlatformName $Configuration
echo $1: %1
echo $2: %2
echo $3: %3
echo $4: %4
goto :Finish

:Finish