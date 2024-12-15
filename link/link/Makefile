#
# Defines Used in the code alterable here
# SOCKET  - debugging, link listens on a socket
# NOCRYPT - des and undes are replaced with null functions
# DEBUG   - for connect.c , wont touch the tty disciplines
#           and wont catch any signals (so we can get core)
# PORT    - if defined changes the debugging port number
# MEMCPY  - if defined, use memcpy() in place of bcopy()
# 

CFLAGS = 

#HPUX needs -DMEMCPY
#CFLAGS =  -DMEMCPY 

#Debugging
#CFLAGS =  -g -DMEMCPY -DDEBUG -DPORT=91234 -DNOCRYPT -DSOCKET 

# connect is a socket client for debugging
all:  link connect

link : link.o slip.o des.o sock.o  d3des.o common.o do_rsa.o
	cc link.o slip.o des.o sock.o d3des.o common.o do_rsa.o -o link $(LIBS)

connect : connect.o slip.o des.o d3des.o common.o do_rsa.o
	cc connect.o slip.o des.o d3des.o common.o do_rsa.o -o connect $(LIBS)

link.o : link.c
slip.o : slip.c common.h bcopy.h
des.o  : des.c bcopy.h
common.o : common.c common.h bcopy.h
connect.o : connect.c
d3des.o : DES/d3des.c
	cc -O -c DES/d3des.c
do_rsa.o : RSA/rsa.c
	cd RSA;make do_rsa.o;cp do_rsa.o ..

# DEBUGGING
sock.o : sock.c bcopy.h

clean :
	/bin/rm -f *.o core 

clobber:
	/bin/rm -f *.o core .link* connect link *Error* *Debug*

