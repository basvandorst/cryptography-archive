@echo off
REM release install builder
REM $Id: RELEASE.BAT,v 1.2 1997/05/20 23:18:41 philipn Exp $

mkdir release

copy pgprecip\%1\pgprecip.dll release
copy pgpcmdlg\%1\pgpcmdlg.dll release
copy pgpwctx\%1\pgpwctx.dll release
copy pgptray\%1\pgptray.exe release
copy pgpkeyserver\%1\pgpks.dll release
copy pgpkeys\%1\pgpkeys.exe release
copy eudoraplugin\%1\PGPPlugin.dll release
copy lib\bn.dll release
copy lib\simple.dll release
copy lib\pgp.dll release
copy lib\keydb.dll release
copy pgpexch\%1\pgpexch.dll release
copy install\help\pgp50.hlp release
copy install\extras\keygen.avi release
