Thu Jun 10 22:44:16 1993

Version 0.80

This is an NNTP newsreader for Microsoft Windows 3.1.  You can use it
to read and post Usenet News.

Requirements:

(winvnstd.zip)
   A WINSOCK compliant TCP/IP product.  Make sure WINSOCK.DLL is in
   your path before starting Windows.

   *or*

(winvnpwk.zip)
   A DEC Pathworks or Microsoft LanManager compatible WSOCKETS.DLL
   Dynamic Link Library.  Make sure Windows can find it.  (set your
   PATH environment variable).

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


The on-line documentation no longer matches the program exactly,
but is pretty close.  The documentation will be updated soon.

---------------------------------------------------------------------------

Thanks to Mark Riordan for the original WinVN!

Many people have worked on WinVN - it's been a fun and international
project coordinating work on a piece of software.

Here are a few of them:

	Matt Bretherton		MBretherton@cmutual.com.au
	Jim Dumoulin 		dumoulin@titan.ksc.nasa.gov
	Michael Downs 		mdowns@eos.arc.nasa.gov
	Michael Finken		finken@conware.de
	Sam Rushing		rushing@titan.ksc.nasa.gov

	(countless others I don't really mean leave out)

You can reach (or join) 'The Developers' by sending mail to:

winvn@titan.ksc.nasa.gov

(This is not a snazzy list server, it's an alias entry on a VMS Vax.)


The official FTP site for WinVN is at titan.ksc.nasa.gov (128.159.4.20)
in the directory [pub.win3.winvn]

Always check there for the latest binaries and sources.

---------------------------------------------------------------------------

quick list of things undocumented:

o in the group window, middle click will mark an article as read
  double-right click will 'catch-up' to that article (mark all as read)
o smtp mail allows only *one* recipient.  (sorry!)
o forwarding an article with MAPI is not yet supported
o double-clicking on a <message_id@host> in an article will attempt
  to grab that article from the server.  This is a brain-dead feature,
  but can be convenient at times.
o will try to use MAPI unless the winvn.ini param SMTPHost is set.

-Sam

