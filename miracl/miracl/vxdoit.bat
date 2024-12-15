$ ! MIRACL - VAX VMS Version 2.0
$ ! This batch file creates miracl.olb from its component parts
$ ! using the Digital C Compiler Version 3.4, the Digital
$ ! assembler and library utility
$ ! Also included are the commands to create all the example programs
$ ! using the Digital linker
$ ! 
$ ! Read your compiler documentation for further information
$ ! 
$ ! Invoke as "@vxdoit". It is assumed that the compiler, assembler, librarian
$ ! and linker are all accessible, and that the linker will search the standard
$ ! C libraries
$ !
$ ! Provided mainly as a guide for creating a batch file tailored
$ ! specifically to your own configuration.
$ ! 
$ ! 
$ ! compile MIRACL modules
$ cc bncore
$ cc bnsmall
$ cc bnarth0
$ cc bnarth1
$ cc bnarth2
$ cc bnround
$ cc bnio1
$ cc bnio2
$ cc bngcd
$ cc bnxgcd
$ cc bnarth3
$ cc bnrand
$ cc bnprime
$ cc bnflash
$ cc bndouble
$ cc bnbuild
$ cc bnflsh1
$ cc bnpi
$ cc bnflsh2
$ cc bnflsh3
$ cc bnflsh4
$ ! Assemble bnmuldv.c
$ macro bnmuldv.c
$ ! create MIRACL library 'miracl.olb'
$ !
$ del miracl.olb;*
$ lib/create miracl bnflsh4
$ lib/insert miracl bnflsh3,bnflsh2,bnpi,bnflsh1,bndouble,bnflash,bnsmall
$ lib/insert miracl bnprime,bnrand,bnarth3,bnxgcd,bngcd,bnio1,bnio2,bnround
$ lib/insert miracl bnbuild,bnarth2,bnarth1,bnsmall,bnmuldv,bnarth0,bncore
$ del bn*.obj;*
$ del bn*.lis;*
$ !
$ ! compile and link example programs
$ cc pollard
$ link pollard,miracl/lib
$ del pollard.obj;*
$ cc williams
$ link williams,miracl/lib
$ del williams.obj;*
$ cc lenstra
$ link lenstra,miracl/lib
$ del lenstra.obj;*
$ cc qsieve
$ link qsieve,miracl/lib
$ del qsieve.obj;*
$ cc brent
$ link brent,miracl/lib
$ del brent.obj;*
$ cc fact
$ link fact,miracl/lib
$ del fact.obj;*
$ cc palin
$ link palin,miracl/lib
$ del palin.obj;*
$ cc hail
$ link hail,miracl/lib
$ del hail.obj;*
$ cc brute
$ link brute,miracl/lib
$ del brute.obj;*
$ cc mersenne
$ link mersenne,miracl/lib
$ del mersenne.obj;*
$ cc rsakey
$ link rsakey,miracl/lib
$ del rsakey.obj;*
$ cc encode
$ link encode,miracl/lib
$ del encode.obj;*
$ cc decode
$ link decode,miracl/lib
$ del decode.obj;*
$ cc okakey
$ link okakey,miracl/lib
$ del okakey.obj;*
$ cc enciph
$ link enciph,miracl/lib
$ del enciph.obj;*
$ cc deciph
$ link deciph,miracl/lib
$ del deciph.obj;*
$ cc roots
$ link roots,miracl/lib
$ del roots.obj;*
$ cc hilbert
$ link hilbert,miracl/lib
$ del hilbert.obj;*
$ cc sample
$ link sample,miracl/lib
$ del sample.obj;*
$ cc calc
$ link calc,miracl/lib
$ del calc.obj;*
$ del *.map;*
$ exit

