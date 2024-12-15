-----BEGIN PGP SIGNED MESSAGE-----

Welcome to the PGPfone Distribution Kit (Windows '95 Version)

Congratulations! You have begun the PGPfone installation process.  If
you are reading this file, it means that you have already successfully
downloaded the pgpf10b1.zip file and unzipped it. You should now have
three files (besides the ZIP file itself).

README.TXT --   This File
PGPF10BI.ZIP -- Contains the Actual PGPfone for Windows '95
PGPF10BI.ASC -- Contains a PGP Signature from Jeffrey I. Schiller
                on the PGPF10BI.ZIP file.

The Digital Signature on PGPF10BI.ZIP permits you to verify that you
in fact have a good copy of PGPfone. However to do so you will first
have to have (or obtain) PGP. You can get PGP from the MIT
Distribution site at "http://web.mit.edu/network/pgp.html." This is
the MIT PGP Home Page, it contains a lot of information on PGP
including how to get it.

If you do not have PGP and are not planning on getting it now, skip
the rest of this message, Unzip PGPF10BI.ZIP and you are on your way
(additional README files as well as the PGPfone manual are included in
it).

Why do you want to verify PGPfone?

Well the simple answer is that we expect that many BBS systems and
Internet sites will choose to redistribute PGPfone (as many
redistribute PGP today).  By verifying the distribution you can
convince yourself that you have a distribution that has not been
tampered with!

Shall we continue...

To verify the distribution you will need the 1024bit key for "Jeffrey
I.  Schiller <jis@mit.edu>" (me!). Its keyid is "0x0DBF906D" and is
available from PGP keyservers the world over. It is also included as
part of the MIT PGP Distribution.

Step 1: Make sure you have my key in your PGP keyring file.

        PGP comes with a file "keys.asc" that contains my key as well as
        that of Phil Zimmermann and a few other luminaries. Use
        PGP to read this file in, it will tell you that the file contains
        keys and will ask if you want them included on your public keyring.
        The correct answer is "Yes."

Step 2:

        Invoke PGP on the file PGPF10BI.ASC. It will inform you that the
        file contains a signature but no text. It will then either
        automatically find the PGPF10BI.ZIP file or ask you to tell it
        what file to process. The answer is "PGPF10BI.ZIP."

At this point PGP should tell you:

"Good Signature from user Jeffrey I. Schiller <jis@mit.edu>"

It may also give you a warning that my key is not trusted (if you have
not configured PGP to trust my key). See the PGP manual for information
about what this is all about.

-----BEGIN PGP SIGNATURE-----
Version: 2.6.2

iQCVAwUBMVIGSsUtR20Nv5BtAQE6ZQP8DiHm4Jisb8ozHOvJFN+rd1hYy2rnD6qT
4AYMaVTzftqyxX9qWVADiqlvlK0t39jv1mKT9DHCXei4jdL4AQozDl4OL6FSMBZv
/V/2wCQLuvoEBumgUKVoYeGkfZt7kIf/dN/+j2UDuGuC4rPf+Bxmg44TZT7cg4Cx
mywCLY8FaYI=
=td2h
-----END PGP SIGNATURE-----
