rem MIRACL - IBM PC/MS-DOS Version 3.2
rem This batch files creates miracl.lib from its component parts
rem using the Watcom v7.0 (or greater) compiler, the Pharlap
rem 386ASM assembler and the Pharlap 386LIB librarian utility
rem Also included are the commands to create some of the example programs
rem using the Phar Lap 386LINK utility
rem Run protected mode programs using RUN386 utility
rem Read your documentation for further information
rem NOTE: Use large code generator, 386wcgl.exe
rem
rem Take care to use the correct mirdef.h32. Watcom C does not differentiate
rem between #include <mirdef.h> and #include "mirdef.h" (which it should)
rem It will use a mirdef.h from the current directory if it finds it.
rem
rem Invoke as "wcdoit". It is assumed that paths have been correctly set up
rem to the compiler, assembler, librarian and linker.
rem
rem Compile MIRACL modules
rem use large code generator
rem set wcg386=\wcpath\bin\386wcgl.exe
wcl386 /c /s /oaxt mrcore.c
wcl386 /c /s /oaxt mrarth0.c
wcl386 /c /s /oaxt mrarth1.c
wcl386 /c /s /oaxt mrarth2.c
wcl386 /c /s /oaxt mrsmall.c
wcl386 /c /s /oaxt mrround.c
wcl386 /c /s /oaxt mrio1.c
wcl386 /c /s /oaxt mrio2.c
wcl386 /c /s /oaxt mrgcd.c
wcl386 /c /s /oaxt mrjack.c
wcl386 /c /s /oaxt mrxgcd.c
wcl386 /c /s /oaxt mrarth3.c
wcl386 /c /s /oaxt mrrand.c
wcl386 /c /s /oaxt mrprime.c
wcl386 /c /s /oaxt mrcrt.c
wcl386 /c /c /oaxt mrscrt.c
wcl386 /c /s /oaxt mrfast.c
wcl386 /c /s /oaxt mrfrnd.c
wcl386 /c /s /oaxt mrflash.c
wcl386 /c /s /oaxt /fpc mrdouble.c
wcl386 /c /s /oaxt mrbuild.c
wcl386 /c /s /oaxt /fpc mrflsh1.c
wcl386 /c /s /oaxt mrpi.c
wcl386 /c /s /oaxt /fpc mrflsh2.c
wcl386 /c /s /oaxt /fpc mrflsh3.c
wcl386 /c /s /oaxt /fpc mrflsh4.c
rem 
rem Assemble mrmuldv.c
386asm mrmuldv.c   
rem
rem Create library 'miracl.lib'
del miracl.lib
386lib miracl -c mrflsh4 mrflsh3 mrflsh2 mrpi mrflsh1 mrio2
386lib miracl -a mrio1 mrdouble mrflash mrrand mrprime mrcrt mrscrt mrfast  
386lib miracl -a mrjack mrfrnd mrxgcd mrgcd mrround mrbuild mrarth3 mrarth2 
386lib miracl -a mrarth1 mrarth0 mrsmall mrcore mrmuldv 
del mr*.obj
rem
rem Compile and link example programs
wcl386 /oaxt mersenne.c miracl.lib
wcl386 /oaxt genkey.c miracl.lib
wcl386 /oaxt encode.c miracl.lib 
wcl386 /oaxt decode.c miracl.lib 
wcl386 /oaxt /fpc sample.c miracl.lib 
wcl386 /oaxt brent.c miracl.lib 
wcl386 /oaxt pollard.c miracl.lib
wcl386 /oaxt fact.c miracl.lib 
wcl386 /oaxt /fpc ratcalc.c miracl.lib

