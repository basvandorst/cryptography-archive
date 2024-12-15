LIBTARGET = libpgpsockets.a
OBJS = PGPsockets.o

LIBS = -lsocket -lnsl 

CC= gcc
OPT= -O -g
DEFINES=-DPGP_DEBUG=1 -DPROTOTYPES -D_REENTRANT
LOCALINCLUDES= -I../../../../../pub/include\
-I../../../../../unix\
-I../../../../../../pfl/common\
-I../../../../../../pfl/unix

CFLAGS=	$(OPT) $(OSDEF) -W -Wshadow -Wpointer-arith -Wmissing-prototypes\
	-Wwrite-strings -DHAVE_CONFIG_H $(DEFINES) $(LOCALDEFINES) \
	$(LOCALINCLUDES) $(CCOPTS)

all:: $(LIBTARGET) DONE

$(LIBTARGET): $(OBJS)
	$(AR) ruv $(LIBTARGET) $(OBJS)

DONE:
	@set -x; echo $(OBJS) > $@

clean:
	rm -f $(OBJS) $(LIBTARGET)

# These empty targets are to keep the sdk build happy
headers:
depend:

