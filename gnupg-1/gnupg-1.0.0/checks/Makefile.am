## Process this file with automake to create Makefile.in

TESTS = version.test mds.test \
	decrypt.test decrypt-dsa.test \
	sigs.test sigs-dsa.test \
	encrypt.test encrypt-dsa.test  \
	seat.test clearsig.test encryptp.test detach.test \
	armsigs.test armencrypt.test armencryptp.test \
	signencrypt.test signencrypt-dsa.test \
	armsignencrypt.test armdetach.test \
	armdetachm.test detachm.test genkey1024.test \
	conventional.test


TEST_FILES = pubring.asc secring.asc plain-1o.asc plain-2o.asc plain-3o.asc \
	     plain-1.asc plain-2.asc plain-3.asc plain-1-pgp.asc \
	     pubring.pkr.asc secring.skr.asc secdemo.asc pubdemo.asc

DATA_FILES = data-500 data-9000 data-32000 data-80000 plain-large

EXTRA_DIST = defs.inc run-gpg run-gpg.patterns $(TESTS) $(TEST_FILES) \
	     mkdemodirs signdemokey
CLEANFILES = prepared.stamp x y yy z out err  $(DATA_FILES) \
	     plain-1 plain-2 plain-3 options trustdb.gpg *.lock .\#lk* \
	     pubring.gpg secring.gpg pubring.pkr secring.skr
DISTCLEANFILES = pubring.gpg~


all-local: prepared.stamp

distclean-local:
	./mkdemodirs --clean

prepared.stamp: ./pubring.gpg ./secring.gpg ./plain-1 ./plain-2 ./plain-3 \
		./pubring.pkr ./secring.skr $(DATA_FILES)
	 echo timestamp >./prepared.stamp


./pubring.gpg: $(srcdir)/pubring.asc
	 ../g10/gpg --yes --dearmor -o ./pubring.gpg $(srcdir)/pubring.asc

./secring.gpg: $(srcdir)/secring.asc
	 ../g10/gpg --yes --dearmor -o ./secring.gpg $(srcdir)/secring.asc

./pubring.pkr: $(srcdir)/pubring.pkr.asc
	 ../g10/gpg --yes --dearmor -o ./pubring.pkr $(srcdir)/pubring.pkr.asc

./secring.skr: $(srcdir)/secring.skr.asc
	 ../g10/gpg --yes --dearmor -o ./secring.skr $(srcdir)/secring.skr.asc

./plain-1: $(srcdir)/plain-1o.asc
	 ../g10/gpg --yes --dearmor -o ./plain-1 $(srcdir)/plain-1o.asc

./plain-2: $(srcdir)/plain-2o.asc
	 ../g10/gpg --yes --dearmor -o ./plain-2 $(srcdir)/plain-2o.asc

./plain-3: $(srcdir)/plain-3o.asc
	 ../g10/gpg --yes --dearmor -o ./plain-3 $(srcdir)/plain-3o.asc



data-500:
	../tools/mk-tdata   500  >data-500
data-9000:
	../tools/mk-tdata  9000  >data-9000
data-32000:
	../tools/mk-tdata 32000  >data-32000
data-80000:
	../tools/mk-tdata 80000  >data-80000
plain-large:
	cat $(srcdir)/../doc/HACKING \
	    $(srcdir)/../doc/DETAILS \
	    $(srcdir)/../doc/FAQ	 >plain-large

