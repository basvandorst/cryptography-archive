rem MIRACL - IBM PC/MS-DOS Version 2.0
rem This batch files creates miracl.lib from its component parts
rem using the Power C v1.02 (or greater) compiler, and the Power C MERGE
rem librarian. Contributed by Dr. K.J. Vines of Altrincham, Cheshire, England.
rem Also included are the commands to create all the example programs
rem using the Power C PCL link utility.
rem
rem Read your compiler documentation for further information
rem 
rem Invoke as "pcdoit". It is assumed that paths have been correctly set up to
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
pc/fa bncore
pc/fa bnarth0
pc/fa bnarth1
pc/fa bnarth2
pc/fa bnsmall
pc/fa bnround
pc/fa bnio1
pc/fa bnio2
pc/fa bngcd
pc/fa bnxgcd
pc/fa bnarth3
pc/fa bnrand
pc/fa bnprime
pc/fa bnflash
pc/fa bndouble
pc/fa bnbuild
pc/fa bnflsh1
pc/fa bnpi
pc/fa bnflsh2
pc/fa bnflsh3
pc/fa bnflsh4
rem 
rem This implementation uses C version of bnmuldv.c
pc/fa bnmuldv.c
rem
rem Create library 'miracl.lib'
del miracl.lib
merge miracl  bnflsh4 bnflsh3 bnflsh2 bnpi bnflsh1
merge miracl  bnio2 bnio1 bndouble
merge miracl  bnflash bnrand bnprime
merge miracl  bnxgcd bngcd bnround bnbuild
merge miracl  bnarth3 bnarth2 bnarth1 bnarth0
merge miracl  bnsmall bncore bnmuldv
del bn*.mix
rem
rem Compile and link example programs
pc/fa brute
pcl brute.mix ;miracl
del brute.mix
pc/fa mersenne
pcl mersenne.mix ;miracl
del mersenne.mix
pc/fa rsakey
pcl rsakey.mix ;miracl
del rsakey.mix
pc/fa encode
pcl encode.mix ;miracl
del encode.mix
pc/fa decode
pcl decode.mix ;miracl
del decode.mix
pc/fa okakey
pcl okakey.mix ;miracl
del okakey.mix
pc/fa enciph
pcl enciph.mix ;miracl
del enciph.mix
pc/fa deciph
pcl deciph.mix ;miracl
del deciph.mix
pc/fa pi
pcl pi.mix ;miracl
del pi.mix
pc/fa sample
pcl sample.mix ;miracl
del sample.mix
pc/fa roots
pcl roots.mix ;miracl
del roots.mix
pc/fa hilbert
pcl hilbert.mix ;miracl
del hilbert.mix
pc/fa brent
pcl brent.mix ;miracl
del brent.mix
pc/fa pollard
pcl pollard.mix ;miracl
del pollard.mix
pc/fa williams
pcl williams.mix ;miracl
del williams.mix
pc/fa lenstra
pcl lenstra.mix ;miracl
del lenstra.mix
pc/fa qsieve
pcl qsieve.mix ;miracl
del qsieve.mix
pc/fa fact
pcl fact.mix ;miracl
del fact.mix
pc/fa palin
pcl palin.mix ;miracl
del palin.mix
pc/fa hail
pcl hail.mix ;miracl
del hail.mix
pc/fa ratcalc
pcl ratcalc.mix ;miracl
del ratcalc.mix

