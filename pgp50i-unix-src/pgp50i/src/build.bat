REM $Id: build.bat,v 1.1.2.4 1997/06/19 22:01:02 philipn Exp $
REM
REM Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.

if "%2"=="NO_RSA" goto NORSA
if "%2"=="RSAREF" goto RSAREF
if "%1"=="" goto DONE
echo %1
echo %1 >> build.txt

if "%3"=="" goto NORMAL
:SPECIAL
echo NMAKE /f "%1.mak CFG="%2 - Win32 %3"
NMAKE /f "%1.mak" CFG="%2 - Win32 %3" >> build.txt
goto DONE

:NORMAL
echo NMAKE /f "%1.mak CFG="%1 - Win32 %2"
NMAKE /f "%1.mak" CFG="%1 - Win32 %2" >> build.txt
goto DONE

:RSAREF
echo NMAKE /f "%1.mak CFG="%1 - Win32 %2"
NMAKE /f "%1.mak" CFG="%1 - Win32 %2 Release" >> build.txt

:NORSA
echo NMAKE /f "%1.mak CFG="%1 - Win32 %2"
NMAKE /f "%1.mak" CFG="%1 - Win32 %2 Release" >> build.txt

:DONE
