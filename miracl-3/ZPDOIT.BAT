rem MIRACL - IBM PC/MS-DOS Version 3.1
rem This batch files creates miracl.lib from its component parts
rem using the Zortech v1.0 (or greater) C++ compiler, the Microsoft
rem Macro assembler and the Microsoft LIB librarian utility
rem Also - rename Zortech header stream.hpp to stream.hxx 
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "zpdoit". It is assumed that paths have been correctly set up to
rem the compiler, macro assembler, librarian and linker.
rem
rem Compile MIRACL modules
ztc mrcore -o -c 
ztc mrarth0 -o -c
ztc mrarth1 -o -c
ztc mrarth2 -o -c
ztc mrsmall -o -c
ztc mrround -o -c
ztc mrio1 -o -c  
ztc mrio2 -o -c  
ztc mrjack -o -c
ztc mrgcd -o -c  
ztc mrxgcd -o -c 
ztc mrarth3 -o -c
ztc mrrand -o -c 
ztc mrprime -o -c
ztc mrcrt -o -c
ztc mrscrt -o -c
ztc mrfast -o -c
ztc mrfrnd -o -c
ztc mrflash -o -c
ztc mrdouble -o -c
ztc mrbuild -o -c 
ztc mrflsh1 -o -c 
ztc mrpi -o -c 
ztc mrflsh2 -o -c
ztc mrflsh3 -o -c
ztc mrflsh4 -o -c
rem Assemble mrmuldv.c
masm /mx /z /r /di8086s mrmuldv.c;
rem Create library 'miracl.lib'
del miracl.lib
lib miracl;
lib miracl +mrflsh4+mrflsh3+mrflsh2+mrpi+mrflsh1;
lib miracl +mrio2+mrio1+mrdouble+mrflash+mrrand+mrprime+mrcrt+mrscrt+mrfast;
lib miracl +mrjack+mrfrnd+mrxgcd+mrgcd+mrround+mrbuild+mrarth3+mrarth2;
lib miracl +mrarth1+mrarth0+mrsmall+mrcore+mrmuldv;
del mr*.obj
rem Compile and Link C++ versions of example programs where possible
ztc -o big
ztc -o flash
ztc -o number
ztc -o fact big.obj miracl.lib
ztc -o hail big.obj miracl.lib
ztc -o brute big.obj miracl.lib
ztc -o brent big.obj miracl.lib
ztc -o sample flash.obj miracl.lib
ztc -o pollard number.obj big.obj miracl.lib
ztc -o williams number.obj big.obj miracl.lib
ztc -o lenstra number.obj big.obj miracl.lib
ztc -o qsieve big.obj miracl.lib
ztc -o mersenne big.obj miracl.lib
ztc -o palin.c miracl.lib
ztc -o genkey.c miracl.lib
ztc -o encode.c miracl.lib
ztc -o decode.c miracl.lib
ztc -o enciph.c miracl.lib
ztc -o deciph.c miracl.lib
ztc -o keyids.c miracl.lib
ztc -o ratcalc.c miracl.lib


