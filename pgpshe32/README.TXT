-----BEGIN PGP SIGNED MESSAGE-----

PGPShell Version 3.2                                        7/94
====================


Notes
- -----

This is an update of PGPShell v3.1.  Although it was mainly
released because of the new MIT PGP v2.6, don't think for a 
second that I didn't use the opportunity well!  See the 
WHATSNEW.DOC file for details and the numerous improvements 
with this version.


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
consult the WHAT_IS.PGP text file for a quick primer and details
about where to get PGP.



Virus Infection/Tampering of PGPSHELL.EXE
- -----------------------------------------

If you want to check the file integrity of the PGPSHELL.EXE executable
file, at the DOS prompt type:  PGPSHELL crc

The CRC checksum for the unregistered, shareware version (only) should 
be 62395.  If the output doesn't agree with this figure, then the 
PGPSHELL.EXE file may contain a virus, has been reverse-engineered, 
altered, or is a counterfeit.  If you haven't added my public key to 
your key ring yet (KEPLER.ASC) you should and then check the signature 
integrity of this README file by typing PGPSHELL README.TXT.  This will 
tell you if this readme file has been tampered with.

If you have any questions about the use of PGPShell just ping me at
still@rintintin.colorado.edu.

-----BEGIN PGP SIGNATURE-----
Version: 2.3a

iQCVAgUBLhmo8pE2Jn5vTkk3AQGfbQP+OOTjOmS9OH1mBExosp9TQfrXaOhKxx5X
3+EAapaV3oLKXie1vkrQbCTNE3HfUl7DKsw+5dduJGACboqZ60p+6rQtiE3LM0bA
piA1Jh+DyHE2+1gqwBDnALvrbuiyQHVsH8azdSEQ2HMOqgfm0dcR8mMJN4XtIheK
Ql6Hqoiv9XQ=
=RJEu
-----END PGP SIGNATURE-----
