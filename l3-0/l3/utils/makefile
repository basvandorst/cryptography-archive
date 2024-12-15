INSTDIR=/usr/local/bin
all: tencrypt tdecrypt tvsign tsign

tencrypt: tencrypt.src
	sed "s?XYZZY?$(INSTDIR)?" <tencrypt.src >tencrypt

tdecrypt: tdecrypt.src
	sed "s?XYZZY?$(INSTDIR)?" <tdecrypt.src >tdecrypt

tsign: tsign.src
	sed "s?XYZZY?$(INSTDIR)?" <tsign.src >tsign

tvsign: tvsign.src
	sed "s?XYZZY?$(INSTDIR)?" <tvsign.src >tvsign

clean:
	rm -f tencrypt tdecrypt tvsign tsign

install: tencrypt tdecrypt tvsign tsign
	install -c -m 0755 tencrypt $(INSTDIR)
	install -c -m 0755 tdecrypt $(INSTDIR)
	install -c -m 0755 tvsign $(INSTDIR)
	install -c -m 0755 tsign $(INSTDIR)

