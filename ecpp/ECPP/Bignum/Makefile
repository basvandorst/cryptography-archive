# Makefile for the package of bignum

default:
	make ARCH=`./ark` TYPE=`./ark` all

all: bignum
	mv distrib/BigNum.a $(ARCH)

bignum:
	cd distrib; make $(TYPE)

prgm:
	cd distrib;\
	case ${ARCH} in \
		68K|sun3) make 68K;;\
		sparc|sun4) make sparc;;\
		mips) make mips;;\
		vax) make vax;;\
		pyramid) make pyramid;;\
		ns) make ns;;\
		*) make C32;;\
	esac;

clean:
	make ARCH=`./ark` aclean

aclean:
	cd distrib; make tidy
	cd $(ARCH); /bin/rm *.a
