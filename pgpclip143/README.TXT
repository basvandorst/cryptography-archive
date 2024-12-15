PGPClip by Michael Meyer
========================

Disclaimer:
===========
This program is free.  Use at your own risk.

What it is:
===========
PGPClip is a simple front end to PGP which uses the clipboard for its
operations.

It is not a full blown replacement for many of the other Windows based
front ends for PGP, but one which allows quick encryption, decryption,
and signing using the clipboard.


Example of use, Mail Signing:
=============================
Type your mail message in Eudora or other Windows based program, from
that program's menu choose "Edit|Select All", then "Edit Cut".  This
will cut your text to the clipboard.

Then activate PGPClip, check the "Sign" checkbox, then click the
Encrypt/Sign button to sign the message.  PGP will appear in a DOS box,
enter your pass phrase, switch back to your mail package then choose
"Edit|Paste", voila, your signed message will appear.

Or, use PGPClip's clipboard editor, not as fancy as a real editor, but
quick.  Clear the clipboard, from PGPClip, bring up the clipboard
editor, type in your message, choose ok, sign/encrypt, then go to your
mailer and paste in the signed/encrypted text.


Distribution:
=============
PGPClip releases are posted in:

    Newsgroup:      alt.security.pgp

    CompuServe:     NCSAFORUM, lib: ENCRYPTION

Send me mail for the latest version. Please send your current version
number so that I can avoid sending you a duplicate.  Send your message
in the following format:

Ex.
    mailto: mmeyer@oacis.com

    I have version 1.3.3.
	send PGPClip mailaddress


Other:
======
Check the "Encrypt" checkbox to encrypt your message.

Check both to encrypt and sign, to sign and encrypt.

Check Show Names to show the list of keys, click on a key to toggle its
selection.  If no keys are selected, PGP will prompt you for recipients,
for encryption, if encrypt is checked.

Type in the name of an alternate keyring in the text box next to the
"Get Names" button to override PGP's default keyring.

If you select too many user keys for encryption, or the path to your
alternate keyring is too long, the command line to PGP could exceed 127
characters, a DOS limit, causing a VB error when you choose an action.

PGPClip is written in Visual Basic 3.0 and requires VBRUN300.DLL,
GRID.VBX, CMDIALOG.VBX, and THREED.VBX.

VB Runtime files can be found at ftp.cica.indiana.edu as:
    (these are the updated versions)
    /pub/pc/win3/patches/threed.zip
    /pub/pc/win3/programr/vbasic/grid.exe
    /pub/pc/win3/util/vbrun300.exe

Updated versions are also available at ftp.microsoft.com as:
    /softlib/mslfiles/VBRUN300.EXE
    /softlib/mslfiles/VBGRID.EXE
    threed ?

Note: PGPClip uses the updated GRID.VBX with the following attributes:

    Date: 15-JUNE-1993
    Time: 5:26 p.m.
    Size: 45136
    Version: 03.00.0538


If you should get out of string space errors, try to reduce the amount
of text you are encrypting/signing.  You have run into a VB String
limit.


INI Settings for PGPClip in PGPCLIP.INI:
========================================

