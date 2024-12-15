echo off
if "%OS%"=="Windows_NT" goto winnt
set deltree=deltree /y
set noerrors= 
goto ok

:winnt
set deltree=rd /s /q
set "noerrors=2>nul"

:ok
if "%javac%"=="" set javac=java -ms2m sun.tools.javac.Main

echo Deleting temporary directories...
%deltree% tmp %noerrors%
echo Deleting classfiles...
rem Does del /s /q work on Win95?
del /s /q *.class >nul
echo Compiling utility programs...
%javac% util\*.java
