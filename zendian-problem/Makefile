#	Makefile for "chain" utility

#	last edit:	92/02/11	D A Gwyn

#	SCCS ID:	@(#)chain.mk	1.1 (edited for export)

PRODUCT = chain
MAKEFIL	= Makefile
CFILES	= $(PRODUCT).c
OBJS	= $(PRODUCT).o
TFILES	= $(PRODUCT).in1 $(PRODUCT).exp1 \
	  $(PRODUCT).in2 $(PRODUCT).exp2 \
	  $(PRODUCT).in3 $(PRODUCT).exp3
MFILES	= $(PRODUCT).1
BINDIR	= /usr/local/bin
MANDIR	= /usr/man/local/man1
BINPERM	= 755
MANPERM	= 644
CFLAGS	= -O
LDFLAGS = # -n
INS	= cp
SPOOL	= | lp
NROFF	= nroff -Tlp -man
TROFF	= troff -Taps -man
POST	= | daps
PROOF	= | proof

all:		$(PRODUCT) $(MFILES)

$(PRODUCT):	$(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)
	size $@

print:		$(MFILES) $(MAKEFIL) $(CFILES) $(TFILES)
	@( for i in $(MFILES) ; do $(NROFF) $$i ; done ; \
	  pr $(MAKEFIL) $(CFILES) $(TFILES) $(MFILES) ) $(SPOOL)

typeset:	$(MFILES)
	@for i in $(MFILES) ; do $(TROFF) $$i $(POST) ; done

proof:	$(MFILES)
	@for i in $(MFILES) ; do $(TROFF) $$i $(PROOF) ; done

lint:		$(CFILES)
	lint $(CFILES) > $(PRODUCT).lint

flow:		$(HEADERS) $(CFILES)
	cflow $(CFILES) > $(PRODUCT).flow

xref:		$(HEADERS) $(CFILES)
	cxref -c -s -w132 $(CFILES) > $(PRODUCT).xref

test:		$(PRODUCT) $(TFILES)
	./$(PRODUCT) -w '?' < $(PRODUCT).in1 > $(PRODUCT).out 2> $(PRODUCT).err
	@if cmp $(PRODUCT).exp1 $(PRODUCT).out ; \
	then	echo 'Test 1 okay' ; \
	else	echo 'Test 1 failed; differences:' ; \
		diff $(PRODUCT).exp1 $(PRODUCT).out ; \
		exit 1 ; \
	fi
	./$(PRODUCT) -d '-' < $(PRODUCT).in2 > $(PRODUCT).out 2> $(PRODUCT).err
	@if cmp $(PRODUCT).exp2 $(PRODUCT).out ; \
	then	echo 'Test 2 okay' ; \
	else	echo 'Test 2 failed; differences:' ; \
		diff $(PRODUCT).exp2 $(PRODUCT).out ; \
		exit 1 ; \
	fi
	./$(PRODUCT) -d '' < $(PRODUCT).in3 > $(PRODUCT).out 2> $(PRODUCT).err
	@if cmp $(PRODUCT).exp3 $(PRODUCT).out ; \
	then	echo 'Test 3 okay' ; \
	else	echo 'Test 3 failed; differences:' ; \
		diff $(PRODUCT).exp3 $(PRODUCT).out ; \
		exit 1 ; \
	fi

compare:	all test
	cmp $(BINDIR)/$(PRODUCT) $(PRODUCT)
	for i in $(MFILES) ; do cmp $(MANDIR)/$$i $$i ; done

install:	all test
	-chmod $(BINPERM) $(PRODUCT)
	@if cmp -s $(BINDIR)/$(PRODUCT) $(PRODUCT); \
	 then	echo $(PRODUCT) unchanged; \
	 else	$(INS) $(PRODUCT) $(BINDIR); \
		echo $(PRODUCT) installed; \
	 fi
	-chmod $(MANPERM) $(MFILES)
	@for i in $(MFILES) ; \
	do	if cmp $(MANDIR)/$$i $$i ; \
		then	echo $$i unchanged; \
		else	$(INS) $$i $(MANDIR); \
			echo $$i installed; \
		fi ; \
	done

clean:
	for i in $(CFILES) $(TFILES) ; \
	do	if [ -f s.$$i ]; \
		then	if [ -f p.$$i ]; \
			then	echo "*** File \"$$i\" is being edited."; \
			else	rm -f $$i; \
			fi; \
		fi; \
	done
	-rm -f $(OBJS) $(PRODUCT).lint $(PRODUCT).flow \
		$(PRODUCT).xref $(PRODUCT).out $(PRODUCT).err

clobber:	clean
	for i in $(MFILES) ; \
	do	if [ -f s.$$i ]; \
		then	if [ -f p.$$i ]; \
			then	echo "*** File \"$$i\" is being edited."; \
			else	rm -f $$i; \
			fi; \
		fi; \
	done
	-rm -f $(PRODUCT)
