@ECHO OFF

ECHO.
ECHO Let's Setup Some Important Paths
ECHO.

SET Installshield_Path=c:\ishield
SET INSTRC=%Installshield_Path%\Dialogs\Include 
SET Installshield_Program_Path=%Installshield_Path%\Program
SET COMPILE=%Installshield_Program_Path%\compile.exe
SET ICOMP=%Installshield_Program_Path%\icomp.exe        
SET PACK=%Installshield_Program_Path%\packlist.exe
SET SPLIT=%Installshield_Program_Path%\split.exe

ECHO.
ECHO Make sure there aren't any read-only files
attrib -r setup\*.*
attrib -r disk1\*.*

copy %Installshield_Path%\_isres.dll tempstuff\
copy %Installshield_Path%\isdbgn.dll setup\
copy %Installshield_Path%\compile.exe setup\
copy %Installshield_Path%\packlist.exe setup\
copy %Installshield_Path%\_inst32i.ex_ Disk1\
copy %Installshield_Path%\_isdel.exe Disk1\
copy %Installshield_Path%\_setup.dll Disk1\
copy %Installshield_Path%\setup.exe Disk1\
copy %Installshield_Path%\uninst.exe Disk1\

del data\data.*

if EXIST disk2 rmdir /S /Q disk2
if EXIST disk3 rmdir /S /Q disk3
if EXIST disk4 rmdir /S /Q disk4
if EXIST disk5 rmdir /S /Q disk5
if EXIST disk6 rmdir /S /Q disk6
if EXIST disk7 rmdir /S /Q disk7
if EXIST disk8 rmdir /S /Q disk8
if EXIST disk9 rmdir /S /Q disk9
if EXIST disk10 rmdir /S /Q disk10
if EXIST disk11 rmdir /S /Q disk11
if EXIST disk12 rmdir /S /Q disk12
if EXIST disk13 rmdir /S /Q disk13
if EXIST disk14 rmdir /S /Q disk14
if EXIST disk15 rmdir /S /Q disk15

ECHO Compile The Setup Script
%COMPILE% setup\setup.rul -g
pause



ECHO.
ECHO Copy Output to Disk1
copy setup\setup.ins disk1
REM copy setup\setup.dbg disk1

ECHO.
ECHO Make Sure that Old Data is Destroyed
if exist data\data.z del data\data.z

ECHO.
ECHO Compile the Data in the Data Directory
%ICOMP% -i data\*.* data\data.z

cd Data
%SPLIT% data.z -f1420 -d1@100

ECHO.
ECHO  Copy Output to Disks

copy data.1 ..\disk1
echo. > ..\disk1\disk1.id

if NOT EXIST data.2 goto copiedallsegments
mkdir ..\disk2
echo. > ..\disk2\disk2.id
copy data.2 ..\disk2

if NOT EXIST data.3 goto copiedallsegments
mkdir ..\disk3
echo. > ..\disk3\disk3.id
copy data.3 ..\disk3

if NOT EXIST data.4 goto copiedallsegments
mkdir ..\disk4
echo. > ..\disk4\disk4.id
copy data.4 ..\disk4

if NOT EXIST data.5 goto copiedallsegments
mkdir ..\disk5
echo. > ..\disk5\disk5.id
copy data.5 ..\disk5

if NOT EXIST data.6 goto copiedallsegments
mkdir ..\disk6
echo. > ..\disk6\disk6.id
copy data.6 ..\disk6

if NOT EXIST data.7 goto copiedallsegments
mkdir ..\disk7
echo. > ..\disk7\disk7.id
copy data.7 ..\disk7

if NOT EXIST data.8 goto copiedallsegments
mkdir ..\disk8
echo. > ..\disk8\disk8.id
copy data.8 ..\disk8

if NOT EXIST data.9 goto copiedallsegments
mkdir ..\disk9
echo. > ..\disk9\disk9.id
copy data.9 ..\disk9

if NOT EXIST data.10 goto copiedallsegments
mkdir ..\disk10
echo. > ..\disk10\disk10.id
copy data.10 ..\disk10

if NOT EXIST data.11 goto copiedallsegments
mkdir ..\disk11
echo. > ..\disk11\disk11.id
copy data.11 ..\disk11

if NOT EXIST data.12 goto copiedallsegments
mkdir ..\disk12
echo. > ..\disk12\disk12.id
copy data.12 ..\disk12

if NOT EXIST data.13 goto copiedallsegments
mkdir ..\disk13
echo. > ..\disk13\disk13.id
copy data.13 ..\disk13

if NOT EXIST data.14 goto copiedallsegments
mkdir ..\disk14
echo. > ..\disk14\disk14.id
copy data.14 ..\disk14

if NOT EXIST data.15 goto copiedallsegments
mkdir ..\disk15
echo. > ..\disk15\disk15.id
copy data.15 ..\disk15

:copiedallsegments

ECHO.
ECHO Create the PackList
cd ..\setup
%PACK% setup.lst

ECHO.
ECHO Copy Output to Disk1
copy setup.pkg ..\disk1
del setup.pkg

ECHO.
ECHO Make Sure that Old Lib is Destroyed
del ..\tempstuff\_setup.lib

ECHO.
cd ..
copy Disk1\license.txt tempstuff
copy Data\Docs\readme.txt tempstuff
ECHO Create the Temporary Stuff
%ICOMP% -i tempstuff\*.* tempstuff\_setup.lib

ECHO.
ECHO Copy Output to Disk1
copy tempstuff\_setup.lib disk1


ECHO.
ECHO Clean up after MeSelf
SET Installshield_Path=
SET  Installshield_Program_Path=
SET COMPILE=
SET ICOMP=
SET PACK=

ECHO Build.bat Completed
ECHO.
ECHO.

@ECHO ON
