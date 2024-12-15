#
# Makefile for the Intermediate Values for RC6 algorithms
#

CC=		bcc32
CFLAGS=	-Oi -6 -v -A -a4 -O2

OBJ=	aes_iv.obj
TRACE_OBJ= 

#
# Rules
#

all:	kat-iv.exe

#
# Dependencies
# 

aes_iv.obj:	aes_iv.c aes_iv.h
	$(CC) $(CFLAGS) -c aes_iv.c

kat_iv.obj:	kat_iv.c
	$(CC) $(CFLAGS) -c kat_iv.c

kat-iv.exe:	kat_iv.obj $(OBJ)
	$(CC) $(CFLAGS) -ekat-iv.exe kat_iv.obj $(OBJ)

runtests:
	-kat-iv.exe

#
# Clean
#

clean:
	-del *.obj

cleanexes:
        -del kat-iv.exe

cleantests:
	-del dec_vkiv.txt
	-del dec_vtiv.txt
	-del ecb_vkiv.txt
	-del ecb_vtiv.txt

cleanall: clean cleanexes cleantests

#
# End Of Makefile
#
