released to the public domain.

The legal kludge makes output from PGP 2.6 from an Eastern University
incompatible with earlier versions after Sept 1 1994.

A Usenet article has documented a discovery by Paul Elliott that the
pgp 2.6 legal kludge can be disabled by invoking it with the following
parameters:

pgp +CERT_DEPTH=0 +LEGAL_KLUDGE=OFF +CERT_DEPTH=real_desired_value others

This program invokes pgp with the above
parameters. "real_desired_value" is taken from the CONFIG.TXT
file. This will cause the legal kludge that makes PGP from an Eastern
University incompatible with earlier versions of PGP to be turned
off. This program does not address the incompatible signature format
problem. This does not modify the code or the executable of PGP in any
way. It simply invokes it with unusual parameters.  Therefore it
should be legal.


The program searches your config.txt file for the default value of
CERT_DEPTH. It will run slightly faster if this variable is at the top
of that file.



The program to be invoked may be controlled by the environment
variable "PGPEU". EU stands for Eastern university.  This variable may
specify the file name or a complete path.  If this variable is
undefined, the program indicated by the hard coded string "PGPEXE"
will be invoked this string may be defined with a -DPGPEXE= flag at
compile time.


By default (that is if PGPEU is undefined) PGPNOKLG.EXE
will search your path for a program called "PGP.EXE".

Thus the command:

PGPNOKLG -e file

will call PGP 2.6 in a way so that the result will be
compatible with earlier versions of PGP.

The program PGPNOKLX.EXE invokes (by default) a program called "PGP26.EXE".
So you could take the original program "PGP.EXE" from an Eastern
University and rename it to "PGP26.EXE" somewhere in your path.
You could the rename "PGPNOKLX.EXE" to "PGP.EXE" then when
PGP is invoked normally it will really be PGPNOKLX which
will invoke the original pgp (AS PGP26) in a way that output
will be compatible with earlier versions. Thus scripts, shells and mail
programs that are designed to invoke pgp could continue to work,
but in a way that the output is compatible with earlier versions
of PGP.

This program has been ported to MSDOS and OS/2. Somebody please port
to all other platforms.
