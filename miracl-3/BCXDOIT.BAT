rem MIRACL - IBM PC/MS-DOS Version 3.2
rem This batch files creates 80386 version of miracl.lib from its component
rem parts using the Borland C++ V3.1 (or greater) compiler
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "bcxdoit". It is assumed that paths have been correctly set up to
rem the compiler, librarian and linker.
rem
rem Provided mainly as a guide for creating a batch file tailored
rem specifically to your own configuration.
rem
rem Compile MIRACL modules
bcc -c -3 -O -g200 mrcore.c
bcc -c -3 -O mrarth0.c
bcc -c -3 -O -B mrarth1.c
bcc -c -3 -O -B mrarth2.c
bcc -c -3 -O mrsmall.c
bcc -c -3 -O mrround.c
bcc -c -3 -O mrio1.c
bcc -c -3 -O mrio2.c
bcc -c -3 -O mrgcd.c
bcc -c -3 -O mrjack.c
bcc -c -3 -O mrxgcd.c
bcc -c -3 -O mrarth3.c
bcc -c -3 -O mrrand.c
bcc -c -3 -O mrprime.c
bcc -c -3 -O mrcrt.c
bcc -c -3 -O mrscrt.c
bcc -c -3 -O mrfast.c
bcc -c -3 -O mrfrnd.c
bcc -c -3 -O mrflash.c
bcc -c -3 -O mrdouble.c
bcc -c -3 -O mrbuild.c
bcc -c -3 -O mrflsh1.c
bcc -c -3 -O mrpi.c
bcc -c -3 -O mrflsh2.c
bcc -c -3 -O mrflsh3.c
bcc -c -3 -O mrflsh4.c
rem 
rem Assemble mrmuldv.c ; be careful to use correct version from mrmuldv.any
tasm -ml mrmuldv.c          
rem
rem Create library 'miracl.lib'
del miracl.lib
tlib miracl
tlib miracl +mrflsh4+mrflsh3+mrflsh2+mrpi+mrflsh1
tlib miracl +mrio2+mrio1+mrdouble+mrflash+mrrand+mrprime+mrcrt+mrscrt+mrfast
tlib miracl +mrjack+mrfrnd+mrxgcd+mrgcd+mrround+mrbuild+mrarth3+mrarth2
tlib miracl +mrarth1+mrarth0+mrsmall+mrcore+mrmuldv
del mr*.obj
bcc -c -3 -O big
bcc -c -3 -O crt
bcc -c -3 -O number
bcc -c -3 -O flash
bcc brent big.obj number.obj miracl.lib
bcc sample flash.obj miracl.lib

