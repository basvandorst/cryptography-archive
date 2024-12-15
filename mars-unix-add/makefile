#
# Makefile for the Mars algorithm
#

CC=		bcc32
CFLAGS=	-Oi -6 -v -A -a4 -O2

OBJ=	sbox.obj sha.obj
TRACE_OBJ=

#
# Rules
#

all:	sbox.exe

#
# Dependencies
# 

sbox.obj:	sbox.c
	$(CC) $(CFLAGS) -c sbox.c

sha.obj:	sha.c
	$(CC) $(CFLAGS) -c sha.c

sbox.exe:	sbox.obj $(OBJ)
	$(CC) $(CFLAGS) -esbox.exe $(OBJ)

#
# Run the Tests
#
runtests: sbox.exe
	-sbox.exe

#
# Clean
#
clean:
	-del *.obj

cleanexes:
	-del sbox.exe

cleantests:
	-del ecb_vk.txt

cleanall: clean cleanexes cleantests

#
# End Of Makefile
#