@echo off
rem anonmail.bat - MSDOS script to assist in using the anonymous remailers
rem Karl L. Barrus - elee9sf@menudo.uh.edu
rem last update 7/14/93

if '%3'=='' goto help

echo :: > zzztemp1.txt
echo Request-Remailing-To: %2 >> zzztemp1.txt
echo. >> zzztemp1.txt

rem Apparently, MSDOS 6.0 has a choice command.  I still have 5.0 though...
if '%3'=='1'  goto noencrypt
if '%3'=='2'  goto noencrypt
if '%3'=='3'  goto noencrypt
if '%3'=='4'  goto noencrypt
if '%3'=='5'  goto noencrypt
if '%3'=='6'  goto encrypt
if '%3'=='7'  goto encrypt
if '%3'=='8'  goto encrypt
if '%3'=='9'  goto encrypt
if '%3'=='10' goto encrypt
if '%3'=='11' goto encrypt
if '%3'=='12' goto encrypt
if '%3'=='13' goto encrypt
if '%3'=='14' goto encrypt
if '%3'=='15' goto encrypt
if '%3'=='16' goto special

echo Improper choice.
goto done

:noencrypt
copy zzztemp1.txt + %1 zzztemp3.txt

goto done

:encrypt
if '%3'=='6'  set pgpremail=hal@alumni.caltech.edu
if '%3'=='7'  set pgpremail=ebrandt@jarthur.claremont.edu
if '%3'=='8'  set pgpremail=phantom@mead.u.washington.edu
if '%3'=='9'  set pgpremail=remailer@rebma.mn.org
if '%3'=='10' set pgpremail=elee7h5@rosebud.ee.uh.edu
if '%3'=='11' set pgpremail=hfinney@shell.portal.com
if '%3'=='12' set pgpremail=remail@tamaix.tamu.edu
if '%3'=='13' set pgpremail=remail@tamsum.tamu.edu
if '%3'=='14' set pgpremail=remailer@utter.dis.org
if '%3'=='15' set pgpremail=remail@entropy.linet.org

echo :: > zzztemp2.txt
echo Encrypted: PGP >> zzztemp2.txt
echo. >> zzztemp2.txt

pgp -ea zzztemp1.txt %pgpremail% 
copy zzztemp2.txt + zzztemp1.asc + %1 zzztemp3.txt

goto done

:special
if '%3'=='16' set pgpremail=remail@extropia.wimsey.com

copy zzztemp1.txt + %1 zzztemp2.txt
pgp -ea zzztemp2.txt %pgpremail%
copy zzztemp2.asc zzztemp3.txt

goto done

:help
echo  Usage: anonmail filename destination_address remailer_number
echo  1: nowhere@bsu-cs.bsu.edu           no encryption
echo  2: hh@cicada.berkeley.edu           no encryption
echo  3: hh@pmantis.berkeley.edu          no encryption
echo  4: hh@soda.berkeley.edu             no encryption
echo  5: 00x@uclink.berkeley.edu          no encryption
echo  6: hal@alumni.caltech.edu           encryption
echo  7: ebrandt@jarthur.claremont.edu    encryption
echo  8: phantom@mead.u.washington.edu    encryption
echo  9: remailer@rebma.mn.org            encryption
echo 10: elee7h5@rosebud.ee.uh.edu        encryption
echo 11: hfinney@shell.portal.com         encryption
echo 12: remail@tamaix.tamu.edu           encryption
echo 13: remail@tamsum.tamu.edu           encryption
echo 14: remailer@utter.dis.org           encryption
echo 15: remail@entropy.linet.org         encryption
echo 16: remail@extropia.wimsey.com       special
echo.
echo NOTE: final output will be placed in the filename specified
goto end
:done
del %1
rename zzztemp3.txt %1
del zzz*.*
:end
