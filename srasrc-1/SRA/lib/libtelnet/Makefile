LIB=	telnet
OBJS=	auth.o encrypt.o genget.o getent.o gettytab.o misc.o \
kerberos.o enc_des.o kerberos5.o sra.o
CFLAGS= -DENCRYPT -DAUTHENTICATE -DKRB4 -DSRA -DDES_ENCRYPT \
-I../kerberosIV/include -I../libbsd/include -I../libpk

lib${LIB}.a: ${OBJS}
	echo building standard ${LIB} library 
	rm -f lib${LIB}.a 
	if [ -f /usr/bin/lorder ]; then \
		ar cq lib${LIB}.a `lorder ${OBJS} | tsort` ; \
	else \
		ar cq lib${LIB}.a ${OBJS}; \
	fi; 
	if [ -f /usr/bin/ranlib ]; then \
		ranlib lib${LIB}.a; \
	fi;

clean:
	rm -f *.o lib${LIB}.a
