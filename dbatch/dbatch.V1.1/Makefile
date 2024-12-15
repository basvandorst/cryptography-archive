#CC	= alint
#CFLAGS	= -abchx -c
CC	= acc
CFLAGS	= -c
#CFLAGS	= -c -DDEBUG

all: dbatch dbatchd

dbatchd:	dbatchd.o base.o ipc.o osd.o job.o user.o host.o
	$(CC) -o dbatchd dbatchd.o base.o ipc.o osd.o job.o user.o host.o -lnsl
	chmod go-rwx dbatchd

dbatch:	dbatch.o base.o ipc.o os.o
	$(CC) -o dbatch dbatch.o base.o ipc.o os.o -lnsl -lrpcsvc
	chmod a+x dbatch

base.o:	dbatch.h base.c
	$(CC) $(CFLAGS) base.c

osd.o:	dbatch.h osd.c
	$(CC) $(CFLAGS) osd.c

os.o:	dbatch.h os.c
	$(CC) $(CFLAGS) os.c

ipc.o:	dbatch.h ipc.c
	$(CC) $(CFLAGS) ipc.c 2>&1 | grep -v /usr/include/sys/ioccom.h

job.o:	dbatch.h job.c
	$(CC) $(CFLAGS) job.c

user.o:	dbatch.h user.c
	$(CC) $(CFLAGS) user.c

host.o:	dbatch.h host.c
	$(CC) $(CFLAGS) host.c

dbatchd.o:	dbatch.h dbatchd.c
	$(CC) $(CFLAGS) dbatchd.c

dbatch.o:	dbatch.h dbatch.c
	$(CC) $(CFLAGS) dbatch.c
