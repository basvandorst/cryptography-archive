@echo off
if "%OS%"=="Windows_NT" goto winnt
set deltree=deltree /y
set noerrors= 
echo Does this script work on Win95? Send the answer to hopwood@zetnet.co.uk.
goto ok

:winnt
set deltree=rd /s /q
set "noerrors=2>nul"

:ok
if "%jar%"=="" set jar=jar
if "%java%"=="" set java=java

echo Creating empty test directory...
%deltree% test %noerrors%
md test
cd test

echo Extracting Cryptix-classes%CRYPTIX_VERSION%.zip...
%jar% xf ..\..\Cryptix-classes%CRYPTIX_VERSION%.zip

call ..\util\setpwd .
set OLDCP=%CLASSPATH%
set CLASSPATH=%PWD%\cryptix.zip;%PWD%\examples.zip
echo.
echo CLASSPATH = %CLASSPATH%

echo.
echo Checking installation...
java cryptix.provider.Install

echo Running tests...
%java% cryptix.test.TestAll
cd ..

echo.
set CLASSPATH=%OLDCP%
