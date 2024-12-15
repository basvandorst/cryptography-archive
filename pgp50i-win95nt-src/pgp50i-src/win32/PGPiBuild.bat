@echo off
REM PGPiBuild.bat adds building "freeware" to PGP Inc's PGPBUILD.BAT
REM
REM $Id: PGPBUILD.BAT,v 1.3 1997/05/20 23:16:57 philipn Exp $
REM
REM Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.

attrib -r build.txt

rem if exists \msdev\bin\vcvars32.bat call \msdev\bin\vcvars32.bat x86
if "%1"=="" goto USAGE
if "%1"=="freeware" goto FREEWARE
if "%1"=="FREEWARE" goto FREEWARE
if "%1"=="debug" goto DEBUG
if "%1"=="DEBUG" goto DEBUG
if "%1"=="release" goto RELEASE
if "%1"=="RELEASE" goto RELEASE
if "%1"=="clean" goto CLEAN
if "%1"=="CLEAN" goto CLEAN
goto USAGE

:FREEWARE
SET PGPBuild=Release
goto BUILD

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
for %%i in (pgpkeys pgpcmdlg)                     do call clean.bat %%i FreewareRelease
goto DONE

:BUILD
echo > build.txt
rem for %%i in (pnstub pndetect) do call build.bat pndetect %%i %PGPBuild%
for %%i in (PGPphrase TreeList) do call build.bat %%i %PGPBuild%
for %%i in (KeyserverSupport PGPcmdlg PGPrecip) do call build.bat %%i %PGPBuild%
for %%i in (pgpwctx PGPTray PGPkeyserver) do call build.bat %%i %PGPBuild%
for %%i in (PGPkeys EudoraPlugin pgpExch) do call build.bat %%i %PGPBuild%
if "%1"=="freeware" call build.bat PGPcmdlg Freeware_Release
if "%1"=="FREEWARE" call build.bat PGPcmdlg Freeware_Release
if "%1"=="freeware" call build.bat PGPkeys  Freeware_Release
if "%1"=="FREEWARE" call build.bat PGPkeys  Freeware_Release
goto DONE

:USAGE
echo "Usage: pgpbuild DEBUG | RELEASE | FREEWARE | CLEAN"

:DONE
