CC_IRIX = cc
FLAGS_IRIX_KERNEL = -DKERNEL -non_shared -coff -Wx,-G 0 -Wc,-pic0 -r -d -c -Wc,-jalr -fullwarn -I`pwd`/../irix
FLAGS_IRIX_USER = -fullwarn -I`pwd`/../skipd

CC_NEXT = cc
FLAGS_NEXT_KERNEL = -DKERNEL -DMACH -Wall -I`pwd`/../next
FLAGS_NEXT_USER = -Wall -I`pwd`/../skipd

CC_NETBSD = cc
FLAGS_NETBSD_KERNEL = -DKERNEL -Wall -I/sys -I`pwd`/../netbsd
FLAGS_NETBSD_USER = -Wall -I`pwd`/../skipd

CC_SOLARIS = gcc
FLAGS_SOLARIS_KERNEL = -DKERNEL -Wall -mv8 -I`pwd`/../solaris
FLAGS_SOLARIS_USER = -Wall -mv8 -I`pwd`/../skipd

autoconfig:	
	@make `./configure system`

unknown:
	@echo "Could not determine system, please specify..."
	@make usage

usage:	
	@echo ""
	@echo "make irix ...................... build SKIP for Irix 5.3"
	@echo "make next ...................... build SKIP for NextStep"
	@echo "make netbsd .................... build SKIP for NetBSD"
	@echo "make solaris ................... build SKIP for Solaris 2.4"
	@echo ""
	@echo "make clean ..................... clean the source"
	@echo "make dist ...................... build source distribution"
	@echo

irix::	
	cd lib; make CC=$(CC_IRIX) FLAGS_KERNEL="$(FLAGS_IRIX_KERNEL)" FLAGS_USER="$(FLAGS_IRIX_USER)" all; cd ..
	cd skipd; make CC=$(CC_IRIX) FLAGS="$(FLAGS_IRIX_USER)" all; cd ..
	cd irix; make CC=$(CC_IRIX) FLAGS="$(FLAGS_IRIX_KERNEL)" all; cd ..

next::	
	cd lib; make CC=$(CC_NEXT) FLAGS_KERNEL="$(FLAGS_NEXT_KERNEL)" FLAGS_USER="$(FLAGS_NEXT_USER)" all; cd ..
	cd next; make CC=$(CC_NEXT) FLAGS="$(FLAGS_NEXT_KERNEL)" all; cd ..
	cd skipd; make CC=$(CC_NEXT) FLAGS="$(FLAGS_NEXT_USER)" all; cd ..

netbsd::	
	cd lib; make CC=$(CC_NETBSD) FLAGS_KERNEL="$(FLAGS_NETBSD_KERNEL)" FLAGS_USER="$(FLAGS_NETBSD_USER)" all; cd ..
	cd skipd; make CC=$(CC_NETBSD) FLAGS="$(FLAGS_NETBSD_USER)" all; cd ..
	cd netbsd; make CC=$(CC_NETBSD) FLAGS="$(FLAGS_NETBSD_KERNEL)" all; cd ..

solaris::	
	cd lib; make CC=$(CC_SOLARIS) FLAGS_KERNEL="$(FLAGS_SOLARIS_KERNEL)" FLAGS_USER="$(FLAGS_SOLARIS_USER)" all; cd ..
	cd skipd; make CC=$(CC_SOLARIS) FLAGS="$(FLAGS_SOLARIS_USER)" all; cd ..
	cd solaris; make CC=$(CC_SOLARIS) FLAGS="$(FLAGS_SOLARIS_KERNEL)" all; cd ..

load unload uninstall boot unboot:	
	@cd `./configure system`; make $@

install:	autoconfig
	@cd `./configure system`; make $@

depend:	
	cd lib; make kernel; `../configure mkdepend` -I../include -I../`../configure system` kernel/*.c 2>/dev/null; cd ..
	cd lib; make user; `../configure mkdepend` -I../include -I../skipd user/*.c 2>/dev/null; cd ..
	cd skipd; `../configure mkdepend` -I../include -I../`../configure system` *.c 2>/dev/null; cd ..
	cd `./configure system`; `../configure mkdepend` -I../include -I../`../configure system` *.c 2>/dev/null; cd ..

clean:
	cd next; make clean; cd ..
	cd netbsd; make clean; cd ..
	cd solaris; make clean; cd ..
	cd irix; make clean; cd ..
	cd lib; make clean; cd ..
	cd skipd; make clean; cd ..

dist:	clean
	rm -f dist/ENskip-0.10pa.tar.gz
	cd ..; tar cvf ENskip-0.10pa.tar ENskip-0.10pa; gzip ENskip-0.10pa.tar; cd ENskip-0.10pa
	mv ../ENskip-0.10pa.tar.gz dist/
