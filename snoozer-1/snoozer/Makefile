#
#
#   SNOOZER - ver 1.0
#           - (c) Gino Corradin - Gino.Corradin@dei.unipd.it
#
# Local variables : edit these to reflect your local environment
#SNOOZERHOME	=/usr/local/lib/security/snoozer
#SNOOZERHOME	=/tmp/snoozer
SNOOZERHOME	=/home/gino/project/snoozer
IFLAGS		=#-DINFO=3
SEND_MAIL	=-DSEND_MAIL

CC		=gcc
OFLAGS		=-O2

# Don't edit these lines
PREFS		=-DSNOOZERHOME=\"$(SNOOZERHOME)\" $(IFLAGS) $(SEND_MAIL)
CFLAGS		=$(PREFS) $(OFLAGS)
OBJS		=src/snoozer.o src/utils.o \
		src/jobown.o src/jobperm.o src/jobtime.o src/jobsign.o \
		src/crc32.o src/md4.o src/md5.o 
SRCS		=byteorder.c crc32.c debug.h defs.h \
		jobown.c jobperm.c jobsign.c jobtime.c \
		md4.c md5.c snoozer.c utils.c version.h

all: checkvars byteorder $(OBJS) 
	$(CC) $(CFLAGS) -o snoozer $(OBJS)
	@strip snoozer 
	@cp src/reconfig reconfig 
	@cp src/quit quit 

checkvars:
	@if [ ! $(SNOOZERHOME) ];then \
		echo -en "   The SNOOZERHOME variable must be defined in\n"; \
		echo -en "   this Makefile, define it and recompile all.\n"; \
		exit 1; \
	fi
	@if [ ! $(SEND_MAIL) ];then \
		if [ ! -x `grep MAILER config/mail.h|cut -d\" -f2` ];then \
			echo -en "   The MAILER variable in the file config/mail.h\n"; \
			echo -en "   file indicates a wrong filename.\n"; \
			exit 2; \
		fi; \
	fi

byteorder:
	@if [ ! -x src/byteorder ];then \
		echo "Compiling src/byteorder"; \
		(cd src; $(CC) $(OFLAGS) -o byteorder byteorder.c); \
		(cd src; ./byteorder); \
	fi
	@if [ ! -s src/byteorder.h ];then \
		(cd src; ./byteorder); \
	fi

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $< 

clean:
	rm -f $(OBJS) src/byteorder src/byteorder.h
	rm -f snoozer jobtab.errlog jobtab.pid reconfig quit

install: all
	@if [ $(SNOOZERHOME) = `pwd` ];then \
		echo "Installation isn't necessary."; \
		echo "Snoozer is already in destination dir."; \
		exit; \
	else \
		echo "Install the entire package in $(SNOOZERHOME)"; \
		if [ ! -d $(SNOOZERHOME) ];then mkdir $(SNOOZERHOME); fi; \
		chmod 700 $(SNOOZERHOME); \
		install -m 755 -s snoozer $(SNOOZERHOME); \
		install -m 755 reconfig quit $(SNOOZERHOME); \
		if [ ! -d $(SNOOZERHOME)/doc ];then mkdir $(SNOOZERHOME)/doc; fi; \
		install -m 444 doc/* $(SNOOZERHOME)/doc; \
		install -m 444 License Makefile README $(SNOOZERHOME); \
		install -m 644 jobtab $(SNOOZERHOME); \
		if [ ! -d $(SNOOZERHOME)/input ];then mkdir $(SNOOZERHOME)/input; fi; \
		install -m 644 input/* $(SNOOZERHOME)/input; \
		if [ ! -d $(SNOOZERHOME)/output ];then mkdir $(SNOOZERHOME)/output; fi; \
		if [ ! -d $(SNOOZERHOME)/src ];then mkdir $(SNOOZERHOME)/src; fi; \
		for i in $(SRCS); do \
			install -m 444 src/$$i $(SNOOZERHOME)/src; \
		done; \
		install -m 555 src/reconfig src/quit $(SNOOZERHOME)/src; \
		if [ ! -d $(SNOOZERHOME)/config ];then mkdir $(SNOOZERHOME)/config; fi; \
		install -m 444 config/*.h $(SNOOZERHOME)/config; \
		if [ ! -d $(SNOOZERHOME)/sec ];then mkdir $(SNOOZERHOME)/sec; fi; \
	fi

pack:
	@echo "Making backup archive sec/snoozer.tar.gz..."
	@if [ ! -d sec ];then \
		mkdir sec; \
	fi
	@if [ -s sec/snoozer.tar.gz ];then \
		if [ -s sec/snoozer.tar.gzOLD ];then \
			mv -f sec/snoozer.tar.gzOLD sec/snoozer.tar.gzVERYOLD; \
			mv -f sec/snoozer.tar.gz sec/snoozer.tar.gzOLD; \
		else \
			mv -f sec/snoozer.tar.gz sec/snoozer.tar.gzOLD; \
		fi; \
	fi
	@(cd ..;find snoozer -type f | grep -v "/sec/" | grep -v "/tmp/" > snoozer/tmp/.allfiles)
	@(cd ..;tar -czf snoozer/sec/snoozer.tar.gz `cat snoozer/tmp/.allfiles`) 
	@rm -f tmp/.allfiles

