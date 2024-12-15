

         README FOR DES SEARCH ENGINE CONTROLLER SOFTWARE

                         April 23, 1998


Written 1998 by Cryptography Research (http://www.cryptography.com)
for the Electronic Frontier Foundation (EFF).  Placed in the public
domain by Cryptography Research and EFF.

This is unsupported free software.  Use and distribute at your own
risk. U.S. law may regulate the use and/or export of this program.
Foreign laws may also apply.



---------------------------------------------------------------------
Section 1:  Compiling the Programs.

Compiling the programs should be easy.  Using 32-bit Microsoft Visual
C++ for Windows compile as shown below.  For Borland C++ or other
compilers, replace "cl" with the compiler name (e.g., bcc).  On a 16-
bit DOS compiler with a large search array, the large memory model
(Borland's "-ml" flag) is required or the system will run out of
memory.

   > cl search.c keyblock.c chipio.c des.c
   > cl initsrch.c keyblock.c
   > cl autoconf.c chipio.c
   > cl testvec.c sim.c des.c



---------------------------------------------------------------------
Section 2:  Auto-Configuring the Search Array.

The auto-configuration program is an important part of the DES
Cracker.  Because there are a large number of chips in the system, it
is inevitable that a few fail.  By automatically removing defective
units, it is not necessary to repair the system when failures do
occur.

The program "autoconf.exe" will automatically identify the
configuration of a search array.  With the I/O port base address at
210 hex, simply run the program with the command:

   > autoconf search.cfg -t

Note that the "-t" flag performs register testing (recommended if the
search system might contain defective chips that need to be avoided).
If the I/O port is at an address other than 210, specify the address.
The "-v" flag provides verbose output.  For example:

   > autoconf search.cfg 210 -t -v

When autoconf completes, it will print the total number of chips to
the screen and save the configuration information to the
configuration file.  The configuration can be edited (e.g., with
grep) to remove defective units not caught with autoconf.

(Note that this first release does not implement search unit testing 
code except for the register tests.) 



---------------------------------------------------------------------
Section 3:  Initializing a Search.

The search parameters have to be specified before a key can be found.
The program initsrch creates a "search context" file that contains
these search parameters and a list of the regions of keyspace that
remain to be searched.

The search parameters can either be entered into initsrch or
specified on the command line.  To enter them manually, run initsrch
with no parameters:

   > initsrch

The program will then prompt for the search context file.  Press
enter for the default filename ("search.ctx").

Next, the program will prompt for a search mode.  Five modes are 
supported and are described in the following sections. 

   K - Known plaintext
   E - ECB ASCII text
   C - CBC ASCII text
   B - Blaze challenge
   M - Manual parameter specification


1.  Known plaintext searching

This is the simplest (and most common) mode of operation.  If a
complete DES plaintext/ciphertext pair is known, this mode can be
used to quickly search for the key.  When prompted, enter the
plaintext in hexadecimal form (e.g., "123456789ABCDEF0") and press
enter.  Next, enter the ciphertext, also in hexadecimal. The program
will then create a search context file and exit.


2.  ECB ASCII text searching

If your target message is known to be ASCII text and was encrypted
using DES ECB mode, enter two different ciphertexts.  The program
will create the search context file and exit.  The program is
configured to include all letters ("a-z" and "A-Z"), numbers ("0-9"),
and common punctuation (ASCII zero, tab, linefeed carriage return,
space, and common punctuation (!"'(),-.^_).  For other character
sets, use the manual parameter specification option.


3.  CBC ASCII text searching

If your message is ASCII text and was encrypted using DES CBC mode,
this option lets you specify an initialization vector and two
ciphertext messages.  The CBC mode ASCII option uses the same ASCII
text characters as ECB ASCII.


4.  The Blaze challenge

Matt Blaze's DES challenge involves searching for a key such that a
repeated plaintext byte produces a repeated ciphertext byte.  This
option will search for keys that meet the challenge. Simply specify
the desired repeated ciphertext byte.


5.  Manual parameter specification

The manual parameter mode allows direct control over the search
parameters. The manual mode requires entering more data than the
other modes; it is often easier to pipe input from a script file,
e.g.:

   > initsrch < search.scr

First, enter the plaintext vector.  This is 64 hex digits long and
specifies the bytes that can appear in "valid" plaintexts. The most
significant bit of the left-hand digit specifies whether ASCII 255
can appear, and the least significant bit of the last digit specifies
whether ASCII zero can appear. For example, the plaintext vector for
the ASCII text modes is:

   0000000000000000000000000000000007FFFFFFC7FFFFFE8FFF738700002601

Next, enter the initialization vector for the first DES, if any.
This will be XORed onto the first plaintext before its validity is
checked.

Next, enter the two ciphertexts (ciphertext 0 and ciphertext 1).
These may be the same or different.

Next, enter the plaintext byte mask.  This sets bits that should be
ignored in the plaintext.  For example, if the left-hand byte of the
plaintext is unknown or can have any value, the plaintext byte mask
would be set to 80 (hex).

Finally, enter the searchInfo byte. Bit 1 of this byte specifies
whether CBC mode should be used.  If so, the first ciphertext will be
XORed onto candidate plaintexts produced by decrypting the second
ciphertext.  Bit 2 of searchInfo specifies whether the extraXor
operation should be done. This operation XORs the right half of the
plaintext onto the left half before it is checked.  (For the Blaze
challenge, the desired plaintext has a single byte repeated.  The
extraXor operation will set the left half of the plaintext to zero if
the plaintext is good.  The plaintextByteMask can then be set to 0x0F
to ignore the right half and the plaintextVector has only the bit for
ASCII zero set.)


5.  The search context file

The search context file contains a header, the search parameters, and
2^24 bits corresponding to the unsearched key regions.  The search
parameters are: plaintextVector (32 bytes), plaintextXorMask (8
bytes), ciphertext0 (8 bytes), ciphertext1 (8 bytes),
plaintextByteMask (1 byte), and searchInfo (1 byte).  Each search
region includes 2^32 keys. The first bit (the MSB of the first key
region byte) corresponds to the keys 00000000000000 through
000000FFFFFFFF, in 56-bit notation.  (To produce the 56-bit form of a
64-bit DES key, delete the eight parity bits.)



---------------------------------------------------------------------
Section 4:  Running a Search.

The most common way to run a search is to type:

   > search search.cfg search.ctx logfile -q

The "-q" flag requests quiet output, which prints less information to
the screen.  The search.cfg file is produced by autoconf, and
search.ctx is produced by initsrch.  The logfile will contain a list
of candidate keys encountered.

If a search is stopped partway through, work done in partially-
completed key regions is lost, but completed regions are noted in the
search context file.  Note that a complete search will produce a
rather large amount of data in the logfile.  If hard disk space is
limited, it may be desirable to stop the search occasionally (for
example, daily) to purge the logfile.



---------------------------------------------------------------------
Section 5:  Porting to other platforms.

When porting to other platforms, some code changes or additions may
be required.  The following may not be found on all systems:

      stricmp:    This is a case-insensitive strcmp found on many
          compilers.  If it isn't present, you can either use strcmp
          (though commands will become case sensitive) or write one.

      SEEK_SET:   A constant (equal to zero) used to tell fseek()
          to go to a fixed offset.  Usually defined in stdio.h

      kbhit(void):  Returns true if a key has been pressed. (Used to
          check for commands during searches.)

      getch(void):  Reads a keystroke from the keyboard.

      inportb(unsigned portNum):  Reads a byte from an I/O port. Used
          only by chipio.c.  On other platforms, inportb may need to
          be emulated.  (For Visual C++, inportb is implemented in
          chipio.c as inline assembly language.)

      outportb(int portNum, int value):  Sends a byte to an I/O port.
          Used only by chipio.c.  On other platforms, outportb may
          need to be emulated.  (For Visual C++, outportb is
          implemented in chipio.c as inline assembly language.)



---------------------------------------------------------------------
Section 6:  Final comments

As this code goes to press, there was little opportunity for testing
and the code has not undergone any of the assurance, code review, or
testing processes we normally use.  When working on the code, you
you may find a few bugs.  Feedback, as always, is appreciated.

Paul Kocher, Josh Jaffe, and everyone else at Cryptography Research
would like to thank John Gilmore and the EFF for funding this unique
project, and AWT for their expert hardware work!


