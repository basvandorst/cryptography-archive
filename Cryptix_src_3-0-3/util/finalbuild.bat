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

set OLDCP=%CLASSPATH%
set CLASSPATH=.

echo Creating empty directories for classfiles...
%deltree% build tmp %noerrors%
md build tmp tmp\classes tmp\examples

echo Building classes...
cd src
call build -d ..\tmp\classes
cd ..

if "%cryptix11%"=="" goto no_cryptix11
    echo Creating cryptix11.zip...
    cd tmp\classes
    %jar% cfM ..\..\build\cryptix11.zip *
    cd ..\..
:no_cryptix11

echo Building extra classes for Java 1.0.2...
cd src10
call build

echo Creating cryptix.zip file...
cd ..\tmp\classes
xcopy /s /q ..\classes10\* . >nul
rem Hack to avoid confusing jar (it would otherwise use the wrong
rem version of java.util.zip.*):
set CLASSPATH=dummy
%jar% cf0M ..\..\build\cryptix.zip *
set CLASSPATH=.
cd ..\..

echo Building examples...
cd src
call build-examples -d ..\tmp\examples

echo Creating examples.zip file...
cd ..\tmp\examples
rem No classes in java.* should be included.
%deltree% ..\examples\java %noerrors%
%jar% cf0M ..\..\build\examples.zip *
cd ..\..

echo Copying properties files...
md build\cryptix-lib
copy src\cryptix-lib\*.properties build\cryptix-lib >nul
md build\ijce-lib
copy src\ijce-lib\*.properties build\ijce-lib >nul

echo.
set CLASSPATH=%OLDCP%
