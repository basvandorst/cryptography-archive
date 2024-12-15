REM $Id: clean.bat,v 1.1.2.2 1997/06/12 02:41:14 mhw Exp $
REM
REM Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.

@echo off
if "%1"=="" goto DONE

del %1\%2\*.obj
del %1\%2\*.lib
del %1\%2\*.dll
del %1\%2\*.exe


:DONE
