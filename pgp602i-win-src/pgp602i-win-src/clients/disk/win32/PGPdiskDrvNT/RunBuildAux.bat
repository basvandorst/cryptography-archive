@echo off

set NTVERSION=400

rem Batch file to build Driver::Works projects
rem
rem		Arguments are:
rem				arg 1		checked or free
rem				arg 2		passed to BUILD program

rem ensure that environment is set up correctly
if "%BASEDIR%"=="" goto NeedEnvironment
if "%DRIVERWORKS%"=="" goto NeedVDW

rem check arguments
if "%1"=="free" goto argok
if "%1"=="checked" goto argok
goto badarg
:argok

del %basedir%\build.dat
call %basedir%\bin\setenv %basedir% %1

%PGPDISKSOURCEDRIVE%:

cd %PGPDISKSOURCEPATH%\PGPdiskDrvNT\Source
copy *.cpp ..
copy Function.h ..

cd %PGPDISKSOURCEPATH%\Shared
copy *.cpp %PGPDISKSOURCEPATH%\PGPdiskDrvNT

cd %PGPDISKSOURCEPATH%\Encryption
copy *.cpp %PGPDISKSOURCEPATH%\PGPdiskDrvNT

cd %PGPDISKSOURCEPATH%\..\..\..\libs\pfl\common

copy pgpDebug.c %PGPDISKSOURCEPATH%\PGPdiskDrvNT\pgpDebug.cpp
copy pgpLeaks.c %PGPDISKSOURCEPATH%\PGPdiskDrvNT\pgpLeaks.cpp
copy pgpMem.c %PGPDISKSOURCEPATH%\PGPdiskDrvNT\pgpMem.cpp

cd %PGPDISKSOURCEPATH%\PGPdiskDrvNT

if NOT EXIST %cpu% mkdir %cpu%
if NOT EXIST %cpu%\checked mkdir %cpu%\checked
if NOT EXIST %cpu%\free mkdir %cpu%\free

path %basedir%\bin;%path%
%basedir%\bin\build.exe -m -w -z %2

goto done

:NeedEnvironment
@echo DDK environment (BASEDIR) must be set up before building driver
goto Done

:NeedVDW
@echo DRIVERWORKS environment variable must be defined before building driver
goto Done

:NeedNTVersion
@echo NTVERSION environment variable must be defined before building driver
goto Done

:badarg
@echo Must specify "checked" or "free" to VDW.BAT
goto Done

:done

del *.cpp
del Function.h

if NOT EXIST MC_RAN.### goto no_MC_RAN
echo *** If you see exactly one _error_ message, you can safely ignore it.
echo *** Developer Studio is mistaking the normal output of MC (message
echo *** compiler) for a real _error_.
echo ***
echo *** If you build again, Studio should report no _errors_ since MC will
echo *** not be invoked.
echo ***
del MC_RAN.###
:no_MC_RAN

@echo End of RUNBUILD.BAT
