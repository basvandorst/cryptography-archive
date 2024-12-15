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

if "%ARCH%"=="" set ARCH=x86-win32
echo Creating empty nzip\cryptix-lib\bin directory...
%deltree% nzip %noerrors%
md nzip
md nzip\cryptix-lib
md nzip\cryptix-lib\bin

echo Copying native libraries...
copy native\build\%ARCH%\* nzip\cryptix-lib\bin >nul

echo Creating Cryptix-%ARCH%%CRYPTIX_VERSION%.zip file...
cd nzip
del ..\..\Cryptix-%ARCH%%CRYPTIX_VERSION%.zip %noerrors%
%jar% cfM ..\..\Cryptix-%ARCH%%CRYPTIX_VERSION%.zip *
cd ..

echo Deleting nzip directory...
%deltree% nzip %noerrors%
