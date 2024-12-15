OFLAGS = -g
DFLAGS = -DDEBUG

CFLAGS = -c $(OFLAGS) $(DFLAGS)
LDFLAGS = $(OFLAGS) $(DFLAGS)# -p

BN = ../Bignum

bigmod:
	make ARCH=`$(BN)/ark` \
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" abigmod

abigmod:
	cd $(ARCH);\
	make -f ../Makefile.bm ARCH=$(ARCH) \
	CFLAGS="$(CFLAGS) -D$(ARCH)" LDFLAGS="$(LDFLAGS) -D$(ARCH)" all

testgcd:
	make ARCH=`ark` "CFLAGS=$(CFLAGS) -g -DHYPERDEBUG -DDEBUG"\
	 "LIB=$(LIBG)" dtestgcd

clean:
	make ARCH=`../Bignum/ark` aclean

aclean:
	cd $(ARCH); /bin/rm *.o
