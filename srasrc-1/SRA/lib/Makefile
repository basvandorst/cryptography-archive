SUBDIRS= gmp-1.2 kerberosIV/krb libbsd libdes/des libpk libtelnet libutil

all:
	@for i in $(SUBDIRS); \
		do (cd $$i && echo $$i && $(MAKE) CC="$(CC)" LDADD="$(LDADD)") || exit; \
	done

clean:
	@for i in $(SUBDIRS); \
		do (cd $$i && echo $$i && $(MAKE) clean) || exit; \
	done
