-----BEGIN PGP SIGNED MESSAGE-----

PGPShell Version 3.2a                                      7/94
=====================


Notes for PGPShell v3.2a
- ------------------------

This is a quick maintenance release which takes care of two
minor, yet annoying bugs:  If an angle-bracketed e-mail is
missing from a UserID, the UserID will now display correctly
in the KMS and the e-mail field will read "no e-mail address."
The other fix takes care of the binary garbage that would
display on the validity field of your key if the PGP "MyName"
variable was not correctly entered.  Now it will just be
blank until you enter in a correct name in PGP's CONFIG.TXT.


Notes for PGPShell v3.2
- -----------------------

This is an update of PGPShell v3.1.  Although it was mainly
released because of the new MIT PGP v2.6, don't think for a 
second that I didn't use the opportunity well!  See the 
WHATSNEW.DOC file for details and the numerous improvements 
with this version.


Configuration
- --------------

Configuring PGPShell is easy.  Simply set the "MyName" variable
in PGP's CONFIG.TXT configuration file to your UserID name to 
correctly use PGPShell.  Consult the PGP documentation for details 
on the MyName variable if you are unsure how to set it.  

Beyond that, ensure that you correctly set the PGPPATH and TZ
DOS environment variables before using either PGP or PGPShell.
These two items are explained at great lengths in the PGP docs.

If you do not have PGP (and aren't sure what I'm talking about!)
consult the WHAT_IS.PGP text file for a quick primer and details
about where to get PGP.



Virus Infection/Tampering of PGPSHELL.EXE
- - -----------------------------------------

If you want to check the file integrity of the PGPSHELL.EXE executable
file, at the DOS prompt type:  PGPSHELL crc

The CRC checksum for the unregistered, shareware version (only) should 
be 13389.  If the output doesn't agree with this figure, then the 
PGPSHELL.EXE file may contain a virus, has been reverse-engineered, 
altered, or is a counterfeit.  This README file is itself, certified 
by my own signature.  This means that no one can alter the checksum 
(or any other words) without destroying the PGP certification that I
have created.  Add my public key to your keyring and then you can check
the PGP certification with the PGP command:  PGP readme.txt.  If you
are in doubt or just want a fresh copy you can always call my BBS and
download the latest version.

If you have any questions about the use of PGPShell just ping me at
still@rintintin.colorado.edu.


On a Personal Note
- ------------------

My wife has decided to go to graduate school in the fall of 1995 which
means that if you have downloaded this file during the summer of 1995
I may not be at this e-mail address!  It is likely that I'll have to  
get a new P.O. Box whereever we end up too.  Look for regular postings
during this time period on alt.security.pgp if I am no longer at my
e-mail address.  Also, try to contact me (Still, James Still or some
such) on America On-Line since I'll probably get an account there.
I hate moving, but what can you do?

-----BEGIN PGP SIGNATURE-----
Version: 2.3a

iQCVAgUBLiwS3JE2Jn5vTkk3AQEalAP/Ws1q16lt1Okm2VY6Rn0l5+Hb11RdpcqB
p6wk0yBaYknRD6tteugEx5tg+zQftDZ9RqqbBY3jtJBX7W4hbtJAL0UAcpmxTC2i
ZCoSSSM4iO7EihIo4K9yztKAK88ZUi04gMs/TtNJAe6nl5zCBg2hO6kgUgNTdWd5
kz9KNlVsUbk=
=92Yu
-----END PGP SIGNATURE-----
