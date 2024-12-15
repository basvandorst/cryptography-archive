OFLAGS = -g
DFLAGS = -DDEBUG -DDISTRIB# -DBENCH
DLIB = $$\(LIB.$(ARCH)\)

CFLAGS = -c $(OFLAGS) $(DFLAGS)
LDFLAGS = $(OFLAGS) $(DFLAGS)# -p

BN = ../../Bignum

all: runecpp

runecpp: 
	make ARCH=`$(BN)/ark` \
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" DLIB=$(DLIB) arunecpp

arunecpp:
	cd $(ARCH); \
	make -f ../Makefile.ecpp ARCH=$(ARCH) \
	CFLAGS="$(CFLAGS) -D$(ARCH)" LDFLAGS="$(LDFLAGS) -D$(ARCH)" \
	DLIB=$(DLIB) runecpp

checkcertif: 
	make ARCH=`$(BN)/ark` \
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" DLIB=$(DLIB) acheckcertif

acheckcertif:
	cd $(ARCH); \
	make -f ../Makefile.ecpp ARCH=$(ARCH) \
	CFLAGS="$(CFLAGS) -D$(ARCH)" LDFLAGS="$(LDFLAGS) -D$(ARCH)" \
	DLIB=$(DLIB) checkcertif

pford: 
	make ARCH=`$(BN)/ark` \
	CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" DLIB=$(DLIB) apford

apford:
	cd $(ARCH); \
	make -f ../Makefile.ecpp ARCH=$(ARCH) \
	CFLAGS="$(CFLAGS) -D$(ARCH)" LDFLAGS="$(LDFLAGS) -D$(ARCH)" \
	DLIB=$(DLIB) pford

clean:
	make ARCH=`$(BN)/ark` aclean

aclean:
	cd $(ARCH); /bin/rm *.o

