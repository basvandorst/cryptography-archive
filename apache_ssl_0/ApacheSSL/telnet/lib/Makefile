#SUBDIRS= gmp-1.2 kerberosIV/krb libbsd libdes/des libpk libtelnet libutil
SUBDIRS= gmp-1.3.2 libbsd libpk libtelnet libutil

SUBDIRS= libbsd libtelnet libutil

all:
	@for i in $(SUBDIRS); \
		do (cd $$i && echo $$i && $(MAKE) CC="$(CC)" LDADD="$(LDADD)") || exit; \
	done

clean:
	@for i in $(SUBDIRS); \
		do (cd $$i && echo $$i && $(MAKE) clean) || exit; \
	done
