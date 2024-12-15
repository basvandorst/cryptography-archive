rem MIRACL - IBM PC/MS-DOS Version 3.2
rem This batch files creates miracl.lib from its component parts
rem using the Borland C++ V3.1 (or greater) compiler
rem Also included are the commands to create some of the example programs
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "bcdoit". It is assumed that paths have been correctly set up to
rem the compiler, librarian and linker.
rem
rem Provided mainly as a guide for creating a batch file tailored
rem specifically to your own configuration.
rem
rem Compile MIRACL modules
bcc -c -O -g200 mrcore.c
bcc -c -O mrarth0.c
bcc -c -O mrarth1.c
bcc -c -O mrarth2.c
bcc -c -O mrsmall.c
bcc -c -O mrround.c
bcc -c -O mrio1.c
bcc -c -O mrio2.c
bcc -c -O mrgcd.c
bcc -c -O mrjack.c
bcc -c -O mrxgcd.c
bcc -c -O mrarth3.c
bcc -c -O mrrand.c
bcc -c -O mrprime.c
bcc -c -O mrcrt.c
bcc -c -O mrscrt.c
bcc -c -O mrfast.c
bcc -c -O mrfrnd.c
bcc -c -O mrflash.c
bcc -c -O mrdouble.c
bcc -c -O mrbuild.c
bcc -c -O mrflsh1.c
bcc -c -O mrpi.c
bcc -c -O mrflsh2.c
bcc -c -O mrflsh3.c
bcc -c -O mrflsh4.c
rem 
rem Assemble mrmuldv.c
bcc -c mrmuldv.c
rem
rem Create library 'miracl.lib'
del miracl.lib
tlib miracl
tlib miracl +mrflsh4+mrflsh3+mrflsh2+mrpi+mrflsh1
tlib miracl +mrio2+mrio1+mrdouble+mrflash+mrrand+mrprime+mrcrt+mrscrt+mrfast
tlib miracl +mrjack+mrfrnd+mrxgcd+mrgcd+mrround+mrbuild+mrarth3+mrarth2
tlib miracl +mrarth1+mrarth0+mrsmall+mrcore+mrmuldv
del mr*.obj
rem
rem Compile and link C++ versions of example programs where possible
bcc -c big
bcc -c number
bcc -c flash
bcc -c crt
bcc brute big.obj miracl.lib
bcc sample flash.obj miracl.lib
bcc brent big.obj number.obj miracl.lib
bcc pollard number.obj big.obj miracl.lib
bcc genkey big.obj miracl.lib
bcc encode big.obj miracl.lib
bcc decode crt.obj big.obj miracl.lib 
bcc fact big.obj miracl.lib
bcc palin big.obj miracl.lib
bcc keyids.c miracl.lib
bcc -g200 ratcalc.c miracl.lib
bcc factor.c miracl.lib
bcc dssign shs.c big.obj miracl.lib
bcc dssver shs.c big.obj miracl.lib
bcc dssgen big.obj miracl.lib

