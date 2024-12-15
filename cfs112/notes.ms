.TL
CFS Version 1.1
Installation and Operation
.AU
Matt Blaze
.AI
AT&T Bell Laboratories
101 Crawfords Corner Road, Room 4G-634
Holmdel, NJ 07733

mab@research.att.com
(for cfs questions, use cfs@research.att.com)

.NH
General
.PP
CFS pushes encryption services into the Unix(tm) file system.  It
supports secure storage at the system level through a standard Unix
file system interface to encrypted files.  Users associate a
cryptographic key with the directories they wish to protect.  Files in
these directories (as well as their pathname components) are
transparently encrypted and decrypted with the specified key without
further user intervention; cleartext is never stored on a disk or sent
to a remote file server.  CFS employs a novel combination of DES
stream and codebook cipher modes to provide high security with good
performance on a modern workstation.  CFS can use any available file
system for its underlying storage without modification, including
remote file servers such as NFS.  System management functions, such as
file backup, work in a normal manner and without knowledge of the key.
.PP
CFS as distributed runs under SunOS and, with a little coaxing,
several other BSD-derived systems including BSD/386.  It has also
been ported by users to Solaris, Ultrix, Linux, and several other popular
OSs, but is unlikely to work on such systems "out of the box".
"#ifdefs" for most of these systems are included in the distribution;
see the Makefile for compilation details.  User-contributed patches for
porting CFS to platforms not included in the distribution are made available
in the CFS-USERS mailing list archive.  You should check the archive
before undertaking a major porting effort.  (Of course, I encourage you
to share any ports, patches or enhancements you develop.)
.PP
CFS runs entirely at user level, as a local NFS server running on the
client machine's "loopback" interface.  The system consists of
.B
cfsd
.R
(the CFS server daemon), and a small suite of tools
.B
(cmkdir, cattach, cdetach,
.R
and
.B
ssh)
.R
that create encrypted directories and manage keys as they are used.
CFS, and the motivation and principles behind it, is described in
detail in:
.IP
Matt Blaze, "A Cryptographic File System for Unix."
.I
Proc. 1st ACM
Conference on Computer and Communications Security,
.R
Fairfax, VA, November 1993.
.PP
This paper is available for anonymous ftp from research.att.com, in
the file /dist/mab/cfs.ps.  You should read this paper before
attempting to install and use CFS.  Details on the usage of each of
the CFS commands (cattach, etc.) can be found in the man pages
included in this distribution.  You can print them with troff -man,
and should install them wherever local manpages go on your system.
.PP
Basically, CFS provides a mechanism to associate "real" directories
(on other file systems) that contain encrypted data with temporary
"virtual" names through which users can read and write cleartext.
These virtual names appear under the CFS mount point, which is usually
called /crypt (this document assumes that convention).  Users create
encrypted directories on regular file systems (e.g., in their home
directories) using the
.B cmkdir
command, which creates the directory and assigns to it a cryptographic
"passphrase" which will be used to encrypt its contents.  To actually
use an encrypted directory, it must be "attached" to CFS using the
.B cattach
command, which asks for the passphrase and installs an association
between the "real" directory and a temporary name under /crypt.
Cleartext is read and written under the virtual directory in /crypt,
but the files are stored in encrypted form (with encrypted names) in
the real directory.  When the directory is not in use, the association
is removed with the
.B cdetach
command, which deletes the cleartext virtual directory under /crypt
(but not the ciphertext files, of course).  When CFS is run on a
client workstation, the cleartext data (and the cryptographic key
passphrase) are never stored on a disk or sent over a network, even
when the real directory is located on a remote file server.
(Actually, the virtual memory system can theoretically violate this
rule; see the "Internals" section, below.)
.PP
CFS is implemented as a server, called
.B cfsd,
for the Sun Network File System (NFS) protocol plus extensions for
associating keys with directories.  cfsd monitors the localhost
virtual network interface for remote procedure call requests from the
local machine.  Once the local machine invokes an NFS "mount" on the
localhost interface for the CFS mount point (/crypt), cfsd handles
file system operations for the mounted file system as if it were a
remote file server.  Initially, /crypt appears completely empty.  The
user interface programs (such as the
.B cattach
command) send RPCs to cfsd giving the information required to manage
the attached virtual directories that appear under /crypt (e.g., what
name to use, what key to use, the name of the directory on the real
file system, etc.).
.NH 2
License
.IP
.ce 9999
Copyright (c) 1992, 1993, 1994 by AT&T.
.ce 0
.IP
Permission to use, copy, and modify this software without fee
is hereby granted, provided that this entire notice is included in
all copies of any software which is or includes a copy or
modification of this software and in all copies of the supporting
documentation for such software.
.IP
This software is subject to United States export controls.  You may
not export it, in whole or in part, or cause or allow such export,
through act or omission, without prior authorization from the United
States government and written permission from AT&T.  In particular,
you may not make any part of this software available for general or
unrestricted distribution to others, nor may you disclose this software
to persons other than citizens and permanent residents of the United
States and Canada. 
.IP
THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
.PP
Please abide by the restrictions in the above license agreement.  The
limitations on distribution are there to meet legal requirements that
are taken quite seriously by the government and by AT&T's lawyers.  If
you violate them, you risk creating all sorts of unpleasantness for
yourself and for others (e.g., me).  If the export requirements are
not generally abided by, it will also make it increasingly difficult
to release future such efforts.
.PP
It would be a good idea to check with me before including CFS as part
of some other software distribution.

