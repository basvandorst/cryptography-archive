@echo off
rem  resume.bat -- Batch file to run Placement Services credentials system
rem  Called as:  resume dir
rem    where x is the drive letter and path for the needed Placement files.
rem  Mark Riordan   29 July 1992
if not %1a==a goto okparam
echo Usage:  resume drive_letter_of_frep_files
goto veryend
:okparam
cls
type %1\opening.frp
if exist file del file
if exist msuadd.dat del msuadd.dat
:mainmenu
type %1\main.frp
rem echo What do you want to do:
rem echo .
rem echo .   1.  Create a new resume.
rem echo .   2.  Retrieve an existing resume from the server.
rem echo .   3.  Retrieve an existing resume from floppy drive A:
rem echo .   4.  Retrieve an existing resume from floppy drive B:
rem echo .
%1\ask_bat 1 1 2 2 3 3 4 4 5 5 /f /h /t:400 /p "-> Type 1, 2, 3, 4, or 5: " 
if errorlevel 255 goto timeout
if errorlevel 5 goto done
if errorlevel 4 goto getfromb
if errorlevel 3 goto getfroma
if errorlevel 2 goto getfromserver
if errorlevel 1 goto newresume
goto mainmenu
:getfromserver
%1\xmitfrep -g -r resume
if errorlevel 2 goto severeprob
if errorlevel 1 goto badget
ren file msuadd.dat
goto runmsu
:getfromb
type %1\blank1.frpÿ
echo Insert your floppy into drive B: and press Enter when ready.
pause
copy b:msuadd.dat
if not exist msuadd.dat goto badget
goto runmsu
:getfroma
type %1\blank1.frpÿ
echo Insert your floppy into drive A: and press Enter when ready.
pause
copy a:msuadd.dat
if not exist msuadd.dat goto badget
goto runmsu
:runmsu
%1\msu
rem cls
:savemenu
type %1\save.frp
rem echo What do you want to do with this copy of your resume?
rem echo .ÿ
rem echo .   1.  Save a copy of it on the server.
rem echo .   2.  Copy it to floppy drive A:
rem echo .   3.  Copy it to floppy drive B:
rem echo .   4.  Abandon any changes you've made this session.
rem echo .ÿ
%1\ask_bat 1 1 2 2 3 3 4 4  /f /h /t:600 /p "Type 1, 2, 3, or 4: " 
if errorlevel 255 goto timeout
if errorlevel 4 goto abandon
if errorlevel 3 goto puttob
if errorlevel 2 goto puttoa
if errorlevel 1 goto puttoserver
goto savemenu
:puttoserver
copy msuadd.dat file
%1\xmitfrep -p -r resume
if not errorlevel 1 goto askalsofloppy
echo I could not store your resume.  
echo Please contact a Computer Laboratory consultant for help.
:askalsofloppy
echo Do you also want to make a copy on a floppy disk?
%1\ask_bat y 1 n 2 /f /h /t:600  /p  "  Type Y or N: " 
if errorlevel 2 goto done
if errorlevel 1 goto whichfloppy
goto askalsofloppy
:whichfloppy
echo Which floppy drive, A: or B:?
%1\ask_bat a 1 b 2 q 3 /f /h /t:600  /p "Type A or B, or Q to quit: " 
if errorlevel 3 goto done
if errorlevel 2 goto puttob
if errorlevel 1 goto puttoa
goto whichfloppy
:puttoa
echo Insert your floppy into drive A: and press Enter when ready.
pause
copy msuadd.dat a:
goto done
:puttob
echo Insert your floppy into drive B: and press Enter when ready.
pause
copy msuadd.dat b:
goto done

echo ***
echo *** There were problems storing your resume.  Please get help.
echo ***
goto savemenu
:noget
echo Do you want to create a resume from scratch?
%1\ask_bat y 1 n 2 /f /h /t:600 /p "   Type Y or N "
if errorlevel 255 goto timeout
if errorlevel 2 goto done
:newresume
copy %1\msuadd0.dat msuadd.dat
goto runmsu
:abandon
echo Any changes you made this session will be lost.  Are you sure?
%1\ask_bat y 1 n 2 /f /h /t:600 /p "   Type Y or N: " 
if errorlevel 255 goto timeout
if errorlevel 2 goto savemenu
goto done
:badget
type %1\blank1.frpÿ
echo There were problems retrieving your old resume.
type %1\blank1.frpÿ
goto noget
:severeprob
echo ***
echo ***  Unable to retrieve your resume.  Try again later.
echo ***
goto done
:timeout
echo Looks as if you've gone away.
:done
echo Exiting Resume Credentials System...
goto veryend
:veryend
