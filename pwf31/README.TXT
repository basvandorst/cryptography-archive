-----BEGIN PGP SIGNED MESSAGE-----

IMPORTANT!!! PLEASE READ THIS! 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PGP WinFront 3.1 Readme File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*** Read this file BEFORE installing PGP WinFront. It
contains important information not found in the help
files or in other readme files included in the package.

	This file contains three sections:

		MISC
		NEW FEATURES NOT IN HELPFILE
		LAST MINUTE NEWS


MISC
~~~~

- - Revision Number

	The revision number of PGP WinFront in this package is: 3.1.0 
	[initial release of version 3.1]

	The filename, however, is still PWF30.EXE.

- - Assumptions

	All the documentation assumes that:
		- You have the PGPATH environmental variable set
		- PGP.EXE is in your path
		- You use the supplied PIF files
		- You are aware of the differences between
		  PGP 2.3a, MIT PGP 2.6, and PGP 2.6ui
		- You are aware of the fact that that PGP WinFront
		  has SOME features which can only be used with versions of
		  PGP that conform to the MIT PGP 2.6 feature set.
		- You are aware of the fact that PGP WinFront does not do
		  any PGP.EXE version checking, with the exception of 
		  reading in the PGP Public Keyring. This is because 2.3 and
		  2.6 display different keyring information.

- - My Public Key 

	You can get my public key by sending email to me at
	rbarclay@trentu.ca. Your SUBJECT MUST READ:

		GET KEY

	You should get the key within minutes of placing your
	request.

- - Security

	PGP WinFront is distributed in a zip file called PWF31.ZIP.

	Inside this zip file is another ZIP file, called PWF.ZIP, and
	the signature file for PWF.ZIP called PWF.SIG.

	After you run setup, another signature file will be
	decompressed. This file is called PWF30.SIG and it is the
	signature for the decompressed file PWF30.EXE

- - Installation
	
	To install PGP WinFront, run setup.exe. YOU NEED VBRUN300.DLL!

	Please don't email me asking for VBRUN300.DLL. Its on virtually 
	every major FTP site. On FTP.CICA.INDIANA.EDU it SHOULD be in 
	\pub\pc\win3\utils as VBRUN300.ZIP. Also, you can use archie 
	or veronica to locate it.

- - Installation Problems

	If the setup program chokes on a VBX (for example THREED.VBX),
	do the following:

	1) Exit Windows
	2) delete the VBX from your system directory
	   ie: del c:\windows\system\threed.vbx
	3) Load windows and re-run PGP WinFront Setup.

	(if you have a newer version of the VBX, then don't delete it,
	just rename it and copy it over the one that PWF will install)	

NEW FEATURES NOT IN HELP FILE
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- -       Encrypt To Self

	With this option selected, PWF will automatically encrypt 
	everything with your public key as well.

- -       Use MIT PGP 2.6 Legal Kludge

	With this option selected, MIT PGP 2.6 will continue to generate 2.3 
	compatible files after September 1st, 1994

- -       KeyRing View Moves Windows

	With this option selected, PWF will readjust your screen to fit the 
	main PWF window side-by-side with the keyring window.

- -       Secret KeyRing Location

	If your secret keyring is located anywhere but the default location
	(where PGP is kept), for example on a floppy disk, you can take 
	advantage of this feature. Click in the Sec. Keyring Loc. Box and 
	locate your secret key ring. TO CLEAR THIS BOX: click in it with 
	the RIGHT mouse button. You will then see "<default>" in that box. 
	This means that PGP will use the default secring.pgp file.

	This feature uses the "+secring=" command line paramter. 
	This command line parameter may not be available in PGP 2.3, in
	which case this PGP WinFront feature will not be available.

- -       Public Keyring Location

	You can also specify an alternate Public KeyRing Location.
	This keyring will be used by PGP and also will be the keyring
	read in when you click "KeyRing".

	This feature uses the "+pubring=" command line paramter. 
	This command line parameter may not be available in PGP 2.3, in
	which case this PGP WinFront feature will not be available.