.NH
Installation
.PP
CFS was developed under SunOS 4.1.2 and BSD/386 (BSDI).  I have not
tested it, or even compiled it, on
.I any
other platforms.  If you are interested in porting it to something
different, such as SYSV or whatever, you should first
get it running under one of the known configurations so that you know
what to expect and are sure you have the complete system.  You should
also read and understand the "Internals" section below.  The rest of
this section assumes you are using one of the known systems.  I'm afraid
we do not have the resources to help you install or port CFS; you're
on your own.
.PP
The system is designed to be installed on individual single-user
workstations.  You really should not install (or use) it on a shared
file or compute server, even though such a configuration is
technically possible.  There are three main reasons for this.  First,
CFS trades off encryption speed for memory by precomputing large
stream ciphers for each attached directory.  While this is usually
fine for a small number of attaches, more than four or five at a time
can quickly reduce a system to thrashing.  Furthermore, cfsd is
single-threaded, and therefore does not handle many concurrent I/O
operations very well.  Second, any time an attach is active, an
attacker who can log in to the target system and spoof either the UID
of the legitimate user or "root" can potentially compromise the
cleartext or learn the key by examining the address space of the cfsd
process.  Finally, if the connection between the user and the machine
running CFS is compromised (e.g., by watching the Ethernet traffic
between a terminal server and the host), an attacker can potentially
observe the the entire dialog between the user and CFS, including any
passphrases given to cattach and any cleartext written to /crypt.  In
fact, most successful attacks on cryptographically strong systems do
not attack the encryption scheme at all, but instead rely on poorly
managed installation, key management, and usage protocols.  Be sure
your users understand what parts of the system they are trusting
before CFS is used to protect sensitive data.
.PP
CFS is a user-level NFS server.  It does not, however, include the
.I mount
protocol, so you'll also need to have /etc/mountd (sometimes called
/usr/etc/rpc.mountd) installed on your system.  You will need the
ability to become "root" on the target machine.
.PP
To install, first edit the Makefile for your local configuration.
Everything you should have to edit is toward the beginning of the
file.  Make sure BINDIR and ETCDIR are set to the correct directories.
Check the LIBS and COMPAT variables; for SunOS, these should be empty,
but other systems may require -lrpc and -lcompat.  CFLAGS should be
just -O for SunOS.  If you're running a variant of BSD4.4, add
-DBSD44.  If your client system does not put NFS requests on a
privileged port, add -DANYPORT.  (If you set ANYPORT, note that any
logged in user can fairly easily spoof any currently attached user's
files, so this is another reason not to run CFS on a multi-user
machine.)  If your system does not support NFS mounts on ports other
than 2049, add -DCFS_PORT=2049; you will not be able to simultaneously
run the target system as an NFS server under this configuration.
.PP
Now type "make".  If everything compiles correctly, become root and do
a "make install".  Now start up /usr/local/etc/cfsd (as root); you
should see "cfs ready" with the PID of the daemon.
.PP
You'll need a "bootstrap" mount point exported to localhost.  Note
that exporting file systems to localhost is somewhat dangerous; there
is a bug in most versions of the portmapper that can allow remote
access to such file systems.  In order to get CFS running, however,
you need something to provide a file handle for the "root" of the CFS
file system.  To play it safe, I suggest exporting an empty mode 000
directory.  It is used only to get CFS mounted by your local NFS
client code.  If your version of the portmapper doesn't support remote
RPC forwarding, or if you don't have a regular NFS server running on
your machine, or if you already have a file system exported to
localhost for some other purpose, you can use a "real" file system
(such as /) for this purpose.  Note that most versions of mount do not
allow you to export more than one mount point on a given file system.
.DS
# mkdir /null
# chmod 0 /null
.DE
.PP
Now create the real CFS mount point (mkdir /crypt); it should be mode
755.  Edit /etc/exports to export the bootstrap file system to
localhost, e.g.:
.DS
/null localhost
.DE
and make sure mountd is running and sees it (you may have to start up
mountd by hand or do a "/etc/exportfs -a").  If you're using a
different file system as the bootstrap mountpoint, use that in place
of /null in the mount command below.
.PP
Now you should be able to mount CFS just as you would a remote NFS
server:
.DS
# /etc/mount -o port=3049,intr localhost:/null /crypt
.DE
(omitting the "port=3049" part if your local system does not support
that option).
.PP
If all goes well, you should be able to ls /crypt, and you should see
localhost:/ as a mounted NFS server, e.g.:
.DS
# /etc/mount
\fI[other stuff]\fP
localhost:/null on /crypt type nfs (port=3049,intr,rw)
# 
.DE
.PP
Test CFS by cmkdir'ing and cattach'ing a sample directory.  A good
test is to copy the distribution sources into your CFS directory and
make them there; compare the resulting binaries and attempt to run the
user commands (like cmkdir) from there.
.PP
Now edit /etc/rc.local to automatically start all this up at boot
time after mountd is started up.  I use:
.DS
if [ -x /usr/local/etc/cfsd ]; then
        /usr/local/etc/cfsd && \\
                /etc/mount -o port=3049,intr localhost:/null /crypt
