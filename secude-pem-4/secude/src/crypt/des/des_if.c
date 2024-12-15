/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*
 *  DES interface module between sec_encrypt/sec_decrypt
 *  and Phil Karn's DES package
 *
 *  WS 6.12.90
 *
 *  Last change: 7.12.90
 *
 *  Imports from Karn's:
 *
 *  desinit(mode) int mode;
 *  setkey_(key) char *key;
 *  endes(block) char *block;
 *  dedes(block) char *block;
 *  desdone()
 *
 *  Exports to libdes.a:
 *
 *  des_encrypt(in_octets, out_bits, more, keyinfo)
 *  des_decrypt(in_bits, out_octets, more, keyinfo)
 *  read_dec(fd, buf, len, pin)
 *  write_enc(fd, buf, len, pin)
 *  string_to_key(pin) 
 *
 */

#define STD 0
#define PEM 1
#define RAW 2
/* STARCOS */
#define B1  3
/* STARCOS - ENDE */

#include "secure.h"
#include <stdio.h>
#include <fcntl.h>

#ifdef MAC
#include <Unix.h>
#include <stdlib.h>
#include <string.h>
#include "Mac.h"
#endif /* MAC */

static char	iv[8];  /* Initialization vector */
static int	DESINIT = FALSE;
static char     des3;
static unsigned char	des_hash_key1[8] = 
{ 
	0x9a, 0xd3, 0xbc, 0x24, 0x10, 0xe2, 0x8f, 0x0e };


static unsigned char	des_hash_key2[8] = 
{ 
	0xe2, 0x95, 0x14, 0x33, 0x59, 0xc3, 0xec, 0xa8 };

#ifdef __STDC__
       int	desinit	(int mode, Boolean des3);
       int	desdone	(Boolean des3);
       int	setkey_	(char *key);
       int	setkey1	(char *key);
       int	endes	(char *block);
       int	endes1	(char *block);
       int	dedes	(char *block);
       int	dedes1	(char *block);
       long	f	(unsigned long r, unsigned char subkey[8]);
       unsigned long	byteswap	(unsigned long x);
static int	desfirst	(KeyInfo *keyinfo, int des_type);
#else

       int	desinit	();
       int	desdone	();
       int	setkey_	();
       int	setkey1	();
       int	endes	();
       int	endes1	();
       int	dedes	();
       int	dedes1	();
       long	f	();
       unsigned long	byteswap	();
static int	desfirst	();

#endif /* not __STDC__ */

/***************************************************************
 *
 * Procedure des_encrypt
 *
 ***************************************************************/
#ifdef __STDC__

int des_encrypt(
	OctetString	 *in_octets,
	BitString	 *out_bits,
	More		  more,
	KeyInfo		 *keyinfo
)

#else

int des_encrypt(
	in_octets,
	out_bits,
	more,
	keyinfo
)
OctetString	 *in_octets;
BitString	 *out_bits;
More		  more;
KeyInfo		 *keyinfo;

#endif

