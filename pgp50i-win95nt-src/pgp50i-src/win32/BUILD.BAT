REM $Id: BUILD.BAT,v 1.1.1.1 1997/04/29 00:23:28 philipn Exp $
REM
REM Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.

@echo off
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

:DONE
