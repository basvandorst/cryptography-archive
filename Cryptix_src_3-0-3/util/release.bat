@echo off
if not "%CRYPTIX_VERSION%"=="" goto ok
echo Set CRYPTIX_VERSION to the suffix of the release files, in the form "-x_y_z".
goto exit

:ok
call util\zipdoc
call util\zipclasses
call util\zipsrc
call util\zipnative
echo.
:exit
