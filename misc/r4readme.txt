
                              ========
                              R4DCRYPT
                              ========
      
                Version 1.0 -- Written June 1994
                     Released September 1995.


This distribution of R4DCRYPT consists of four files:

  1)  README.TXT    - This file.
  2)  R4DCRYPT.C    - C source code  
  3)  R4DCRYPT.EXE  - Password recovery program (DOS compilation)
  4)  CRYPTED.DAT   - Sample input file (R4DCRYPT.C as encrypted 
                      by Reflection 4 with password 'BIHAM&SHAMIR') 

CONTENTS
--------
1. Introduction
2. Example of Usage 
3. Life Expectancy
4. Copyright, Trademarks, and Disclaimer


1. Introduction
---------------
R4DCRYPT is a little utility program that performs password recovery
and decipherment of files encrypted by Reflection 4 software, which
is a widely used communications and terminal emulation package.

R4's encryption capability is intended to protect login passwords
contained in script files.  When executed by R4, the scripts perform
automatic login to remote host computers.  The user must supply a
password to execute the encrypted script;  however, the encryption
algorithm is weak, and the remote login passwords contained in the
script can be determined by deciphering it, as R4DCRYPT demonstrates.

2. Example of Usage
-------------------
R4DCRYPT will be useful to anyone who has (i) inadvertently or
deliberately deleted all copies of their plaintext script or (ii)
forgotten their password.

Suppose the original RCL script file, LOGON.RCL, was encrypted with the
password MYPASSWORD, and the encrypted file is called LOGCRPTD.RCL.  The
file LOGON.RCL has been deleted for security.

With Reflection 4 running, you can execute the encrypted script (login
to a remote host) by using <Alt>-F10 to bring up the Reflection command
line and entering "LOGCRPTD.RCL/MYPASSWORD" (provided you remember
the password).

Also, if your host login password has changed, you must edit the RCL
script accordingly.  WRQ does NOT provide a program to decrypt scripts
(even when the password is known).  Run R4DCRYPT to decipher LOGCRPTD.RCL
-- the program displays MYPASSWORD and creates a file R4DCRYPT.DAT, which
is identical to LOGON.RCL.

It's best to maintain a backup copy of each plaintext script with login
passwords excised;  however, "accidents happen".
 
3. Life Expectancy
------------------
R4DCRYPT deciphers files encrypted by CRYPT.COM 3.4, which was supplied
as part of Version 4.2 of Reflection. R4DCRYPT has not been tested with
other versions of CRYPT.COM.

The author advocates the use of strong encryption methods, and hopes
that R4DCRYPT will _NOT_ be compatible with later (improved ?) versions
of CRYPT.COM :-)

In fact, as a courtesy, the author gave a copy of R4DCRYPT to WRQ about
a year ago (via their technical support BBS), along with a message
outlining the security problem, but R4DCRYPT has not been distributed
generally until now.

Through recent postings in Internet newsgroups, Reflection 4 users have
been informed that R4DCRYPT is now being made generally available.

4. Copyright, Trademarks, and Disclaimer
----------------------------------------
R4DCRYPT is freeware;  nevertheless, some encryption software is subject
to export regulations.  While the encryption algorithm here is very weak,
it is up to you to ensure that your acquisition, use, and distribution
of R4DCRYPT is legal, given your particular circumstances.

Reflection 4 is a commercial product created by Walker Richer & Quinn, Inc.,
and "Reflection" is a registered trademark.  The command file encryption
utility CRYPT.COM is copyright by Walker Richer & Quinn, Inc.
