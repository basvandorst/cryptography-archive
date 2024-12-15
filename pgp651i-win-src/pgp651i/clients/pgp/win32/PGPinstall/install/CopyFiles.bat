@echo off

set INSTALLPGPDISK=NO
set INSTALLPGPADMIN=NO

set PGPBASEDIR=..\..\..\..\..
set LIBSDIR=%PGPBASEDIR%\libs
set PGPNETDIR=%PGPBASEDIR%\clients\net\win32
set PGPDISKDIR=%PGPBASEDIR%\clients\disk\win32
set RELEASEDIR=..\..\PGPClient\Release
set INSTALLDIR="Setup Files\Compressed Files\0009-English\Intel 32"

REM Delete existing DATA directory and create common hierarchy

if EXIST DATA rmdir /Q /S Data
if EXIST "Setup Files" rmdir /Q /S "Setup Files"
if EXIST "Component Definitions\Default.cdf" del "Component Definitions\Default.cdf"

REM Create data directories:
MKDIR DATA
MKDIR DATA\NTCMDLINE
MKDIR DATA\NTCMDLINEDOCS
MKDIR DATA\PGP
MKDIR DATA\PGP\DLLS
MKDIR DATA\PGP\DOCS
MKDIR DATA\PGP\DOCS\ADMINONLY
MKDIR DATA\PGP\EUDORA
MKDIR DATA\PGP\EUDORAV4
MKDIR DATA\PGP\EXCHANGE
MKDIR DATA\PGP\EXES
MKDIR DATA\PGP\HELP
MKDIR DATA\PGP\KEYRINGS
MKDIR DATA\PGP\MEMLOCK
MKDIR DATA\PGP\OE
MKDIR DATA\PGP\SAMPLEKEYS
MKDIR DATA\PGP\SDK
MKDIR DATA\PGPDISK
MKDIR DATA\PGPDISK\DLLS
MKDIR DATA\PGPDISK\DRIVER
MKDIR DATA\PGPDISK\DRIVER95
MKDIR DATA\PGPDISK\HELP
MKDIR DATA\PGPDISK\MFC
MKDIR DATA\PGPNET
MKDIR DATA\PGPNET\DRIVER
MKDIR DATA\PGPNET\DRIVER95
MKDIR DATA\PGPNET\INF
MKDIR DATA\TEMP
MKDIR DATA\USERGUIDE

MKDIR "Setup Files"
MKDIR "Setup Files\Compressed Files"
MKDIR "Setup Files\Compressed Files\0009-English"
MKDIR "Setup Files\Compressed Files\0009-English\Intel 32"
MKDIR "Setup Files\Compressed Files\0009-English\OS Independent"
MKDIR "Setup Files\Compressed Files\Language Independent"
MKDIR "Setup Files\Compressed Files\Language Independent\Intel 32"
MKDIR "Setup Files\Compressed Files\Language Independent\OS Independent"
MKDIR "Setup Files\Uncompressed Files"
MKDIR "Setup Files\Uncompressed Files\0009-English"
MKDIR "Setup Files\Uncompressed Files\0009-English\Intel 32"
MKDIR "Setup Files\Uncompressed Files\0009-English\OS Independent"

REM Check arguments
if "%1" == "DS" goto ds
if "%1" == "PP" goto pp
if "%1" == "FREEWARE" goto freeware
goto badargs

:ds
set INSTALLPGPDISK=YES
set INSTALLPGPADMIN=YES
set DOCSDIR=%PGPBASEDIR%\docs\builds\ds
set LICENSE=%DOCSDIR%\License.lic
set SUPPORTDIR=..\supportfiles\business
copy "Component Definitions\DS.cdf" "Component Definitions\Default.cdf"
goto goodargs

:pp
set INSTALLPGPDISK=YES
set DOCSDIR=%PGPBASEDIR%\docs\builds\pp
set LICENSE=%DOCSDIR%\License.lic
set SUPPORTDIR=..\supportfiles\personal
copy "Component Definitions\PP.cdf" "Component Definitions\Default.cdf"
goto goodargs

:freeware
set DOCSDIR=%PGPBASEDIR%\docs\builds\freeware
set LICENSE=%DOCSDIR%\Freeware.txt
set SUPPORTDIR=..\supportfiles\freeware
copy "Component Definitions\Freeware.cdf" "Component Definitions\Default.cdf"
goto goodargs

:goodargs

REM Copy Installer helpers

copy %LIBSDIR%\pfl\win32\InstallDLL\Release\install.dll	%INSTALLDIR%
copy %PGPBASEDIR%\clients\disk\win32\PGPDiskInstallHelper\Release\PGPDskIH.dll %INSTALLDIR%
copy %LICENSE% %INSTALLDIR%\License.txt

copy ..\supportfiles\toprightlogo\neta.bmp 	%INSTALLDIR%
copy ..\supportfiles\toprightlogo\netax.bmp 	%INSTALLDIR%

copy %DOCSDIR%\WhatsNew.txt 	%INSTALLDIR%
copy %SUPPORTDIR%\SideBar.bmp 	%INSTALLDIR%
copy %SUPPORTDIR%\SideBarX.bmp 	%INSTALLDIR%
copy %SUPPORTDIR%\SideBarXX.bmp %INSTALLDIR%
copy %SUPPORTDIR%\pubring.pkr	DATA\PGP\KEYRINGS
copy %SUPPORTDIR%\secring.skr	DATA\PGP\KEYRINGS

