Copyright (c) Nick Payne 1996-1999

Cryptext v3.21

OVERVIEW

Cryptext is a public domain Windows 95 / NT4 shell extension that performs strong file encryption. It uses a combination of SHA-1 and RC4 to encrypt files using a 160-bit key. The current versions of Cryptext can always be found on my home page at http://www.pcug.org.au/~njpayne, and are mirrored to the ftp site ftp.funet.fi/pub/crypt/utilities/file.

This version of Cryptext is available in English, French, German, Spanish, and Portugese language versions. The ZIP file for the English version is CRYPTEXT.ZIP, for the French version CRYPTFRA.ZIP, for the German version CRYPTDEU.ZIP, for the Spanish version CRYPTESP.ZIP, and for the Portugese version CRYPTPOR.ZIP. Only the Cryptext program itself is language specific. The readme file and help file are still English language for all versions.

Cryptext is public domain software, and may be used in any way, for any purpose, at no cost. It may be distributed by any means. Note that Cryptext contains strong cryptographic routines upon which some countries place distribution and/or use restrictions (but see below, it does not appear to be covered by the Wassenaar agreement). Verify that you are allowed to use or distribute Cryptext before doing so.

CRYPTEXT AND THE WASSENAAR AGREEMENT

The Wassenaar list of dual-use technologies says, in its 'General Technology Note': 
Controls do not apply to "technology" "in the public domain", to "basic scientific research" or to the minimum necessary information for patent applications."

The 'General Software Note' adds the following: 
The Lists do not control "software" which is either:
1. [..] [..]
2. "In the public domain".

"In the public domain" is defined as:
This means "technology" or "software" which has been made available without restrictions upon its further dissemination. N.B. Copyright restrictions do not remove "technology" or "software" from being "in the public domain".

DISCLAIMER OF WARRANTY
THIS SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND WITHOUT WARRANTIES AS TO PERFORMANCE OF MERCHANTABILITY OR ANY OTHER WARRANTIES WHETHER EXPRESSED OR IMPLIED. BECAUSE OF THE VARIOUS HARDWARE AND SOFTWARE ENVIRONMENTS INTO WHICH THIS PROGRAM MAY BE PUT, NO WARRANTY OF FITNESS FOR PARTICULAR PURPOSE IS OFFERED. GOOD DATA PROCESSING PROCEDURE DICTATES THAT ANY PROGRAM BE THOROUGHLY TESTED WITH NON-CRITICAL DATA BEFORE RELYING ON IT. THE USER MUST ASSUME THE ENTIRE RISK OF USING THE PROGRAM.

Any product or brand names mentioned in this document are trademarks or registered trademarks of their respective owners.

**IMPORTANT** This version of Cryptext implements a checksum to prevent accidental decryption of a file with the wrong password, and is *not* compatible with any version of Cryptext prior to v2.30. If you have files you have encrypted with a version of Cryptext older than v2.30, you must decrypt them before installing this new version. If you do not, you will not be able to decrypt these files without un-installing the new version and re-installing the previous version.

Alternatively, you can use the console application DECRYPT.EXE, which is included in the ZIP file, to decrypt any file encrypted with Cryptext version 2.0 or later. However, DECRYPT.EXE can only decrypt a single file at a time, and makes no check that you are using the same password for decryption as you used when encrypting the file.

If you are presently using Cryptext v1.x, you *must* decrypt all encrypted files before installing the new version of Cryptext, or you will have no way of decrypting previously encrypted files.

WHAT'S NEW IN THIS VERSION

Added Portugese version of Cryptext. The encryption/decryption progress bar now uses the smooth progress style available with COMCTL32.DLL v4.70 and later.

TO INSTALL CRYPTEXT

1. If you have a previous version of Cryptext older than v2.30 installed, decrypt any encrypted files.

2. Unzip the installation executable CRYPT320.EXE from the ZIP file and run it.

To install Cryptext on Windows NT you must be logged in as Administrator or a user who is a member of the Administrators group. This is because only an Administrator has update rights to the part of the registry where shell extensions are registered.

TO UNINSTALL CRYPTEXT

1. From the Start menu, select Settings, then Control Panel, then Add/Remove Programs.

2. Select the "Cryptext (Remove only)" entry.

3. Click on the Add/Remove button.

DISTRIBUTION

Cryptext may be distributed by any means: eg. BBS, web or ftp sites, shareware dealers, and shareware CDs.

HOW DOES CRYPTEXT WORK?

1. When you install Cryptext it adds "Encrypt" and "Decrypt" items to the context menu you get when right-clicking on files or directories in Explorer.

2. When you encrypt a file, Cryptext takes your passphrase and uses the SHA-1 one-way hash function to generate a 160-bit key.

