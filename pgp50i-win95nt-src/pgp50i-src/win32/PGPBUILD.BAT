REM $Id: PGPBUILD.BAT,v 1.3 1997/05/20 23:16:57 philipn Exp $
REM
REM Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.

@echo off
attrib -r build.txt

rem if exists \msdev\bin\vcvars32.bat call \msdev\bin\vcvars32.bat x86
if "%1"=="" goto USAGE
if "%1"=="debug" goto DEBUG
if "%1"=="release" goto RELEASE
if "%1"=="DEBUG" goto DEBUG
if "%1"=="RELEASE" goto RELEASE
if "%1"=="clean" goto CLEAN
goto USAGE

:DEBUG
SET PGPBuild=Debug
goto BUILD


:RELEASE
SET PGPBuild=Release
goto BUILD

:CLEAN
for %%i in (pgpphrase treelist pgprecip pgpcmdlg) do call clean.bat %%i debug
for %%i in (keyserversupport pgpwctx pgptray)     do call clean.bat %%i debug
for %%i in (pgpkeyserver pgpkeys eudoraplugin)    do call clean.bat %%i debug
for %%i in (pgpExch)                              do call clean.bat %%i debug
for %%i in (pgpphrase treelist pgprecip pgpcmdlg) do call clean.bat %%i release
for %%i in (keyserversupport pgpwctx pgptray)     do call clean.bat %%i release
for %%i in (pgpkeyserver pgpkeys eudoraplugin)    do call clean.bat %%i release
for %%i in (pgpExch)                              do call clean.bat %%i release
goto DONE

:BUILD
echo > build.txt
if "%2"=="clean" do call clean.bat pgpphrase %PGPBuild%
if "%2"=="clean" do call clean.bat treelist %PGPBuild%
if "%2"=="clean" do call clean.bat pgprecip %PGPBuild%
if "%2"=="clean" do call clean.bat pgpcmdlg %PGPBuild%
if "%2"=="clean" do call clean.bat keyserversupport %PGPBuild%
if "%2"=="clean" do call clean.bat pgpwctx %PGPBuild%
if "%2"=="clean" do call clean.bat pgptray %PGPBuild%
if "%2"=="clean" do call clean.bat pgpkeyserver %PGPBuild%
if "%2"=="clean" do call clean.bat pgpkeys %PGPBuild%
if "%2"=="clean" do call clean.bat eudoraplugin %PGPBuild%
if "%2"=="clean" do call clean.bat pgpexch %PGPBuild%
rem for %%i in (pnstub pndetect) do call build.bat pndetect %%i %PGPBuild%
for %%i in (PGPphrase TreeList) do call build.bat %%i %PGPBuild%
for %%i in (KeyserverSupport PGPcmdlg PGPrecip) do call build.bat %%i %PGPBuild%
for %%i in (pgpwctx PGPTray PGPkeyserver) do call build.bat %%i %PGPBuild%
for %%i in (PGPkeys EudoraPlugin pgpExch) do call build.bat %%i %PGPBuild%

goto DONE

:USAGE
echo "Usage: pgpbuild DEBUG | RELEASE"

:DONE
