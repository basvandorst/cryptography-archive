-----BEGIN PGP SIGNED MESSAGE-----

PGPShell Version 3.1
- --------------------

This is mainly a maintenance release, but I've added a few zingers that
many people asked for.  Ver 3.1 is much more robust and stable, taking
into account all the things that can go wrong when working with PGP!

If you want to check the file integrity of the PGPSHELL.EXE executable
file, I've added a command-line option to check the CRC of the file at
the DOS prompt:  PGPSHELL crc

The CRC checksum should be 57045.  If the output doesn't agree with
this figure, then the PGPSHELL.EXE file may contain a virus, has been
reverse-engineered, altered, or is a counterfeit.  If you haven't added
my public key to your key ring yet (KEPLER.ASC) you should and then
check the signature integrity of this README file by typing 
PGPSHELL README.TXT.


This is the ZIP archive integrity output for the executable that you
can check by typing:  PKUNZIP -t -v PGPSHE31.ZIP.


 CRC-32     Name
 --------   ------------   
 30d8c0a7   PGPSHELL.EXE


-----BEGIN PGP SIGNATURE-----
Version: 2.3a

iQCVAgUBLZClI5E2Jn5vTkk3AQG7QgP9Ep6BeceRk6B5vgywnUQgBcPImfy44omi
XVUjSbBqAQ9xcIEicXrmiFc2mJOXtMSfkRUV9TrjIKq8FxnKnc7/wkuUnwuLbNdJ
B1G2NOWcTrZgq6P0urg30joFVIOGSofyWbrYVfNFeBRUuzTFKEBe150j2aveCsng
PXT6JGOTGb0=
=RmTy
-----END PGP SIGNATURE-----
