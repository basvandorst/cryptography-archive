# make          used to build the executable
# make lint     used for debugging
# make show     used to compute the encrypted password

CC      = acc -O2

all:	dep_backup

dep_backup:	dep_backup.c idea.c idea.h
	$(CC) -o dep_backup dep_backup.c

show:	dep_backup.c idea.c idea.h
	$(CC) -o show_dep_backup -DSHOWPASSWORD dep_backup.c
	./show_dep_backup -
	/bin/rm -f show_dep_backup

lint:	dep_backup.c idea.c idea.h
	alint -abchx dep_backup.c
