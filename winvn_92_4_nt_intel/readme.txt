Friday Sep 2, 1994 8:30 pm

Version 0.92.4

This is an NNTP newsreader for Microsoft Windows 3.1 or Windows/NT.  You
can use it to read and post Usenet News, and send email (via SMTP or MAPI)

Requirements:

For WINSOCK (v1.0 or higher) compliant TCP/IP stacks (called WINSOCK.DLL):
(using gwinsock.dll)
   A WINSOCK compliant TCP/IP product such as Trumpet, PC/TCP,
   NetManage TCP/IP, Microsoft LanManager 2.2, DEC Pathworks 5.0.
   Make sure WINSOCK.DLL is in your path before starting Windows.

   *or*

For NON-WINSOCK TCP/IP stacks:
(using pwksock.dll)
   A DEC Pathworks v4.1 or Microsoft LanManager 2.1a compatible
   WSOCKETS.DLL Dynamic Link Library.  Make sure Windows can find it.
   (set your PATH environment variable).

   An NNTP News Server.  It *must* support one of the following
   NNTP extenstions:

	XHDR (most news servers can do this)
	or
	XOVER (InternetNetNews is an example of a news server that does XOVER)
   WinVN will use XOVER if your server supports it.

   A 'NEWSRC' file.  An example is provided.  You *must* have the
   file there; WinVN won't write one out just randomly.  It can be
   empty.

   A 'WINVN.INI' file.  Edit this file to point to your news server,
   or you can later change parameters from within WinVN.

   An optional 'WINVN' environment variable, telling WinVN where to find
   its two configuration files: 'WINVN.INI' and 'NEWSRC'.  You have to
   set this variable BEFORE you start up windows.

   You can also tell WINVN where to find the two files by giving
   command-line options with the 'Properties' menu under the Program Manager
   - here are a couple of examples
	(specify a directory to look in)
	winvn.exe d:\winvn\myserver
	(specify each of the two files separately)
	winvn.exe e:\winvn\myserver\winvn.ini u:\users\rushing\newsrc
   [I use this to set up different icons for different news servers.]

The on-line documentation no longer matches the program exactly.
Lots of new undocumented features have been added, but most should
be obvious.

---------------------------------------------------------------------------

Thanks to Mark Riordan for the original WinVN!

Many people have worked on WinVN - it's been a fun, international
project.

