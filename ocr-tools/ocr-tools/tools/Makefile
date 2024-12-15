all: unmunge repair munge

OPT = -g -O -W -Wall
COMMON_OBJS = util.o

UNMUNGE_OBJS = $(COMMON_OBJS) unmunge.o
MUNGE_OBJS = $(COMMON_OBJS) munge.o
REPAIR_OBJS = $(COMMON_OBJS) heap.o mempool.o subst.o repair.o

unmunge: $(UNMUNGE_OBJS)
	$(CC) $(OPT) -o $@ $(UNMUNGE_OBJS)

munge: $(MUNGE_OBJS)
	$(CC) $(OPT) -o $@ $(MUNGE_OBJS)

repair: $(REPAIR_OBJS)
	$(CC) $(OPT) -o $@ $(REPAIR_OBJS)

.c.o:
	$(CC) $(OPT) -o $@ -c $<

clean:
        -rm -f *.o munge unmunge repair core *.core

unmunge.o: util.h
munge.o: util.h
repair.o: heap.h mempool.h util.h subst.h
heap.o: heap.h
mempool.o: mempool.h
subst.o: subst.h