fi
.DE
Note that if you are using a system that does not allow ports other
than 2049 for NFS, you'll have to make sure the rc file starts up
mountd but
.I not
any nfsd processes.  The default behavior of the rc files on most NFS
systems checks for the existence of /etc/exports and starts both
mountd and nfsd if it's there; you'll have to edit out any lines that
try to start up nfsd on such systems.
.PP
If any of this fails, recheck the configuration in the Makefile and
make sure all of the appropriate options are there.  If you get
"permission denied" when you try to ls /crypt, you probably need
-DANYPORT in CFLAGS.

.NH
Internals	
.PP
The public CFS distribution differs a bit from the version described
in the ACM COCCS paper.  First of all, smartcard support is not
included; all keys must be managed via passphrases.  Some of the
interfaces to the various programs are a bit different as well.
.PP
On the upside, the DES-based encryption scheme has been considerably
strengthened.  At attach time, CFS divides the passphrase into two DES
keys, K1 and K2.  K1 is used to create two DES pseudorandom stream
ciphers, each 128KB long, S1 and S2.  To encrypt a file block, it is
first XORd against a unique bitstream derived from the inode number of
the file.  The result is then XORd against S1 based on its offset
in the file.  This is then DES ECB encrypted with K2, and the result
of that is XORd against the appropriate position in S2.  The resulting
ciphertext is what is stored.  The cipher is reversed in the obvious
manner.  Filenames are similarly encrypted.
.PP
There does not appear to be a feasible attack that allows an
independent search for the two subkeys K1 and K2; in a known-plaintext
attack, an attacker would have to try all 2**112 key combinations.
Note that the two keys may be vulnerable to independent exhaustive
search under a so-called "linear" attack, but this attack appears to
require a large number of chosen plaintexts encrypted under the same
inode number.  Under most conditions where the attacker cannot
introduce large numbers of chosen plaintexts, I believe CFS encryption
is equivalent in practice to "3-DES", against which there is no known
feasible attack (as far as the public research community knows, anyway).
Note that this is not the same as a "proof".  In any event, CFS is always
at least as secure as 1-DES.  If you are willing to trade off a great
deal of performance, you can use 3-DES directly.
.PP
A true 3-DES (2 key) mode is now available in this release; this mode
uses the same hybrid stream/codebook scheme as above, but encrypts
both the static stream and the individual cipherblocks three times
(with the two passphrase-derived subkeys).  This mode is more
"standard" and is probably stronger than the default cipher mode, but
at considerable performance expense.  To get true 3-DES, specify "-3"
with \fBcmkdir\fP.  Keep in mind that the limited entropy of most
user-selected key passphrases represents a much greater practical
threat than a direct attack against even single DES.
.PP
CFS semantics should be roughly the same as those of any other
NFS-mounted file system, with a couple of minor differences.  Because
of the encoding of pathnames as hex strings, path components are
limited to half the normal size and full pathnames can only be about
half as long as the normal maximum.  Also, holes in files (created by
writing beyond the current end of file) are filled in with random
garbage rather than zeros.  This does not appear to break any standard
applications, although some program somewhere might depend on this
property.
.PP
NFS attribute caching can confuse CFS under certain conditions.  In
particular, if you create a directory, copy something into it and
immediately rename rename the directory, the directory's contents may
not be visible until the client attribute cache times out and the file
handles for the existing files are looked up again.  Cache update
typically takes 30 seconds in the worst case, and seems to be a
problem most often under Linux.  The inconsistencies are only rarely
noticeable under typical workloads and do not lead to any actual
corruption of stored data.  If it bothers you, turn off attribute
caching by adding the "noac" option to the /crypt mount command.  Note
that this option may affect performance.
.PP
To enhance portability, CFS now stores the IV derived from the inode
in a separate file rather than in the gid of the file.  This is done
using an ugly hack that involves creating a symbolic link that points
to a string containing the IV.  Under highly concurrent operation with
multiple machines creating files in the same encrypted directories,
there may be some undetected race conditions here.  Having separate
IVs for each file can be disabled with the "-l" option to
.B cattach;
under this option, identical files will encrypt to the same
ciphertext.
.PP
I think CFS is secure against most non-cryptanalytic attacks,
especially on a single-user machine, but there are a few weak areas of
which you should be mindful.  If your system pages to an untrusted
disk, cfsd could leak cleartext and keys to the paging device.  It is
best to be sure you have sufficient real memory on the client system
to avoid this.  Unfortunately, I know of no portable way to prevent a
user process from paging.  Also, an error in cfsd could cause a
coredump to an untrusted device, leaking keys and cleartext in the
process.
.PP
cfsd uses lots of memory for each attached directory.  If you must
install CFS on a multiuser system, you should reduce NINSTANCES (in
cfs.h) to a small enough value to avoid thrashing.
.PP
This release makes it reasonably easy to add other 64 bit codebook
ciphers.  To add a cipher, you'll need to provide a block encrypt/
decrypt function and add hooks for it in the following routines:
\fIcipher(), mask_cipher(), pwcrunch(),\fP and \fIcopykey().\fP  You'll
also need to modify \fIcmkdir.c, admproto.x, and cfs.h\fP to refer to the
new cipher.  If your cipher does not have a 64 bit codebook mode, it
will be more painful to add, since CFS assumes this size in far too many
places.
.PP
The file ver.c contains a static string with information on the
version of CFS from which it was derived.  In particular,
the cryptlevel number refers to the encryption format; versions
of CFS with the same cryptlevel should be able to share encrypted
directories without any problems.  cryptlevels below 10000 are
reserved for official releases, 10001 and up are for end-user
defined formats.  If you are going to be changing the encryption
scheme in CFS and would like to reserve a cryptlevel number,
let me know at cfs@research.att.com.
.PP
I maintain a mailing list of developers and users of CFS for
discussion of problems, updates, and other areas of common interest.
To subscribe, send mail to cfs-users-request@research.att.com with the
string "subscribe cfs-users" in the message body.
.SH
Acknowledgements
.PP
CFS was ported to HP/UX, Irix, Linux and AIX by Mark Henderson
(markh@wimsey.com).  Ed Gould (ed@left.wing.org) made CFS work under
Ultrix 4.2.  Solaris 2.3 support was added by Mark Stein
(Mark.Stein@Eng.Sun.COM).  Several other users contributed bug
reports, suggestions, and advice.  Thanks!
.LP
August 28, 1994
