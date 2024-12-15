Private Idaho 2.6b1 (beta) - freeware
1/15/96
copyright 1995, 1996 Joel McNamara

What is it?
-----------
This is a new release of Private Idaho (a PGP and anonymous
remailer front-end for Windows).

New 2.6b Features
-----------------
Winsock "Connect/Disconnect" for on-line and off-line work
Receive and store PGP messages from POP3 mail servers
Improved scripting for mail transfers
"Hard" carriage return word wrap
Reply to ">"
Revised help file
Various bug fixes and user interface tweaks

New Things Since 2.6b
---------------------
Got rid of red and green "lights" next to Send button.  These
were meant to indicate whether a Winsock connection was made or not.
Now, the Send button and the Send message and Scan for PGP messages
commands in the E-mail menu are disabled until a connection is made.  
In addition, a "Winsock not connected" message is displayed on the 
e-mail status line.  This should be more intuitive for people.

YOU MUST HAVE A WINSOCK CONNECTION TO SCAN FOR PGP MESSAGES OR
SEND E-MAIL DIRECTLY FROM PRIVATE IDAHO.

The nym field has now been expanded.  It was previously only limited
to 10 characters.

Scans for both PGP encryption and signature header messages.

If you are having problems with sending e-mail directly from Private
Idaho or scanning for PGP messages, add the following line to the
[Options] section of the PIDAHO.INI file:

SMTPLog=true

This will create a log file in the directory containing Private Idaho
that lists the entire SMTP and POP3 conversation between the mail
server and Private Idaho.  If you have problems, e-mail me a copy of
the log file.  It should help in trying to debug what's happening.


Installing for the first time
-----------------------------
If this is the first time you've used Private Idaho, read the
installation instructions in the PIDAHO.HLP file.


Updating an older version
-------------------------
If you're using 2.6b, just replace the old PIDAHO.EXE file with the one
contained in the .ZIP file.

If you're using an older, 2.5 version of Private Idaho, to update, do
the following:

Replace the following files with the new versions:

PIDAHO.EXE
PIDAHO.HLP

Add the following file to the C:\WINDOWS\SYSTEM directory:

IPP20.VBX

The remailer information files have also been updated (RMINFO.TXT,
CPMAILER.TXT, and RMKEYS.TXT) so if you have older copies, you may
want to replace these files also.

If you're using a pre-2.5 version, do a complete re-install.  There's
lots of new features, and some file formats have changed.


Connecting to the Internet
--------------------------
Load the help file and look at this section.  It should answer all of
the questions you have (particularly about licensing IPPort 2.0 - which
I'm in the process of doing now).


Miscellaneous Notes
-------------------
Yes, a 32-bit version will make its debut, hopefully during the
first half of 1996.  (Be prepared for it to be somewhere between double
and triple its current size though.  Visual Basic 4.0 does not
produce small .EXEs.)

I wanted to get this version out before the end of year, and have
tried to include as many bug fixes and requests as possible.
Unfortunately, I haven't been able to implement all requested
features.

I'm going to be extremely busy the first three months of the new
year, so please be patient with me replying to your questions and
comments, as well as releasing new versions.  I'll try to get back
to you as soon as I can, but "customer support" is not going to be
as responsive as it has in the past.

Direct any comments, questions, requests to: joelm@eskimo.com
