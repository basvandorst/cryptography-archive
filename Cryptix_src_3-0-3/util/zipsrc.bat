@echo off
if "%OS%"=="Windows_NT" goto winnt
set deltree=deltree /y
set noerrors= 
goto ok

:winnt
set deltree=rd /s /q
set "noerrors=2>nul"

:ok
if "%jar%"=="" set jar=jar

call util\copysrc

echo Creating Cryptix_src%CRYPTIX_VERSION%.zip file...
del Cryptix_src%CRYPTIX_VERSION%.zip %noerrors%
cd tmp
%jar% cfM ..\Cryptix_src%CRYPTIX_VERSION%.zip *
cd ..

echo Deleting temporary copy of source...
%deltree% tmp %noerrors%
