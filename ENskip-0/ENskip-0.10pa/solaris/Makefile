CFLAGS = -O2 -I../include $(FLAGS)

INSTALLDIR = /usr/kernel/drv
BININSTALLDIR = /opt/skip
OBJ = skipmod.o skip.o queue.o interface.o com.o
LIB = ../lib/libskip_kernel.a
BINARIES = skipd skip_attach skip_detach skip_stat skip_addkey
SKIPDCONF = skipd.conf
MOD = skip
CONF = $(MOD).conf
RC = init.d/skip

autoconfig:	
	@cd ..; make $@; cd solaris

all:	$(MOD)
	

$(MOD):	$(OBJ) $(LIB)
	$(LD) -r -o $(MOD) $(OBJ) $(LIB)

install:	
	-mkdir -p $(BININSTALLDIR)
	install -s -c $(BININSTALLDIR) -m 644 -u root -g sys Makefile
	install -s -c $(INSTALLDIR) -m 755 -u root -g sys $(MOD)
	install -s -c $(INSTALLDIR) -m 644 -u root -g sys $(CONF)
	install -s -c /etc/init.d -m 744 -u root -g sys $(RC)
	install -s -c /etc -m 600 -u root -g sys ../skipd/$(SKIPDCONF)
	for i in $(BINARIES); do \
	  install -s -c $(BININSTALLDIR) -m 700 -u root -g sys ../skipd/$$i; \
	done
	add_drv -m '* 0600 root root' $(MOD)
	ln /etc/init.d/skip /etc/rc2.d/S01skip
	ln /etc/init.d/skip /etc/rc2.d/K01skip
	sync; sync

uninstall:
	-rem_drv $(MOD)
	-rm -f $(INSTALLDIR)/$(MOD)
	-rm -f $(INSTALLDIR)/$(CONF)
	-rm -f /etc/$(RC) /etc/$(SKIPDCONF) /etc/rc2.d/???skip
	-for i in $(BINARIES); do rm -f $(BININSTALLDIR)/$$i; done
	-rm -f /dev/$(MOD) '/devices/pseudo/clone@0:skip'
	-rm -f $(BININSTALLDIR)/Makefile
	sync; sync

load:	
	modload skip; modinfo | grep skip #> BASIS

unload:	
	modunload -i `modinfo | grep skip | awk '{ print $$1 }'` 

boot:
	ln /etc/init.d/skip /etc/rc2.d/S01skip
	ln /etc/init.d/skip /etc/rc2.d/K01skip

unboot:
	rm /etc/rc2.d/?01skip


clean:	
	-rm -f *.o $(MOD) Makefile.bak .depend
	sed -e '/^# Do not delete, dependencies are assumed below this line/q' >.Makefile Makefile; mv .Makefile Makefile

# Do not delete, dependencies are assumed below this line
