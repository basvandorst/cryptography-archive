set SAVEPROMPT=%PROMPT%
set PROMPT=
rem InstallShield program path
rem INSTRC is required by setup.rul
set INSTALLSHIELD=c:\ishield32
set INSTRC=%INSTALLSHIELD%\Dialogs\Include
set PROGRAM=%INSTALLSHIELD%\Program
set EXEBUILD=%INSTALLSHIELD%\utility\exebldr\exebuild.exe

rem Make sure there aren't any read-only files
attrib -r setup\*.*
attrib -r Data\*.* /s

rem Compile the uninstaller
cd Uninstaller
deltree /y Disk1
md disk1
%PROGRAM%\compile.exe Uninst~1\setup.rul -g
pause to examine any error messages
move uninstall\setup.ins Disk1

copy %PROGRAM%\_isres.dll tempstuff
%PROGRAM%\icomp.exe -i tempstuff\*.* Disk1\_setup.lib

rem Copy the standard InstallShield files to Disk 1
copy %PROGRAM%\setup.exe Disk1
copy %PROGRAM%\_inst32i.ex_ Disk1
copy %PROGRAM%\_setup.dll Disk1
copy %PROGRAM%\_isdel.exe Disk1

rem Build the Uninstall executable.  Kludged because bloody stupid
rem obnoxious exebuild can't find its stub if given a relative DestDir.
md \{is_kludge}
start /wait %EXEBUILD% Uninst~1\exebuild.ini
cd ..
move \{is_kludge}\setupex.exe Data\Pgp55\Uninstall.exe

rem Okay, on with the installer... compile it (-g is debugging info)
%PROGRAM%\compile.exe setup\setup.rul -g
pause to examine any error messages

rem Make directories for disk images
deltree /y Images
md Images
md Images\Disk1
rem >Images\Disk1\Disk1.ID
md Images\Disk2
rem >Images\Disk2\Disk2.ID
md Images\Disk3
rem >Images\Disk3\Disk3.ID
md Images\Disk4
rem >Images\Disk4\Disk4.ID
                          
rem Pack up the data in the Data Directory
%PROGRAM%\icomp.exe -i Data\*.* Images\data.z                     

rem Copy the standard InstallShield files to Disk 1
copy %PROGRAM%\setup.exe Images\Disk1
copy %PROGRAM%\_inst32i.ex_ Images\Disk1
copy %PROGRAM%\_setup.dll Images\Disk1
copy %PROGRAM%\_isdel.exe Images\Disk1

rem Copy setup script to Disk1
copy setup\setup.ini Images\Disk1
copy setup\setup.ins Images\Disk1
REM These two lines enable debugging`
REM copy setup\setup.dbg Images\Disk1
REM copy %PROGRAM%\isdbgn.dll Images\Disk1

rem Create the PackList SETUP.PKG
rem packlist gets confused by relative paths,
rem so it has to be created in place and moved

cd setup
%PROGRAM%\packlist.exe setup.lst
move setup.pkg ..\Images\Disk1
cd ..

rem Assemble the temporary stuff (deleted when install finishes)
copy Data\PGP55\license.txt tempstuff
copy Data\PGP55\readme.txt tempstuff
copy %PROGRAM%\_isres.dll tempstuff
del tempstuff\_setup.lib
%PROGRAM%\icomp.exe -i tempstuff\*.* Images\Disk1\_setup.lib

rem Split the main file into 1420K pieces, except the first is 375K.
rem (To leave room for all the other stuff on Disk 1.)
cd Images
%PROGRAM%\split.exe data.z -f1420 -d1@375
del data.z

rem  Copy Output to Disks (some may fail)
move data.1 Disk1
move data.2 Disk2
if errorlevel 1 deltree /y Disk2
move data.3 Disk3
if errorlevel 1 deltree /y Disk3
move data.4 Disk4
if errorlevel 1 deltree /y Disk4

cd ..
rem Build the full executable
start /wait %EXEBUILD% setup\exebuild.ini
move \{is_kludge}\setupex.exe PGPinst.exe
deltree /y \{is_kludge}

rem Clean up the environment
set INSTALLSHIELD=
set INSTRC=
set PROGRAM=
set EXEBUILD=

rem Build.bat complete
set PROMPT=%SAVEPROMPT%
set SAVEPROMPT=
