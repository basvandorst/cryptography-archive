CFILES = hpwd.c demo.c
OBJS = demo.o hpwd.o
HFILES = descrip.h b_order.h

CFLAGS = -traditional -c
CC = gcc

pwd: $(OBJS) $(HFILES)
	$(CC) $(OBJS) -o hpwd

$(OBJS): $(CFILES)
	$(CC) $(CFLAGS) $(CFILES)
