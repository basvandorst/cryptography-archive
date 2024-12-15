@echo off
if "%OS%"=="Windows_NT" goto winnt
set noerrors= 
goto ok

:winnt
set "noerrors=2>nul"

:ok
if "%jar%"=="" set jar=jar

if exist build\* goto zip
call util\finalbuild

:zip
echo Creating Cryptix-classes%CRYPTIX_VERSION%.zip file...
cd build
del ..\..\Cryptix-classes%CRYPTIX_VERSION%.zip %noerrors%
%jar% cfM ..\..\Cryptix-classes%CRYPTIX_VERSION%.zip *
cd ..

:exit

