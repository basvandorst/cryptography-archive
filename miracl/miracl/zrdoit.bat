rem MIRACL - IBM PC/MS-DOS Version 2.0
rem This batch files creates miracl.lib from its component parts
rem using the Zorland v1.1 compiler, the Microsoft  MASM
rem Macro assembler and the Microsoft LIB librarian utility
rem (ZORLIB could be used instead)
rem Also included are the commands to create all the example programs
rem using the Microsoft LINK utility
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "zrdoit". It is assumed that paths have been correctly set up to
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
zc bncore -c
zc bnarth0 -c
zc bnarth1 -c
zc bnarth2 -c
zc bnsmall -c
zc bnround -c
zc bnio1 -c
zc bnio2 -c
zc bngcd -c
zc bnxgcd -c
zc bnarth3 -c
zc bnrand -c
zc bnprime -c
zc bnflash -c
zc bndouble -c
zc bnbuild -c
zc bnflsh1 -c
zc bnpi -c
zc bnflsh2 -c
zc bnflsh3 -c
zc bnflsh4 -c
rem Assemble bnmuldv.c
masm bnmuldv.c;
rem Create library 'miracl.lib'
del miracl.lib
lib miracl;
lib miracl +bnflsh4+bnflsh3+bnflsh2+bnpi+bnflsh1;
lib miracl +bnio2+bnio1+bndouble+bnflash+bnrand+bnprime;
lib miracl +bnxgcd+bngcd+bnround+bnbuild+bnarth3+bnarth2+bnarth1+bnarth0;
lib miracl +bnsmall+bncore+bnmuldv;
del bn*.obj
rem Compile and Link example programs
zc brute -c
link c+brute,brute,brute,miracl nl;
del brute.obj
zc mersenne -c
link c+mersenne,mersenne,mersenne,miracl nl;
del mersenne.obj
zc rsakey -c
link c+rsakey,rsakey,rsakey,miracl nl;
del rsakey.obj
zc encode -c
link c+encode,encode,encode,miracl nl;
del encode.obj
zc decode -c
link c+decode,decode,decode,miracl nl;
del decode.obj
zc okakey -c
link c+okakey,decode,decode,miracl nl;
del okakey.obj
zc enciph -c
link c+enciph,enciph,enciph,miracl nl;
del enciph.obj
zc deciph -c
link c+deciph,deciph,deciph,miracl nl;
del deciph.obj
zc pi -c
link c+pi,pi,pi,miracl nl;
del pi.obj
zc sample -c
link c+sample,sample,sample,miracl nl;
del sample.obj
zc roots -c
link c+roots,roots,roots,miracl nl;
del roots.obj
zc hilbert -c
link c+hilbert,hilbert,hilbert,miracl nl;
del hilbert.obj
zc pollard -c
link c+pollard,pollard,pollard,miracl nl;
del pollard.obj
zc williams -c
link c+williams,williams,williams,miracl nl;
del williams.obj
zc lenstra -c
link c+lenstra,lenstra,lenstra,miracl nl;
del lenstra.obj
zc qsieve -c
link c+qsieve,qsieve,qsieve,miracl nl;
del qsieve.obj
zc brent -c
link c+brent,brent,brent,miracl nl;
del brent.obj
zc fact -c
link c+fact,fact,fact,miracl nl;
del fact.obj
zc palin -c
link c+palin,palin,palin,miracl nl;
del palin.obj
zc hail -c
link c+hail,hail,hail,miracl nl;
del hail.obj
del *.map
