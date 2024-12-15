PGP 6.0.2 Read Me
   
Copyright (c) 1998 by Network Associates, Inc., 
and its Affiliated Companies.  All Rights Reserved.


Thank you for using Network Associates' products. 
This Read Me file contains important information regarding 
PGP 6.0.2. Network Associates strongly recommends that you 
read this entire document.

Network Associates welcomes your comments and suggestions. 
Please use the information provided in this file to 
contact us.

Warning: Export of this software may be restricted by
the U.S. Government.


      *****  IMPORTANT NOTE FOR PGPDISK USERS *****

An internal security review of PGPdisk revealed a security 
flaw. This affects PGPdisk 1.0 for Windows and the version 
of PGPdisk that shipped with PGP 6.0 for Windows.  THIS DOES 
NOT AFFECT OTHER PORTIONS OF PGP, ONLY PGPDISK FOR WINDOWS.  

The version of PGPdisk that accompanies this release of PGP 
(6.0.2) eliminates the problem.  After you install PGP 6.0.2 
and reboot your machine, the PGPdisk Conversion Wizard will 
scan your machine for any existing PGPdisk volumes and will 
update them automatically.  Note that the conversion process 
will remove any alternate passphrases and public keys (with 
the exception of ADKs) associated with the volume.

You must use the master passphrase to convert each volume.

PGP will automatically upgrade older volumes as you attempt
to use them. We strongly recommend that you update all your 
PGPdisk volumes to the new version immediately, including 
any volumes that may exist on backup media or network devices. 
If you choose not to convert a volume, copy the data to a new 
PGPdisk and wipe the original using the PGP 6.0.2 wipe feature.

Instructions on how to run the PGPdisk Conversion Wizard 
are included in this document.

          *******************************



___________________
WHAT'S IN THIS FILE

- New Features
- Known Issues
- RSA Support
- PGPdisk Conversion Wizard
- Important Fixes
- Additional Information
- Contacting Network Associates


_____________________
NEW FEATURES IN 6.0.2

- PGP Version 6.0.2 includes a Secure Viewer feature 
known in previous versions of PGP (Version 2.x) as 
"For Your Eyes Only" or FYEO.  A message encrypted with 
the Secure Viewer option is only viewable within the 
read-only Secure Viewer window.  The recipient of the 
message is unable to save the message as plaintext.  
The Secure Viewer uses a special font that prevents 
TEMPEST attacks. For more information on Secure Viewer, 
please see the What's New section in the PGP User's 
Guide .pdf file.

- Forcing the Recipient Dialog to appear can now be
accomplished by holding down a modifier key in the email
plugin to show the Recipient Dialog even in cases where
the recipients are all valid.  This key varies by plugin,
but is either Shift or Control.

- The Outlook plugin now supports Personal Distribution
lists.

- PGPtray now works with files copied to the clipboard 
from Explorer.  It previously only worked with copied text.


____________
KNOWN ISSUES

- The maximum size for RSA keys is 2048 bits. This is due 
to limitations in the RSA libraries and our desire to remain 
backward compatible with previous versions of PGP.  (Some
previous versions of PGP did function with large RSA keys,
but creation of these keys has never been supported.)

- Mismatching your keyring files can result in data loss.  
Your public keyring file and private keyring file must be 
kept in sync.  If, for instance, you select a public 
keyring file that does not contain the public portion of 
your private key and do not also change the private keyring 
to the corresponding file, you and others will not be able to
encrypt to exported versions of your key after that time.  
In most cases, simply updating your key from a public copy on 
a keyserver will fix this.  However, it is recommended that 
the keyring files always be kept in sync.  A future version 
is expected to correct this issue.

- Using a Split Key as a public key for PGPdisk will not 
allow reconstitution of the key through the usual dialog 
provided by the rest of the product for Split Keys.  In 
order to use such a key to open a PGPdisk, you must first 
rejoin the key in PGPkeys.

- Some PGP 6.0 features are incompatible with previous 
versions of PGP, so we now feature a "compatible" export 
format that strips incompatible features such as Photo IDs 
from keys. Selecting "Use 6.0 Extensions" in the Export 
dialog enables these features to be exported.  By default, 
we export in compatible mode.  You may change the default 
in the Advanced preferences dialog.  When sending a key to 
a PGP Certificate Server Version 2.0 or above, Photo IDs 
always accompany the key. The default LDAP server in 
PGP 6.0 supports this.

