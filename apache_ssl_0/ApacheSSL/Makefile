# Master makefile for making an SSL version of apache
ALL=	rsaref/install/rsaref.a \
	ssllibs apache/httpsd apache/httpd 

# Choose a C compiler that does ANSI prototypes
CC= gcc
# On BSDI use gcc2
# CC=gcc2

all:	
	@for i in $(ALL) ;\
	do \
	echo making $$i ; \
	$(MAKE) CC=$(CC) $$i ; \
	done;

apache/httpd:
	@(cd apache; \
	./rmssl; \
	./Configure; \
	$(MAKE) CC=$(CC))

apache/httpsd: 
	@(cd apache; \
	./mkssl; \
	./Configure; \
	$(MAKE) CC=$(CC))

ssllibs:
	@(cd ssl; \
	$(MAKE) CC=$(CC))

rsaref/install/rsaref.a:
	@(cd rsaref/install; \
	$(MAKE) CC=$(CC) rsaref.a)

binaries:
	@(cd ssl; \
		$(MAKE) INSTALLTOP=`pwd`/../binary/ssl install)
	@(cd util; \
		$(MAKE) INSTALLTOP=`pwd`/../binary/ssl install)
	cp apache/http*d binary

install:
	@(cd ssl; \
		$(MAKE) install)
	@(cd util; \
		$(MAKE) install)
	cp apache/http*d serv-root
	cp -r serv-root /usr/local/etc/httpd
	mkdir -p /var/log/www /var/log/wwwsec

clean:
	@(cd apache; $(MAKE) clean)
	@(cd rsaref/install; $(MAKE) clean)
	@(cd ssl; $(MAKE) clean)
	@(cd ssl/crypto/des; $(MAKE) -f Makefile.uni clean)
