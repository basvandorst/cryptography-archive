-----BEGIN PGP SIGNED MESSAGE-----

PGPShell Version 3.3                                      4/4/95
====================


Notes
- -----

This is a minor update of PGPShell v3.2a. Some functions have   
been bulletproofed and a few interface changes were made. The
biggest change is that PGPShell is released as freeware.


Configuration
- -------------

Configuring PGPShell is easy.  Simply set the "MyName" variable
in PGP's CONFIG.TXT configuration file to your UserID name to 
correctly use PGPShell.  Consult the PGP documentation for details 
on the MyName variable if you are unsure how to set it.  

Beyond that, ensure that you correctly set the PGPPATH and TZ
DOS environment variables before using either PGP or PGPShell.
These two items are explained at great lengths in the PGP docs.

If you do not have PGP (and aren't sure what I'm talking about!)
consult the WHERE_IS.PGP text file for a quick primer and details
about where to get PGP.




Virus Infection/Tampering of PGPSHELL.EXE
- -----------------------------------------

If you want to check the file integrity of the PGPSHELL.EXE executable
file, at the DOS prompt type:  PGPSHELL crc

The CRC checksum should be 40143.  If the output doesn't agree with
this figure, then the PGPSHELL.EXE file may contain a virus, has been
reverse-engineered, altered, or is a counterfeit.  If you haven't added
my public key to your key ring yet (STILL.ASC) you should and then
check the signature integrity of this README file by typing 
PGPSHELL README.TXT.  This will tell you if this readme file has been
tampered with.

Contacting Me
- -------------

All of the e-mail accounts on my public key will be obsolete sometime
during the summer of 1995 (or when the sysadmin discovers that I am
no longer at the university...) I will check in regularly at the
alt.security.pgp newsgroup to keep in touch, answer user support
questions, and as soon as I know, post my new e-mail address.

-----BEGIN PGP SIGNATURE-----
Version: 2.6.2

iQCVAwUBL4FwV5E2Jn5vTkk3AQE9wQP/eahuePA5ku5A2WEUT1yzweACKRhv/ESz
NVRxBfbQyHXE1SyhFfZbHANpyccirCGB0Nu7GaGMdIvRzdcac/OsEk6JGHl0xoGV
FWBVeOYfQrs4T70vVlHh6yicElNd3r9DA7glpu1VY9niE8OWnNen09aee/PKHa9u
NLLgVkiTOik=
=9OQ8
-----END PGP SIGNATURE-----
