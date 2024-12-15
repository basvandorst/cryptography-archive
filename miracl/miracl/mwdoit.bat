rem MIRACL - IBM PC/MS-DOS Version 2.0
rem This batch files creates miracl.lib from its component parts
rem using the Mark Williams v2.0 (or greater) compiler, the Mark Williams
rem as assembler and lb librarian utility
rem Also included are the commands to create all the example programs
rem using the Mark Williams ld utility
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "mwdoit". It is assumed that paths have been correctly set up to
rem the compiler, assembler, librarian and linker.
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
cc bncore.c -c
cc bnarth0.c -c
cc bnarth1.c -c
cc bnarth2.c -c
cc bnsmall.c -c
cc bnround.c -c
cc bnio1.c -c
cc bnio2.c -c
cc bngcd.c -c
cc bnxgcd.c -c
cc bnarth3.c -c
cc bnrand.c -c
cc bnprime.c -c
cc bnflash.c -c
cc bndouble.c -c
cc bnbuild.c -c
cc bnflsh1.c -c
cc bnpi.c -c
cc bnflsh2.c -c
cc bnflsh3.c -c
cc bnflsh4.c -c
rem
rem Assemble bnmuldv.c
as -o bnmuldv.o bnmuldv.c
rem
rem Create library 'miracl.lib'
del miracl.olb
lb c  miracl.olb
lb r  miracl.olb bnflsh4.o bnflsh3.o bnflsh2.o bnpi.o bnflsh1.o
lb r  miracl.olb bncore.o bnarth0.o bnarth1.o bnarth2.o bnround.o bnio1.o
lb r  miracl.olb bnio2.o bngcd.o bnxgcd.o bnarth3.o bnbuild.o bnsmall.o
lb r  miracl.olb bnrand.o bnprime.o bnflash.o bndouble.o bnmuldv.o
del bn*.o
rem
rem Compile and link example programs
cc brute.c miracl.olb
del brute.o
cc mersenne.c miracl.olb
del mersenne.o
cc rsakey.c miracl.olb
del rsakey.o
cc encode.c miracl.olb
del encode.o
cc decode.c miracl.olb
del decode.o
cc okakey.c miracl.olb
del okakey.o
cc enciph.c miracl.olb
del enciph.o
cc deciph.c miracl.olb
del deciph.o
cc pi.c miracl.olb -lm
del pi.o
cc sample.c miracl.olb -lm
del sample.o
cc roots.c miracl.olb -lm
del roots.o
cc hilbert.c miracl.olb
del hilbert.o
cc brent.c miracl.olb
del brent.o
cc pollard.c miracl.olb
del pollard.o
cc williams.c miracl.olb
del williams.o
cc lenstra.c miracl.olb
del lenstra.o
cc qsieve.c miracl.olb
del qsieve.o
cc fact.c miracl.olb
del fact.o
cc palin.c miracl.olb
del palin.o
cc hail.c miracl.olb
del hail.o

