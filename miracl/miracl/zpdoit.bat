rem MIRACL - IBM PC/MS-DOS Version 2.0
rem This batch files creates miracl.lib from its component parts
rem using the Zortech v1.0 (or greater) C++ compiler, the Microsoft
rem Macro assembler and the Microsoft LIB librarian utility
rem IMPORTANT NOTE: Set path to use Microsoft LINK (Its more reliable)
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "zpdoit". It is assumed that paths have been correctly set up to
rem the compiler, macro assembler, librarian and linker.
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
ztc bncore -c 
ztc bnarth0 -c
ztc bnarth1 -c
ztc bnarth2 -c
ztc bnsmall -c
ztc bnround -c
ztc bnio1 -c  
ztc bnio2 -c  
ztc bngcd -c  
ztc bnxgcd -c 
ztc bnarth3 -c
ztc bnrand -c 
ztc bnprime -c
ztc bnflash -c
ztc bndouble -c
ztc bnbuild -c 
ztc bnflsh1 -c 
ztc bnpi -c 
ztc bnflsh2 -c
ztc bnflsh3 -c
ztc bnflsh4 -c
rem Assemble bnmuldv.c
masm /mx /z /r /di8086s bnmuldv.c;
rem Create library 'miracl.lib'
del miracl.lib
lib miracl;
lib miracl +bnflsh4+bnflsh3+bnflsh2+bnpi+bnflsh1;
lib miracl +bnio2+bnio1+bndouble+bnflash+bnrand+bnprime;
lib miracl +bnxgcd+bngcd+bnround+bnbuild+bnarth3+bnarth2+bnarth1+bnarth0;
lib miracl +bnsmall+bncore+bnmuldv;
del bn*.obj
rem Compile and Link example programs
ztc -B fact miracl.lib
del fact.obj
ztc -B hail miracl.lib
del hail.obj
ztc -B brent miracl.lib
del brent.obj
ztc -B sample miracl.lib
del sample.obj
ztc -B pollard miracl.lib
del pollard.obj
del *.map

