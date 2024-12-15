@echo off

rem ....Build components...
SET Installshield_Path=c:\ishield
SET INSTRC=%Installshield_Path%\Dialogs\Include 
SET Installshield_Program_Path=%Installshield_Path%\Program
SET COMPILE=%Installshield_Program_Path%\compile.exe
SET ICOMP=%Installshield_Program_Path%\icomp.exe        
SET PACK=%Installshield_Program_Path%\packlist.exe
SET SPLIT=%Installshield_Program_Path%\split.exe

echo. > Disk1\Disk1.id

if EXIST tempstuff\_setup.lib del /Q tempstuff\_setup.lib
copy %Installshield_Path%\_isres.dll tempstuff\
copy %Installshield_Path%\setup.exe Disk1\
copy %Installshield_Path%\_inst32i.ex_ Disk1\
copy %Installshield_Path%\_isdel.exe Disk1\
copy %Installshield_Path%\_setup.dll Disk1\

%COMPILE% Uninst~1\setup.rul -g
if not exist Uninst~1\setup.ins pause

copy Uninst~1\setup.ins disk1

%ICOMP% -i tempstuff\*.* tempstuff\_setup.lib

ECHO.
ECHO Copy Output to Disk1
copy tempstuff\_setup.lib disk1