- -       Alternate Public Keyring

	It is possible for you to set up what is known as an "alternate
	public keyring." It is expected that this keyring will be used
	only sparingly, and will probably be very large, for example, the
	master public keyring available on many FTP sites. These large
	keyrings are handy to have, but they slow down PGP most of
	the time because of their large size.

	In any event, when you press "Keyring", you will notice just 
	above the "Refresh Keyring" button that there is a section with
	a check box and a text box. When you click in the text box, you
	will be asked to select an alternate keyring. PWF will then read 
	the contents of that keyring in. When you click the check box 
	"Use Alternate Pub. Keyring", PWF will switch its key list over
	to the alternate keyring. From then on, every action in the
	keyring viewer or on the main screen, ie: encrypt, key view, 
	etc... will use the alternate keyring. 

	To remind you of this, the text right above the parameter edit 
	box will change to say "Parameter Edit Box : [Using Alternate
	Public Keyring]" when the alternate keyring is active.

	To return to using your default keyring, simply deselect the
	"Use Alternate Pub. Keyring" check box.

	The name of your alternate keyring will be saved from session to
	session, however, the "Use Alternate Pub. Keyring" setting will
	not be. This is because it is assumed that users will generally
	want to use their default public keyring most of the time.

	This feature uses the "+pubring=" command line paramter. 
	This command line parameter may not be available in PGP 2.3, in
	which case this PGP WinFront feature will not be available.

- -	Wiper

	A new option on the Options screen allows you to specify a
	Wiper. This file will be activated when you press the Delete
	button instead of the regular erase only DOS delete feature.

	The internal representation of the Wiper function is:

	wiper.exe <name of selected file>

	ie: wiper.exe c:\myfile.txt

	where "wiper.exe" is the file you have defined as your wiper.

	*** I won't be held responsible for improper wipes, whether
	or not you follow my instructions ***

- -	Quick Access

	This feature allows for simple macros to be created. Click 
	the QA button and inspect the sample macros for an 
	idea of how these work.

	When you first run PGP WinFront, it will offer
	to install some sample macros. You should click
	YES.

	However, to see these macros, you first need to 
	close PGP WinFront, and then reload it. Then,
	click QA (Quick Access).

- - 	History

	PWF keeps a history of the command lines that you have run
	during the current session. See the History section of QA
	for information on how this works and on how to disable it
	if you want to.

- - 	Using PWF.INI instead of WIN.INI to store config info

	You can copy the PGP WinFront section of the WIN.INI 
	(including the header [PGP WinFront]) and save it in a file
	called PWF.INI in the directory where PWF31.EXE is located.

	Then you can remove the PGP WinFront section of WIN.INI.
	This will not work unless you store the file properly in
	the same directory as the executable.

LAST MINUTE NEWS
~~~~~~~~~~~~~~~~

- -       When you select input from clipboard, the input file will ALWAYS be 
	wiped, and the clipboard will be cleared only if you have chosen
	"Wipe Input File"

- -       If you repeatedly get an "Error creating Keyring" message,
	ensure that your PGPPATH variable is correct and that PGP is
	in your path. The PGPPATH setting is only the PATH to PGP, NOT
	the PATH and EXE name of PGP. 

- -       If your machine locks up after you exit your editor after choosing
	"Create", try using a different editor. Some users have had problems
	with DeskEdit, but not with Notepad, for example. If that still 
	doesn't work, send me email and TURN OFF the "Highlight PGP Created 
	Files" Option.

- -       If you find a bug, let me know!!! 

- -       I'd really like to hear your comments! I sometimes cannot answer my 
	mail every day (or every week), but I WILL respond.

- -       Read the help file completely! I like to answer questions from users 
	but please look for the answer in the help file! If some section 
	is not clear, please email me and I'll fix it in the next version.

- -       Please upload this program to your favourite FTP site or BBS!
	Compuserve, AOL, Genie and Prodigy users: Please upload this
	program yo your service as I don't have any accounts there. Users
	of all these services have access to Internet email so they can
	all email me for assistance if they need it.


- --ross barclay

-----BEGIN PGP SIGNATURE-----
Version: 2.6.1

iQBVAwUBLxxpVdgpRteEZ9JhAQFQ6QH+N51BtcVf1kAFkCRKOrNUci0DYVHJsrSi
LRhRcPXhtomlyWWuclOas39wUT/RsiR1LnVf8ZUz15WQJ01hqctAlw==
=+3+0
-----END PGP SIGNATURE-----
