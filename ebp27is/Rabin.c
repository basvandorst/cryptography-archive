/*
 * rabin.c - These functions wrap and unwrap message digests (MDs) and
 * data encryption keys (DEKs) in padding and Rabin-encrypt them into
 * multi-precision integers. Even though Philip did not implement the
 * Rabin public key scheme, Loefgren/Jarl used a lot of his code to
 * implement it for EBP.
 *
 * EBP(c) - Even Better Privacy - is the new generation of public key
 * cryptography. EBP is based on the code to PGP 2.6.3(i) and was developed
 * by Niklas Jarl (LTH, ISS-NUS), Jonas Loefgren (LTH, ISS-NUS) and 
 * Dr. Yongfei Han (ISS-NUS). The main purpose of this release is to let
 * PGP users and other people that may need strong cryptography try the new
 * algorithms and give them the option to choose the algorithms they trust
 * the most. 
 * 
 * WARNING! This program is for non-commercial use only! It may not be used
 * commercially in any way. To do so you need a written permission not only
 * by the authors of EBP, but also the authors and patentees of the various 
 * algorithms. However, look out for the coming program called 
 * "PC Privacy(c)", which will be a fully commercial cryptographic program
 * with even more choice of algorithms. 
 *
 *  Now over to the old PGP information:
 *
 * (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
 * The author assumes no liability for damages resulting from the use
 * of this software, even if the damage results from defects in this
 * software.  No warranty is expressed or implied.
 *
 * Note that while most PGP source modules bear Philip Zimmermann's
 * copyright notice, many of them have been revised or entirely written
 * by contributors who frequently failed to put their names in their
 * code.  Code that has been incorporated into PGP from other authors
 * was either originally published in the public domain or is used with
 * permission from the various authors.
 *
 * PGP is available for free to the public under certain restrictions.
 * See the PGP User's Guide (included in the release package) for
 * important information about licensing, patent restrictions on
 * certain algorithms, trademarks, copyrights, and export controls.
 */

#include <string.h> 	/* for mem*() */
#include "mpilib.h"
#include "mpiio.h"
#include "genprime.h"
#include "ebp.h"
#include "rabin.h"
#include "random.h"	/* for cryptRandByte() */

/* These functions hide all the internal details of Rabin-encrypted
 * keys and digests.  They owe a lot of their heritage to
 * the preblock() and postunblock() routines in mpiio.c.
 */

/* Header used to check which of the four Rabin solutions that is the right one */
static byte header[] = {73,03,11,39,50};

/* Abstract Syntax Notation One (ASN.1) Distinguished Encoding Rules (DER)
   encoding for RSA/HAVAL/MD5, used in PKCS-format signatures. */
static byte asn_array[] = {	/* PKCS 01 block type 01 data */
	0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
	0x02,0x05,0x05,0x00,0x04,0x10 };

/* This many bytes from the end, there's a zero byte */
#define ASN_ZERO_END 3

/* Bytes of pre-header random padding. */
#define MSBRND 9

/* The decryption with Rabin's scheme is done in the following steps:
 *
 * First we calculate	
 *		
 *		m0 = C^[(p+1)/4] mod p
 *		m1 = (p - C^[(p+1)/4]) mod p
 *		m2 = C^[(q+1)/4] mod q 
 *		m3 = (p - C^[(q+1)/4]) mod q
 *
 * where C is the ciphertext.
 *
 * With the a and b chosen to be
 *
 *		a = q*(1/q mod p)
 *		b = p*(1/p mod q)
 *
 * we caclulate the four possible solutions
 *
 *		M0 = (a*m0 + b*m2) mod n
 *		M1 = (a*m0 + b*m3) mod n
 *		M2 = (a*m1 + b*m2) mod n
 *		M3 = (a*m1 + b*m3) mod n
 *
 * One of these solutions is the decrypted ciphertext.
 * To be able to distinguish which solution is the right one, a header is 
 * added to the message.
 * 
 * In the detailed implementation, there is a temporary, temp, used to
 * hold intermediate results, the exponent (p+1)/4 is temporarily held in 
 * m[1] and (q+1)/4 in m[3].  
 * With this, you should be able to understand the code below.
 */
