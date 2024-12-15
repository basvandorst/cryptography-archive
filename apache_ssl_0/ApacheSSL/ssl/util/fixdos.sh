#!/bin/sh

mv configur	Configure
mv copyrigh	COPYRIGHT
mv rambling	RAMBLINGS
mv files	FILES
mv porting	PORTING
mv readme	README
mv readme.050	README.050
mv readme.051	README.051
mv todo		TODO
mv version	VERSION
mv verisign	Verisign
mv install	INSTALL
mv win31.txt	WIN31.TXT

# Poor old crypto/des directory :-(
mv crypto/des/changes	crypto/des/CHANGES
mv crypto/des/copyrigh	crypto/des/COPYRIGHT
mv crypto/des/des.pm	crypto/des/DES.pm
mv crypto/des/des.pod	crypto/des/DES.pod
mv crypto/des/des.xs	crypto/des/DES.xs
mv crypto/des/files	crypto/des/FILES
mv crypto/des/gnumakef	crypto/des/GNUmakefile
mv crypto/des/install	crypto/des/INSTALL
mv crypto/des/imakefil	crypto/des/Imakefile
mv crypto/des/kerberos	crypto/des/KERBEROS
mv crypto/des/modes.des	crypto/des/MODES.DES
mv crypto/des/makefile.pl	crypto/des/Makefile.PL
mv crypto/des/makefile.uni	crypto/des/Makefile.uni
mv crypto/des/des_cryp.man	crypto/des/des_crypt.man
mv crypto/des/pc1	crypto/des/PC1
mv crypto/des/pc2	crypto/des/PC2
mv crypto/des/doip	crypto/des/doIP
mv crypto/des/dopc1	crypto/des/doPC1
mv crypto/des/dopc2	crypto/des/doPC2
mv crypto/des/readme	crypto/des/README
mv crypto/des/version	crypto/des/VERSION

mv apps/asn1pars.c	apps/asn1parse.c
mv apps/ssleay.con	apps/ssleay.conf
mv test/ssleay.con	test/ssleay.conf
mv test/test.con	test/test.conf


for i in makefile.ssl */makefile.ssl */*/makefile.ssl
do
	mv $i `dirname $i`/Makefile.ssl
done

/bin/rm -f makefile */makefile */*/makefile

chmod a+rx Configure
chmod a+rx util/*.sh
chmod a+rx apps/g_ssleay.pl
chmod a+rx crypto/bn/bn_prime.pl
chmod a+rx crypto/conf/keysets.pl
chmod a+rx crypto/des/des.pl
chmod a+rx crypto/des/shifts.pl
chmod a+rx crypto/des/testdes.pl
chmod a+rx crypto/error/err_code.pl
chmod a+rx crypto/error/err_genc.pl
chmod a+rx crypto/lhash/num.pl
chmod a+rx crypto/objects/obj_dat.pl

for i in test/testx509 test/testrsa test/testcrl test/testgen test/testenc
do
	chmod a+rx $i
done

chmod a+rx certs/tools/*
# (cd certs; tools/c_rehash *.pem)
make -f Makefile.ssl links
make -f Makefile.ssl clean

(cd apps; sh ./mklinks.sh)

# (cd certs; tools/c_rehash *.pem)
