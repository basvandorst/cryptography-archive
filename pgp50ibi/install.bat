@echo off
 if not exist pgp.exe goto nopgp
 if "%1" == "" goto help
 if "%1" == "o" goto pgpo
 if "%1" == "O" goto pgpo
 if "%1" == "f" goto pgpf
 if "%1" == "F" goto pgpf
 goto help
:pgpo
 echo Creating only the PGP 2.6.3 compatible executables
 copy pgp.exe pgpo.exe >nul
 del pgp.exe >nul
 goto outahere
:pgpf
 echo Creating the full alternativ executables
 copy pgp.exe pgpe.exe >nul
 copy pgp.exe pgps.exe >nul
 copy pgp.exe pgpv.exe >nul
 copy pgp.exe pgpo.exe >nul
 del pgp.exe >nul
 goto outahere
:help
 echo INSTALL must be called with one parameter telling what versions
 echo of the executables you want.
 echo.
 echo INSTALL F
 echo      this installes all executables, including the original
 echo      executables and the PGP 2.6.3 compatible executable
 echo.
 echo INSTALL O
 echo      this installes only the keymanagement and the PGP 2.6.3
 echo      compatible executable. Should be sufficient for most people
 echo.
 goto outahere
:nopgp
 echo ERROR: can't find PGP.EXE
 goto outahere
:outahere
