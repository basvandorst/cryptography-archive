rem MIRACL - IBM PC/MS-DOS Version 2.0
rem This batch files creates miracl.lib from its component parts
rem using the Microsoft v5.0 (or greater) compiler, the Microsoft
rem Macro assembler and the Microsoft LIB librarian utility
rem Also included are the commands to create all the example programs
rem using the Microsoft LINK utility
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "ms5doit". It is assumed that paths have been correctly set up
rem to the compiler, macro assembler, librarian and linker.
rem
rem Note - use with care. There is insufficient space for all of the files
rem created by this batch file on the distribution diskette alone.
rem
rem Provided mainly as a guide for creating a batch file tailored
rem specifically to your own configuration.
rem
rem On a diskette based system these commands should be executed manually,
rem changing diskettes as necessary
rem
rem Compile MIRACL modules
cl /c bncore.c
cl /c bnarth0.c
cl /c bnarth1.c
cl /c bnarth2.c
cl /c bnsmall.c
cl /c bnround.c
cl /c bnio1.c
cl /c bnio2.c
cl /c bngcd.c
cl /c bnxgcd.c
cl /c bnarth3.c
cl /c bnrand.c
cl /c bnprime.c
cl /c bnflash.c
cl /c bndouble.c
cl /c bnbuild.c
cl /c bnflsh1.c
cl /c bnpi.c
cl /c bnflsh2.c
cl /c bnflsh3.c
cl /c bnflsh4.c
rem 
rem Assemble bnmuldv.c
masm bnmuldv.c;                 /* masm/r bnmuldv.c for 8087 version */
rem
rem Create library 'miracl.lib'
del miracl.lib
lib miracl;
lib miracl +bnflsh4+bnflsh3+bnflsh2+bnpi+bnflsh1;
lib miracl +bnio2+bnio1+bndouble+bnflash+bnrand+bnprime;
lib miracl +bnxgcd+bngcd+bnround+bnbuild+bnarth3+bnarth2+bnarth1+bnarth0;
lib miracl +bnsmall+bncore+bnmuldv;
del bn*.obj
rem
rem Compile and link example programs
cl /c brute.c
link brute,,,miracl;
del brute.obj
cl /c mersenne.c
link mersenne,,,miracl;
del mersenne.obj
cl /c rsakey.c
link rsakey,,,miracl;
del rsakey.obj
cl /c encode.c 
link encode,,,miracl;
del encode.obj
cl /c decode.c 
link decode,,,miracl;
del decode.obj
cl /c okakey.c 
link okakey,,,miracl;
del okakey.obj
cl /c enciph.c 
link enciph,,,miracl;
del enciph.obj
cl /c deciph.c 
link deciph,,,miracl;
del deciph.obj
cl /c pi.c 
link pi,,,miracl;
del pi.obj
cl /c sample.c 
link sample,,,miracl;
del sample.obj
cl /c roots.c 
link roots,,,miracl;
del roots.obj
cl /c hilbert.c 
link hilbert,,,miracl;
del hilbert.obj
cl /c brent.c 
link brent,,,miracl;
del brent.obj
cl /c pollard.c 
link pollard,,,miracl;
del pollard.obj
cl /c williams.c
link williams,,,miracl;
del williams.obj
cl /c lenstra.c
link lenstra,,,miracl;
del lenstra.obj
cl /c qsieve.c
link qsieve,,,miracl;
del qsieve.obj
cl /c fact.c 
link fact,,,miracl;
del fact.obj
cl /c palin.c 
link palin,,,miracl;
del palin.obj
cl /c hail.c 
link hail,,,miracl;
del hail.obj
cl /c ratcalc.c 
link ratcalc,,,miracl;
del ratcalc.obj
del *.map