- In some plugins, the Extract Public Key feature has 
been merged into the Decrypt/Verify feature.  When in 
doubt, use Decrypt/Verify to extract public keys.

- Sometimes, after a PGPdisk volume is unmounted and placed 
in the Recycle Bin and the Bin is emptied, Windows reports 
that the drive letter associated with the PGPdisk volume 
is no longer accessible.  This behavior should cease after 
the system is rebooted.  This should not interfere with the 
normal operation of your system.

- On Windows 95/98, the Free Space Wiper will reset every 
time another program writes to your disk while the wiping 
process is in progress.  This is similar to defragmentation 
programs, and is required by the Operating System.  For 
optimal wiping, make sure to close all open applications.  
On some systems with very low amounts of RAM, it may be 
advisable to also shutdown Windows Explorer.

- If you experience a long delay searching the server 
sending mail when using the Exchange/Outlook email plugin 
with an Exchange Server and the Encrypting to Unknown Keys 
synchronization checkbox is checked, this can be caused 
by the email account having many SMTP addresses on the 
Exchange Server or by having many servers specified 
in your PGP preferences.

- The Windows Explorer provides PGP with information
only about the target of a shortcut and not the shortcut 
itself.  If you use the Wipe feature in the Explorer, 
the shortcut itself will not be wiped.  The actual target 
will be wiped.  When using PGPtools, the shortcut will 
also be wiped.


___________
RSA SUPPORT

If you are interested in using or creating RSA keys, 
please review the following information to determine 
which version of PGP 6.0.2 you need:

Use                      Product (Mac or Windows)
------------------------------------------------------------    
Use existing             all versions of PGP 6.0.2, 
    OR                   including PGP Freeware
 create new    
 Diffie-Hellman/DSS keys

Use existing     	 PGP for Personal Privacy with RSA 
    OR			 PGP Desktop Security with RSA
 create new RSA keys                       


Limited and **unsupported** RSA functionality is available in 
the Windows versions (ONLY) of PGP through Microsoft's 
Cryptographic API (CAPI), as installed with Internet Explorer 
4.0.  If you have installed the 128-bit version of IE 4.0 
or later on your system, PGP will use CAPI to provide basic 
RSA functionality.  With CAPI, you will be unable to generate 
RSA keys nor use non-standard key sizes.  If you find that this 
RSA implementation does not meet your needs, you will need 
to purchase the standard RSA Add-on.  See the table above 
to determine which product best suits your needs.  

If you have purchased an incorrect version or are interested
in purchasing an RSA Add-on version of PGP please contact the 
Network Associates Customer Care department at: 

Phone (408) 988-3832
Monday-Friday, 6:00 A.M. - 6:00 P.M. Pacific time


_________________________
PGPDISK CONVERSION WIZARD

An internal security review of PGPdisk revealed a security 
flaw. This affects PGPdisk 1.0 for Windows and the version 
of PGPdisk that shipped with PGP 6.0 for Windows.  THIS DOES 
NOT AFFECT OTHER PORTIONS OF PGP, ONLY PGPDISK FOR WINDOWS.  

We strongly suggest you upgrade all existing PGPdisk volumes.
PGP version 6.0.2 includes the PGPdisk Conversion Wizard, 
which automatically upgrades earlier versions of PGPdisk volumes. 
See the note at the beginning of this ReadMe file for more
information.

Note: When you convert a PGPdisk volume, the main passphrase 
for the PGPdisk volume and any ADKs associated with the 
volume are preserved; however, alternate passphrases and 
public keys that are associated with the PGPdisk volume 
are not preserved. 

___________________________________________________________
To convert all of your PGPdisk volumes, follow these steps:

1. If the installer detects any previous version of PGPdisk 
   on your system, the PGPdisk Conversion Wizard automatically 
   appears when you restart your system after PGP 6.0.2 
   installation is complete. 

   If the installer fails to detect a previous version of 
   PGP or PGPdisk, you can manually start the PGPdisk 
   Conversion Wizard by opening the PGPdisk application 
   and selecting Convert Pre-v6.0.2 disks from the File 
   menu.

2. If you want to convert all of your PGPdisk volumes on 
   your system now, click Next. 

   The Conversion Wizard searches your system for PGPdisk 
   volumes that have not been converted.

3. Select all PGPdisk volumes listed in the Conversion 
   Wizard.

4. Click Next.

   A warning message appears and prompts you to start the 
   PGPdisk Conversion Wizard.

