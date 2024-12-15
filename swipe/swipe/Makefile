#
# swIPe Makefile
#

DISTROOT=./DIST-04JUN94/RELEASE/swipe
DIRS= user modload

all:
	for i in $(DIRS); do \
		(cd $$i; make all) \
	done

clean:
	for i in $(DIRS); do \
		(cd $$i; make clean) \
	done


release:
	rm -rf ${DISTROOT}
	mkdir -p ${DISTROOT}/doc
	mkdir -p ${DISTROOT}/user
	mkdir -p ${DISTROOT}/modload
	mkdir -p ${DISTROOT}/sys/netinet
	cp Makefile ${DISTROOT}
	for i in README COPYRIGHT swipe-id.txt swipe-usenix.ps; do \
		(cp doc/$$i ${DISTROOT}/doc) \
	done
	for i in Makefile doload spr.c sswo.c swprep; do \
		(cp user/$$i ${DISTROOT}/user) \
	done
	for i in Makefile mlsw.c; do \
		(cp modload/$$i ${DISTROOT}/modload) \
	done
	for i in debug.h if_sw.c if_sw.h ip_swipe.c ip_swipe.h ip_swipedes.c ip_swipemd5.c; do \
		(cp sys/netinet/$$i ${DISTROOT}/sys/netinet) \
	done


