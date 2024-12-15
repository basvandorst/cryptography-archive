/*--- WVDOC.C -- File to contain programmer documentation for WinVN  */
/*  Written by Mark Riordan  November 1989 & February 1990           */
/*  1100 Parker   Lansing MI  48912                                  */

#if 0

WINVN SOURCE CODE

This file contains documentation about the internals of WinVN.
It's written as a C function to be included in WinVN because it seems
to me that separate internals documents tend to get lost, or
separated from the source code.

This document assumes that you have used WinVN and have read WINVN.WRI.
Of course, the more you know about Windows programming, the better.

The source to WinVN is distributed amoungst several different
files.  Functions are not always placed in the most logical source file.
The source files are:

WINVN.C     Main program and initialization.
WVART.C     Window proc for viewing text of articles, plus related functions.
WVBLOCK.C   Functions related to textblocks and lines.
WVCNCM.C    Dialog procedures for setting comm parameters & other dialog boxes.
WVDOC.C     This file.
WVFILE.C    File I/O functions.  Used for NEWSRC and saving articles to disk.
WVGROUP.C   Window proc for Group windows, plus related functions.
WVHEADER.C  Routines to create postings.
WVSCREEN.C  Functions related to manipulating windows.
WVSOCK.C    Communications support.
WVUSENET.C  Window proc for Net (main) window, plus related functions.
WVUTIL.C    Utility routines, including comm FSA.

There are only a few header files:

WINVN.H     contains function prototypes and constants that we're
            going to give right to Windows, such as menu ids, etc.
WVGLOB.H    contains typedefs and global variables.  This should, perhaps,
            some day be separated into several different header files.
WINUNDOC.H  Taken directly from Charles Petzold's _Programming Windows_.

Of course, there are the usual Windows definitions (.DEF) and resource
compiler (.RC) files, as well as the makefile (WINVN.) and the automatic
response file for the linker (.LNK).  There are several icon (.ICO) files,
which describe the appearance of the windows when they are iconified
(minimized).  I invite you to admire my clever art work.


DISPLAYING TEXT

Quite a bit of the code in WinVN concerns the displaying of text in
windows, and manipulating the data structures that contain that text.  I
would like to have used edit child windows in WinVN.  These are entities
provided by Windows; they know all about scrolling, displaying of text,
editing text, etc.  However, these magic entities put their data in the
local heap; therefore, all the edit child windows in a Windows program must
have a total of less than 64K worth of data.  I considered this
unacceptable.  Also, edit child windows know only about text, and I needed
to store more than just text.  For instance, for the name of a newsgroup, I
need to store the numbers of the articles that have already been read from
that newsgroup.  This meant that I had to duplicate a lot of the
functionality of a "standard" text window by large amounts of code in
WinVN.

While I was at it, I provided the capability to display text in something
other than the system font.  8 point Helvetica bold, for instance, is
fairly attractive and avoids the problems presented by the system font,
which is fairly wide and makes it difficult to display lines of text
greater than 76 or so characters long.  The text-displaying code in
WinVN undertakes to be fairly efficient at putting up text on the screen by
doing screen bit-block transfers when possible.


WINVN DATA STRUCTURES:  DOCUMENTS, TEXTBLOCKS, AND LINES

Although in the case of an edit child window there is a (very simple) data
structure corresponding to the text on the screen, in general in Windows
there is no universal data structure that is designated to store
information to be displayed in a window.  The displaying of text in a
window is a very "manual" process (unless you use one of Windows' built-in
facilities [like edit child windows], which I am not).  Therefore, I had to
invent my own data structure to correspond to the text you see in a window
in WinVN.

I call this data structure a "document".  In WinVN, there is exactly one
"document" for each Network, Group, or Article window.  The document not
only stores text that's to be displayed in the window, but also contains
information such as:

-- what part of the text is currently being displayed in the window,
-- pointers to other documents which are superior or subordinate
   to this one,
-- information on the last text string search done on the document,
   in case the user wants to do a "Find Next",
-- and so on.

