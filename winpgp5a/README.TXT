****************************************
    WinPGP® 5.01 Production Release 
****************************************
As of: 07/04/96

This is the production release of WinPGP® 5.0 32bit.  

New with this release:

- WinPGP now supports email via the MS-Exchange Client
- The program now does both multifile Encryption and Decryption
- The Clipboard Editor is displayed as a Window and has wordwrap
- WinPGP now supports multiple recipient selection
- Improvements have been made to the Key Management section
- A new registration security system is implemented

*******************************
NOTE: Installation Instructions
*******************************

Using the downloadable release:

1. Put the zip file into a temporary directory, i.e., C:\temp.  

2. Unzip the file.

3. Run WinPGP5i.EXE to install.

*******************************
   *** IMPORTANT NOTES!!! ***
*******************************

1. WinPGP® contains none of the PGP® executables and incorporates none
of the PGP® source code in any way or form.  This program simply 
provides the user with a convenient Windows GUI interface.

2. Users of WinPGP® must accept this disclaimer of warranty: 

"WinPGP® is supplied as is.  The author disclaims all warranties, expressed or implied, 
including, without limitation, the warranties of merchantability and of fitness for any purpose.
The author assumes no liability for damages, direct or consequential, which
may result from the use of WinPGP®."

3. Note that when WinPGP® 5.0 runs bulk decrypt, the original *.PGP files are
NOT deleted automatically.  Feedback from the user community as to whether this should
remain is desired.

4. Due to limitations in PGP 2.6.2 or 2.7.1 itself, Long Filenames are not supported
at this time. However, if you are using PGP 2.6.3 32 bit this is not true.
If you attempt to run an encryption in a directory that has long pathnames, PGP will 
balk.  I.e., "C:\WinPGP 5" will not work. Also, files that are encrypted in bulk encryption
will be reduced to their MSDOS 8.3 formats.  Decrypting will not recover the original 
long filenames.

5. Note that this version is designed to run in Windows 95 and 
Windows NT 3.51 or higher.  It will NOT run properly in 
Windows 3.1 or 3.11 even if Win32s is installed.  If you are using
Windows 3.1 or 3.11, please use the 16 bit WinPGP 4.1 instead.





