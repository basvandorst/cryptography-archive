SSLINC=/usr/local/ssl/include
SSLLIB=/usr/local/ssl/lib
CFLAGS=-Wall -O2 -I$(SSLINC) -g

OBJS= pfx.o pfx_asfe.o pfx_bitm.o pfx_ccrl.o pfx_decr.o pfx_crts.o \
 pfx_espk.o pfx_lib.o pfx_mac.o pfx_pass.o pfx_pbe.o pfx_priv.o pfx_sbag.o \
 pfx_supd.o pfx_objs.o pfx_init.o pfx_add.o pfx_encr.o pfx_pk.o pfx_chn.o \
 pfx_utl.o hmac.o pfx_hash.o pfx_key.o pfx_crpt.o

pfx:	$(OBJS) pfx.h
	cc -g -L$(SSLLIB) -o pfx $(OBJS) -lcrypto

$(OBJS): pfx.h

clean:	
	rm -f pfx $(OBJS) core

proto:
	cproto -I$(SSLINC) pfx*.c >pfxproto.h
