#
# swIPe Makefile
#

OBJ= sswo.o

# DEBUGFLAGS=-DDEBUGSW

INCLUDES= -I../sys
LIBRARIES= -lkvm
CFLAGS=  $(INCLUDES) $(DEBUGFLAGS) $(LIBRARIES)

all:	sswo spr

clean:
	rm -f $(OBJ) sswo spr *~
	@echo done!