{
	static int	first = TRUE, des_type, padding;
        AlgEnc   algenc;
	AlgMode  algmode;
        AlgSpecial algspecial;
	static unsigned int	remaining_octets;
	static char	remaining_buf[8];

	register int	encrypted_blocks, octets_to_be_encrypted, encrypted_octets;
	register char	*inp, *outp;
	register int	i;
	unsigned int	remaining_octets_new;
	char	*proc = "des_encrypt";

	if (first) {
                algenc = aux_ObjId2AlgEnc(keyinfo->subjectAI->objid);
                algmode = aux_ObjId2AlgMode(keyinfo->subjectAI->objid);
                algspecial = aux_ObjId2AlgSpecial(keyinfo->subjectAI->objid);

                if(algenc == DES3) des3 = TRUE;
                else des3 = FALSE;

		if (algmode == CBC) des_type = CBC;
		else des_type = ECB;

		padding = RAW;
		if (algspecial == WITH_PEM_PADDING) padding = PEM;
		if (algspecial == WITH_PADDING)	padding = STD;
/* STARCOS */
		if (algspecial == WITH_B1_PADDING) padding = B1;
/* STARCOS - ENDE */

		remaining_octets = 0;
		if (desfirst(keyinfo, des_type) < 0) {
			aux_add_error(EINVALID, "desfirst failed", 0, 0, proc);
			return(-1);
		}
		first = FALSE;
	}
	if (more == SEC_END) first = TRUE;

	octets_to_be_encrypted = in_octets->noctets;
	inp = in_octets->octets;
	outp = out_bits->bits + (out_bits->nbits / 8);

	encrypted_octets = 0;

	if (remaining_octets) {

		for (i = remaining_octets; i < 8; i++) {
			if (octets_to_be_encrypted - 1 >= 0) {
				remaining_buf[i] = *inp++;
				octets_to_be_encrypted--;
				remaining_octets++;
			} else break;
		}
		if (i == 8) {
			bcopy(remaining_buf, outp, 8);
			if (des_type == CBC) 
                                endes_cbc(outp);
			else 
				endes_ecb(outp);
			outp += 8;
			encrypted_octets = 8;
			remaining_octets = 0;
		}

	}

	encrypted_blocks = octets_to_be_encrypted / 8;
	remaining_octets_new = octets_to_be_encrypted % 8;

	if (encrypted_blocks) {
		octets_to_be_encrypted = encrypted_blocks * 8;
		if (inp != outp) 
			bcopy(inp, outp, octets_to_be_encrypted);
		for (i = 0; i < encrypted_blocks; i++) {
			if (des_type == CBC) 
				endes_cbc(outp);
			else 
				endes_ecb(outp);
			inp += 8;
			outp += 8;
		}
		encrypted_octets += octets_to_be_encrypted;
	}

	out_bits->nbits += (encrypted_octets * 8);

	if (remaining_octets_new) {
		for (i = remaining_octets; i < remaining_octets + remaining_octets_new; i++) 
			remaining_buf[i] = *inp++;
		remaining_octets += remaining_octets_new;
	}

	if (more == SEC_END) {
		switch (padding) {
		case STD:
			remaining_buf[7] = remaining_octets;
			bcopy(remaining_buf, outp, 8);
			if (des_type == CBC) 
				endes_cbc(outp);
			else 
				endes_ecb(outp);
			encrypted_octets += 8;
			out_bits->nbits += 64;
			break;
		case PEM:
			for (i = remaining_octets; i < 8; i++) 
				remaining_buf[i] = 8 - remaining_octets;
			bcopy(remaining_buf, outp, 8);
			if (des_type == CBC) 
				endes_cbc(outp);
			else 
				endes_ecb(outp);
			encrypted_octets += 8;
			out_bits->nbits += 64;
			break;
		case RAW:
			if (remaining_octets == 0) 
				break;
			for (i = remaining_octets; i < 8; i++) 
				remaining_buf[i] = 0;
			bcopy(remaining_buf, outp, 8);
			if (des_type == CBC) 
				endes_cbc(outp);
			else 
				endes_ecb(outp);
			encrypted_octets += 8;
			out_bits->nbits += 64;
			break;
/* STARCOS */
		case B1:
			remaining_buf[remaining_octets++] = 0x80;
			for (i = remaining_octets; i < 8; i++) 
				remaining_buf[i] = 0x00;
			bcopy(remaining_buf, outp, 8);
			if (des_type == CBC) 
				endes_cbc(outp);
			else 
				endes_ecb(outp);
			encrypted_octets += 8;
			out_bits->nbits += 64;
			break;
/* STARCOS - ENDE */
		}
		c_desdone(des3);
	}

	return(encrypted_octets * 8);
}


/***************************************************************
 *
 * Procedure endes_cbc
 *
 ***************************************************************/
#ifdef __STDC__

void endes_cbc(
	char	 *outblock
)

#else

void endes_cbc(
	outblock
)
char	 *outblock;

#endif

{
	register char	*cp, *cp1;
	register int	i;

	/* CBC mode; chain in last cipher word */

	cp = outblock;
	cp1 = iv;
	for (i = 8; i != 0; i--) *cp++ ^= *cp1++;

	endes_ecb(outblock);	/* in-block encryption */

	/* Save outblockgoing ciphertext for chain */

	bcopy(outblock, iv, 8);
}

/***************************************************************
 *
 * Procedure endes_ecb
 *
 ***************************************************************/
#ifdef __STDC__

void endes_ecb(
	char	 *outblock
)

#else

void endes_ecb(
	outblock
)
char	 *outblock;

#endif

