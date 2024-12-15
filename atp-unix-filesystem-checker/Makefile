#
# @(#) Makefile for atp
#

# Remove -DMD4 if you want CRC32 checksums (crackable).
DEFINES	= -DMD4
#DEFINES	=

CFLAGS	= -O $(DEFINES) +DA1.0
# +DA1.0 is just an hp-ux machine-dep define.
LDFLAGS	= -s +DA1.0
MAKE	= /bin/make
LIBDES	= libdes
RCSDIR	= RCS
BINDIR	= /staff/vince/bin

CCCOBJ	= atp.o crc32.o md4.o sub.o sub2.o
SUMOBJ	= md4.o sub2.o sum4.o crc32.o
SRC	= atp.c crc32.c md4.c sub.c sub2.c sum4.c
HEADER	= atp.h md4.h
ETC	= Makefile README

all	: atp sum4

atp	: $(CCCOBJ)
	(cd $(LIBDES); $(MAKE))
	cc $(LDFLAGS) -o $@ $(CCCOBJ) -L $(LIBDES) -ldes
	@ctags -w $(SRC) $(HEADER)

sum4	: $(SUMOBJ)
	cc $(LDFLAGS) -o $@ $(SUMOBJ)

clean	:
	(cd $(LIBDES); $(MAKE) clean)
	/bin/rm -f core gmon.out atp $(CCCOBJ) $(SUMOBJ)

ci	:
	ci -q -l -mAutoDeposit $(SRC) $(HEADER) $(ETC)

dep	:
	makedepend $(SRC) $(HEADER)

wc	:
	wc -l $(SRC) $(HEADER)

tar	: clean
	tar cf atp.tar $(SRC) $(HEADER) $(ETC) $(LIBDES)
	compress atp.tar

lint	:
	lint $(DEFINES) $(SRC)

install	: all
	cpset atp $(BINDIR) 700
	cpset sum4 $(BINDIR) 700

# DO NOT DELETE THIS LINE -- make depend depends on it.

atp.o: /usr/include/stdlib.h /usr/include/sys/stdsyms.h /usr/include/unistd.h
atp.o: /usr/include/sys/unistd.h /usr/include/time.h /usr/include/sys/time.h
atp.o: /usr/include/errno.h /usr/include/sys/errno.h /usr/include/signal.h
atp.o: /usr/include/sys/signal.h /usr/include/sys/types.h
atp.o: /usr/include/sys/syscall.h /usr/include/string.h /usr/include/dirent.h
atp.o: /usr/include/sys/dirent.h atp.h /usr/include/stdio.h
atp.o: /usr/include/sys/stat.h /usr/include/fcntl.h /usr/include/sys/fcntl.h
atp.o: md4.h libdes/des.h
md4.o: md4.h
sub.o: /usr/include/termio.h /usr/include/sys/termio.h
sub.o: /usr/include/sys/stdsyms.h /usr/include/sys/ioctl.h
sub.o: /usr/include/syslog.h atp.h /usr/include/stdio.h
sub.o: /usr/include/sys/types.h /usr/include/sys/stat.h /usr/include/fcntl.h
sub.o: /usr/include/sys/fcntl.h /usr/include/unistd.h
sub.o: /usr/include/sys/unistd.h md4.h libdes/des.h
sub.o: /usr/include/sys/sysmacros.h
sub2.o: atp.h /usr/include/stdio.h /usr/include/sys/types.h
sub2.o: /usr/include/sys/stdsyms.h /usr/include/sys/stat.h
sub2.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h /usr/include/unistd.h
sub2.o: /usr/include/sys/unistd.h md4.h libdes/des.h
sum4.o: /usr/include/dirent.h /usr/include/sys/dirent.h
sum4.o: /usr/include/sys/stdsyms.h /usr/include/errno.h
sum4.o: /usr/include/sys/errno.h atp.h /usr/include/stdio.h
sum4.o: /usr/include/sys/types.h /usr/include/sys/stat.h /usr/include/fcntl.h
sum4.o: /usr/include/sys/fcntl.h /usr/include/unistd.h
sum4.o: /usr/include/sys/unistd.h md4.h libdes/des.h
atp.o: /usr/include/stdio.h /usr/include/sys/types.h
atp.o: /usr/include/sys/stdsyms.h /usr/include/sys/stat.h
atp.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h /usr/include/unistd.h
atp.o: /usr/include/sys/unistd.h md4.h libdes/des.h