[PGPClip]                 ; Section for PGPClip settings

    Top=5250              ; Top and left in TWIPS for PGPClip's location.
    Left=4725             ;

    KeyTop=5250           ; Top and left in TWIPS for PGPClip's location.
    KeyLeft=4725          ; With keys showing.

    KeyRing=              ; Override PGP keyring.  If blank, use PGP keyring.

    Language=English      ; Look in section called [English] for message
                          ; text.  The language section must exist.

    ReadKeys=1            ; 1 (default) will read keyring at startup,
                          ; 0 will cause PGPClip to skip reading keyring.

    ShowKeys=0            ; 1 will cause the list of keys to show up
                          ; at start.
                          ; 0 (default)

    PauseAfter=1          ; 1 (default) will cause PGPClip to pause after
                          ; sign/encrypt operations.

    Sign=1                ; 1 (default) will cause PGPClip to start with
                          ; the Sign check box checked.

    Encrypt=0             ; 1 will cause PGPClip to start with the Encrypt
                          ; check box checked.  0 (default)

    pgpVersion=2.6        ; "2.6" (default)
                          ; "2.3" For 2.3/2.3a formatted keys.
                          ; Note: If you have PGP 2.3a set "pgpVersion=2.3".

    PGPExe=c:\apps\pgp    ; "" (default) Search PATH for PGP.EXE.
                          ; Specify the full filespec to PGP.EXE if it
                          ; is not in your PATH, or if it is not
                          ; called PGP.EXE.
                          ; The trailing .EXE is not required.

    SaveSettings=0        ; 1 (default) will cause PGPClip save Top, Left,
                          ; at exit.


[English]                 ; Section for language messages.
                          ; See provided PGPCLIP.INI for msgkeys and text.

    msgkey=message text.  ; This provides the text for messages in PGPClip.
                          ; Use the english as a basis for your messages.

[xxxxFont]                ; Used to specify font properties for controls.
FontBold=0                ;
FontItalic=0              ; The following sections are used: [GridFont]
FontName=MS Sans Serif    ; [ClipFont] [ButtonFont] [LabelFont].
FontSize=8.25             ;
FontStrikethru=0          ;
FontUnderline=0           ;


Make sure that you have set "MyName=your name" in PGP's config.txt, or
else PGP will use the first key in your secret keyring for signing.

You can also type in your keyid or user id in the "Sender" text box to
override PGP's default.

Please mail comments or suggestions.

If you have created non-english language messages, feel free to mail
them to me for inclusion into the distribution.

If they contain 8-bit characters send them in ASCII armor. MS Sans Serif
is the font for textboxes and labels.  Be sure to use characters from
the Windows ANSI character set, not the IBM PC character set.

You can credit yourself for the language section by entering message
text in the [language] section under the key: "ABOUT_XLATE_TXT"

Ex:
[English]
  ABOUT_XLATE_TXT=English translation by John Smith <jsmith@pocahontas.com>



Changes: 1.4.3
==============
Added ability to set font properties for: Button, Label/Text, Clipboard
Editor, Key Grid.  See PGPClip.ini for [xxxxFont] section for examples.

Added error message: SETFONT_ERR

Added new strings for new features:
    CNV_ENC_TXT=Encrypt Conventionally

Added conventional encryption option.

Fixed overflow bug on close of iconized window in win95.

Made sure that the PGPClip startup dir is set to be the current dir
before performing pgp operations.

Added french messages.


Changes: 1.4.2
==============
Fixed some stuff with the language strings.

Added new strings for new features:
    EXTRACTKEY_CMD_TXT=Extract Key
    ADDKEY_CMD_TXT=Add Key
    DELKEY_CMD_TXT=Delete Key
    SAVEAS_CMD_TXT=Save As...
    OPEN_CMD_TXT=Open...

I don't have translations for these, please send if you have
translations for these.

Added Add Key from clipboard.
Added Extract key to clipboard.
Added Delete key.
Addes Open and Save As to clipboard editor.


Changes: 1.4.1
==============
If the key is prefixed by "pub@" treat it as the start of a new
keyblock.  This is a public key that has been disabled.

Added Drag and Drop support.  From the file manager drag a text file to
either the clipboard edit window, or the main window, and the text of
that file will be placed into the edit window, and clipboard.

Minor editing of readme.

Minor changes in generating distribution.

Show command line and length if too long.

Added German messages.

Added Spanish messages.


Changes: 1.4
============
Distribution is now in PGP ASCII armor.

Clipboard Edit window should be properly positioned for use on a
VGA (640x480) screen.