int Rabin_Decrypt(unitptr rabout, unitptr rabin,
		  unitptr p, unitptr q, unitptr n, unitptr a, unitptr b)
{
	unit m[4][MAX_UNIT_PRECISION];
	unit M[MAX_UNIT_PRECISION];
    unit temp[MAX_UNIT_PRECISION];

	byte *front;
	int status, j, k;
	boolean found = FALSE;


/*  Compute m[1] = (p+1)/4 and m[3] = (q+1)/4 */
	mp_move(temp,p);
	mp_inc(temp);
	mp_init(M,4);
	mp_div(m[0],m[1],temp,M); /* m[0]=rem, M=div, m[2]=0 */
	mp_init(m[2],0);
	if (mp_compare(m[0],m[2]))
		return -1;
	mp_move(temp,q);
	mp_inc(temp);
	mp_div(m[0],m[3],temp,M);
	if (mp_compare(m[0],m[2]))
		return -1;
	

	mp_mod(temp, rabin, p);
	status = mp_modexp(M,temp, m[1], p); /*  m0 = C^((p+1)/4) mod p */
    if (status < 0) {		/* mp_modexp returned an error. */
		mp_init(rabout, 1);
		return status;		/* error return */
    }
	stage_modulus(n);     /* Used for mp_modmult */
	mp_modmult(m[0],a,M);    /* m[0] =  a*m0 */

	mp_move(temp,p); /*   m1 = (p - C^((p+1)/4) mod p  */	
	mp_sub(temp,M);
	mp_modmult(m[1],a,temp); /* m[1] = a*m1 */



	mp_mod(temp, rabin, q);
	status = mp_modexp(M, temp, m[3], q);/*  m2 = C^((q+1)/4) mod q */
    if (status < 0) {		/* mp_modexp returned an error. */
		mp_init(rabout, 1);
		return status;		/* error return */
    }
	stage_modulus(n);     /* Used for mp_modmult */
	mp_modmult(m[2],b,M);    /* m[2] = b*m2 */

	mp_move(temp,q);	                 /*  m3 = (q - C^((q+1)/4) mod q	*/
	mp_sub(temp,M);
	mp_modmult(m[3],b,temp); /* m[3] = b*m3 */
		
	
/* Calculate the four possible solutions
 *   M0 = (a*m0 + b*m2) mod n
 *	 M1 = (a*m0 + b*m3) mod n
 *	 M2 = (a*m1 + b*m2) mod n
 *	 M3 = (a*m1 + b*m3) mod n
 */
	j=0;
	while (j<2 & !found){
	k=2;
	while (k<4 & !found){
		mp_move(temp,m[j]);	/* M = (a*mj + b*mk) mod n */
		mp_add(temp,m[k]);
		mp_mod(M,temp,n);

		/* Is this the right solution? */
		mp_convert_order((byte *)M);
		front = (byte *)M;			/* Start of block */

		found = TRUE;
		while (!*front++)		/* Skip the leading 0's */
			;
		front += MSBRND;      /* And skip the first random padding */
		if (memcmp(front, header, sizeof(header)))/* Check the five byte header */
			found = FALSE;

		if (found)
			mp_move(rabout,M);
		k++;
	}
	j++;
	}
	
    mp_burn(temp);		/* burn the evidence on the stack... */
    mp_burn(m[0]);
    mp_burn(m[1]);
    mp_burn(m[2]);
    mp_burn(m[3]);
    mp_burn(M);

 	if (!found)
		return -1; /* If no solution is found */
	return 0;			/* normal return */
}				/* Rabin_Decrypt */


/* The signation with Rabin's scheme is done in the following steps:
 *
 * First we calculate	
 *
 *		m0 = C^[(p+1)/4] mod p
 *		m1 = (p - C^[(p+1)/4]) mod p
 *		m2 = C^[(q+1)/4] mod q 
 *		m3 = (p - C^[(q+1)/4]) mod q
 * 
 * where C is the message digest to be signed.
 * With the a and b chosen to be
 *
 *		a = q*(1/q mod p)
 *		b = p*(1/p mod q)
 *
 * we calculate one possible solution
 *
 *		M0 = (a*m0 + b*m2) mod n
 *
 * The next step is to check whether this solution can be used to sign the 
 * message. If not, we change the random padding to get a new C and then we 
 * start all over again until we have a C which can be used for signing.
 * This is needed because only C's which are a quadratic residues modulo n, 
 * can be used, i.e., if x^2 mod n = C mod n has a solution. 
 * 
 * In the detailed implementation, there is a temporary, temp, used to
 * hold intermediate results, pexp is the exponent (p+1)/4 and qexp is 
 * (q+1)/4.  
 * With this, you should be able to understand the code below.
 */

		  
