rem MIRACL - IBM PC/MS-DOS Version 2.0
rem This batch files creates miracl.lib from its component parts
rem using the Microsoft v3.0/v4.0 compiler, the Microsoft
rem Macro assembler and the Microsoft LIB librarian utility
rem Also included are the commands to create all the example programs
rem using the Microsoft LINK utility
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "msdoit". It is assumed that paths have been correctly set up to
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
msc bncore;
msc bnarth0;
msc bnarth1;
msc bnarth2;
msc bnsmall;
msc bnround;
msc bnio1;
msc bnio2;
msc bngcd;
msc bnxgcd;
msc bnarth3;
msc bnrand;
msc bnprime;
msc bnflash;
msc bndouble;
msc bnbuild;
msc bnflsh1;
msc bnpi;
msc bnflsh2;
msc bnflsh3;
msc bnflsh4;
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
msc brute;
link brute,,,miracl;
del brute.obj
msc mersenne;
link mersenne,,,miracl;
del mersenne.obj
msc rsakey;
link rsakey,,,miracl;
del rsakey.obj
msc encode;
link encode,,,miracl;
del encode.obj
msc decode;
link decode,,,miracl;
del decode.obj
msc okakey;
link okakey,,,miracl;
del okakey.obj
msc enciph;
link enciph,,,miracl;
del enciph.obj
msc deciph;
link deciph,,,miracl;
del deciph.obj
msc pi;
link pi,,,miracl;
del pi.obj
msc sample;
link sample,,,miracl;
del sample.obj
msc roots;
link roots,,,miracl;
del roots.obj
msc hilbert;
link hilbert,,,miracl;
del hilbert.obj
msc brent;
link brent,,,miracl;
del brent.obj
msc pollard;
link pollard,,,miracl;
del pollard.obj
msc williams;
link williams,,,miracl;
del williams.obj
msc lenstra;
link lenstra,,,miracl;
del lenstra.obj
msc qsieve;
link qsieve,,,miracl;
del qsieve.obj
msc fact;
link fact,,,miracl;
del fact.obj
msc palin;
link palin,,,miracl;
del palin.obj
msc hail;
link hail,,,miracl;
del hail.obj
msc ratcalc;
link ratcalc,,,miracl;
del ratcalc.obj
del *.map

