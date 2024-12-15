@echo off

mkdir release

SET VERSION=%1
if "%1"=="freeware" SET VERSION=Release
if "%1"=="FREEWARE" SET VERSION=Release
copy pgprecip\%VERSION%\pgprecip.dll release
copy pgpcmdlg\%VERSION%\pgpcmdlg.dll release
copy pgpwctx\%VERSION%\pgpwctx.dll release
copy pgptray\%VERSION%\pgptray.exe release
copy pgpkeyserver\%VERSION%\pgpks.dll release
copy pgpkeys\%VERSION%\pgpkeys.exe release
copy eudoraplugin\%VERSION%\PGPPlugin.dll release
copy lib\bn.dll release
copy lib\simple.dll release
copy lib\pgp.dll release
copy lib\keydb.dll release
copy pgpexch\%VERSION%\pgpexch.dll release
copy install\help\pgp50.hlp release
copy install\extras\keygen.avi release
if "%1"=="freeware" copy pgpcmdlg\FreewareRelease\pgpcmdlg.dll release
if "%1"=="FREEWARE" copy pgpcmdlg\FreewareRelease\pgpcmdlg.dll release
if "%1"=="freeware" copy pgpkeys\FreewareRelease\pgpkeys.exe release
if "%1"=="FREEWARE" copy pgpkeys\FreewareRelease\pgpkeys.exe release
