From rushing@niven.ksc.nasa.gov Thu Jun  9 20:38:06 1994
Received: from TITAN.KSC.NASA.GOV by scss3.cl.msu.edu (4.1/4.7)  id AA05256; Thu, 9 Jun 94 20:38:05 EDT
Received: from niven.ksc.nasa.gov by TITAN.KSC.NASA.GOV with SMTP;
          Thu, 9 Jun 1994 20:28:23 -0400 (EDT)
Received: by niven.ksc.nasa.gov; (5.65/1.1.8.2/12Apr94-0752PM)
	id AA22764; Thu, 9 Jun 1994 20:28:20 -0400
Date: Thu, 9 Jun 1994 20:28:20 -0400
From: Samual M. Rushing <rushing@niven.ksc.nasa.gov>
Message-Id: <9406100028.AA22764@niven.ksc.nasa.gov>
Newsgroups: news.software.readers,alt.winsock,comp.os.msdos.mail-news
To: winvn@titan.ksc.nasa.gov
Subject: [ANNOUNCE] WinVN Version 0.91.4 released
Reply-To: rushing@titan.ksc.nasa.gov
Distribution: world
Status: OR

Version 0.91.4 of WinVN is now available from ftp.ksc.nasa.gov (see
below for more ftp info).  Sources and binaries for Win16, NT Intel,
and NT Alpha/AXP are available.

This version fixes many of the problems that people have been having
with the 0.91 series.  There are a few new features, too, including
word wrap in the post/mail edit windows.

There are bound to be bugs in this release.  We are trying to make
0.92 a 'stable' release, but make these interim releases in order to
ease the strain on our mailboxes.  If you are happily using an older
version of winvn, don't delete it to try this one out.

Here's the README:
---------------------------------------------------------------------------
Version 0.91.4

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
	XOVER (InterNetNews is an example of a news server that does XOVER)
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

You can reach (or join) the developers by sending mail to:

winvn@titan.ksc.nasa.gov

(This is not a snazzy list server, it's an alias entry on a VMS Vax.)

An easy way to keep up on winvn developments is to point your
newsreader at 'niven.ksc.nasa.gov' (128.159.4.21).  There should be a
single newsgroup, (mail.winvn) which is a gatewayed version of the
mailing list.  Announcements and discussions of common problems can
be found here.  It's very likely the best place to get the scoop on
_your_ problem.  You can't post to the group (yet), but sending email to
the above address is equivalent.

The official FTP site for WinVN is at ftp.ksc.nasa.gov (128.159.4.20)
in the directory [pub.win3.winvn]

Always check there for the latest binaries and sources.

---------------------------------------------------------------------------

quick list of things undocumented:

o in the group window, middle click will mark an article as read
  double-right click will 'catch-up' to that article (mark all as read)
  shift-lift acts like middle click.  Holding down the mouse and dragging
  works as well.
o forwarding an article with MAPI is not yet supported
o double-clicking on a <message_id@host> in an article will attempt
  to grab that article from the server.  This is a brain-dead feature,
  but can be convenient at times.
o will try to use MAPI unless the winvn.ini param SMTPHost is set.

New documentation is being worked on (to catch up with the many new
features) by Jim Dumoulin (dumoulin@titan.ksc.nasa.gov), and you should
be able to get a pre-release version of it from titan, too.

-Sam

[canned blurb]
The latest version of winvn is always available from
ftp.ksc.nasa.gov, in the directory
[.pub.win3.winvn].
16-bit WinVN is now distributed as a single binary,
with different DLL's for different stacks.

winvn_XX_Y.zip  is version 0.XX.Y.
winvnsrc_XX_Y.zip contains the source distribution.

Some NT binaries can usually be found in
[.pub.win3.winvn.nt]

If you need to use ftp-by-mail, use this path instead:
ripem.msu.edu:/pub/pc/win/winvn/titan.ksc.nasa.gov/

Some NT binaries are also available from shannon.mee.tcd.ie,
in the directory /winvn.