A document contains "lines".  The information stored in each line in a
document depends upon the type of document, but within a document, all
lines have the same format.  Documents are stored in global memory and are
described by the typedef TypLine:

   +----------------------------------+          -\
   |  length of line in bytes         |  2 bytes   \
   +----------------------------------+             ] TypLine
   |  Unique line identifier          |  4 bytes   /
   +----------------------------------+          -/
   |  variable number of data bytes   |  variable length
   +----------------------------------+
   |  length of line in bytes (again) |  2 bytes
   +----------------------------------+

Different types of documents use different data structures in the
"variable number of data bytes" field.  Incidentally, this is painful
to implement using C structs and results in a lot of typecasts.  In
retrospect, I could have made it slightly better by using unions and
structs, but I'm going to leave it the way it is.

In traditional C programming, a programmer might implement a document as a
linked list of TypLine's.  However, this would not have been a very good
idea for WinVN.  For one thing, in a long session with WinVN, thousands of
lines need to be allocated and deallocated.  Allocating and deallocating
lots of variable-length data structures would result in fragmented memory
(though Windows can deal with this to some extent).  Also, it would have
been necessary to execute frequent calls to GlobalLock and GlobalUnlock,
incurring some performance penalty.  Finally, I didn't like the idea of
using a very large number of global handles in Windows--it's wasteful and
might not scale well to Windows 3.0.

I therefore decided to implement a textblock system.  Relatively large,
fixed-length blocks of memory are allocated from global memory.  The entire
block is "locked" in memory whenever a line is being accessed.  I am
careful to be a good Windows citizen and use moveable blocks, which I lock
only for a short period of time while I am accessing the data in the block.
This style of programming should port relatively easily to the Macintosh.

A textblock has a header of type TypBlock and is described below.
The block is BLOCK_SIZE bytes long.  Actually, the code is written
to support different-sized textblocks for different documents, but
currently all textblocks are the same length.  The active data in
a textblock ends with a pseudo-line that flags the end of the block.


   +----------------------------------+
   |  Textblock header--various fields|  sizeof(TypBlock)
   +----------------------------------+
   |  Text Lines                      |  variable length
   +----------------------------------+
   |  END_OF_BLOCK                    |  2 bytes  -\
   +----------------------------------+             ] TypLine
   |  Unique pseudo line identifier   |  4 bytes  -/
   +----------------------------------+
   |  Possible unused space           |  variable length
   +----------------------------------+


Textblocks within a document are doubly-linked to each other.  Each
textblock contains a pointer to its parent document.  Textblocks are
dynamically allocated and freed from global memory.

A document consists of a data structure of type TypDoc, and the linked
list of text blocks pointed to by that data structure.  The TypDoc
structure contains quite a bit of information relating to a screen window
and the data in that window.  Several fields in TypDoc answer the question
"Where can I find a particular line?"  The answer is in the form of three
pieces of data:

-- The Line ID of the line.  Each line throughout the entire execution of a
   copy of WinVN has its own, unique 4-byte integer line ID.

   This line ID is enough, in itself, to uniquely identify the line
   in question.  Line IDs are simply bits patterns; lines are not
   necessarily stored in increasing line ID order.  Therefore, there is no
   index that can be used to locate a line given its line ID.  To locate
   a line, one can either sequentially scan the entire document looking
   for a matching line ID.  However, it is useful for performance reasons
   to first look to see if the line is still in the same place as
   it was last time you noticed it.  The next two pieces of information
   indicate where the line was last found.  WinVN uses these to attempt
   to locate the line; if the line has moved, a sequential scan of the
   document is then used.

-- The handle of the textblock that most recently contained the line.
   Lines can move from textblock to textblock when the addition of
   text into the middle of a document causes a textblock split.

-- The offset in bytes of the line from the beginning of its textblock.

These three pieces of information ought to be combined into a struct.
As it is, they are separate fields in several different places.

Document data structures are always stored in local memory.  This is
because they are frequently referenced and because they don't take up much
space.


DIFFERENT TYPES OF DOCUMENTS

