@echo off

REM echo ____________________________________ 
REM echo TargetPath    %1
REM echo TargetDir     %2
REM echo PlatformName  %3
REM echo Configuration %4
REM echo ____________________________________ 

REM if [%2] EQU [] goto :Usage
REM 
REM echo %1 =^> %2..\..\
REM copy %1 %2..\..\ >nul

REM execute tests

"%1" --gtest_filter=-CtrlHandlers.*

goto :Finish

:Usage
Echo Usage: $TargetPath $TargetDir $PlatformName $Configuration
echo $1: %1
echo $2: %2
echo $3: %3
echo $4: %4
goto :Finish

:Finish
