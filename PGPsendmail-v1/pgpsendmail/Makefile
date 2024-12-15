# Makefile for  PGPsendmail  by  Richard Gooch
# The SENDMAIL line should be uncommented if you don't have privilege to
# replace  /usr/lib/sendmail  .You will then need to manually install
# PGPsendmail somewhere and make sure your Mail User Agent is told to use
# PGPsendmail instead of  /usr/lib/sendmail
# For EMACS ViewMail, you might add the following to your  ~/.emacs  file:
# (setq sendmail-program "/user/fred/pgp/bin/pgpsendmail")
# If you do have privilege, make sure the SENDMAIL line is commented out,
# otherwise your machine will be rooted (ask an Australian what that means;-).
# Check this for every new version of PGPsendmail/Unix
# Note also: you may need to change "/usr/lib/sendmail" to "/usr/sbin/sendmail"
# on some systems.
#
#SENDMAIL = '-DSENDMAIL="/usr/lib/sendmail"'

DEFINES =

CC = cc
CFLAGS = $(DEFINES) -I. "-DVERSION=\"`cat version`\""

LDFLAGS =

TARGETS =	blindness_assist pgpsendmail pgpdaemon sendpgppass pgppipe

all:	$(TARGETS) clean

BLINDNESS_ASSIST_OBJ =	blindness_assist.o

blindness_assist.o:	blindness_assist.c
	$(CC) $(CFLAGS) $(SENDMAIL) -c blindness_assist.c

blindness_assist:	$(BLINDNESS_ASSIST_OBJ)
	$(CC) -s -o blindness_assist $(BLINDNESS_ASSIST_OBJ) $(LDFLAGS)


PGPSENDMAIL_OBJ =	pgpsendmail.o strings.o keylist.o spawn.o options.o \
			conn_to_pgpd.o memory.o misc.o

pgpsendmail.o:	pgpsendmail.c
	$(CC) $(CFLAGS) $(SENDMAIL) -c pgpsendmail.c

pgpsendmail:	$(PGPSENDMAIL_OBJ)
	$(CC) -s -o pgpsendmail $(PGPSENDMAIL_OBJ) $(LDFLAGS)
	@-./blindness_assist
	-./install pgpsendmail


PGPDAEMON_OBJ =	pgpdaemon.o spawn.o strings.o memory.o misc.o

pgpdaemon:	$(PGPDAEMON_OBJ)
	$(CC) -s -o pgpdaemon $(PGPDAEMON_OBJ) $(LDFLAGS)
	-./install pgpdaemon


SENDPGPPASS_OBJ =	sendpgppass.o conn_to_pgpd.o strings.o spawn.o \
			memory.o misc.o

sendpgppass:	$(SENDPGPPASS_OBJ)
	$(CC) -s -o sendpgppass $(SENDPGPPASS_OBJ) $(LDFLAGS)
	-./install sendpgppass


PGPPIPE_OBJ =	pgppipe.o conn_to_pgpd.o memory.o misc.o

pgppipe:	$(PGPPIPE_OBJ)
	$(CC) -s -o pgppipe $(PGPPIPE_OBJ) $(LDFLAGS)
	-./install pgppipe


clean:
	rm -f *.BAK *.log *.sed .#* *~ *.bak */*.bak */tmp* .*defaults */*~
	-chmod u+rw,go-w * 2> /dev/null

distclean:
	rm -f *.BAK *.log *.sed .#* *~ */*~ *.bak */*.bak */tmp* .*defaults
	rm -f $(TARGETS) *.o
	-chmod u+rw,go-w * 2> /dev/null

# Dependencies
pgpsendmail.o:	pgpsendmail.h	karma.h	version
strings.o:	pgpsendmail.h	karma.h
keylist.o:	pgpsendmail.h	karma.h
spawn.o:	pgpsendmail.h	karma.h
options.o:	pgpsendmail.h	karma.h
pgpdaemon.o:	pgpsendmail.h	karma.h	version
conn_to_pgpd.o:	pgpsendmail.h	karma.h
sendpgppass.o:	pgpsendmail.h	karma.h	version
memory.o:	pgpsendmail.h	karma.h
pgppipe.o:	pgpsendmail.h	karma.h	version
misc.o:		pgpsendmail.h	karma.h
