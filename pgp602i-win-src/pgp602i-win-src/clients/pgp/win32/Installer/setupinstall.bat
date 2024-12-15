@echo off

set INSTALLPGPDISK=
set ISADMIN=
set RELEASEDIR=..\PGP55\Release
set LIBSDIR=..\..\..\..\libs
set SECUREMEMORYDIR=%LIBSDIR%\pfl\win32\SecureMemory
set PGPDISKDIR=..\..\..\..\clients\disk\win32

REM Delete existing DATA directory and create common hierarchy

if EXIST DATA rmdir /Q /S Data
mkdir DATA
mkdir DATA\DOCS

REM Check arguments
if "%1" == "DSADMIN" goto dsadmin
if "%1" == "DSCLIENT" goto dsclient
if "%1" == "PERSONAL" goto personal
if "%1" == "FREEWARE" goto freeware
goto badargs

:dsadmin
set INSTALLPGPDISK=YES
set ISADMIN=YES
copy docs\DSReadMe.txt DATA\DOCS\ReadMe.txt
copy licenses\license.txt Disk1\License.txt
copy keyrings\DSpubring.pkr tempstuff\pubring.pkr
copy bmps\business\sidebar.bmp tempstuff\
copy bmps\business\sidebarx.bmp tempstuff\
copy bmps\business\setup.bmp Disk1\
goto goodargs

:dsclient
set INSTALLPGPDISK=YES
copy docs\DSReadMe.txt DATA\DOCS\ReadMe.txt
copy licenses\license.txt Disk1\License.txt
copy keyrings\DSpubring.pkr tempstuff\pubring.pkr
copy bmps\business\sidebar.bmp tempstuff\
copy bmps\business\sidebarx.bmp tempstuff\
copy bmps\business\setup.bmp Disk1\
goto goodargs

:personal
set INSTALLPGPDISK=YES
copy docs\PPReadMe.txt DATA\DOCS\ReadMe.txt
copy licenses\reglic.txt Disk1\License.txt
copy keyrings\PPpubring.pkr tempstuff\pubring.pkr
copy bmps\personal\sidebar.bmp tempstuff\
copy bmps\personal\sidebarx.bmp tempstuff\
copy bmps\personal\setup.bmp Disk1\
goto goodargs

:freeware
copy docs\FreewareReadMe.txt DATA\DOCS\ReadMe.txt
copy licenses\freelic.txt Disk1\License.txt
copy keyrings\PPpubring.pkr tempstuff\pubring.pkr
copy bmps\freeware\sidebar.bmp tempstuff\
copy bmps\freeware\sidebarx.bmp tempstuff\
copy bmps\freeware\setup.bmp Disk1\
goto goodargs

:goodargs

copy %LIBSDIR%\pfl\win32\InstallDLL\Release\Install.dll tempstuff\
copy Extras\adobe.url tempstuff\
copy Extras\eudora.url tempstuff\

mkdir DATA\WINSYSDIR
copy %RELEASEDIR%\pgp60cl.dll DATA\WINSYSDIR
copy %RELEASEDIR%\pgp60mn.dll DATA\WINSYSDIR
copy %RELEASEDIR%\pgp60sc.dll DATA\WINSYSDIR
copy %RELEASEDIR%\pgp60hk.dll DATA\WINSYSDIR

mkdir DATA\SDKWINSYS
copy %RELEASEDIR%\pgp_sdk.dll DATA\SDKWINSYS
copy %RELEASEDIR%\pgpsdkNL.dll DATA\SDKWINSYS
copy %RELEASEDIR%\pgpsdkUI.dll DATA\SDKWINSYS

mkdir DATA\PGP60
copy %RELEASEDIR%\PGPkeys.exe DATA\PGP60
copy %RELEASEDIR%\PGPlog.exe DATA\PGP60
copy %RELEASEDIR%\PGPtools.exe DATA\PGP60
copy %RELEASEDIR%\PGPtray.exe DATA\PGP60
copy Extras\40ComUpd.exe DATA\PGP60
copy docs\PGP60.cnt DATA\PGP60
copy docs\PGP60.hlp DATA\PGP60

