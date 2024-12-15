CFLAGS_KERNEL=-O2 -I../include -I. $(FLAGS_KERNEL)
CFLAGS_USER=-g -I../include -I. $(FLAGS_USER)
OFILES_KERNEL=kernel/skipcache.o kernel/ipsum.o kernel/longlong.o kernel/sequence.o kernel/sign.o kernel/crypt.o kernel/cryptdes.o kernel/cryptidea.o kernel/cryptrc4.o kernel/cryptsimple.o kernel/signmd5.o kernel/ipsp.o kernel/random.o
OFILES_USER=user/skipcache.o user/sign.o user/crypt.o user/cryptdes.o user/cryptrc4.o user/cryptsimple.o user/signmd5.o user/cryptidea.o user/iaddr.o user/longlong.o user/parse.o user/random.o

autoconfig load unload install uninstall:	
	@cd ..; make $@; cd lib

all:	libskip_kernel.a libskip_user.a
	

kernel user:
	mkdir $@; for i in *.c; do ln -s ../$$i $@; done

$(OFILES_KERNEL):	
	$(CC) $(CFLAGS_KERNEL) -o $@ -c $*.c

$(OFILES_USER):		
	$(CC) $(CFLAGS_USER) -o $@ -c $*.c

libskip_user.a:	user $(OFILES_USER)
	ar cr $@ $(OFILES_USER)
	if test -s /bin/ranlib; then /bin/ranlib $@; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib $@; fi; \
	fi

libskip_kernel.a:	kernel $(OFILES_KERNEL)
	ar cr $@ $(OFILES_KERNEL) $(OFILES); \
	if test -s /bin/ranlib; then /bin/ranlib $@; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib $@; fi; \
	fi

clean:	
	-rm -rf kernel user *.a *~ Makefile.bak .depend
	sed -e '/^# Do not delete, dependencies are assumed below this line/q' >.Makefile Makefile; mv .Makefile Makefile

# Do not delete, dependencies are assumed below this line
