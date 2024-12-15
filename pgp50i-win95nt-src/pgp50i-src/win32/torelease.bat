@echo off
REM $Id: torelease.bat,v 1.1.1.1 1997/04/29 00:23:28 philipn Exp $

if not exist release\. mkdir release
if not exist release\maps\. mkdir release\maps
if not exist release\libs\. mkdir release\libs
if not exist release\includes\. mkdir release\includes

copy pgpw45\release\pgpw45_32.exe release
copy pgpwck\release\pgpwck_32.dll release
copy pgpw2x\release\pgpw2x_32.dll release
copy simple\release\smplpgp_32.dll release
copy enclyptor\release\enclyptor.exe release\enclypt_32.exe
copy pgpwctx\release\pgpwctx.dll release
copy plugin\release\plugin.dll release
copy pndetect\release\pndetect.exe release
copy pnstub\release\pnstub.dll release
copy hooktest\release\pgptest.exe release

copy pgpw45\release\*.map release\maps
copy pgpwck\release\*.map release\maps
copy pgpw2x\release\*.map release\maps
copy simple\release\*.map release\maps
copy enclyptor\release\*.map release\maps
copy pgpwctx\release\*.map release\maps
copy plugin\release\*.map release\maps

copy simple\release\*.lib release\libs

copy include\smplpgp.h release\includes