copy %RELEASEDIR%\PGPcl.dll 	DATA\PGP\DLLS
copy %RELEASEDIR%\PGPhk.dll 	DATA\PGP\DLLS
copy %RELEASEDIR%\PGPmn.dll 	DATA\PGP\DLLS
copy %RELEASEDIR%\PGPsc.dll 	DATA\PGP\DLLS

copy %RELEASEDIR%\PGPKeys.exe 	DATA\PGP\EXES
copy %RELEASEDIR%\PGPLog.exe 	DATA\PGP\EXES
copy %RELEASEDIR%\PGPTools.exe 	DATA\PGP\EXES
copy %RELEASEDIR%\PGPTray.exe 	DATA\PGP\EXES
copy ..\uninstall\Uninstall.exe DATA\PGP\EXES

copy %RELEASEDIR%\PGPEudoraPlugin.dll 	DATA\PGP\EUDORA\PGPPlugin.dll
copy %RELEASEDIR%\PGPEudoraPluginV4.dll DATA\PGP\EUDORAV4\PGPPlugin.dll

copy ..\supportfiles\SampleKeys.asc	DATA\PGP\SAMPLEKEYS
copy ..\supportfiles\WhatIsThis.txt	DATA\PGP\SAMPLEKEYS

copy %RELEASEDIR%\PGPExch.dll 		DATA\PGP\EXCHANGE

copy %LIBSDIR%\pfl\win32\SecureMemory\PGPmemlock9x\Release\PGPmemlock.vxd DATA\PGP\MEMLOCK
copy %LIBSDIR%\pfl\win32\SecureMemory\PGPmemlockNT\Release\PGPmemlock.sys DATA\PGP\MEMLOCK

copy %RELEASEDIR%\PGPOE.dll 	DATA\PGP\OE

copy %DOCSDIR%\..\ds\PGP.cnt 			DATA\PGP\HELP
copy %DOCSDIR%\..\ds\PGP.hlp 			DATA\PGP\HELP
copy %DOCSDIR%\..\common\IntroToCrypto.pdf 	DATA\PGP\DOCS\IntroToCrypto.pdf
copy %DOCSDIR%\WhatsNew.htm 			DATA\PGP\DOCS
copy %DOCSDIR%\..\ds\PGPWinUsersGuide.pdf 	DATA\USERGUIDE

copy %RELEASEDIR%\PGP_SDK.dll 	DATA\PGP\SDK
copy %RELEASEDIR%\PGPsdkNL.dll 	DATA\PGP\SDK
copy %RELEASEDIR%\PGPsdkUI.dll 	DATA\PGP\SDK

copy %PGPNETDIR%\Service\Release\IKEService.exe			DATA\PGPNET
copy %PGPNETDIR%\Install\Enumerator\Release\SetAdapter.exe	DATA\PGPNET
copy %PGPNETDIR%\App\NetApp\\Release\PGPNet.exe			DATA\PGPNET
copy %PGPNETDIR%\App\NetApp\\Release\PGPNet.exe			DATA\PGPNET
copy %DOCSDIR%\..\ds\PGPnet.cnt 				DATA\PGPNET
copy %DOCSDIR%\..\ds\PGPnet.hlp 				DATA\PGPNET
copy %PGPNETDIR%\Install\Inf\NT\OEMSetup.inf			DATA\PGPNET\INF
copy %PGPNETDIR%\Install\Inf\NT\PGPRebind.inf			DATA\PGPNET\INF
copy %PGPNETDIR%\Driver\Release\PGPnet.sys			DATA\PGPNET\DRIVER
copy %PGPNETDIR%\Driver\Win95Release\PGPnet.vxd			DATA\PGPNET\DRIVER95

copy %PGPBASEDIR%\clients\pgp\cmdline\Release\PGP.exe 	DATA\NTCMDLINE
copy %DOCSDIR%\..\cmdline\PGPCmdLineGuide.pdf 		DATA\NTCMDLINEDOCS

if "%INSTALLPGPADMIN%" == "NO" goto checkdisk

copy %DOCSDIR%\..\ds\PGPAdministratorsGuide.pdf DATA\PGP\DOCS\ADMINONLY
copy %RELEASEDIR%\PGPAdmin.exe 			DATA\PGP\EXES

:checkdisk

if "%INSTALLPGPDISK%" == "NO" goto doneOK

copy %PGPDISKDIR%\PGPDiskApp\Release\PGPDisk.exe 		DATA\PGPDISK
copy %PGPDISKDIR%\PGPDiskResident\Release\PGPDiskResident.exe 	DATA\PGPDISK
copy %DOCSDIR%\..\ds\PGPdisk.cnt 				DATA\PGPDISK\HELP
copy %DOCSDIR%\..\ds\PGPdisk.hlp 				DATA\PGPDISK\HELP
copy %PGPDISKDIR%\PGPDiskShellExt\Release\PGPdskSE.dll 		DATA\PGPDISK\DLLS
copy %PGPDISKDIR%\PGPDiskDrvWin95\Release\PGPdisk.pdr 		DATA\PGPDISK\DRIVER
copy %PGPDISKDIR%\PGPDiskDrvNT\i386\free\PGPdisk.sys 		DATA\PGPDISK\DRIVER

:doneOK
@echo End of CopyFiles.bat
goto done

:badargs
@echo Invalid build name argument

:done
