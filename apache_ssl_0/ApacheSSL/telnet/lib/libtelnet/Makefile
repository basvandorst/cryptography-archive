LIB=	telnet
#OBJS=	auth.o encrypt.o genget.o getent.o gettytab.o misc.o \
#kerberos.o enc_des.o kerberos5.o sra.o

OBJS=	auth.o encrypt.o genget.o getent.o gettytab.o misc.o \
enc_des.o sra.o ssl.o

#CFLAGS= -DENCRYPT -DAUTHENTICATE -DKRB4 -DSRA -DDES_ENCRYPT \
#-I../kerberosIV/include -I../libbsd/include -I../libpk

# WAS
#CFLAGS= -DENCRYPT -DAUTHENTICATE -DSRA -DUSE_SSL -DDES_ENCRYPTION \
#-I../libbsd/include -I../libpk

CFLAGS= -DAUTHENTICATE -DUSE_SSL \
-I../libbsd/include -I$(SSLTOP)/include -I../libpk -g

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
