CFLAGS = -O2 -I../include $(FLAGS)
OBJ = skipmod.o skip.o queue.o interface.o com.o
LIB = ../lib/libskip_kernel.a

MOD = skip
INSTALLDIR = /opt/skip
BINARIES = skipd skip_attach skip_detach skip_stat skip_addkey
SKIPDCONF = skipd.conf
MOD = skip
RC = init.d/skip
MAJOR = 79

autoconfig:	
	@cd ..; make $@; cd irix

all:	$(MOD)
	

$(MOD):	$(OBJ) $(LIB)
	$(LD) -non_shared -coff -G 0 -r -d -o $(MOD) $(OBJ) $(LIB)

install:	
	-mkdir -p $(INSTALLDIR)
	install -s -f $(INSTALLDIR) -m 600 -u root -g sys Makefile
	install -s -f $(INSTALLDIR) -m 700 -u root -g sys $(MOD)
	install -s -f /etc -m 744 -u root -g sys $(RC)
	(cd ../skipd; \
	install -s -f /etc -m 400 -u root -g sys $(SKIPDCONF); \
	for i in $(BINARIES); do \
		install -s -f $(INSTALLDIR) -m 700 -u root -g sys $$i; \
	done; cd ../irix)
	-mknod /dev/skip c $(MAJOR) 0; chmod 600 /dev/skip
	ln -s /etc/init.d/skip /etc/rc2.d/S31skip
	ln -s /etc/init.d/skip /etc/rc2.d/K31skip
	chkconfig -f skip on
	@./patchrtsymtab
	@/etc/autoconfig
	sync; sync

uninstall:	
	-rm -f /etc/config/skip
	-rm -f $(INSTALLDIR)/$(MOD)
	-rm -f /etc/$(RC) /etc/$(SKIPDCONF) /etc/rc2.d/???skip
	-for i in $(BINARIES); do rm -f $(INSTALLDIR)/$$i; done
	-rm -f /dev/$(MOD)
	-rm -f $(INSTALLDIR)/Makefile
	sync; sync

load:	
	@ml ld -v -f skip -p skip -s $(MAJOR)

unload:
	@ml unld -v `ml list | grep skip | awk '{ print $$2 }'`

boot:	
	ln -s /etc/init.d/skip /etc/rc2.d/S31skip
	ln -s /etc/init.d/skip /etc/rc2.d/K31skip

unboot:
	rm -f /etc/rc2.d/?31skip

clean:	
	-rm -f *.o $(MOD) Makefile.bak .depend
	sed -e '/^# Do not delete, dependencies are assumed below this line/q' >.Makefile Makefile; mv .Makefile Makefile

# Do not delete, dependencies are assumed below this line