PGPClip reads PGPCLIP.INI from the startup directory, then reads
PGPCLIP.INI from the windows directory.  Values from the windows
directory override settings from the startup directory.  Why?  Well this
allows PGPClip to be put on a fileserver, with defaults settings,
allowing individual users to customize PGPClip to their liking.

Added support for different keyrings.

All strings are configurable, even for labels and buttons. For use with
foreign languages.

PGPCLIP.INI contains the message text, it must exist in the startup
directory.

Added .INI entries for defaults for check boxes, see PGPCLIP.INI
description.

PGPClip stretched to allow text to fit in better, mainly for non-english
users, where messages could be longer.

Fixed fluff in icon.


Changes: 1.3.4
==============
Fixed Resize bug, if text edit windows is too small.
Added temp.pif, pif file for PGPClip operations.


Changes: 1.3.3
==============
Added a text editor for clipboard text, allows editing of text on
PGPclip.

Check to see if initial dimensions are on the screen, if not adjust.

Added copyright notice to sources.


Changes: 1.3.2
==============

Added a clipboard clear, before setting text on the clipboard. Other
formats were still staying there unchanged.   I.E., Write stores several
formats onto the clipboard and prefers its own, such that it would not
use PGPClip's text, but its unchanged preferred version of the text.

PGPClip is now slapped into one file for distribution, containing a
uuencoded Zip file, my public key, and some instructions.

Added option to skip reading of keyring at startup, useful if you only
use PGPClip for signing/verifying signatures, saves startup time.

    [PGPClip]
    ReadKeys=1            ; 1 (default) will read keyring at startup,
                          ; 0 will cause PGPClip to skip reading keyring.


Changes: 1.3.1
==============

Added keyword "pgpVersion" to [PGPClip] section.  This enables special
processing for 2.3 formatted keys, maybe even 2.3a.

Valid options are "2.3" or nothing.

    [PGPClip]
    pgpVersion=2.3        ; for 2.3 formatted keys


Changes: 1.3
============

Changed Sign and Encrypt to be check boxes, allowing signing and
encrypting.

If neither is selected, the action will be to store the text in
compressed ascii armor.

Consolidated file io routines.

Changed file io routines to allow for larger files, reducing out of
string space messages.

PGPClip will remember its window locations.

PGPClip will move its window if the names appear off of the edge of the
screen.

The location of PGG[.exe] can be specified in win.ini, under PGPClip,
PGPExe.  The .EXE is not needed.

    PGPExe=c:\apps\pgp    ; This is for c:\apps\pgp.exe.  Use when PGP.EXE
                          ;   is not in your path, or if it is not called
                          ;   PGP.EXE.

Allow only one instance of PGPClip, if it is already running, switch to
it.


Changes: 1.2
============
Main screen simplified and made smaller.

Added option to pause after PGP execution for signing and encrypting.
Pause is always enabled for Decrypting/Verifying.

Added more error handling.


Changes: 1.1
============
Stabilized code, ready for release.

Changes: 1.0
============
Internal development, not released.


==========================================
Michael Meyer: mmeyer@oacis.com
KE6LPL - 5.0L - I am the NRA, and I vote !
Mail or finger for PGP public key.

-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: 2.6.2

mQCNAi4jS0MAAAEEAKP1rjPM+6e7+9FPKrtR1ZDOShJFxZk7ugJhXv3OKJcJLvZF
U5OUTH1Rna/W9qSz0Ns9FWpiBNjKPooZo8oc0sHaxen5WcDX1hS2hYmCeDDduss4
b1XdpbAn39XW3S8DVPETvT1tGN+clHgvfblY1t91UnLp+KxVK+SKDX+aaQfFAAUR
tCNNaWNoYWVsIFAuIE1leWVyIDxtbWV5ZXJAb2FjaXMuY29tPrQsTWljaGFlbCBQ
LiBNZXllciA8NzIzMjEuMzAyN0Bjb21wdXNlcnZlLmNvbT4=
=7PdH
-----END PGP PUBLIC KEY BLOCK-----

