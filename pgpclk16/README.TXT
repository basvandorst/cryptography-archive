
PGPClick
16-bit and 32-bit versions
Version 2.5
Released: October 6, 1996

written by Robert E. Wilson
Copyright © 1995, 1996 by Robert E. Wilson, All Rights Reserved


Introduction:

    PGPClick is a windows shell for Philip Zimmermann's Pretty Good Privacy
(PGP) program.  PGP is a DOS based program that is run from the DOS command
line using command line switches to modify its actions.  I found use of the
program to be cumbersome from within the Windows environment.  For this
reason, I wrote the PGPClick shell.  It originally was designed to take
whatever text was on the Windows clipboard and encrypt (or decrypt) it,
placing the result back on the clipboard.  It has been extended to be able
to copy highlighted text from a window, to the clipboard, process it, and
paste the result back to the original window, replacing the original text.
This provides close to one-button encryption from most email programs.

    This package includes ONLY the PGPClick program.  It has only been tested
with PGP version 2.6.2.  Others have successfully used it with the inter-
national version of PGP, version 2.6.3i.  You must obtain PGP separately.
It is available from several sites, and can be found by using your favorite
WWW search engine, using the keyword PGP.  The U.S. government has placed
certain restrictions on the export of cryptography programs outside of the
U.S. PGP is affected by these restrictions.  PGPClick is not as it does not
perform any cryptographic functions on its own, but uses PGP to do so.
Please make sure you have a legally obtained copy of PGP to use with
PGPClick.  I accept no responsibility for illegal uses of PGP via PGPClick.


Shareware announcement:

    PGPClick is being distributed as shareware.  It is expected that those
using the program will provide me with some compensation.  In order to be
different, I'm asking for a couple of packs of Star Trek trading cards
(unopened), along with your name, address, and email address (if you really
like PGPClick, send more packs!).  In exchange, I'll notify you of updates
when available, and give priority to making any changes you feel are
helpful.  Since some people may prefer to send money rather than go out
and buy some trading cards, I'll also accept $5 (U.S.).


Warranty information:

    At the time of release, there are NO bugs that I am aware of in PGPClick.
This does not mean that none exist.  I will try to fix any bugs found as
I am able, but can NOT guarrantee that they will be fixed, or fixed quickly.
This is not my main job, and I only have one system configuration on which
to test the program.  I will, however, try my best to make the  program as
stable as possible within my resources.

    PGPClick comes with NO WARRANTIES OF ANY KIND.  It is to be used at your
own risk.  I (Robert E. Wilson) assume NO RESPONSIBILITY FOR ANY LOST DATA
OR OTHER DAMAGES CAUSED BY THE USE OF PGPClick.


System requirements:

    The 16-bit version (PGPClick.exe, found in the distribution pgpclk16.zip)
requires Windows 3.1 or Windows for Workgroups 3.11.

    The 32-bit version (PGPClick32.exe, found in the distribution pgpclk32.zip)
requires Windows 95. It should also work with Windows NT (not tested).

    The PGP program itself is NOT included with PGPClick.


Installation:

    Before running the PGPClick instllation program (setup.exe), you may need to
install the Visual Basic 4.0 runtime routines.  The files can be found in the
archives vb40rt16.zip (16-bit runtimes) or vb40rt32.zip (32-bit runtimes).  To
install these files, simply unzip them and move the contents to your windows\system
directory.

    The Setup.exe program will install PGPClick and necessary files into
the directory of your choice.  Before running PGPClick, it is recommended
that you have already installed PGP and checked that it is functioning
properly.  Please follow the PGP documentation for instructions on installing
PGP.  Make sure the PGP.INI or CONFIG.TXT file has been editted to specify your
UserID in the "MyName" entry and that your public key ring is identified in the
"PubRing" entry.

    To install PGPClick, first unzip the pgpclk??.zip file into an empty directory.
If installing the 32-bit version, please use a 32-bit unzip program, otherwise you
will need to rename the file "PGPClick.ex_" to "PGPClick32.ex_".  Once the files
are unzipped, run the setup.exe program and follow the prompts.

    Once the PGP is installed and working, and PGPClick is installed, the
program can be run.  The first time PGPClick is run, go to the Preferences
dialog in the Help menu and specify the location of PGP.EXE and the path of
a temporary directory (found in the "Paths" tab of the Preferences dialog).

    The file PGP.PIF should be moved to the directory in which PGP.EXE resides.
It will inform Windows to run PGP in a window and not exit when PGP completes
running.  This allows any errors returned by PGP to be viewed.

    Further information on using the program is found in the help file.  In addition,
I have set up a web page listing helpful hints, and any bug notifications and fixes
as they are found.  Point your browser to:
        http://www.ncinter.net/~rewilson/pgpclick.html.

    Please see the separate file, uninst16.txt (16-bit version) or uninst32.txt
(32-bit version) for a list of files installed by PGPClick.


Upgrading from an earlier version of PGPClick:

    There are two ways to upgrade to version 2.5.  The first is to download either the
16-bit upgrade file (pclkup16.zip) or the 32-bit upgrade file (pclkup32.zip).  Unzip the
archives and copy the tabctlxx.ocx (xx=16 or 32) to your windows\system directory.  The
remaining files replace your older PGPClick.exe and PGPClick.hlp files.  This method can
only be used when upgrading from PGPClick version 2.0.

    The second way to upgrade is to do a complete new re-installation using the full
PGPClick distribution package.  The following paragraphs describe this procedure.

    To upgrade the 16-bit version, simply perform the installation as noted above.
When upgrading the 32-bit version, the setup program does not always properly remove
the "Uninstall PGPClick" entry from the Windows 95 "Add/Remove Programs" list, resulting
in two "Uninstall PGPClick" entries (one from the old version and one from the new.  To
avoid this problem, first uninstall the older version, then install the new version as
described above.  I do not know if this problem only effects Windows 95 installations, or
if Windows NT installations are also effected.

    Some earlier versions of PGPClick were known as PGPClip.  As there were several
different programs available with that same name, I changed the name of my PGPClip to
PGPClick in order to reduce confusion.  If you are upgrading from my older PGPClip
program, you will lose the preferences you had set in PGPClip.  In order to restore
them, do one of the following:

16-bit/Windows 3.1: Rename the PGPCLIP.INI file in your windows directory to PGPCLICK.INI

32-bit/Windows 95: Rename the "HKEY_CURRENT_USER\Software\VB and VBA Program Settings\pgpclip"
key in your registry to "HKEY_CURRENT_USER\Software\VB and VBA Program Settings\PGPClick".
Note that case IS significant.  Only attempt this if you are proficient in editing the registry
AND have made a backup first.  It will be much quicker to just re-enter your preferences in
PGPClick than to restore your system from a trashed registry!

-------------------------------------------------------------------------------

Registration:

Please send the following information to me at:

     Robert E. Wilson
     4729 Oakbark Ct.
     Erie, PA 16506

     email: rewilson@ncinter.net



Name (First M.I. Last):_____________________________________________________

Address:____________________________________________________________________

        ____________________________________________________________________

City:______________________________  State:_______  Zip Code:_______________

Email address:______________________________________________________________


Payment enclosed:   __ 2 unopened packs of Star Trek trading cards
 (select one)       __ $5 (U.S.)

Version of PGPClick Used:     __ 16-bit            __ 32-bit
