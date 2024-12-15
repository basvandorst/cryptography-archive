DESCRIPTION
-----------
win_dll is a collection of Windows DLL's, with source code, that implements
the popular strong cryptosystems and a hashing algorithm. This package is
freeware, you can do whatever you like with it as long as you agree that I
(the author) assume absolutely no responsibility for any consequences. You
should also be aware of any local restrictions on the use of crypto
software that may be in effect in your country.

This package implements the following:

DES     The well known U.S. standard
IDEA    A very strong cipher used in PGP
MD5     A secure hashing algorithm by noted cryptographer, Ron Rivest.
MDC     For want of a better name, a cipher based on the strong
        cryptographic properties of MD5.
DIAMOND A cipher designed to be stronger than DES. Has a variable key size
        and comes with a faster `lite' version.
CRUTILS Interface to the above ciphers that provides for encryption and
        decryption of memory blocks.

Each zipped directory contains its own description and API usage
instructions. If you think the men in dark glasses might have interfered
with this package before it got to you then you can authenticate it using
the supplied PGP signature and my public key which you can get by fingering
me.


FILES
-----
win_dll.sig     - PGP signature of win_dll.zip
win_dll.zip     - zip containing all the files, stored uncompressed. zip
                  gets better compression if allowed to process all the
                  files all together like this rather than one at a time.
                  You need to use `pkunzip -d' to correctly extract this
                  zip.


+---------------------------+----------------------------------------------+
| Andy <asb@cs.nott.ac.uk>  | PGP key fingerprint: EC 80 9C 96 54 63 CC 97 |
|    finger for PGP key     |                    : FF 7D C5 69 0B 55 23 63 |
+---------------------------+----------------------------------------------+
