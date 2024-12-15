LIB=	des
CFLAGS= -DKERBEROS -I. -I../../kerberosIV/include -I../../libbsd/include
OBJS =	cbc_encrypt.o cksum.o debug_decl.o des.o key_parity.o \
	key_sched.o new_rnd_key.o pcbc_encrypt.o quad_cksum.o \
	random_key.o read_password.o string_to_key.o util.o weak_key.o
CLEANFILES= key_perm.h s_table.h ip.c p.c fp.c odd.h p_table.h

TDIR =	..

lib${LIB}.a: $(CLEANFILES) $(OBJS)
	echo building standard ${LIB} library 
	rm -f lib${LIB}.a 
	if [ -f /usr/bin/lorder ]; then \
		ar cq lib${LIB}.a `lorder ${OBJS} | tsort`  ; \
	else \
		ar cq lib${LIB}.a ${OBJS} ; \
	fi;	
	if [ -f /usr/bin/ranlib ]; then \
		ranlib lib${LIB}.a; \
	fi;

key_perm.h:
	cd ${TDIR}/make_key_perm; make CC="$(CC)"
	${TDIR}/make_key_perm/make_key_perm key_perm.h

p_table.h:
	cd ${TDIR}/make_p_table; make CC="$(CC)"
	${TDIR}/make_p_table/make_p_table p_table.h

s_table.h:
	cd ${TDIR}/make_s_table; make CC="$(CC)"
	${TDIR}/make_s_table/make_s_table s_table.h

ip.c:
	cd ${TDIR}/make_ip; make CC="$(CC)"
	${TDIR}/make_ip/make_ip ip.c

p.c:
	cd ${TDIR}/make_p; make CC="$(CC)"
	${TDIR}/make_p/make_p p.c

fp.c:
	cd ${TDIR}/make_fp; make CC="$(CC)"
	${TDIR}/make_fp/make_fp fp.c

odd.h:
	cd ${TDIR}/make_odd; make CC="$(CC)"
	${TDIR}/make_odd/make_odd odd.h

clean:
	rm -f *.o lib${LIB}.a $(CLEANFILES)
	cd ${TDIR}/make_key_perm; make clean
	cd ${TDIR}/make_p_table; make clean
	cd ${TDIR}/make_s_table; make clean
	cd ${TDIR}/make_ip; make clean
	cd ${TDIR}/make_p; make clean
	cd ${TDIR}/make_fp; make clean
	cd ${TDIR}/make_odd; make clean