Here are a few of them:

	Matt Bretherton		MBretherton@cmutual.com.au
	Harvey Brydon		brydon@tulsa.dowell.slb.com
        John S. Cooper          jcooper@netcom.com
	Michael Downs 		mdowns@eos.arc.nasa.gov
	Jim Dumoulin 		dumoulin@titan.ksc.nasa.gov
	Michael Finken		finken@conware.de
	Don Gardner		gardnerd@keystone.ksc.nasa.gov
	Jody Glasser		jglas@tdycont.com
	Masaki Ishido		ishidou@yhp.hp.com
        Conor Nolan             cnolan@tcd.ie
	Mark Riordan		mrr@scss3.cl.msu.edu
	Sam Rushing		rushing@titan.ksc.nasa.gov

	(countless others I don't really mean leave out)

You can send comments to the WinVN developers by sending mail to:

       winvn@titan.ksc.nasa.gov

If you don't mind alot of traffic on WinVN, you can join the developers 
by sending mail to:

       winvn-request@titan.ksc.nasa.gov

(This is not a snazzy list server, it's an alias entry on a VMS Vax.)

An easy way to keep up on winvn developments is to point your
newsreader at 'niven.ksc.nasa.gov' (128.159.4.21).  There should be a
single newsgroup, (mail.winvn) which is a gatewayed version of the
mailing list.  Announcements and discussions of common problems can
be found here.  It's very likely the best place to get the scoop on
_your_ problem.    If mail gets much worse, we may consider starting 
an ALT group.

The official FTP site for WinVN is at ftp.ksc.nasa.gov (128.159.4.20)
in the directory [pub.win3.winvn]

Always check there for the latest binaries and sources.

Matching hardcopy documentation by Jim Dumoulin (dumoulin@titan.ksc.nasa.gov)
for this release is available on FTP.KSC.NASA.GOV.

-Sam Rushing & Jim Dumoulin

--------------------------------------------------------
Improvements and new features in Version 0.92.4
  (martin/smr)	bug fixes in hashtable stuff, now _really_ should work
                with lots of groups.
  (mrr)		update to online help

Improvements and new features in Version 0.92.3
This is mostly a bug-fix release.

  (jmartin)	new newgroup dialog, more robust with lots of groups
  (jcooper)	encoding/decoding bugfixes, understands more encodings
  (mrr)		new authorization-control capability
  (smr)		various winsock fixes
  (jg/mi)	bogus # of articles in dialog caption fixed

Improvements and new features in Version 0.92.2

  (smr)         fixed problem with determining NNTP vs port number
  (jd/smr)      fixed problem with Gensock not returning error strings
  (jd/finken)   fixed problem with estimated num of articles getting zeroed.
  (jd)          fixed conflicts with Accelerator Keys
  (jd)          fixed up strict function specs in WVUTIL.C and WINVN.C
  (jcooper)     decoder now understands WinCode, TIN, EnUU, POST etc
  (jcooper)     Attachment now allows disabling of MIME
  (jcooper)     New attachment dialog
  (jcooper)     Customizable experimental MIME types
  (jcooper)     Menus in group/article windows are disabled when retrieving
  (jcooper)     fixed coding window minimize problem fixed finally
  (jcooper)     fixed close-window bugs and cleaned up
  (jcooper)     fixed util/encode bug
  (cnolan/smr)	fixed Auto-Execute for Win32
  (smr)		better handling of nntp server disconnect
  (smr)		ensure QUIT command is sent to server on exit/disconnect

---------------------------------------------------------
Improvements and new features in Version 0.92.1

  (smr)		simpler and more reliable winsock support
  (smr)         fixes to support Microsoft Chicago Beta
  (smr)		more robust SMTP send code 
  (smr)         removed debuging code for SMTP 250 messages
  (smr)		better handling of long header lines with word wrap
  (gardnerd)	copy to clipboard from article windows
  (cnolan)	fixes for win32, bugfixes
  (jcooper)	auto-execute of decoded files (via file manager/associate)
  (pearse)	show only articles newer than last read
  (jd)          fixed additional printing problems with some drivers
  (jd)          made version info box a scrollable dialog
  (jd)          fixed startup errors for people that have no printers
  (jd)          fixed crashes upon new connects to servers >3000 new groups
  (jd)          made nntpserver and mail address scrollable for long names
  (jd)          fixed improper casting for more reliable operations
  (jd)          standardized border on all WinVN dialog boxes
  (jd)          fixed bug with truncating long mail addresses
  (jd)          fixed improper cleanup on canceling Threshold dialog
  (jd)          changed look and feel of article Threshold dialog box

------------------------------------------------------------
Improvements and new features in Version 0.91.6

  (JD)   Printing bug introduced in V0.91.5 is fixed

  (JD)   New Printing Dialog Box with support for printing individual
         pages, multiple copies, and changing default printer.

  (JD)   Added a Printer Setup Entry in Article Window

  (JD)   Gensock include code fixed in WinVN.RC so the App Studio 
         can now be brought up for developers without errors.
         Also, identifiers are now in sorted order in WinVN.h.

  (JD)   Added an option in the Article View Menu for Find Next Unseen

  (JD)   Added the beginnings of a "POWER" button.  You can now
         press SPACE while reading an article and it will scroll
         down the page.  If at the end of the page, it will fetch
         the next unread article in the selected group.   In a future
         release, it will also be made to switch to the next unread article
         in the next subscribed group.  The F2 key can also be used.

  (JD)   Fixed Save on Exit to read the Confirm Save on Exit flag.

  (JD)   Changed Menu text of the Quit command to Quit without Saving.

  (SMR)  Bug in Mail forwarding is fixed

  <DG>   Horizontal Scrolling is now by pixels instead of Chars

