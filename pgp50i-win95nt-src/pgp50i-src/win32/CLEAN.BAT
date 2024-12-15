REM $Id$
REM
REM Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.

@echo off
if "%1"=="" goto DONE

del %1\%2\*.obj
del %1\%2\*.lib
del %1\%2\*.dll
del %1\%2\*.exe


:DONE
