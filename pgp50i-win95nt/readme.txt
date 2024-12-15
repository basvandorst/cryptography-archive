
README file for PGP International Freeware, Version 5.0
-------------------------------------------------------


OVERVIEW
========

Welcome to this international freeware release of PGP 5.0, commonly
referred to as PGP 5.0i.  This archive contains PGP 5.0i binaries
for Windows 95 and Windows NT 4.0.  The source code is available in
another archive named pgp50i-win95nt-src.zip.

The binaries contained in this archive are compiled from source code
that was published in the following books by PGP, Inc.:

 1) Pretty Good Privacy (tm) 5.0
    Platform Independent Source Code
    June 14, 1997
    ISBN 0-9649654-5-3

 2) Pretty Good Privacy (tm) 5.0
    Windows 95/NT Specific Source Code
    June 14, 1997
    ISBN 0-9649654-6-1

 3) Pretty Good Privacy (tm) 5.0
    Platform Independent Source Code, 2nd edition
    July 9, 1997 
    ISBN 0-9649654-8-8

 4) Pretty Good Privacy (tm) 5.0
    Windows 95/NT Specific Source Code, 2nd edition
    July 9, 1997
    ISBN 1-9649654-9-6

 5) Pretty Good Privacy (tm) 5.0
    Unix Specific Source Code, 2nd edition
    July 16, 1997
    ISBN 1-891064-01-0

The books were exported from the USA in accordance with the US Export
Regulations, and the pages were then scanned and OCRed to make the
source available in electronic form.

More than 70 people from all over Europe worked for over 1000 hours
to make this release possible.  We are pleased to be able to present
a PGP version that is 100% legal to use outside the USA, because no
source code was exported in electronic form.

NB! This is copyrighted software.  Please read LICENSE.txt before
you start using PGP 5.0i.


DIFFERENCES BETWEEN US AND INTERNATIONAL VERSION
================================================

The following changes have been made to the source code after it has
been scanned in order to produce this version of PGP:

  1. The version number has been suffixed with an 'i' (for
     'international')
  2. The default keyserver has been changed from pgpkeys.mit.edu
     to horowitz.surfnet.nl
  3. The splash screen has been updated to reflect the new version
 

DOCUMENTATION
=============

The file pgp50manual.pdf contains the PGP User's Guide for
Windows 95/NT.  The manual is in PDF format, so you will probably
need Adobe Acrobat Reader to view or print it.  Adobe Acrobat can
be downloaded from:

  http://www.adobe.com/prodindex/acrobat/readstep.html

For more information on PGP 5.0i, including a list of known bugs,
answers to FAQs, etc., please visit the International PGP Home
Page, available online at:

  http://www.pgpi.com/


INSTALLATION
============

Installation is usually straight-forward.  Just run Setup.exe and follow
the instructions.

IF YOU HAVE BEEN RUNNING PGP 5.0 AND YOU WANT TO SWITCH TO PGP 5.0i
THEN COPY YOUR SECRING.SKR AND PUBRING.PKR BEFORE UN-INSTALLING PGP 5.0
BECAUSE THE UNINSTALL OF PGP 5.0 DELETES THESE KEYRINGS AND A PREVIOUSLY
GENERATED KEY MIGHT GET LOST.

NOTE: If you have previously installed PGP 5.0, Windows may be locking
many of the DLLs you need to update.  To unlock the DLLs:

  Exit any applications that may be using PGP (this includes PGPkeys,
    PGPtray, Eudora, Exchange and Windows Explorer).

  If you're running Windows 95:
    Ensure that you have an MSDOS window open.
    Select Start/Shutdown
    On the resulting dialog box, hold down control, alt and shift, and
      simultaneously left-click "No."  This will shutdown the Windows
      desktop.

  On all Windows platforms:
    Enter the MSDOS window, and delete the following files:
      c:\windows\system\pgpwctx.dll
      c:\windows\system\pgp.dll

  On Windows 95:
    To get your desktop back, execute:
      explorer
    Windows may complain about missing files (especially PGPtray); do
      not accept any programs it finds as possible matches.  This
      problem will be resolved after you finish installing PGP 5.0i.


IMPORTANT, LAST-MINUTE RELEASE NOTES
====================================

1) The Eudora plugin that is shipped with PGP 5.0 is not included in
this release, because the .rc file necessary to build the plugin was
missing from the source code books.  The Eudora plugin will be included
in a later release.

