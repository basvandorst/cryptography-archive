rem MIRACL - IBM PC/MS-DOS Version 2.0
rem This batch files creates miracl.lib from its component parts
rem using the Turbo C v1.0 (or greater) compiler, the Microsoft MASM
rem macroassembler V3.0, and the Microsoft LIB librarian utility
rem Also included are the commands to create all the example programs
rem using the Turbo TLINK utility
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "tcdoit". It is assumed that paths have been correctly set up to
rem the compiler, librarian and linker.
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
tcc -c bncore
tcc -c bnarth0
tcc -c bnarth1
tcc -c bnarth2
tcc -c bnsmall
tcc -c bnround
tcc -c bnio1
tcc -c bnio2
tcc -c bngcd
tcc -c bnxgcd
tcc -c bnarth3
tcc -c bnrand
tcc -c bnprime
tcc -c bnflash
tcc -c bndouble
tcc -c bnbuild
tcc -c bnflsh1
tcc -c bnpi
tcc -c bnflsh2
tcc -c bnflsh3
tcc -c bnflsh4
rem 
rem Assemble bnmuldv.c
tcc -c -B bnmuldv.c
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
tcc brute miracl.lib
del brute.obj
tcc mersenne miracl.lib
del mersenne.obj
tcc rsakey miracl.lib
del rsakey.obj
tcc encode miracl.lib
del encode.obj
tcc decode miracl.lib
del decode.obj
tcc okakey miracl.lib
del okakey.obj
tcc enciph miracl.lib
del enciph.obj
tcc deciph miracl.lib
del deciph.obj
tcc pi miracl.lib
del pi.obj
tcc sample miracl.lib
del sample.obj
tcc roots miracl.lib
del roots.obj
tcc hilbert miracl.lib
del hilbert.obj
tcc brent miracl.lib
del brent.obj
tcc pollard miracl.lib
del pollard.obj
tcc williams miracl.lib
del williams.obj
tcc lenstra miracl.lib
del lenstra.obj
tcc qsieve miracl.lib
del qsieve.obj
tcc fact miracl.lib
del fact.obj
tcc palin miracl.lib
del palin.obj
tcc hail miracl.lib
del hail.obj
tcc ratcalc miracl.lib
del ratcalc.obj

