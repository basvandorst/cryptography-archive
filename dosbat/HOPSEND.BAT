@echo off
rem hopsend.bat - allows routing a message through various remailers
rem Karl Barrus - elee9sf@menudo.uh.edu
rem last update 7/14/93

rem if no parameters, print help file
if '%1'=='' goto help

rem get file name
shift
set filename=%0
if not exist %filename% goto errnofile

rem set up encrypted pgp header
echo :: > zzztemp1.txt
echo Encrypted: PGP >> zzztemp1.txt
echo. >> zzztemp1.txt

rem create null message
echo. > zzztemp3.txt

rem get email address
shift
set to=%0
if '%to%'=='' goto errmail

rem start building encryption headers...
set firsttime=yes
:repeat
shift
if '%0'=='' goto finish

if '%0'=='1'  set rto=nowhere@bsu-cs.bsu.edu
if '%0'=='2'  set rto=hh@cicada.berkeley.edu
if '%0'=='3'  set rto=hh@pmantis.berkeley.edu
if '%0'=='4'  set rto=hh@soda.berkeley.edu
if '%0'=='5'  set rto=00x@uclink.berkeley.edu
if '%0'=='6'  set rto=hal@alumni.caltech.edu
if '%0'=='7'  set rto=ebrandt@jarthur.claremont.edu
if '%0'=='8'  set rto=phantom@mead.u.washington.edu
if '%0'=='9'  set rto=remailer@rebma.mn.org
if '%0'=='10' set rto=elee7h5@rosebud.ee.uh.edu
if '%0'=='11' set rto=hfinney@shell.portal.com
if '%0'=='12' set rto=remail@tamaix.tamu.edu
if '%0'=='13' set rto=remail@tamsun.tamu.edu
if '%0'=='14' set rto=remailer@utter.dis.org
if '%0'=='15' set rto=remail@entropy.linet.org
if '%0'=='16' set rto=remail@extropia.wimsey.com

rem set up remailing request header
echo :: > zzztemp2.txt
echo Request-Remailing-To: %to% >> zzztemp2.txt
echo. >> zzztemp2.txt

if '%firsttime%'=='no' goto skipthis
set firsttime=no
copy zzztemp2.txt + %filename% zzztemp4.txt > nul
del zzztemp2.txt
rename zzztemp4.txt zzztemp2.txt

:skipthis
rem append previous message
copy zzztemp2.txt + zzztemp3.txt zzztemp4.txt > nul
del zzztemp2.txt
rename zzztemp4.txt zzztemp2.txt

if '%0'=='1' goto none
if '%0'=='2' goto none
if '%0'=='3' goto none
if '%0'=='4' goto none
if '%0'=='5' goto none

pgp -ea zzztemp2.txt %rto% > nul
copy zzztemp1.txt + zzztemp2.asc zzztemp3.txt > nul
goto next

:none
copy zzztemp2.txt zzztemp3.txt > nul

:next
set to=%rto%

goto repeat

:finish
del %filename%
rename zzztemp3.txt %filename%
echo Remember to mail %filename% to %to% 

goto done

:help
echo Usage: hopmail filename address remailer# [remailer#]...
echo Address: destination address
echo.
echo Remailers:
echo  1: nowhere@bsu-cs.bsu.edu
echo  2: hh@cicada.berkeley.edu
echo  3: hh@pmantis.berkeley.edu
echo  4: hh@soda.berkeley.edu
echo  5: 00x@uclink.berkeley.edu
echo  6: hal@alumni.caltech.edu
echo  7: ebrandt@jarthur.claremont.edu
echo  8: phantom@mead.u.washington.edu
echo  9: remailer@rebma.mn.org
echo 10: elee7h5@rosebud.ee.uh.edu
echo 11: hfinney@shell.portal.com
echo 12: remail@tamaix.tamu.edu
echo 13: remail@tamsun.tamu.edu
echo 14: remailer@utter.dis.org
echo 15: remail@entropy.linet.org
echo 16: remail@extropia.wimsey.com
goto end

:errmail
echo Error: no destination specified
goto done

:errnofile
echo Error: file %filename% does not exist
goto end

:done
del zzz*.*

:end