2) RSA key generation is not supported in this version.  If you want to
generate RSA keys, use PGP 2.6.3i instead or download the source code
(pgp50i-win95nt-src.zip) and build PGP 5.0i for Personal Privacy.

3) Using Microsoft Word as your editor in Outlook is not currently
supported.  The Exchange/Outlook Plug-in will not be available in Word.
Switching your editor back to the Outlook editor will correct the issue.


TECHNICAL DETAILS
=================

The pages were scanned using HP ScanJet 4+ and Agfa StudioScan IIsi
flatbed scanners and OmniPage Pro 7.0 OCR software.

The printed books contained line and page checksums to help
validating the accuracy of the scanned code.  After the pages were
scanned, they were proof-read and any errors corrected until the
checksums agreed.  Because of this, you can be quite sure that no
errors have been introduced into the files on their way from paper
to electronic form, and that the files in this archive are
(practically) identical to the original files, as published by PGP,
Inc.

The source code books also contained a list of MD5 and SHA hashes for
all the files, to make it easier to validate each file.  However,
because the checksum algorithm used in the first edition of the
source code books (from which most of the files have been scanned)
does not take whitespace into consideration, we have not always been
able to reproduce files that are 100% identical to the originals.
They differ only in that they contain too many or too few spaces or
TABs (which are irrelevant to the compiler anyway); apart from that
they are identical to the originals.  Sadly, the MD5 and SHA hashes
are sensitive to whitespace, so only a few of the files in this
archive produce the same hashes as the originals. :-(


CREDITS
=======

This project could not have been accomplished without the help of a
large number of people, who devoted a greater or lesser amount of
their valuable spare-time in order to make PGP 5.0 freely available
to the world.  Their help have been much appreciated, whether they
have been operating the scanners, proof-read the source code, helped
organizing or contributed in some other way.

We would like to thank Peter Aandahl (se), Frederik Andersen (dk),
Ronny Arild (no), Anders Arnholm (se), Sigmund Austigard (no), Harald
Böhme (de), Piete Brooks (uk), Sico Bruins (nl), Børge Brunes (no),
Michail Brzitwa (de), Terje Bråten (no), Ingmar Camphausen (de), Luis
Colorado (es), Carlo Dapor (ch), Martin Dickopp (de), Håvard Eidnes (no),
Patrick Feisthammel (ch), Michael Fischer v. Mollard (de), Wolfgang
Formann (de), Otto Forster (de), Gustav Foseid (no), Jens Frank (de),
Bjørn Frantzen (no), Mark Grant (uk), Kari Granö (fi), Jani Hakala (fi),
Kolbjørn Halvorsen (no), Florian Helmberger (at), Jeremy Henty (uk), Luis
Hernandez (uk), Alex Le Heux (nl), Hans Hübner (de), Petteri Jantti (uk),
Bo Johansson (se) Arne H. Juul (no), Stephen Kapp (uk), Eivind Kjorstad
(no), Anja Kristoffersen (no), Delman Lee (uk), Paul Leyland (uk), Klaus
Lichtenwalder (de), Sten Lindgren (se), David Lucas (uk), Ernst Molitor
(de), Harald Musum (no), Øyvind Møll (no), Karin Nijssen (nl), Jürgen
Peus (de), Jens von Pilgrim (de), Johan Riise (no), Naresh Sharma (nl),
Klaus Singvogel (de), Dag-Erling Smørgrav (no), Øyvind Solheim (no),
Frank Stajano (uk), Alexander Stellwag (de), Thorkild Stray (no), Pierre
Suter (de), Ole Jørgen Tetlie (no), Maartje Toorenaar (nl), Helge René
Urholm (no), Jan Viljanen (se), Andreas Wespe (de), Reto A. Wiget (ch),
Ulrikke Ytteborg (no) and Dag Øien (no) for their invaluable help.

Special thanks go to the Norwegian National Library for scanning part
of the source code, UNINETT (the Norwegian Academic Network) for
funding, and to the attendants at HIP '97 for being so patient and
for providing last-minute proof-reading help.

It's been a lot of work organizing this thing, but it's been a lot of
fun, too.  Whether you are an experienced PGP user or completely new
to PGP, we hope that you will enjoy this new version, and that you
will think it was worth all the effort we put into it.


Happy encrypting,


Teun Nijssen <Teun.Nijssen@kub.nl>
Stale Schumacher <stale@hypnotech.com>
