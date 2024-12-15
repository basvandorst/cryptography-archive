@echo off
if (%1)==() goto help
if (%2)==() goto err
if (%3)==() goto err
sscv04 %1 %2 %3 %4
if errorlevel 1 goto end
e %1
goto end
:help
sscv04
goto end
:err
echo Incomplete command line
:end
