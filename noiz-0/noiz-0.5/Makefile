#####  noiz/Makefile
#
#       Anticopyright (A) 1995 Henry Strickland <strick@yak.net>
#
#       This package is placed this package in the public domain.
#
#       Because this package is free, there is no warranty
#       for it whatsoever.  Caveat hacker.
#
#
#       This file Makefile is dedicated to the ascii	TAB	character.
#

BIN=/usr/local/bin
NOIZ=/etc/noiz

OWNER=root
GROUP=kmem

CC= gcc
CFLAGS= -O -D'NOIZ_FILE="$(NOIZ)"'


all : noizinit noizstir noizout noizspin

noizstir : noizstir.o md5.o 
	$(CC) -o noizstir $(CFLAGS) noizstir.o md5.o

noizout : noizout.o md5.o 
	$(CC) -o noizout $(CFLAGS) noizout.o md5.o

clean:
	rm -f *.o
	rm -f noizinit noizstir noizout noizspin
	rm -f a.out core

install: all
	test -d $(BIN) || mkdir $(BIN)
	strip noizinit noizout noizstir noizspin
	install -o $(OWNER) -g $(GROUP) -m  550 noizinit $(BIN)
	install -o $(OWNER) -g $(GROUP) -m 2555 noizstir $(BIN)
	install -o $(OWNER) -g $(GROUP) -m 2555 noizout $(BIN)
	install -o $(OWNER) -g $(GROUP) -m  555 noizspin $(BIN)
	$(BIN)/noizinit
	chown $(OWNER) $(NOIZ)
	chgrp $(GROUP) $(NOIZ)
	chmod 660 $(NOIZ)
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	ls -li / /var/adm/ | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	netstat -s | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	$(BIN)/noizspin | $(BIN)/noizstir
	:
	:
	: Suggestion for your crontab:
	:   "* * * * * $(BIN)/noizspin | $(BIN)/noizstir"
	:

# END $Header: /mvp/fjord/strick/yaxen/noiz-0.5/RCS/Makefile,v 1.6 95/02/07 15:35:36 strick Exp Locker: strick $
