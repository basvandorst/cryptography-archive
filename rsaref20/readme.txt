RSAREF is now being distributed with a new verification utility
called "RSAcheck".  The distribution file and the file containing the
RSAREF License Agreement (info.txt) have both been digitally signed,
as well as the file you are currently reading (readme.txt).

To use RSAcheck: 

1) Make sure that the program supplied is compatible with your system.
   If it is not, check RSA's ftp site for a version of RSAcheck compiled
   for your system, or send electronic mail to
   rsaref-administrator@rsa.com. Please specify what platform and
   operating system you are inquiring about in your message.

2) Run RSAcheck and, when prompted, supply the name of the file that you
   wish to verify, and the name of its associated signature file. (See 
   "What the results mean:" later in this file for an explanation of
   the output from RSAcheck.)

To verify the file you are currently reading, run RSAcheck. Enter
"readme.txt" when prompted for the signed file, and "readme.sig" when
prompted for the signature file.

To verify the RSAREF distribution file, rerun RSAcheck and specify
the name of the distribution file (depending on your platform, this
will be rsaref.tar, rsaref.zip, or rsaref.sea) as the signed file and
rsaref.sig for the signature file.

Once you have opened the distribution file, you can also verify the
RSAREF License Agreement file (info.txt) found in the doc directory
in the same manner. Its associated signature file is info.sig, which
is also in the doc directory.

What the results mean:

There are two unique result levels. The first level verifies that the
signature matches the file. If

                  !!! SIGNATURE DOES NOT VERIFY !!!

is displayed and you have supplied the right data file and signature
file names, then the data file (or signature file) is corrupt or has
been tampered with.  You should obtain a new copy of both the data file
and the signature file from RSA.

The other possible result is 

                          SIGNATURE VERIFIED

This indicates that the signature file does indeed belong to this
data file, and that the data is exactly the same as when it was
signed.


The second level of results indicates the status of the "certificate"
for this RSA Public Key, based on the certificate chain and the
certificate revocation list(s) that pertain to this key.  A
description of each status is provided here:

Valid:               This certificate is valid, and is not revoked.

Pending:             The validity period of this certificate has not
                     yet started.

Expired:             The validity period of this certificate has passed.
                     Typically, a certificate is valid for a period of
                     one year.

Revoked:             This certificate should no longer be trusted.

CRL expired:         The Certificate Revocation List, which is a list of
                     certificates that have been revoked, is not current.
                     This means that the certificate could actually be
                     revoked at this time. 

Revocation unknown:  This means that no revocation list is associated
                     with this certificate, at all. It is generally a 
                     good idea to not trust certificates that have 
                     this status.