{
        if(des3) {
                endes(outblock);
                dedes1(outblock);
                endes(outblock);
        }
        else endes(outblock);
        return;
}

/***************************************************************
 *
 * Procedure des_decrypt
 *
 ***************************************************************/
#ifdef __STDC__

int des_decrypt(
	BitString	 *in_bits,
	OctetString	 *out_octets,
	More		  more,
	KeyInfo		 *keyinfo
)

#else

int des_decrypt(
	in_bits,
	out_octets,
	more,
	keyinfo
)
BitString	 *in_bits;
OctetString	 *out_octets;
More		  more;
KeyInfo		 *keyinfo;

#endif

{
	static int	first = TRUE, des_type, padding;
        AlgEnc   algenc;
        AlgMode  algmode;
        AlgSpecial  algspecial;
	static unsigned int	remaining_bits, remaining_octets;
	static unsigned char	remaining_buf[8];

	register int	decrypted_blocks, octets_to_be_decrypted, bits_to_be_decrypted, decrypted_octets;
	register char	*inp, *outp;
	register int	i;
	unsigned int	remaining_bits_new, remaining_octets_new;
	char	*proc = "des_decrypt";
	unsigned char   c;
	int		index;

	if (first) {
                algenc = aux_ObjId2AlgEnc(keyinfo->subjectAI->objid);
                algmode = aux_ObjId2AlgMode(keyinfo->subjectAI->objid);
                algspecial = aux_ObjId2AlgSpecial(keyinfo->subjectAI->objid);

                if(algenc == DES3) des3 = TRUE;
                else des3 = FALSE;

		if (algmode == CBC) des_type = CBC;
		else des_type = ECB;

		padding = RAW;
		if (algspecial == WITH_PEM_PADDING) padding = PEM;
		if (algspecial == WITH_PADDING)	padding = STD;
/* STARCOS */
		if (algspecial == WITH_B1_PADDING) padding = B1;
/* STARCOS - ENDE */

		remaining_bits = 0;
		remaining_octets = 0;
		if (desfirst(keyinfo, des_type) < 0) {
			aux_add_error(EINVALID, "desfirst failed", 0, 0, proc);
			return(-1);
		}
		first = FALSE;
	}
	if (more == SEC_END) first = TRUE;

	bits_to_be_decrypted = in_bits->nbits;
	inp = in_bits->bits;
	outp = out_octets->octets + out_octets->noctets;

	decrypted_octets = 0;

	if (remaining_bits) {

		/* This works only if remaining_bits is a multiple of 8.
                   Major stuff must be inserted here to handle arbitrary in_bits->nbits  */

		for (i = remaining_octets; i < 8; i++) {
			if (bits_to_be_decrypted - 8 >= 0) {
				remaining_buf[i] = *inp++;
				bits_to_be_decrypted -= 8;
				remaining_bits += 8;
				remaining_octets++;
			} else 
				break;
		}
		if (i == 8) {
			bcopy(remaining_buf, outp, 8);
			if (des_type == CBC) 
				dedes_cbc(outp);
			else 
				dedes_ecb(outp);
			outp += 8;
			decrypted_octets = 8;
			remaining_octets = 0;
			remaining_bits = 0;
		}

	}

	decrypted_blocks = bits_to_be_decrypted / 64;
	remaining_bits_new = bits_to_be_decrypted % 64;
	if (decrypted_blocks) {
		octets_to_be_decrypted = decrypted_blocks * 8;
		if (inp != outp) 
			bcopy(inp, outp, octets_to_be_decrypted);
		for (i = 0; i < decrypted_blocks; i++) {
			if (des_type == CBC) 
				dedes_cbc(outp);
			else 
				dedes_ecb(outp);
			inp += 8;
			outp += 8;
		}
		decrypted_octets += octets_to_be_decrypted;
	}

	out_octets->noctets += decrypted_octets;

	if (remaining_bits_new) {
		if (more == SEC_END) {
			aux_add_error(EINVALID, "input not multiple of eight", 0, 0, proc);
			return(-1);
		}
		remaining_octets_new = remaining_bits_new / 8;
		if (remaining_bits_new % 8) 
			remaining_octets_new++;
		for (i = remaining_octets; i < remaining_octets + remaining_octets_new; i++) 
			remaining_buf[i] = *inp++;
		remaining_octets += remaining_octets_new;
		remaining_bits += remaining_bits_new;
	}

	if (more == SEC_END) {
		c_desdone(des3);
		switch (padding) {
		case STD:
			outp--;
			i = *outp;
			if (i < 0 || i > 7) {
				aux_add_error(EDECRYPTION, "wrong bytecount (STD)", 0, 0, proc);
				return(-1);
			}
			i = 8 - i;
			out_octets->noctets -= i;
			decrypted_octets -= i;
			break;
		case PEM:
			outp--;
			i = *outp;
			if (i <= 0 || i > 8) {
				aux_add_error(EDECRYPTION, "wrong bytecount (PEM)", 0, 0, proc);
				return(-1);
			}
			out_octets->noctets -= i;
			decrypted_octets -= i;
			break;
/* STARCOS */
		case B1:
			outp--;
			c = *outp;
			/* test last padding byte */
			if ((*outp != 0x00) && ((*outp & 0xFF) != 0x80)) {
				aux_add_error(EDECRYPTION, "wrong bytecount (PEM)", 0, 0, proc);
				return(-1);
			}
			index = 1;
			while (c == 0x00) {
				index++;
				outp--;
				c = *outp;
			}
			    
			if ((*outp & 0xFF) != 0x80) {
				aux_add_error(EDECRYPTION, "wrong bytecount (PEM)", 0, 0, proc);
				return(-1);
			}
			out_octets->noctets -= index;
			decrypted_octets -= index;
			break;
/* STARCOS - ENDE */
		}
	}

	return(decrypted_octets);
}


