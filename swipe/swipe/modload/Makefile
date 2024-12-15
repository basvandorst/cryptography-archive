#
# swIPe Makefile
#

MOBJ= mlsw.o
NOBJ= if_sw.o ip_swipe.o ip_swipemd5.o ip_swipedes.o


OBJ= $(MOBJ) $(NOBJ)

# DEBUGFLAGS=-DDEBUGSW -DSWIPEDEBUG

INCLUDES= -I../sys
CFLAGS= -DKERNEL -DINET -DNSW=1 $(INCLUDES) $(DEBUGFLAGS) -Dsun4c

all:	swipemod.o

swipemod.o: $(OBJ)
	ld -r -o swipemod.o $(OBJ)

clean:
	rm -f $(OBJ) swipemod.o *~
	@echo done!

%.o: ../sys/netinet/%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<




