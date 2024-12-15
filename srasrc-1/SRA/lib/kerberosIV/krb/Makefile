LIB=	krb
CFLAGS= -DKERBEROS -I../include
OBJS=	cr_err_reply.o create_auth_reply.o create_ciph.o \
	create_death_packet.o create_ticket.o debug_decl.o decomp_ticket.o \
	dest_tkt.o extract_ticket.o fgetst.o get_ad_tkt.o get_admhst.o \
	get_cred.o get_in_tkt.o get_krbhst.o get_krbrlm.o get_phost.o \
	get_pw_tkt.o get_request.o get_svc_in_tkt.o get_tf_fullname.o \
	get_tf_realm.o getrealm.o getst.o in_tkt.o k_gethostname.o klog.o \
	kname_parse.o kntoln.o kparse.o krb_err_txt.o krb_get_in_tkt.o \
	kuserok.o log.o mk_err.o mk_priv.o mk_req.o mk_safe.o month_sname.o \
	netread.o netwrite.o one.o pkt_cipher.o pkt_clen.o rd_err.o \
	rd_priv.o rd_req.o rd_safe.o read_service_key.o recvauth.o \
	save_credentials.o send_to_kdc.o sendauth.o stime.o tf_util.o \
	tkt_string.o util.o krb_err.et.o


lib${LIB}.a: ${OBJS}
	echo building standard ${LIB} library 
	rm -f lib${LIB}.a 
	if [ -f /usr/bin/lorder ]; then \
		ar cq lib${LIB}.a `lorder ${OBJS} | tsort`  ; \
	else \
		ar cq lib${LIB}.a ${OBJS} ; \
	fi;
	if [ -f /usr/bin/ranlib ]; then \
		ranlib lib${LIB}.a; \
	fi;

krb_err.et.c: ../compile_et/compile_et.c
	(cd ../compile_et;make CC="$(CC)")
	../compile_et/compile_et krb_err.et -n

clean:
	rm -f *.o lib${LIB}.a 
	(cd ../compile_et;make clean)