3. For each file selected, it then concatenates the key from step 2 with a salt value generated from (a) the number of 100-nanosecond intervals since January 1, 1601, and (b) a 32-bit random number, and hashes this concatenation with SHA-1 to produce the key which is used for the encryption. This step is taken to ensure that no two files are encrypted with the same keystream.

4. The file is then encrypted in 16Kb blocks. Each block is read, encrypted, and then written back to disk, so that when the encryption of the file is complete, the original file has been completely overwritten by the encrypted version. Note, however, that an attacker with sophisticated equipment for scanning the hard disk can recover the data that was previously written to the disk, even after it has been overwritten. See http://www.cs.auckland.ac.nz/~pgut001/secure_del.html for a discussion of the techniques used.

5. The salt values from step 3 are stored in plaintext with the encrypted file, so that the file can be successfully decrypted when the correct passphrase is supplied. There is no requirement that these values be kept secret, only that they be unique for each file.

6. In order to verify your passphrase on second and subsequent executions, Cryptext takes the key generated in step 2, adds it to the end of your passphrase, and applies the SHA-1 function to the concatenation of the passphrase and key. The resultant hash is stored in the registry so that subsequent passphrases can be checked for validity by being put through the same two-step hash and compared with the stored value.

For decryption, Cryptext first reads the values saved in step 5 from the encrypted file and concatenates them with your hashed passphrase to obtain the decryption key.

VERIFICATION

With encryption software, it is important to be able to verify both that the encryption program has not been tampered with and that the encryption algorithm and implementation are as claimed. Most commercial packages fail on both these points.

The file CRYPTEXT.DLL.SIG contains the PGP signature of CRYPTEXT.DLL. If you have a copy of PGP version 5.x or 6.x, you can use this together with my public key to verify that CRYPTEXT.DLL has not been altered before it reached you. My public key is available either from a PGP keyserver or from my web page at http://www.pcug.org.au/~njpayne/verify.html. After you have installed Cryptext, you will find CRYPTEXT.DLL in the WINDOWS\SYSTEM\SHELLEXT directory (on Windows 95/98) or WINNT\SYSTEM32\SHELLEXT (on Windows NT).

To verify that Cryptext actually does what I claim, CRYPTEXT.ZIP contains a second zip file, DECRYPT.ZIP. This contains the source code and a Visual C++ make file to build a Win32 console application that uses SHA-1 and RC4 to decrypt files which have been encrypted with Cryptext. To verify Cryptext:
1. Extract the contents of DECRYPT.ZIP into a directory
2. Eyeball the source code to verify that it does what it claims
3. Use the make file to build DECRYPT.EXE
4. Encrypt a file with Cryptext
5. Run DECRYPT.EXE in a DOS window to decrypt the file. You need to supply two command line parameters. The first is the passphrase; the second is the filename. e.g.
	  DECRYPT "my pass phrase" c:\temp\test.txt.$#!
Don't forget that a file encrypted with Cryptext will have the added extension "$#!" which is not shown in Explorer.

NOTES

1. If you are running on Windows NT 4 then you have the option to retain the encryption password in memory for the duration of your NT session. If you are running on Windows 95 you must enter the password for each encryption or decryption.

2. After the initial execution of Cryptext, you cannot change your encryption password unless you know the existing password. If you forget the password, use REGEDIT to delete the registry key HKEY_CURRENT_USER\Software\Cryptext, and the next time you try to encrypt or decrypt a file, Cryptext will ask you for a new password. If you have files you have encrypted, and you have forgotten the password, then those files are not recoverable.

3. Cryptext assumes that the file system on which it is running supports long filenames. When it encrypts a file it adds the extension ".$#!" to the filename. This name change will fail if the existing filename already exceeds 252 characters or if the file system does not support long filenames (such as a NetWare server volume which does not have long namespace support loaded). The resulting file is still encrypted but it does not have the extension which Cryptext recognises, and you will not be able to decrypt it until you change the file extension to ".$#!".

4. Password selection. Cryptext allows a password to be up to 255 characters long. As a file encryption password is generally in use for much longer than a login password, you should use more care selecting it. For more information on good password selection, use one of the www search engines to search for web documents containing "password", "selection", and "good".

If you are interested in finding out more about encryption and cryptography:
a) RSA's web site at http://www.rsa.com has a good cryptography FAQ available both online and as a downloadable PDF file
b) There is a short introduction to cryptographic concepts and algorithms at http://www.mach5.com/crypto
c) There is a lot of cryptographic source code at the ftp site idea.sec.dsi.unimi.it/pub/security/crypt/code
d) Bruce Schneier's book "Applied Cryptography" has a comprehensive coverage of both protocols and algorithms
e) You can find an encryption library which provides a consistent interface to many encryption algorithms, as both source code and 16- and 32-bit DLLs, at http://www.cs.auckland.ac.nz/~pgut001/cryptlib/index.html.

Nick Payne
njpayne@pcug.org.au
April 1999
