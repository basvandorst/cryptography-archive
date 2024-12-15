-----BEGIN PGP SIGNED MESSAGE-----


To decrypt the documentation, follow these instructions:

1. At the DOS prompt, enter: IDEA3A - IDEA3.TXT
2. Enter key: TinyIDEA
3. Check that the documentation decrypted correctly.

If the documentation did not decrypt correctly, then unzip the
original IDEA3.TXT file again, and retry using the IDEA3B program.
If it still does not work, then do not use this software.

These steps are a precaution, to verify that tiny IDEA will run
correctly on your system.

There are three versions of tiny IDEA included in this package:

    A. Smallest version of tiny IDEA, designed to fit within one
       disk sector. This version is very highly optimized.

    B. The standard version of tiny IDEA. This one has a few safety
       features, mainly to maintain compatibility for other operating
       systems (in the unlikely event that version A will not work).

    C. This version will prompt for the passphrase twice, and will
       not display the passphrase as it is typed.

All three versions of tiny IDEA in this package are functionally
equivalent and will correctly decrypt files encrypted by each other.

It is recommended that you test all three versions, decide which one
you prefer, and copy it to your utilities directory as IDEA.COM. The
documentation assumes you have already done this. If you have not,
then remember that when the IDEA program is mentioned in the
documentation, it is referring to any of the programs IDEA3A, IDEA3B,
or IDEA3C.

Due to the strength of the encryption algorithm, the author disclaims
all responsibilities for the use or misuse of this software. You are
warned that files encrypted with this program will not be recoverable
without the correct key.

The MD5 checksums for the files in this archive are included. To verify
the integrity of the files, use the md5sum program which is included in
the PGP distribution. The command line usage is:

    md5sum -vc readme.txt

My PGP public key, which is required to verify the signature on this file,
is available from my Home Page and also from keyservers.

3dfbf3542908ec440a05e2086bd2e45e *idea3.txt
975f20532ce6f55b232a7609412aec38 *idea3a.asm
dc365cf384e5c30c530c4dc40ebee2d5 *idea3a.com
c9fdfe3e71ec086eb5691b17eb06273a *idea3b.asm
491cd1da972d58bef969a2da9e2fe3e6 *idea3b.com
9936c9286d26d2274fc8c966dc663a5d *idea3c.asm
63ea02787fb376ad54bea9953904e51f *idea3c.com

======================================================
Fauzan Mirza            Department of Computer Science
fauzan@dcs.rhbnc.ac.uk  Royal Holloway, Univ of London
======================================================


-----BEGIN PGP SIGNATURE-----
Version: 2.6.2i

iQCVAwUBMlhGnCEYjO9Y93llAQFa0wP/YfoxSSMXJQpTQLRgrmA6JNy7/DkIljTk
4DcQ2jgaptR4Yng6QclBXNeHMN6G3vOH+KiZSk30tp5e9YJ/fJ77IgyMjGaWMOSm
gpfztVXagQTtOSfYeBSw+5xGSgFNh0blCUiB6Xop+lgA8lghZnAOT5EdDNNPmWuU
fOVaHTq7uVk=
=/Vzf
-----END PGP SIGNATURE-----
