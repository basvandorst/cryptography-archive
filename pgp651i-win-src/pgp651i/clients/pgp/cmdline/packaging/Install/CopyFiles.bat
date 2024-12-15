@echo off

set PGPBASEDIR=..\..\..\..\..
set DOCSDIR=%PGPBASEDIR%\docs\builds
set LIBSDIR=%PGPBASEDIR%\libs
set INSTALLDIR="Setup Files\Compressed Files\0009-English\Intel 32"

REM Delete existing DATA directory and create common hierarchy

if EXIST DATA rmdir /Q /S Data

REM Create data directories:
MKDIR DATA
MKDIR DATA\docs
MKDIR DATA\EXEs
MKDIR DATA\Keyrings

copy %DOCSDIR%\cmdline\PGPCmdLineGuide.pdf DATA\docs
copy %DOCSDIR%\common\IntrotoCrypto.pdf DATA\docs
copy ..\pgp-6.0\docs\WHATSNEW DATA\docs\WhatsNew.txt
copy ..\pgp-6.0\docs\WhatsNew.html DATA\docs\WhatsNew.htm
copy ..\..\CREDITS DATA\docs\Credits.txt

copy ..\..\Release\PGP.exe DATA\EXEs
copy pgp.bat DATA\EXEs

copy ..\pgp-6.0\keyrings\SampleKeys.asc DATA\Keyrings

if "%1" == "FREEWARE" goto freeware

set LICENSE=..\pgp-6.0\docs\LICENSE
goto copyLicense

:freeware
set LICENSE=%DOCSDIR%\freeware\Freeware.txt

:copyLicense

copy %LICENSE% DATA\docs\license.txt

if EXIST "Setup Files\Compressed Files\Language Independent" rmdir /Q /S "Setup Files\Compressed Files\Language Independent"
mkdir "Setup Files\Compressed Files\Language Independent"
mkdir "Setup Files\Compressed Files\Language Independent\OS Independent"
copy %LICENSE% "Setup Files\Compressed Files\Language Independent\OS Independent\license.txt"

if NOT EXIST "Setup Files\Compressed Files\0009-English\OS Independent" mkdir "Setup Files\Compressed Files\0009-English\OS Independent"
if NOT EXIST "Setup Files\Compressed Files\Language Independent\Intel 32" mkdir "Setup Files\Compressed Files\Language Independent\Intel 32"



:doneOK
@echo End of CopyFiles.bat