int Rabin_Sign(unitptr rabout, unitptr rabin,
		  unitptr p, unitptr q, unitptr n, unitptr a, unitptr b)
{
	unit pexp[MAX_UNIT_PRECISION];
	unit qexp[MAX_UNIT_PRECISION];
	unit m0[MAX_UNIT_PRECISION];
	unit m2[MAX_UNIT_PRECISION];
	unit M[MAX_UNIT_PRECISION];
    unit temp[MAX_UNIT_PRECISION];

	byte *point;
	int status, i, Padding;
	boolean found;

	i = (countbytes(n)-1) - 1; 
	Padding = i - MSBRND + 1;
	found = FALSE;

/*  Compute pexp = (p+1)/4 and qexp = (q+1)/4 */
	mp_move(temp,p);
	mp_inc(temp);
	mp_init(M,4);
	mp_div(m0,pexp,temp,M); /* m0=rem, M=div, m2=0 */
	mp_init(m2,0);
	if (mp_compare(m0,m2))
		return -1;
	mp_move(temp,q);
	mp_inc(temp);
	mp_div(m0,qexp,temp,M);
	if (mp_compare(m0,m2))
		return -1;

while (!found){ /* Can this C be used for signing? */

	mp_mod(temp, rabin, p);
	status = mp_modexp(M,temp, pexp, p); /*  m0 = C^pexp mod p	*/
    if (status < 0) {		/* mp_modexp returned an error. */
		mp_init(rabout, 1);
		return status;		/* error return */
    }
	stage_modulus(n);     /* Used for mp_modmult */
	mp_modmult(m0,a,M); /* m0 = a*m0 */


	mp_mod(temp, rabin, q);
	status = mp_modexp(M, temp, qexp, q);/*  m2 = C^qexp mod q	*/
    if (status < 0) {		/* mp_modexp returned an error. */
		mp_init(rabout, 1);
		return status;		/* error return */
    }
	stage_modulus(n);     /* Used for mp_modmult */
	mp_modmult(m2,b,M); /* m2 = b*m2 */

		
	mp_move(temp,m0);	/* M0 = (a*m0 + b*m2) mod n */
	mp_add(temp,m2);
	mp_mod(M,temp,n);

	/* Can this message be used for signing? */
	stage_modulus(n);
	mp_modsquare(temp, M);

	if (!mp_compare(temp, rabin))
		found = TRUE;
	else{
		if (i <= Padding)
			i = Padding + MSBRND;
			
		point = (byte *)rabin + i;
		while (!(*point = cryptRandByte()))/* Change one byte of the non-zero padding */
			;
		i--;
		printf(".");
	}

}
	
	mp_move(rabout, M);
	

    mp_burn(pexp);
    mp_burn(qexp);
    mp_burn(temp);		/* burn the evidence on the stack... */
    mp_burn(m0);
    mp_burn(m2);
    mp_burn(M);

    return 0;			/* normal return */
}				/* rabin_Sign */


/* Procedure used to encrypt a message with Rabin's scheme. */
/* Adds random padding and the header to the user data. */
	int
