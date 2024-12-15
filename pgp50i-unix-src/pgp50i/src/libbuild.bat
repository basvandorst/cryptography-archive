REM $Id: libbuild.bat,v 1.1.2.4 1997/06/19 22:01:07 philipn Exp $
REM
REM Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.

@echo off
attrib -r build.txt

rem checkout the RSAREF src. files so the  lib can be built
rem if exists ../src/rsaref goto RSAFILESEXISTS
rem cvs co PGPLib/rsaref

:RSAFILESEXISTS
rem if exists \msdev\bin\vcvars32.bat call \msdev\bin\vcvars32.bat x86
if "%1"=="" goto USAGE
if "%1"=="debug" goto DEBUG
if "%1"=="release" goto RELEASE
if "%1"=="DEBUG" goto DEBUG
if "%1"=="RELEASE" goto RELEASE
if "%1"=="rsaref" goto RSAREF
if "%1"=="RSAREF" goto RSAREF
if "%1"=="clean" goto CLEAN
if "%1"=="NORSA" goto NORSA
if "%1"=="norsa" goto NORSA

goto USAGE

:NORSA
SET PGPBuild=NO_RSA
goto BUILD

:DEBUG
SET PGPBuild=Debug
goto BUILD

:RSAREF
SET PGPBuild=RSAREF
goto BUILD

:RELEASE
SET PGPBuild=Release
goto BUILD

:CLEAN
for %%i in (lib\bn lib\pgp)	do call clean.bat %%i debug
for %%i in (lib\pgp\keydb lib\simple)	do call clean.bat %%i debug
for %%i in (lib\bn lib\pgp)	do call clean.bat %%i release
for %%i in (lib\pgp\keydb lib\simple)	do call clean.bat %%i release
goto DONE

:BUILD
echo > build.txt
if "%2"=="clean" do call clean.bat lib\bn\bn %PGPBuild%
if "%2"=="clean" do call clean.bat lib\pgp\pgp %PGPBuild%
if "%2"=="clean" do call clean.bat lib\pgp\keydb\keydb %PGPBuild%
if "%2"=="clean" do call clean.bat lib\simple\simple %PGPBuild%
cd lib\bn
call ..\..\build.bat bn %PGPBuild%
cd ..\..\lib\pgp
call ..\..\build.bat pgp %PGPBuild%
cd keydb
call ..\..\..\build.bat keydb %PGPBuild%
cd ..\..\simple
call ..\..\build.bat simple %PGPBuild%
cd ..\..

goto DONE

:USAGE
echo "Usage: pgpbuild DEBUG | RELEASE | RSAREF | NORSA"

:DONE
