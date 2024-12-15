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


#define STD 0
#define PEM 1
#define RAW 2

#include "secure.h"
#include <stdio.h>
#include <fcntl.h>

#ifdef __STDC__
void cip_octets(unsigned char	 *outblock, unsigned Z[7][10] );

#else
void cip_octets();
#endif /* not __STDC__ */

/***************************************************************
 *
 * Procedure des_encrypt
 *
 ***************************************************************/
#ifdef __STDC__

int idea_encrypt(
	OctetString	 *in_octets,
	BitString	 *out_bits,
	More		  more,
	KeyInfo		 *keyinfo
)

#else

int idea_encrypt(
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
	static char	remaining_buf[16];

	register int	encrypted_blocks, octets_to_be_encrypted, encrypted_octets;
	register char	*inp, *outp;
	register int	i;
	unsigned int	remaining_octets_new;
	char	*proc = "des_encrypt";

        static unsigned e_key[7][10],
                d_key[7][10];
        short unsigned key[9];


	if (first) {
                algenc = aux_ObjId2AlgEnc(keyinfo->subjectAI->objid);
                algmode = aux_ObjId2AlgMode(keyinfo->subjectAI->objid);
                algspecial = aux_ObjId2AlgSpecial(keyinfo->subjectAI->objid);


		padding = RAW;
		if (algspecial == WITH_PEM_PADDING) padding = PEM;
		if (algspecial == WITH_PADDING)	padding = STD;

		remaining_octets = 0;

		if (keyinfo->subjectkey.nbits != 128) return(-1);

		for(i=0; i<8; i++ ) {
			key[i+1] = keyinfo->subjectkey.bits[i*2] + keyinfo->subjectkey.bits[i*2+1] * 256;
		}
        	idea_key(key, e_key);

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
			cip_octets((unsigned char *)outp, e_key);
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
			cip_octets((unsigned char *)outp, e_key);
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
				cip_octets((unsigned char *)outp, e_key);
			encrypted_octets += 8;
			out_bits->nbits += 64;
			break;
		case PEM:
			for (i = remaining_octets; i < 8; i++) 
				remaining_buf[i] = 8 - remaining_octets;
			bcopy(remaining_buf, outp, 8);
				cip_octets((unsigned char *)outp, e_key);
			encrypted_octets += 8;
			out_bits->nbits += 64;
			break;
		case RAW:
			if (remaining_octets == 0) 
				break;
			for (i = remaining_octets; i < 8; i++) 
				remaining_buf[i] = 0;
			bcopy(remaining_buf, outp, 8);
			cip_octets((unsigned char *)outp, e_key);
			encrypted_octets += 8;
			out_bits->nbits += 64;
			break;
		}
	}

	return(encrypted_octets * 8);
}

#ifdef __STDC__

int idea_decrypt(
	BitString	 *in_bits,
	OctetString	 *out_octets,
	More		  more,
	KeyInfo		 *keyinfo
)

#else


int idea_decrypt(
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
 
       static unsigned e_key[7][10],
                d_key[7][10];
        short unsigned key[9];

	if (first) {
                algenc = aux_ObjId2AlgEnc(keyinfo->subjectAI->objid);
                algmode = aux_ObjId2AlgMode(keyinfo->subjectAI->objid);
                algspecial = aux_ObjId2AlgSpecial(keyinfo->subjectAI->objid);


		padding = RAW;
		if (algspecial == WITH_PEM_PADDING) padding = PEM;
		if (algspecial == WITH_PADDING)	padding = STD;

		remaining_bits = 0;
		remaining_octets = 0;


		if (keyinfo->subjectkey.nbits != 128) return(-1);

		for(i=0; i<8; i++ ) {
			key[i+1] = keyinfo->subjectkey.bits[i*2] + keyinfo->subjectkey.bits[i*2+1] * 256;
		}
        	idea_key(key, e_key);
		idea_de_key(e_key,d_key);


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
			cip_octets((unsigned char *)outp, d_key);
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
			cip_octets((unsigned char *)outp, d_key);
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
		switch (padding) {
		case STD:
			outp--;
			i = *outp;
			if (i < 0 || i > 7) {
				aux_add_error(EDECRYPT, "wrong bytecount (STD)", 0, 0, proc);
				return(-1);
			}
			i = 8 - i;
			out_octets->noctets -= i;
			decrypted_octets -= i;
			break;
		case PEM:
			outp--;
			i = *outp;
			if (i <= 0 || i > 7) {
				aux_add_error(EDECRYPT, "wrong bytecount (PEM)", 0, 0, proc);
				return(-1);
			}
			out_octets->noctets -= i;
			decrypted_octets -= i;
			break;
		}
	}

	return(decrypted_octets);
}


/***************************************************************
 *
 * Procedure cip_octets
 *
 ***************************************************************/
#ifdef __STDC__

void cip_octets(
	unsigned char	 *outblock,
	unsigned Z[7][10]
)

#else

void cip_octets(
	outblock,
	Z
)
unsigned char	 *outblock;
unsigned Z[7][10];

#endif

{
	unsigned IN[5];
	unsigned OUT[5];
	int i;

	for(i=0;i<4;i++) IN[i+1] = outblock[2*i]+outblock[2*i+1]*0x100;

	idea_cip(IN, OUT, Z);

	for(i=0;i<4;i++) {
		outblock[2*i] = OUT[i+1] % 0x100;
		OUT[i+1] /= 0x100;
		outblock[2*i+1] = OUT[i+1] % 0x100;
	}

}