5. Click Yes to convert the PGPdisk volume.

   The PGPdisk Conversion Wizard screen appears. 

6. Enter the master passphrase for the volume to be
   converted, then click OK.

7. Click Next to start converting the PGPdisk volume.
   The PGPdisk volume will be re-encrypted.  
   
   *** Note that you will lose all passphrases and 
   public keys associated with the volume (excluding 
   the volume's master passphrase and any PGPdisk ADKs).

8. Move your mouse around in a random manner within the 
   Wizard window and/or type characters on the keyboard 
   until the progress bar shown in the dialog box is 
   completely filled in. 

   Your mouse movements and typing are used to generate 
   random information used by the PGPdisk program as part 
   of the encryption (data scrambling) process.

9. Click Next.

   A progress bar indicates how much of the PGPdisk volume 
   has been converted.

10. When the progress bar is at 100%, click Next to finish 
    the process.

11. Click Finish, then mount the volume by entering the 
    volume's master passphrase.

12. The PGPdisk volume is secure and ready to use. 

    Repeat steps 4-10 to convert all of your PGPdisk 
    volumes. Be sure to convert all of your PGPdisk volumes 
    including volumes that are stored on network drives and 
    removable media as well as volumes stored on your local 
    drive(s).

_______________________________________________________
To convert PGPdisk volumes stored on network drives and 
removable media, follow these steps:

1. Install PGP 6.0.2.

2. Mount the PGPdisk volume. PGPdisk will automatically 
   alert you when you attempt to mount volumes that have 
   not been upgraded.  

3. Click Yes to convert the PGPdisk volume.  
   
   *** Note:  If you choose not to convert the volume 
   at this time, the volume is mounted in read-only format 
   so you can access the contents of the encrypted volume 
   to back them up prior to conversion.  You will be unable 
   to write to the volume until you convert it.

   If you click Yes, the PGPdisk Conversion Wizard screen 
   appears. 

4. Click Next to start the Conversion Wizard.
   
   *** Note:  You will lose all passphrases and 
   public keys associated with the volume (excluding 
   the volume's master passphrase and any PGPdisk ADKs).

5. Enter the master passphrase for the volume to be
   converted, then click OK. 

6. Move your mouse around in a random manner within the 
   Wizard window and/or type characters on the keyboard 
   until the progress bar shown in the dialog box is 
   completely filled in. 

   Your mouse movements and typing are used to generate 
   random information used by the PGPdisk program as part of 
   the encryption (data scrambling) process.

7. Click Next.

   A progress bar indicates how much of the PGPdisk volume 
   has been converted.

8. When the progress bar is at 100%, click Next to finish 
   the process.

9. Click Finish, then mount the volume by entering the 
   volume's master passphrase.

10.The PGPdisk volume is secure and ready to use. 

   Repeat steps 2-8 to convert all of your PGPdisk volumes. 


_______________
IMPORTANT FIXES
                        
In addition to the PGPdisk issues and some minor issues,
the following common issues are fixed in this version:

- Fixed: Long passphrases are sometimes reported as
incorrect even though they are typed correctly.

- Fixed: PGPkeys sometimes creates RSA keys that are
displayed as split keys.

- Fixed: "Wrong version" error when using PGP with the
comctl32.dll from IE 5.0 beta.

- Fixed: Mouse can disappear "behind" the Recipient Dialog
if Windows' preference "Hide pointer when typing" is
enabled.

______________________
ADDITIONAL INFORMATION
                        
* PGP DOCUMENTATION * 

For information on how to use PGP, including introductory 
material on cryptography in general, please refer to the 
documentation and online help that accompanies 6.0.2.  

The PGP documentation is distributed in Adobe Acrobat 
format (.pdf).  Free readers are available for this 
format on nearly every platform including Macintosh, 
Windows, DOS, SUN Sparc, HP/UX, SGI, AIX, and others 
from the following web site: 

http://www.adobe.com/acrobat/
                 

_____________________________
CONTACTING NETWORK ASSOCIATES

* TECHNICAL SUPPORT * 

We recommend that you read the PGP documentation before 
you contact technical support to see if your issue is 
addressed in the documentation.  

Please send any problems, comments, or suggestions for this
product to our technical support staff:

Phone  Phone (408) 988-3832
Email  pgpsupport@pgp.com  

See our website for further instructions and please have 
your registration number handy.

Enjoy!


The PGP Development Team