/***************************************************************
 *
 * Procedure dedes_cbc
 *
 ***************************************************************/
#ifdef __STDC__

void dedes_cbc(
	char	 *outblock
)

#else

void dedes_cbc(
	outblock
)
char	 *outblock;

#endif

{
	char	ivtmp[8];
	register char	*cp, *cp1;
	register int	i;


	/* Save incoming ciphertext for chain */

	bcopy(outblock, ivtmp, 8);

	dedes_ecb(outblock);   /* in-block decryption */

	/* Unchain block, save ciphertext for next */

	cp = outblock;
	cp1 = iv;
	for (i = 8; i != 0; i--)	
		*cp++ ^= *cp1++;
	bcopy(ivtmp, iv, 8);
}

/***************************************************************
 *
 * Procedure dedes_ecb
 *
 ***************************************************************/
#ifdef __STDC__

void dedes_ecb(
	char	 *outblock
)

#else

void dedes_ecb(
	outblock
)
char	 *outblock;

#endif

{
        if(des3) {
                dedes(outblock);
                endes1(outblock);
                dedes(outblock);
        }
        else dedes(outblock);
        return;
}


/***************************************************************
 *
 * Procedure desfirst
 *
 ***************************************************************/
#ifdef __STDC__

static int desfirst(
	KeyInfo	 *keyinfo,
	int	  des_type
)

#else

static int desfirst(
	keyinfo,
	des_type
)
KeyInfo	 *keyinfo;
int	  des_type;

#endif

{
	OctetString *algparm;
	char	*proc = "desfirst";

	if (DESINIT == FALSE) {
		if (desinit(0, des3) < 0) return(-1);
		DESINIT = TRUE;
	}

	if (keyinfo->subjectkey.nbits == 64 && !des3) setkey_(keyinfo->subjectkey.bits);
        else if(keyinfo->subjectkey.nbits == 128 && des3) setdoublekey(keyinfo->subjectkey.bits);
	else {
		aux_add_error(EINVALID, "wrong key length", 0, 0, proc);
		return(-1);
	}
	if (des_type == CBC) {
		/* Initialization Vector */
		if ((algparm = (OctetString * )keyinfo->subjectAI->param))
			if (algparm->noctets == 8) 
				bcopy(algparm->octets, iv, 8);
			else bzero(iv, 8);
	}
	return(0);
}

/***************************************************************
 *
 * Procedure setdoublekey
 *
 ***************************************************************/
#ifdef __STDC__

int setdoublekey(
	char	 *key
)

#else

int setdoublekey(
	key
)
char	 *key;

#endif

{
        setkey_(key);
        setkey1(key + 8);
        return(0);
}

/***************************************************************
 *
 * Procedure read_dec
 *
 ***************************************************************/
#ifdef __STDC__

