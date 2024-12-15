SCRIPTS= genkey getcert ns_convert
BINARIES= makerand
INSTALLTOP= /usr/local/ssl
CC= gcc

all: $(SCRIPTS) $(BINARIES)

install: $(SCRIPTS) $(BINARIES)
	cp $(SCRIPTS) $(BINARIES) $(INSTALLTOP)/bin
	cp ssleay.conf $(INSTALLTOP)/lib
	cp -r ../ssl/apps/democa $(INSTALLTOP)

makerand: librand/librand.a makerand.o
	$(CC) -o makerand makerand.o librand/librand.a

librand/librand.a:
	@(cd librand; make)

clean:
	rm -f $(BINARIES) *.o
	@(cd librand; make clean)