rabin_public_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr N)
/* Encrypt a DEK with a public key.  Returns 0 on success.
 * <0 means there was an error.
 * -1: Generic error
 * -3: Key too big
 * -4: Key too small
 */
{
	unit temp[MAX_UNIT_PRECISION];
	unsigned int blocksize;
	int i;	/* Temporary, and holds error codes */
	byte *p = (byte *)temp;

	/*
	 * We are building the mpi in place, except for a possible
	 * byte-order swap to little-endian at the end.  Thus, we
	 * need to fill the buffer with leading 0's in the unused
	 * most significant byte positions.
	 */
	blocksize = countbytes(N) - 1;	/* Bytes available for user data */
	for (i = units2bytes(global_precision) - blocksize; i > 0; --i)
		*p++ = 0;
	/*
	 * The EBP key formats add a type byte, a five byte header, and
	 * a framing byte of 0 to the user data.  The remaining space
	 * is filled with random padding.  
	 */
	i = blocksize - 2 - bytes;

	while (i>blocksize-2-bytes-MSBRND)      /* Start with some random bytes */
		if (*p = cryptRandByte())
			++p, --i;

	if (i < 1)		/* Less than minimum padding? */
		return -4;
	*p++ = CK_ENCRYPTED_BYTE;	/* Type byte */
	memcpy(p, header, sizeof(header)); /* Five byte header */
	p += sizeof(header);
	i -= sizeof(header);

	while (i)			/* Non-zero random padding */
		if (*p = cryptRandByte())
			++p, --i;
	*p++ = 0;			/* Framing byte */
	memcpy(p, inbuf, bytes);	/* User data */
    mp_convert_order((byte *)temp);	/* Convert buffer to MPI */
	stage_modulus(N);
	i = mp_modsquare(outbuf, temp);	/* Do the encryption */
	if (i < 0)
		i = -1;
} /* rabin_public_encrypt */

/* Procedure used to sign a message with Rabin's scheme. */
int
rabin_private_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr P, unitptr Q, unitptr N, unitptr A, unitptr B)
/* Encrypt a message digest with a private key.
 * Returns <0 on error:
 * -1: generic error
 * -4: Key too big
 * -5: Key too small
 */
{
	unit temp[MAX_UNIT_PRECISION];
	byte *p;
	int i;
	unsigned int blocksize;

	p = (byte *)temp;


	/* We are building the mpi in place, except for a possible
	 * byte-order swap to little-endian at the end.  Thus, we
	 * need to fill the buffer with leading 0's in the unused
	 * most significant byte positions.
	 */
	blocksize = countbytes(N) - 1;	/* Space available for data */
	for (i = units2bytes(global_precision) - blocksize; i > 0; --i)
		*p++ = 0;

	i = blocksize - 2 - (algostruct.MDlength/8);		/* Padding needed */
	i -= sizeof(asn_array);		/* Space for type encoding */
	if (i < 0) {
		i = -4;			/* Error code */
		goto Cleanup;
	}
	while (i>blocksize-2-(algostruct.MDlength/8)-sizeof(asn_array)-MSBRND) /* Start with some random bytes */
		if ((*p = cryptRandByte()))
			++p, --i;

	*p++ = MD_ENCRYPTED_BYTE;	/* Type byte */
	while (i)			/* Non-zero random padding */
		if ((*p = cryptRandByte()))
			++p, --i;
	*p++ = 0;			/* Zero framing byte */
	memcpy(p, asn_array, sizeof(asn_array)); /* ASN data */
	p += sizeof(asn_array);
	memcpy(p, inbuf, (algostruct.MDlength/8));	/* User data */

	mp_convert_order((byte *)temp);
	i = Rabin_Sign(outbuf, temp, P, Q, N, A, B);		/* Encrypt */
	if (i < 0)
		i = -1;

Cleanup:
	burn(temp);

	return i;
} /* rabin_private_encrypt */

/* This procedure is used for signature verification.
/* Remove a signature packet from an MPI */
int
rabin_public_decrypt(byteptr outbuf, unitptr inbuf,
	unitptr E, unitptr N)