int read_dec(
	int	  fd,
	char	 *buf,
	int	  len,
	char	 *key
)

#else

int read_dec(
	fd,
	buf,
	len,
	key
)
int	  fd;
char	 *buf;
int	  len;
char	 *key;

#endif

{
	Key deskey;
	static FILE *ff;
	static int	first = TRUE, in, des_type, wout;
	static char	work[8], nwork[8], *w, *nw, *ww;
	int	out = 0, rest;
	AlgMode algmode;
	register int	block;
	register char	*bb;
	char	*proc = "read_dec";

	if (!buf && first == FALSE) {
		c_desdone(FALSE);
		fclose(ff);
		first = TRUE;
		return(0);
	}

	if (!len) return(0);

	if (!key || !strlen(key)) return(read(fd, buf, len));

	if (first) {
		deskey.keyref = 0;
		deskey.pse_sel = (PSESel * )0;
		if (sec_string_to_key(key, &deskey) < 0) {
			AUX_ADD_ERROR;
			return(-1);
		}
		algmode = aux_ObjId2AlgMode(deskey.key->subjectAI->objid);
		if (algmode == CBC) des_type = CBC;
		else des_type = ECB;

                des3 = FALSE;

		if (desfirst(deskey.key, des_type) < 0) {
			aux_free_KeyInfo(&(deskey.key));
			AUX_ADD_ERROR;
			return(-1);
		}

		aux_free_KeyInfo(&(deskey.key));
		if (!(ff = (FILE *)fdopen(fd, "r"))) {
			aux_add_error(ESYSTEM, "fdopen failed", 0, 0, proc);
			return(-1);
		}
		if ((in = fread(work, 1, 8, ff)) != 8) {
			aux_add_error(ESYSTEM, "fread failed", 0, 0, proc);
			return(-1);
		}

		w = work;
		nw = nwork;
		wout = 0;
		if (des_type == CBC) dedes_cbc(w);
		else dedes_ecb(w);
		first = FALSE;
	}

	bb = buf;

	if (len % 8 == 0 && wout == 0) {
		if (in) {
			bcopy(w, bb, 8);
			bb += 8;
			len -= 8;
			out += 8;
		} 
		else return(0);
		if ((in = fread(bb, 1, len, ff))) {
			for (block = 0; block < in / 8; block++) {
				if (des_type == CBC) dedes_cbc(bb);
				else dedes_ecb(bb);
				bb += 8;
				out += 8;
			}
		}
		if (in < len || ((in = fread(w, 1, 8, ff)) != 8)) {
			rest = *(bb - 1);
			if (rest < 0 || rest > 7) {
				aux_add_error(EDECRYPTION, "DES decryption error: wrong bytecount", 0, 0, proc);
				return(-1);
			}
			out -= (8 - rest);
			in = 0;
			return(out);
		}
		if (des_type == CBC) dedes_cbc(w);
		else dedes_ecb(w);
		return(out);
	}

	while (1) {
		if (wout) {
			if (len < wout) {
				bcopy(ww, bb, len);
				ww += len;
				wout -= len;
				out += len;
				return(out);
			}
			bcopy(ww, bb, wout);
			len -= wout;
			out += wout;
			bb += wout;
			wout = 0;
			if (w == &work[0]) {
				w = nwork;
				nw = work;
			} 
			else {
				w = work;
				nw = nwork;
			}
		}
		if (len == 0 || in == 0) return(out);
		if ((in = fread(nw, 1, 8, ff))) {
			if (des_type == CBC) dedes_cbc(nw);
			else dedes_ecb(nw);
		}
		ww = w;
		if (in == 8) wout = 8;
		else wout = w[7];
		if (wout == 0) return(out);
	}
}


/***************************************************************
 *
 * Procedure write_enc
 *
 ***************************************************************/
#ifdef __STDC__

int write_enc(
	int	  fd,
	char	 *buf,
	int	  len,
	char	 *key
)

#else

int write_enc(
	fd,
	buf,
	len,
	key
)
int	  fd;
char	 *buf;
int	  len;
char	 *key;

#endif

