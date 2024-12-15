COMPILER = gcc
D486 = -m486
OPTS = $(D486) -DLITTLE_ENDIAN -O6 -ffast-math -fstrength-reduce -funroll-loops -fomit-frame-pointer -finline-functions -fexpensive-optimizations -Wall

all:	qinit qcrack qext

###############################################################################
##                                                                           ##
##  Do not add any form of dependency compile; it will mess up the misc.h    ##
##  header's exclusion #define                                               ##
##                                                                           ##
###############################################################################

qinit:	qinit.c fcrypt.c misc.c
	$(COMPILER) $(OPTS) -o qinit qinit.c fcrypt.c misc.c

qcrack:	qcrack.c fcrypt.c misc.c
	$(COMPILER) $(OPTS) -o qcrack qcrack.c fcrypt.c misc.c

qext:	qext.c misc.c fcrypt.c
	$(COMPILER) $(OPTS) -o qext qext.c fcrypt.c misc.c

clean:	
	rm -f qcrack qinit qext
