@echo off
echo  VOUCH Version 1.0, Copyright (c) 1993 Awais M. Hussain
echo       NIST Digital Signature Standard
echo -------------------------------------------------------
echo.
echo  Will install VOUCH in %1\VCH 
echo  Assuming autoexec.bat is in drive %2\
echo.
echo  Edit INSTALL.BAT, if the above is not suitable.
echo.
echo -------------------------------------------------------
echo  Cntrl-Break to interrput.
Pause
rem
md %1\vch
md %1\vch\prv
copy vch.exe %1\vch
copy PubKey.exe %1\vch
copy rmln.exe %1\vch
copy dire.exe %1\vch
copy PubKey.vch %1\vch
copy usrGuide.vch %1\vch
copy readme.vch %1\vch  
copy gPubKey.bat %1\vch
copy set_vch.bat %1\vch          
copy vstd.exe %1\vch
copy vstc.exe %1\vch
copy stc.bat %1\vch
copy std.bat %1\vch
copy b2a.exe %1\vch
copy a2b.exe %1\vch
copy myPublic.key %1\vch\prv
copy Juan.key %1\vch\prv  
copy match.vch %1\vch\prv  
rem
set vch=%1\vch\prv
if not errorlevel 0 goto err
echo.
echo Appending "SET VCH=%1\VCH\PRV" statement to %1\AUTOEXEC.BAT
echo.
pause
ren %2\autoexec.bat autoexec.vch
copy %2\autoexec.vch + set_vch.bat %2\autoexec.bat
echo.
echo Initializing....
echo.
%1\vch\vch /i
echo.
echo Please carefully read messages before pressing any keys.
echo.
echo.
echo Generating Keys....
echo.
echo Press Y ; and then press RETURN on the password prompt.
@echo on
%1\vch\vch /k
@echo off
if not exist %1\vch\prv\myPrivat.key goto err
echo.
echo   VOUCH installed in %1\VCH
echo   The VOUCH_directory is %1\VCH\PRV
echo.
echo  Update file "match.vch" in the VOUCH directory.
echo  Edit STC.BAT and STD.BAT, if needed.
echo.
echo.
echo Consult file "usrGuide.vch" for information.
echo Section III of usrGuide.vch is a must reading for security reasons.
echo.
echo.
cd %1\vch
%1
echo   VOUCH installed in %1\VCH > %1\VCH\install.log
echo   The VOUCH_directory is %1\VCH\PRV >> %1\VCH\install.log
echo" >> %1\VCH\install.log
echo  Update file "match.vch" in the VOUCH directory >> %1\VCH\install.log
echo  Edit STC.BAT and STD.BAT, if needed >> %1\VCH\install.log
echo" >> %1\VCH\install.log
echo" >> %1\VCH\install.log
echo Consult file "usrGuide.vch" for information. >> %1\VCH\install.log
echo Section III of usrGuide.vch is a must reading for security reasons. >> %1\VCH\install.log
echo" >> %1\VCH\install.log
goto exit
:err
echo An error occured.
:exit
