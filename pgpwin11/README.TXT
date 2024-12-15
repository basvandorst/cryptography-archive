First, a disclaimer:
I use these progs and enjoy them. You want them? be my guest.
For all you know, these things might be trojans (read the PGP
docs for details). You wanna be sure? check & recompile the source code
(pretty short). The .EXEs are signed by me (*.asc) but I'm not responsible
for my deeds (I was drunk that night).
I'm certainly not responsible for YOURS.
Also (while you're at it), check *.BAT for dirty tricks.

Feel free to comment, etc.

Nimrod Shabtai Kerrett (Abu-Zeresh)                 (.~.)   
Work:[+972-3]6963277 Fun:[+972-3]810253 +----------w--U--w--------|= :
home: zeitgeist@attmail.com             | Power to the people doesn't
TAU:  zzzen@math.tau.ac.il              | come with a standard plug...
Fax: [+972-3]6965645 ATTN Nimrod       ///

-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: 2.3

mQBNAiuzMUAAAAECAK/7vgi1eyendidcWVHqRLasBYX7L/4SwDVY4RRxwvZpfJHW
uF1DMFuuI4D9UnjQXTR5BGiOGq9XDcTGg2Ml/XUABRG0Lk5pbXJvZCBTaGFidGFp
IEtlcnJldHQgPHplaXRnZWlzdEBhdHRtYWlsLmNvbT60IUFkZHJlc3Mgc3ViamVj
dCB0byB2aW9sZW50IGNoYW5nZQ==
=GdTW
-----END PGP PUBLIC KEY BLOCK-----


===============================Contents===================================

This package contains 2 Visual-Basic tricks to help you forget the DOSy
nature of PGP and make you feel like an end user. 

There are 2 programs:
pgpc.exe - the composer [ needs pgpcomp.{bat,pif} & pgpkeys.{bat,pif}]
pgpr.exe - the reader   [ needs pgpread.{bat,pif}]

Other files in the zip:
*.asc - signatures on .EXEs
*.frm,*.mak - VB source code for the paranoid (not documented, but short).

You'll also need: PGP (pgpwin was tested with 2.3)
and VBRUN100.DLL [e.g. pub/pc/win3/util/vbrun100.zip@ftp.cica.indiana.edu]

===============================PGPC.EXE (Ver 1.1)=========================
New for 1.1:
- Bug fix in "ReRead Keys"
- Added "Sign As" text field (because my wife now has her own key)

This is a front end for composing a short ASCII-armored pgp message
The window consists of:

Text-pad: That's where you write the plain-text

To-field: You can write name(s) of recpient(s) here
          NOTE: if the list is empty, no encryption will take place, but
                other things you asked for will happen (e.g. signature,
                eyes-only) If you asked for nothing, you still get ASCII-
                armouring of your ASCII message (can squeeze a few laughs
                at the NSA secret policeman's ball).

To-list:  List of public keys found in your pubring.pgp
          Double-click on a name and it will be added to the To-field:
          It will look like a funny hex number, but don't worry -
          that hexnum IS your mate (as mystic as it sounds).
          This list is generated the first time you run pgpc, or
          if you press the "ReRead Keys" button.

Sign [as]: Sign your text (you will be prompted for password by
           the PGP dos-window).

Sign-as-field: If you have more than 1 key in your secring (e.g. if you're
               married to a PGPer), you can specifiy "sign-as-who" in the
               text-field below. If you don't, default is assumed
               (i.e. last secret key added).

Eyes-Only: Recepient won't be able to save to file (only view)

ReRead-Keys: Will reread your pubring (use it if you've added some keys).
             NOTE: If you delete the file "pgpk$tmp.$$$" before invoking
                   PGPC, you'll get the same result.


Go:       Saves text to temp file, runs PGP on it in a window
          (original file will be wiped since the -w switch is always
          used). The encrypted file is created, and notepad is
          invoked on it. PGPC exits and the rest is up to you:
          copy & paste, save as file, whatever.

NOTE!
=====
 It is assumed that:
- pgpcomp.pif, pgpcomp.bat, pgpkeys.pif & pgpkeys.bat are in the path
  (or current dir)

- pgp.exe is in the path and PGPPATH is set correctly:
  - If you don't want pgp in your path, edit pgpcomp.bat & pgpkeys.bat
    and enter the full path for pgp.exe
  - If you don't understand what PGPPATH is, READ THE PGP MANUAL!
    (as PRZ keeps saying)

==============================PGPR.EXE (still 1.0)=========================
            
This is so simple it's almost stupid:
This program accepts a filename as argument and launches notepad.
It starts searching in the bacground for "-----BEGIN PGP" in the text.
If found, pgpread.pif is launched with the file
(pgpread.bat simply runs "pgp -m %1").

Uses:
1) Some offline mail-readers (e.g. cinetic mail-manager) let you configure
   which prog should be invoked in order to read a message: (default is
   notepad.exe) So you simply put pgpr.exe instead: You'll alway get notepad,
   if needed - you'll also get pgp [-m] in a window.
2) You can associate *.TXT and/or *.ASC with pgpr.exe in the file manager.

You can check pgpr.exe by running it on THIS readme file.
This way you can add my key to your pubring.
Also (recommended), use it on pgpc.asc and pgpr.asc to verify no one
tampered with the .EXEs (But if you're paranoid, maybe the pubkey
above is not mine and this is all a trick. You'd better check &
recompile the source code).

NOTE!
=====
 It is assumed that:
- pgpread.pif & pgpread.bat are in the path (or current dir)

- pgp.exe is in the path and PGPPATH is set correctly:
  - If you don't want pgp in your path, edit pgpread.bat and enter the
    full path for pgp.exe
  - If you don't understand what PGPPATH is, I'm terribly sorry ;)