/* Decrypt a message digest using a public key.  Returns the number of bytes
 * extracted, or <0 on error.
 * -1: Corrupted packet.
 * -3: Key too big
 * -4: Key too small
 * -5: Maybe malformed Rabin packet
 * -7: Unknown conventional algorithm
 * -9: Malformed Rabin packet
 */
{
	unit temp[MAX_UNIT_PRECISION];
	unsigned int blocksize;
	int i, hashtemp;
	byte *front, *back;

	stage_modulus(N);
	i = mp_modsquare(temp, inbuf);	/* Do the decryption */
	if (i < 0) {
		mp_burn(temp);
		return -1;
	}
	mp_convert_order((byte *)temp);
	blocksize = countbytes(N) - 1;
	front = (byte *)temp;			/* Points to start of block */
	i = units2bytes(global_precision);
	back = front + i;			/* Points to end of block */
	i -= countbytes(N) - 1;			/* Expected leading 0's */


	/*
	 * Strip off the padding.  This handles both PKCS and PGP 2.0
	 * formats.  If we're using RSAREF2, we use the padding-removal
	 * code in RSAPublicDecrypt, which accepts only PKCS style.
	 * Oh, well.
	 */

	if (i < 0)				/* This shouldn't happen */
		goto ErrorReturn;
	while (i--)				/* Extra bytes should be 0 */
		if (*front++)
			goto ErrorReturn;

	/* How to distinguish old PGP from PKCS formats.
	 * The old PGP format ends in a trailing type byte, with
	 * all 1's padding before that.  The PKCS format ends in
	 * 16 bytes of message digest, preceded by an ASN string
	 * which is not all 1's.
	 */
	if (back[-1] == MD_ENCRYPTED_BYTE &&
	    back[-17] == 0xff && back[-18] == 0xff) {
		/* Old PGP format: Padding is at the end */
		if (*--back != MD_ENCRYPTED_BYTE)
			goto ErrorReturn;
		hashtemp = *front++;
		if ((hashtemp == MD5_ALGORITHM_BYTE)||(hashtemp == OLDALG)) 
		{
			algostruct.Hashing = OLDALG;
			algostruct.MDlength = 128;
			algostruct.Passes = 4;
		}
		else
		{
			algostruct.Hashing = NEWALG;
			SetLengthandPasses(&algostruct, hashtemp);
		}

		while (*--back == 0xff)	/* Skip constant padding */
			;
		if (*back)		/* It should end with a zero */
			goto ErrorReturn;
	} else {
		/* PKCS format: padding at the beginning */
		front += MSBRND; /* Skip the first random bytes */
		if (*front++ != MD_ENCRYPTED_BYTE)
			goto ErrorReturn;
		while (*front++) /* Skip constant padding */
			;
		if (front[-1])	/* First non-FF byte should be 0 */
			goto ErrorReturn;
		/* Then comes the ASN header */
		if (memcmp(front, asn_array, sizeof(asn_array))) {
			mp_burn(temp);
			return -7;
		}
		front += sizeof(asn_array);
	}


/* We're done - copy user data to outbuf */
	if (back < front)
		goto ErrorReturn;
    blocksize = back-front;
	memcpy(outbuf, front, (algostruct.MDlength/8));
	mp_burn(temp);
	return (algostruct.MDlength/8);
ErrorReturn:
	mp_burn(temp);
	return -9;
} /* rabin_public_decrypt */


/* This procedure decrypts a message with Rabin's scheme.
/* We expect to find random padding and an encryption key */
int
rabin_private_decrypt(byteptr outbuf, unitptr inbuf,
	 unitptr P, unitptr Q, unitptr N, unitptr A, unitptr B)
/* Decrypt an encryption key using a private key.  Returns the number of bytes
 * extracted, or <0 on error.
 * -1: Generic error
 * -3: Key too big
 * -4: Key too small
 * -5: Maybe malformed Rabin
 * -7: Unknown conventional algorithm
 * -9: Malformed Rabin packet
 */
{
	byte *back;
	byte *front;
	unsigned int blocksize;
	unit temp[MAX_UNIT_PRECISION];
	int i;
	
	i = Rabin_Decrypt(temp, inbuf, P, Q, N, A, B);
	if (i < 0) {
		mp_burn(temp);
		return -1;
	}
	front = (byte *)temp;			/* Start of block *//*Kan tas bort*/
	i = units2bytes(global_precision);
	back = (byte *)front + i;		/* End of block */
	blocksize = countbytes(N) - 1;
	i -= blocksize;				/* Expected # of leading 0's */

	if (i < 0)				/* This shouldn't happen */
		goto Corrupted;
	while (i--)				/* Extra bytes should be 0 */
		if (*front++)
			goto Corrupted;
	
	front += MSBRND; /* Skip the first random bytes */
	if (*front++ != CK_ENCRYPTED_BYTE)
		goto Corrupted;
	while (*front++)	/* Skip the header and the non-zero random padding */
		;
	if (back <= front)
		goto Corrupted;
	blocksize = back-front;

	memcpy(outbuf, front, blocksize);
	mp_burn(temp);
	return blocksize;

Corrupted:
	mp_burn(temp);
	return -9;
} /* rabin_private_decrypt */