mkdir "DATA\PGP60\Sample Keys"
copy keyrings\SampleKeys.asc "DATA\PGP60\Sample Keys"
copy keyrings\WhatIsThis.txt "DATA\PGP60\Sample Keys"

mkdir DATA\OEWINSYS
copy %RELEASEDIR%\PGP60oe.dll DATA\OEWINSYS

mkdir DATA\OEPLUGIN
copy %RELEASEDIR%\msimn.exe DATA\OEPLUGIN

mkdir DATA\NTMEMLOC
copy %SECUREMEMORYDIR%\winNT\sys\i386\free\PGPmemlock.sys DATA\NTMEMLOC

mkdir DATA\95MEMLOC
copy %SECUREMEMORYDIR%\win95\Release\PGPmemlock.vxd DATA\95MEMLOC

mkdir DATA\MANUAL
copy docs\PGPWinUsersGuide.pdf DATA\MANUAL

mkdir DATA\EXCHANGE
copy %RELEASEDIR%\pgpExch.dll DATA\EXCHANGE

mkdir DATA\EUDORA
mkdir DATA\EUDORA\REGULAR
mkdir DATA\EUDORA\FOURDOTO
copy %RELEASEDIR%\pgpEudoraPlugin.dll DATA\EUDORA\REGULAR\pgpplugin.dll
copy %RELEASEDIR%\pgpEudoraPluginV4.dll DATA\EUDORA\FOURDOTO\pgpplugin.dll

REM DATA\DOCS created above
copy docs\IntroToCrypto.pdf DATA\DOCS
copy docs\PGPInstallGuide.pdf DATA\DOCS
copy Extras\WhatsNew.doc DATA\DOCS

if NOT EXIST Uninstaller\tempstuff mkdir Uninstaller\tempstuff

REM Remove existing PGPdisk items

if EXIST Uninstaller\tempstuff\PGPdskIH.dll del Uninstaller\tempstuff\PGPdskIH.dll
if EXIST tempstuff\PGPdskIH.dll del tempstuff\PGPdskIH.dll

if NOT DEFINED INSTALLPGPDISK goto checkadmin

copy %PGPDISKDIR%\PGPdiskInstallHelper\Release\PGPdskIH.dll Uninstaller\tempstuff
copy %PGPDISKDIR%\PGPdiskInstallHelper\Release\PGPdskIH.dll tempstuff\

mkdir DATA\PGPDISK

mkdir DATA\PGPDISK\SHELL
copy %PGPDISKDIR%\PGPdiskShellExt\Release\PGPdskSE.dll DATA\PGPDISK\SHELL

mkdir DATA\PGPDISK\PROG
copy %PGPDISKDIR%\PGPdiskApp\Release\PGPdisk.exe DATA\PGPDISK\PROG
copy %PGPDISKDIR%\PGPdiskResident\Release\PGPdiskResident.exe DATA\PGPDISK\PROG
copy docs\PGPdisk.cnt DATA\PGPDISK\PROG
copy docs\PGPdisk.hlp DATA\PGPDISK\PROG

mkdir DATA\PGPDISK\MFC
copy Extras\MFC42.dll DATA\PGPDISK\MFC
copy Extras\msvcrt.dll DATA\PGPDISK\MFC

mkdir DATA\PGPDISK\DRIVERS

mkdir DATA\PGPDISK\DRIVERS\NT
copy %PGPDISKDIR%\PGPdiskDrvNT\i386\free\PGPdisk.sys DATA\PGPDISK\DRIVERS\NT

mkdir DATA\PGPDISK\DRIVERS\95
copy %PGPDISKDIR%\PGPdiskDrvWin95\Release\PGPdisk.pdr DATA\PGPDISK\DRIVERS\95

:checkadmin

if NOT DEFINED ISADMIN goto doneOK

mkdir DATA\ADMIN
mkdir DATA\ADMIN\ClientInstaller
copy %RELEASEDIR%\pgpAdmin.exe DATA\ADMIN
copy docs\PGPAdministratorsGuide.pdf DATA\DOCS

:doneOK
@echo End of setupinstall.bat
goto done

:badargs
@echo Invalid build name argument

:done
