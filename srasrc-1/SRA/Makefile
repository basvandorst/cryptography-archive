SUBDIRS= lib ftp ftpd telnet telnetd login

# uncomment the next line for Solaris 1
#CC= gcc -DSUNOS

# uncomment the next two lines for Solaris2:
CC = gcc -DSUNOS -DSOLARIS2 -DUSE_SHADOW
LDADD= -lsocket -lnsl

# uncomment the next three lines for linux
#CC = gcc -DLINUX -DUSE_SHADOW
#LDADD = -lshadow
#SUBDIRS= lib ftp ftpd telnet telnetd

all:
	@for i in $(SUBDIRS); \
		do (cd $$i && echo $$i && $(MAKE) CC="$(CC)" LDADD="$(LDADD)") || exit; \
	done

clean:
	@for i in $(SUBDIRS); \
		do (cd $$i && echo $$i && $(MAKE) clean) || exit; \
	done
