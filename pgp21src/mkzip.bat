del pgp21.zip
pkzip pgp21.zip readme.doc setup.doc blurb.txt
pkzip pgp21.zip pgp.exe
pkzip pgp21.zip config.txt language.txt pgp.hlp es.hlp fr.hlp
pkzip pgp21.zip pgpdoc1.doc pgpdoc2.doc
pkzip pgp21.zip keys.asc pgpsig.asc
echo PGP MSDOS executable release assembled

del pgp21src.zip
pkzip -aP pgp21src.zip @srcfiles
echo PGP source release assembled

