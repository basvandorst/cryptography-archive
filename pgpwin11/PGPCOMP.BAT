@echo off
pgp -%1 plaintmp.$$$ %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 goto bad
ren plaintmp.asc enc$tmp.$$$
goto done
:bad
echo PGP failed, Sorry > enc$tmp.$$$
:done
