	   Pretty Good Privacy version 2.3a - READ ME FIRST
			Notes by Perry Metzger
		    Edited for 2.3a by Colin Plumb


You are looking at the README file for PGP release 2.3a.  PGP, short for
Pretty Good Privacy, is a public key encryption package; with it, you
can secure messages you transmit against unauthorized reading and
digitally sign them so that people receiving them can be sure they
come from you.

The files pgpdoc1.txt and pgpdoc2.txt contain documentation for the
system.  Before using PGP, PLEASE READ THE DOCUMENTATION.  This tends
to get neglected with most computer software, but cryptography software
is easy to misuse, and if you don't use it properly much of the security
you could gain by using it will be lost!

Security is only as strong as the weakest link, and while the algorithms
in PGP are some of the strongest known in the civilian world, there are
things outside the program's control which can weaken your security as
assuredly as forgetting to lock a vault door.  Even if you are already
familiar with public key cryptography, it is important that you
understand the various security issues associated with using PGP. 

PGP 2.3a, released July 2, 1993 fixes some glitches which made their way
into PGP 2.3, released June 15, 1993.  It will likely be followed by
more updated versions in the months to come, so check around for more
recent updates, especially if you received PGP 2.3a substantially after
the release date.  If there is a more recent release, please acquire it,
and please get the place from which you got PGP 2.3a or an earlier
version to update their release, too.

There are four archives in the PGP 2.3a release.  You will usually only
need one of them.  They are:
- pgp23A.zip	This is the MS-DOS executable release, which includes
		the executable, support files, and basic documentation.
- pgp23srcA.zip	This is a source code release, which includes all the source
		code needed to compile PGP and examples of usage.  This
		contains everything in pgp23A.zip except that the manual is
		not paginated and it does not contain an executable nor an
		associated signature.
- pgp23A.tar.Z	This contains exactly the same files as pgp23srcA.zip, except
		that they use Unix rather than MS-DOS line end conventions.
- pgp23docA.zip	This is the documentation for PGP only.  This can be freely
		exported and is useful to tell people what PGP does.

While we welcome ports to other platforms, if you make your own archive
for distribution, PLEASE INCLUDE THE MANUAL.  It covers important
security and legal issues which a new user must know.
		
Assuming you have a code (non-documentation) release, the file SETUP.DOC
contains information on how to install PGP on your system; this document
is broken up into several sections, each dealing with a different
operating system: PGP is known to run on MS-DOS, UNIX, VMS and OS/2.  Part
of the information in SETUP.DOC might make more sense if you have already
read the manuals.

PGP is distributed under the terms of the GNU General Public Licence,
a copy of which is included.  In brief, this states that PGP is freely
distributable, subject only to the condition that if you make a modified
version and choose to distribute it, you must make it freely distributable
as well.  See the file COPYING for details.

This PGP 2.3 release has several bug fixes over PGP 2.2, and a few
new (although somewhat esoteric) features.  See doc\newfor23.doc
for details.  The most important fix is to compression on MS-DOS.
an invlaid pointer was bring freed, causing unpredictable behaviour.
Sometimes, no harm would reault.  Sometimes. the machine would crash.

MANIFEST for PGP 2.3a MSDOS executable release
---------------------------------------------

Here is a list of files included in the PGP 2.3a MSDOS executable release
file PGP23A.ZIP...

README.DOC	- This file you are reading
BINFILES	- The master list of files, used to generate the archive
PGP.EXE  	- PGP executable program
CONFIG.TXT 	- User configuration parameter file for PGP
LANGUAGE.TXT 	- Sample language file for French and Spanish
PGP.HLP  	- Online help file for PGP
ES.HLP  	- Online help file in Spanish
FR.HLP  	- Online help file in French
KEYS.ASC	- Sample public keys you should add to your keyring
PGPSIG.ASC 	- Detached signature of PGP.EXE, to detect viruses
DOC\SETUP.DOC	- Installation guide
DOC\PGPDOC1.DOC	- PGP User's Guide, Vol I: Essential Topics
DOC\PGPDOC2.DOC	- PGP User's Guide, Vol II: Special Topics
DOC\COPYING	- GNU General Public Licence
DOC\BLURB.TXT	- Brief description of PGP, for BBS indexes
DOC\POLITIC.DOC	- Computer-related political groups


For Clinical Paranoia Sufferers Only
------------------------------------

It is always possible that the PGP you have received has been tampered
with in some way.  This is a risk because PGP is used as a system to
assure security, so those wishing to breach your security could likely
do it by making sure that your copy of PGP has been tampered with.  Of
course, if you receive PGP in a binary distribution, it makes sense to
check it for viruses, and if you receive PGP as source code, looking
for signs of obvious tampering might be a good idea.  However, it is
very difficult to actually determine if the code has no subtle bugs
that have been introduced and that the executable you are using has
not been tampered with in any way.

If you have a previous version of PGP which you already trust, the
cryptographic signature on the executable will assure you that it has
not been tampered with (with the possible exception of a "stealth virus"
already existing on your system).  If you are a really paranoid person,
try getting a cryptographically signed copy of the software from someone
you trust to have a good copy.  It would also likely be good for you to
pay special attention to the sections of the manual on "Vulnerabilities."  
You are going to read the manual, aren't you?
