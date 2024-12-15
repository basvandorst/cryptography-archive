						      November 30, 1993

1. SecuDE: General Description
==============================

SecuDE is a general purpose security toolkit for Unix systems. It 
comprises a library of security functions and a number of utilities 
with the following functionality:

- basic cryptographic functions (RSA, DSA, DES, triple DES, IDEA, 
  various hash functions, including the RFC 1423 defined algorithm 
  suite (based on PKCS #1), DH key agreement (PKCS #3) and OIW defined 
  algorithms), 

- security functions for "data confidentiality", "data integrity", 
  "origin authentication" and "non-repudation of origin" purposes 
  on the basis of digital signatures and symmetric encryption,

- X.509 key certification functions, handling of certification pathes, 
  cross-certification, certificate revocation,

- processing of RFC 1422-defined certificate revocation lists,

- operation of certification authorities (CA) and interaction between 
  certifying CAs and certified users,

- Internet PEM processing according to RFC 1421 - 1424 (see 2.)

- secure access to public X.500 Directories for the storage 
  and retrieval of certificates, cross-certificates and revocation 
  lists (integrated secured DUA, based on QUIPU 8.0, using strong 
  authentication and signed DAP operations, see 3.),

- support of PKCS-defined structures,

- all necessary ASN.1 encoding/decoding (based on Isode-8.0)

- secure storage of all security relevant information (secret keys,
  verification keys, certificates etc.) in a protected area (called
  PSE, Personal Secure Environment). SecuDE offers two alternative
  PSE realizations:
  - a user-associated DES-protected Unix directory 
  - a smartcard environment, connected via RS-232 to the host.
  Both are only accessible through the usage of PINs (Personal Identi-
  fication Numbers). Smartcards require the purchase of a particular 
  smartcard environment where RSA and DES cryptography is done in the 
  smartcard reader; information available on request.

SecuDE comes with a comprehensive Unix manual (350 p.) providing
detailled description of all unctions, utilities and data structures.

2. SecuDE PEM
=============

A PEM implementation is part of SecuDE. It provides a PEM filter
which transforms any input stream into a PEM formatted output stream 
and vice versa, and which should be easily integratable into Mail-UAs.

SecuDE-PEM realizes the Internet Specifications RFC 1421-1424. However, 
it only supports asymmetric key management.

SecuDE-PEM supports the certification and CRL procedures defined in RFC 
1424 and is integrated into the SecuDE CA functionality. As an additional 
functionality which goes beyond RFC 1421 - 1424, SecuDE-PEM may be 
configured with an integrated X.500 DUA which allows, for instance, 
automatic retrieval of certificates and CRLs during the PEM de-enhancement 
process.  

3. X.500 Package
================

a) Secured DUA
--------------

Concerning X.500 Directory access, SecuDE can be configured in three alter-
natives:

  1. Without X.500 DUA functionality. In this case, the SecuDE package
     is self-contained and needs no additional software.

  2. With integrated X.500 DUA, which operates on the basis of simple
     authentication (i.e. with passwords) and unprotected DAP operations.
     SecuDE uses QUIPU-8.0 library functions for this DUA functionality.
     These library functions are not included in the SecuDE package, and
     a standard QUIPU-8.0 installation is additionally required (the Isode/-
     QUIPU libraries libisode.a and libdsap.a are needed to be able to bind 
     the application programs).

  3. With integrated X.500 DUA, which operates on the basis of strong authen-
     tication (i.e. with digital signatures) and signed DAP operations.
     SecuDE uses modified (i.e. security-enhanced) QUIPU-8.0 library 
     functions for this DUA functionality. These library functions are not 
     included in the SecuDE package, and a security-enhanced QUIPU-8.0 
     installation is additionally required (the security-enhanced Isode/QUIPU 
     libraries libisode.a and libdsap.a are needed to be able to bind the 
     application programs).

The security-enhanced Isode/QUIPU package needed for 3. is in the file 
isode8-secude4.tar.Z of this directory. This file contains a complete Isode-8.0
with security-enhancements provided by SecuDE. 

b) Secured DSA
--------------

The file isode8-secude4.tar.Z also contains software for a secured X.500 
DSA. "Secured" means the ability to perform strong authentication during
association setup between DUA and DSA, and subsequent signed operations at the 
DAP level. The DAP operations into which we have incorporated strong authentication 
are those assigned by the standard (X.511) for that purpose, i.e. Bind, Read, 
Compare, Search, List, AddEntry, RemoveEntry, ModifyEntry, and ModifyRDN. We have 
provided both SIGNED arguments and SIGNED results.


4. Use of Smartcards
====================

SecuDE-4.2 supports the use of the GAO/GMD smartcard package Starcos 1.0 as
one realization of the PSE. A serial line interface with 19.200 Baud is required
to connect the Starcos terminal to the workstation. The Starcos terminal is a
high-security crypto device which performs RSA and DES and provides a number of 
physical protection features. RSA key pairs are generated in the Starcos terminal. 
Secret RSA keys never leave the terminal.

A SecuDE/Starcos version using smartcards with RSA capability and dumb smartcard
terminals is in preparation.

5. System Platforms
===================

All programs of SecuDE are written in C except a small number of long-
integer arithmetic programs which are necessary for RSA and DSA. These programs 
are written in assembler language (assembler programs for SUN SPARC,
HP 9000, Moterola 680x0 for SUN/3 and Apollo Domain WS3xx and WS4xx and 
INTEL 286/386 are part of the package). C-only versions of RSA and DSA are 
contained in the package, too. 

SecuDE contains some third-source software, for instance ASN.1 functions
from ISODE-8. It uses the md2, md4 and md5 programs from RFC 1319 - 1321.
The package is self-contained except for the X.500 DUA functionality 
which requires a QUIPU-8.0 installation (libdsap.a and libisode.a)

SecuDE can be installed on SUN/3 or SUN/4 systems with SunOS 4.1.2 or 
SunOS 4.1.3, on HP 9000 workstations with HP-UX, and on Apollo Domain/IX 
systems. The installation on other Unix platforms should be possible with
minor effort. In addition, SecuDE versions for MS-DOS and MacIntosh are in 
preparation.


6. Conditions of use of SecuDE
==============================

Copyright Gesellschaft fuer Mathematik und Datenverarbeitung 
(GMD), 1993.

Permission to use, copy, modify, and distribute this software 
and its documentation for any purpose and without fee is hereby 
granted, provided that this notice and the reference to this 
notice appearing in each software module be retained unaltered, 
and that the name of GMD or any contributor shall not be used in 
advertising or publicity pertaining to distribution of the soft-
ware without specific written prior permission. It is the respon-
sibility of the users of this software to comply with national or 
international export and import regulations, or with licence 
rights from third parties (see below).

GMD and all contributors disclaim all warranties with regard to 
this software, including all implied warranties of merchantibility 
and fitness. I
n no event shall GMD or any contributor be liable 
for any special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether in 
an action of contract, negligence or other tortious action, arising
out of or in connection with the use or performance of this 
software.

The SecuDE package contains cryptographic software for both 
authentication and encryption purposes. This type of software 
might be subject to national or international export or import 
regulations. It contains also software which implements the RSA 
algorithm. The RSA algorithm is patented in USA.
    
Those who get the SecuDE software via public network access are 
advised to get acquainted with the respective regulations and 
licence conditions for the environment where they intend to use 
the software. It is their responsibility not to get in conflict 
with such regulations.

8. Contact
==========

For more information please contact schneider@darmstadt.gmd.de

  
