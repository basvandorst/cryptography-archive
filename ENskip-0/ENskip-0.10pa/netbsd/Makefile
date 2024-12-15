CFLAGS= -O2 -I../include $(FLAGS)
OFILES=skipmod.o com.o interface.o queue.o skip.o
LIB=../lib/libskip_kernel.a

MODOBJ=skiplkm.o
MOD=skipmod

INSTALLDIR=/usr/local/skip
BINARIES = skipd skip_attach skip_detach skip_stat skip_addkey
SKIPDCONF = skipd.conf
RC = rc.skip

autoconfig:	
	@cd ..; make $@; cd netbsd

all:	$(MODOBJ)
	

load:
	@modload -o $(MOD) -e$(MOD) $(MODOBJ)

unload:
	modunload -n $(MOD)

boot:	
	-if [ "`grep '# skip' /etc/rc.local`X" = "X" ]; then \
	   cat rc.local >> /etc/rc.local; \
	fi

unboot:	
	mv -f /etc/rc.local /tmp/.rc.local
	cat /tmp/.rc.local | grep -v '# skip' > /etc/rc.local
	rm -f /tmp/.rc.local

install:	
	install -d $(INSTALLDIR)
	install -c -m 400 -o root -g wheel Makefile $(INSTALLDIR)
	install -c -m 400 -o root -g wheel $(MODOBJ) $(INSTALLDIR)
	install -c -m 500 -o root -g wheel $(RC) $(INSTALLDIR)
	install -c -m 600 -o root -g wheel ../skipd/$(SKIPDCONF) /etc
	for i in $(BINARIES); do \
	  install -s -c -m 700 -o root -g wheel ../skipd/$$i $(INSTALLDIR); \
	done
	-if [ "`grep '# skip' /etc/rc.local`X" = "X" ]; then \
	   cat rc.local >> /etc/rc.local; \
	fi
	sync; sync

uninstall:	
	-rm -f $(INSTALLDIR)/$(MODOBJ) $(INSTALLDIR)/$(MOD) $(INSTALLDIR)/$(RC)
	-rm -f /etc/$(SKIPDCONF)
	-for i in $(BINARIES); do rm -f $(INSTALLDIR)/$$i; done
	mv -f /etc/rc.local /tmp/.rc.local
	cat /tmp/.rc.local | grep -v '# skip' > /etc/rc.local
	rm -f /tmp/.rc.local
	rm -f $(INSTALLDIR)/Makefile
	sync; sync

$(MODOBJ): $(OFILES) $(LIB)
	$(LD) -r -o $(MODOBJ) $(OFILES) $(LIB)

clean:	
	rm -f *.o $(MOD) Makefile.bak .depend
	sed -e '/^# Do not delete, dependencies are assumed below this line/q' >.Makefile Makefile; mv .Makefile Makefile

# Do not delete, dependencies are assumed below this line
