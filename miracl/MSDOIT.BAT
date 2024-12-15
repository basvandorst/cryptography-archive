rem MIRACL - IBM PC/MS-DOS Version 3.2
rem This batch files creates miracl.lib from its component parts
rem using the Microsoft v7.0 (or greater) compiler, the Microsoft
rem Macro assembler and the Microsoft LIB librarian utility
rem Also included are the commands to create some of the example programs
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "msdoit". It is assumed that paths have been correctly set up
rem to the compiler, macro assembler, librarian and linker.
rem
rem Provided mainly as a guide for creating a batch file tailored
rem specifically to your own configuration.
rem
rem Compile MIRACL modules
cl /c mrcore.c
cl /c mrarth0.c
cl /c mrarth1.c
cl /c mrarth2.c
cl /c mrsmall.c
cl /c mrround.c
cl /c mrio1.c
cl /c mrio2.c
cl /c mrgcd.c
cl /c mrjack.c
cl /c mrxgcd.c
cl /c mrarth3.c
cl /c mrrand.c
cl /c mrprime.c
cl /c mrcrt.c
cl /c mrscrt.c
cl /c mrfast.c
cl /c mrfrnd.c
cl /c mrflash.c
cl /c mrdouble.c
cl /c mrbuild.c
cl /c mrflsh1.c
cl /c mrpi.c
cl /c mrflsh2.c
cl /c mrflsh3.c
cl /c mrflsh4.c
rem 
rem Assemble mrmuldv.c
masm mrmuldv.c;                 /* masm/r for 8087 version */
rem
rem Create library 'miracl.lib'
del miracl.lib
lib miracl;
lib miracl +mrflsh4+mrflsh3+mrflsh2+mrpi+mrflsh1;
lib miracl +mrio2+mrio1+mrdouble+mrflash+mrrand+mrprime+mrcrt+mrscrt+mrfast;
lib miracl +mrjack+mrfrnd+mrxgcd+mrgcd+mrround+mrbuild+mrarth3+mrarth2;
lib miracl +mrarth1+mrarth0+mrsmall+mrcore+mrmuldv;
del mr*.obj
rem
cl /c big.cpp
cl /c number.cpp
cl /c flash.cpp
cl /c crt.cpp
rem Compile and link example programs
cl /c brute.cpp
link brute+big,,,miracl;
cl /c genkey.cpp
link genkey+big,,,miracl;
cl /c encode.cpp 
link encode+big,,,miracl;
cl /c decode.cpp 
link decode+big+crt,,,miracl;
cl /c keyids.c
link keyids,,,miracl;
cl /c sample.cpp 
link sample+flash,,,miracl;
cl /c brent.cpp 
link brent+big+number,,,miracl;
cl /c pollard.cpp 
link pollard+big+number,,,miracl;

