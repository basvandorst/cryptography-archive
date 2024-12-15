@echo off

rem Run this file to check signature
rem certificates for AutoPGP .EXE files

::: Thanks to John DeHaven for this file

if "_RESTART"=="%1" goto RESTART

if ""=="%PGPPATH%" echo Please set your PGPPATH variable first.
if ""=="%PGPPATH%" goto END

echo Checking for certifying key on public ring.
echo.
pgp +batchmode +verbose=0 -kv 0xCCEF447D > nul
if not errorlevel 1 goto CHECKIT
pgp +batchmode +verbose=0 -ka key.asc > nul
if not errorlevel 1 goto KEYADDED
echo ERROR: could not find key for "Stale Schumacher"
echo        and could not find file KEY.ASC to add it...
goto END

:KEYADDED
   echo You did not have Stale Schumacher's public key.
   echo For now, we will use the key in KEY.ASC, but it is
   echo strongly recommended that you obtain it from an 
   echo independent source, such as a public key server.
   echo.

:CHECKIT
   %0 _RESTART DECRYPT ENCRYPT INSTALL REGISTER

:RESTART
   shift
   if ""=="%1" goto end
   if not exist %1.sig echo Cannot find file %1.SIG.
   if not exist %1.sig goto RESTART
   if not exist %1.exe echo Cannot find file %1.EXE.
   if not exist %1.exe goto RESTART
   pgp +batchmode +verbose=0 %1.sig %1.exe > nul
   if not errorlevel 1 echo OK: %1.EXE checks good!
if not errorlevel 1 goto RESTART

   echo !!! WARNING !!!
   echo %1.EXE FAILED certification check...
goto RESTART

:END
