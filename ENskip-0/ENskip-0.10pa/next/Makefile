CFLAGS= -O2 -DKERNEL -DMACH -DMACH_USER_API -I../include -I.
MIGFILES=migUser.c migServer.c mig.h
OFILES=main.o interface.o com.o skip.o queue.o migServer.o
LIB=../lib/libskip_kernel.a

INSTALLDIR=/usr/local/skip
BINARIES = skipd skip_attach skip_detach skip_stat skip_addkey
SKIPDCONF = skipd.conf
MOD = skip_reloc
RC = rc.skip

autoconfig:	
	@cd ..; make $@; cd next

all:	skip_reloc
	

LoadCommands:	LoadCommands

UnloadCommands:	UnloadCommands

$(MIGFILES):	skip.defs
	mig skip.defs

skip_reloc:	$(OFILES) LoadCommands UnloadCommands $(LIB)
	kl_ld -n skip -l LoadCommands -u UnloadCommands -i self \
	-d skip_loadable -o $@ $(OFILES) $(LIB)

load:	
	/usr/etc/kl_util -a skip_reloc

unload:	
	/usr/etc/kl_util -d skip

boot:	
	-if [ "`grep '# skip' /etc/rc.local`X" = "X" ]; then \
	   cat rc.local >> /etc/rc.local; \
	fi

unboot:	
	mv -f /etc/rc.local /tmp/.rc.local
	cat /tmp/.rc.local | grep -v '# skip' > /etc/rc.local
	rm -f /tmp/.rc.local

install:	
	-mkdir $(INSTALLDIR)
	install -m 400 -o root -g wheel Makefile $(INSTALLDIR)
	install -m 400 -o root -g wheel $(MOD) $(INSTALLDIR)
	install -m 500 -o root -g wheel $(RC) $(INSTALLDIR)
	install -m 600 -o root -g wheel ../skipd/$(SKIPDCONF) /etc
	for i in $(BINARIES); do \
	  install -s -m 700 -o root -g wheel ../skipd/$$i $(INSTALLDIR); \
	done
	-if [ "`grep '# skip' /etc/rc.local`X" = "X" ]; then \
	   cat rc.local >> /etc/rc.local; \
	fi
	sync; sync

uninstall:
	-rm -f $(INSTALLDIR)/$(MOD) $(INSTALLDIR)/$(RC)
	-rm -f /etc/$(SKIPDCONF)
	-for i in $(BINARIES); do rm -f $(INSTALLDIR)/$$i; done
	mv -f /etc/rc.local /tmp/.rc.local
	cat /tmp/.rc.local | grep -v '# skip' > /etc/rc.local
	rm -f /tmp/.rc.local
	rm -f $(INSTALLDIR)/Makefile
	sync; sync

clean:	
	rm -f *.o *_reloc *_loadable *~ $(MIGFILES) Makefile.bak .depend
	sed -e '/^# Do not delete, dependencies are assumed below this line/q' >.Makefile Makefile; mv .Makefile Makefile

# Do not delete, dependencies are assumed below this line