{
	Key deskey;
	static int	first = TRUE;
	static FILE *ff;
	static char	work[8];
	static char	*w;
	static int	des_type, wout;
	int	out = 0;
	AlgMode algmode;
	register int	block;
	register char	*bb;
	int	i;
	char	*proc = "write_enc";

	if (!buf && first == FALSE) {
		work[7] = wout;
		if (des_type == CBC) endes_cbc(work);
		else endes_ecb(work);
		fwrite(work, 1, 8, ff);
		fclose(ff);
		c_desdone(FALSE);
		first = TRUE;
		return(0);
	}

	if (!len) return(0);

	if (!key || !strlen(key)) return(write(fd, buf, len));

	if (first) {
		deskey.keyref = 0;
		deskey.pse_sel = (PSESel * )0;
		if (sec_string_to_key(key, &deskey) < 0) {
			AUX_ADD_ERROR;
			return(-1);
		}

		algmode = aux_ObjId2AlgMode(deskey.key->subjectAI->objid);
		if (algmode == CBC) des_type = CBC;
		else des_type = ECB;

                des3 = FALSE;

		if (desfirst(deskey.key, des_type) < 0) {
			aux_free_KeyInfo(&(deskey.key));
			AUX_ADD_ERROR;
			return(-1);
		}

		aux_free_KeyInfo(&(deskey.key));
		if (!(ff = (FILE *)fdopen(fd, "w"))) {
			aux_add_error(ESYSTEM, "fdopen failed", 0, 0, proc);
			return(-1);
		}

		w = work;
		wout = 0;
		first = FALSE;
	}

	bb = buf;
	if (wout == 0 && len > 8) {
		for (block = 0; block < len / 8; block++) {
			if (des_type == CBC) endes_cbc(bb);
			else endes_ecb(bb);
			bb += 8;
		}
		out = bb - buf;
		fwrite(buf, 1, out, ff);
		len -= out;
	}
	while (len) {
		*w++ = *bb++;
		wout++;
		out++;
		if (wout == 8) {
			if (des_type == CBC) endes_cbc(work);
			else endes_ecb(work);
			fwrite(work, 1, 8, ff);
			wout = 0;
			w = work;
		}
		len--;
	}
	return(out);
}


/***************************************************************
 *
 * Procedure close_dec
 *
 ***************************************************************/
#ifdef __STDC__

int close_dec(
	int	  fd
)

#else

int close_dec(
	fd
)
int	  fd;

#endif

{
	read_dec(fd, CNULL, 0, CNULL);
	close(fd);

	return(0);
}


/***************************************************************
 *
 * Procedure close_enc
 *
 ***************************************************************/
#ifdef __STDC__

int close_enc(
	int	  fd
)

#else

int close_enc(
	fd
)
int	  fd;

#endif

{
	write_enc(fd, CNULL, 0, CNULL);
	close(fd);

	return(0);
}


/***************************************************************
 *
 * Procedure c_desdone
 *
 ***************************************************************/
#ifdef __STDC__

int c_desdone(
	Boolean	  des3
)

#else

int c_desdone(
	des3
)
Boolean	  des3;

#endif

{

        desdone(des3);
        DESINIT = FALSE;

	return(0);
}


/* This is supposed to be a one-way string-to-key function. */

/***************************************************************
 *
 * Procedure string_to_key
 *
 ***************************************************************/
#ifdef __STDC__

char *string_to_key(
	char	 *asckey
)

#else

char *string_to_key(
	asckey
)
char	 *asckey;

#endif

{
	register char	*p;
	register int	i;
	char	k1[8], k2[8];
	char	*key;
	char	*proc = "string_to_key";

	if (!(key = (char *)malloc(8))) {
		aux_add_error(EMALLOC, "key", 0, 0, proc);
		return(CNULL);
	}
	if (DESINIT == FALSE) {
		if (desinit(0, FALSE) < 0) {
			aux_add_error(EINVALID, "desinit failed", 0, 0, proc);
			free(key);
			return(CNULL);
		}

		DESINIT = TRUE;
	}

	for (i = 0; i < 8; i++) {
		k1[i] = k2[i] = 0;
	}
	for (i = 0, p = asckey; *p; p++, i++) {
		i %= 8;
		k1[i] |= *p;
		k2[i] |= *p;
		setkey_((char *)des_hash_key1);
		endes(k1);
		setkey_((char *)des_hash_key2);
		endes(k2);
	}
	for (i = 0; i < 8; i++) {
		key[i] = k1[i] ^ k2[i];
	}
	return(key);
}