WinVN depends upon having one particular window and corresponding document
existing at all times.  This is the "Net" window, which lists all the
newsgroups.  The document is a global variable named NetDoc.  Information
in NetDoc originates from a file named NEWSRC.  There ought to be, but isn't
yet, code to update the list of available newsgroups from the server.

Each line in the NetDoc document contains a structure of type TypGroup.
It contains, amoung other things, the name of the newsgroup, whether it's
subscribed, which articles have been seen, and possibly a pointer to a
"Group" document.

These Group documents contain a list of the subject lines in the group.
Each line in one of these documents contains a structure of type
TypArticle.  This line contains, amoung other things, an indication as to
whether the article has been seen & possibly a handle to a document
containing the text of the article.  Each Group document contains a pointer
to the line in NetDoc that contains the name of the group.
All Group documents are found in the array GroupDocs.

Lines in Article documents contain simply the text of lines in an article.
Each Article document contains a pointer to the Group document, and the
line in that document, that contains the subject line of the article.  All
Article documents are found in the array ArticleDocs.


POSTING

I had planned to hold off on implementing posting until I had some
mechanism for obtaining information about the poster and for
authenticating that information.   This would be the contents of the
"From:" line, the "Organization:" line, and so on.  The Kerberos and
Hesiod pieces of MIT Project Athena would fit the bill.  But these
programs are difficult to implement, and they don't work over
traditional serial lines, which in my mind is a requirement.

The problem of authentication doesn't bother me personally very much,
but it is an issue at my place of employment, which I hope will someday
be a user of WinVN.  The problem of name resolution (obtaining a name
and other information) does concern me, because frequently personal
computers are used by several different people.  Especially in
microcomputer laboratory or classroom situations, it would be quite
awkward to have one user's name and personal information associated with
a given PC.

While mulling over the proper solution to this problem, I lost patience
and started to implement posting based on profile lines from the [winvn]
section of the WIN.INI file.  As of this writing, I am in the process of
debugging support for posting articles.  I don't know whether I'll
officially release this unauthenticated version.

The profile lines I use to support this kludgely solution, and sample uses, are:

   UserName=Mark Riordan
   MailAddress=riordanmr@clvax1.cl.msu.edu
   Organization=Michigan State University

I should add that proper implementation of authentication would require
that I double-check the contents of the header line after editing by the
user and before actual posting.  Currently, the code in WVHEADER.C does
not check to see if the user has modified any article header lines
before shipping off the article to the news server.

Incidentally, the current posting code seems to have some problems.  I'm
not sure whether the problem is in the posting code or in the
communications code--some lines get garbled in transmission.  Because
I'm working on another projet right now, I don't expect that I'll fix or
even look into this bug until early April 1990.


COMMUNICATIONS

WinVN communicates with the NNTP server via either a serial port
(using standard Windows serial port) or TCP/IP (using FTP Software, Inc.'s
PC/TCP).  The serial support works; it requires that you have a serial
line that's talking directly to NNTP.  Typically, this would be serial
login to some Unix machine which is doing a "telnet <host> 119", 119
being the socket number usually assigned to NNTP.

TCP/IP supports works functionally, but something in the process winds
up overwriting memory.  Usually, depending upon how WinVN is linked,
WinVN crashes and burns shortly after making the connection.  I can't
yet prove that this is the fault of PC/TCP.  I have succeeded in linking
a limited version of WinVN which talks OK to the server, though it's clear
that some memory is still being overwritten.  I've tried solving, and
then working around, the problem--to no avail so far.  About 50% of
the time I've spent programming WinVN has been spent in trying to get
the verdammt PC/TCP stuff to work with my program.  Small, trivial
test Windows programs work fine with PC/TCP.

At any rate, the same parent routines are used to talk to the server
for both serial and TCP/IP connections.  These routines check the
variable UsingSocket and decide which I/O routines to call.

The .LNK file distributed with WinVN requires the FTP Software, Inc.
PC/TCP library.  Calls to this library are used only in WVSOCK.C and
would be easy to dummy up if necessary.

#endif
