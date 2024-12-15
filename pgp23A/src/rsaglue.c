/* rsaglue.c - These functions wrap and unwrap message digests (MDs) and
 * data encryption keys (DEKs) in padding and RSA-encrypt them into
 * multi-precision integers.  This layer was introduced to allow the
 * transparent use of RSAREF for the encryption (in regions where it is
 * legally available - the U.S. and treaty partners), or Philip Zimmermann's
 * mpi library (where permitted by patent law - outside the U.S.).
 *
 * These functions also hide the details of using either PKCS or PGP 2.0
 * style padding and encodings within the integers.  See pgformat.doc
 * for a detailed description of the formats.
 */

#include <string.h> 	/* for mem*() */
#include "mpilib.h"
#include "mpiio.h"
#include "pgp.h"
#include "rsaglue.h"

#ifdef RSAREF		/* defined (or not) in rsaglue.h */
#include "rsaref/test/global.h"
#include "rsaref/source/rsa.h"
#include "rsaref/source/rsaref.h"

int RSAPublicBlock(byte *dest, unsigned *destbytesptr,
                   byte *src, unsigned srcbytes,
                   R_RSA_PUBLIC_KEY *PubKey);
int RSAPrivateBlock(byte *dest, unsigned *destbytesptr,
                    byte *src, unsigned srcbytes,
                    R_RSA_PRIVATE_KEY *PrivKey);

/* Functions to convert to and from RSAREF's bignum formats */

void
rsaref2reg (unitptr to, byte *from, int frombytes)
/* Convert an RSAREF-style MSB-first array of bytes to an mpi-style
 * native-byte-order integer.  (global_precision units long.)
 */
{
	int tobytes;

	tobytes = units2bytes (global_precision);
	if (tobytes > frombytes) {
		memset(to, 0, tobytes - frombytes);
		memcpy((byte *)to + tobytes - frombytes, from, frombytes);
	} else {
		memcpy((byte *)to, from + frombytes - tobytes, tobytes);
	}
#ifndef HIGHFIRST
	hiloswap((byte *)to, tobytes);
#endif
} /* rsaref2reg */

void
reg2rsaref (byte *to, int tobytes, unitptr from)
/* Convert the other way, mpi format to an array of bytes. */
{
	int frombytes;

	frombytes = units2bytes(global_precision);

#ifdef HIGHFIRST
	if (tobytes > frombytes) {
		memset(to, 0, tobytes-frombytes);
		memcpy(to + tobytes - frombytes, (byte *)from, frombytes);
	} else {
		memcpy(to, (byte *)from + frombytes - tobytes, tobytes);
	}
#else
	if (tobytes > frombytes) {
		memcpy(to, (byte *)from, frombytes);
		memset(to + frombytes, 0, tobytes-frombytes);
	} else {
		memcpy(to, (byte *)from, tobytes);
	}
	hiloswap(to, tobytes);
#endif
} /* reg2rsaref */

int
make_RSA_PUBLIC_KEY(R_RSA_PUBLIC_KEY *rpk, unitptr e, unitptr n)
/* Given mpi's e and n, fill in an R_RSA_PUBLIC_KEY structure.
 * Returns -1 on error, 0 on success
 */
{
	rpk->bits = countbits(n);
	if (rpk->bits > MAX_RSA_MODULUS_BITS)
		return -1;

	reg2rsaref(rpk->modulus, MAX_RSA_MODULUS_LEN, n);
	reg2rsaref(rpk->exponent, MAX_RSA_MODULUS_LEN, e);
	return 0;
} /* make_RSA_PUBLIC_KEY */
  
/* Returns -1 on error, 0 on success */
int
make_RSA_PRIVATE_KEY(R_RSA_PRIVATE_KEY *rpk, unitptr e, unitptr d, unitptr p,
                     unitptr q, unitptr dp, unitptr dq, unitptr u, unitptr n)
/* Given a number of necessary mpi's, fill in an R_RSA_PRIVATE_KEY structure.
 * Returns -1 on error, 0 on success
 */
{
	rpk->bits = countbits(n);
	if (rpk->bits > MAX_RSA_MODULUS_BITS ||
	    countbits(p) > MAX_RSA_PRIME_BITS ||
	    countbits(q) > MAX_RSA_PRIME_BITS)
		return -1;

	reg2rsaref(rpk->modulus, MAX_RSA_MODULUS_LEN, n);
	reg2rsaref(rpk->publicExponent, MAX_RSA_MODULUS_LEN, e);
	reg2rsaref(rpk->exponent, MAX_RSA_MODULUS_LEN, d);
	/* The larger prime (p) first */
	reg2rsaref(rpk->prime[0], MAX_RSA_PRIME_LEN, q);
	reg2rsaref(rpk->prime[1], MAX_RSA_PRIME_LEN, p);
	/* d mod (p-1) and d mod (q-1) */
	reg2rsaref(rpk->primeExponent[0], MAX_RSA_PRIME_LEN, dq);
	reg2rsaref(rpk->primeExponent[1], MAX_RSA_PRIME_LEN, dp);
	/* 1/q mod p */
	reg2rsaref(rpk->coefficient, MAX_RSA_PRIME_LEN, u);
	return 0;
} /* make_RSA_PRIVATE_KEY */

#endif /* RSAREF */

/* These functions hide all the internal details of RSA-encrypted
 * keys and digests.  They owe a lot of their heritage to
 * the preblock() and postunblock() routines in mpiio.c.
 */

/* Abstract Syntax Notation One (ASN.1) Distinguished Encoding Rules (DER)
   encoding for RSA/MD5, used in PKCS-format signatures. */
static byte asn_array[] = {	/* PKCS 01 block type 01 data */
	0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
	0x02,0x05,0x05,0x00,0x04,0x10 };
/* This many bytes from the end, there's a zero byte */
#define ASN_ZERO_END 3

int
rsa_public_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr N)
/* Encrypt a DEK with a public key.  Returns 0 on success.
 * <0 means there was an error
 */
{
	unit temp[MAX_UNIT_PRECISION];
#ifdef RSAREF
	R_RSA_PUBLIC_KEY PubKey;
	R_RANDOM_STRUCT Random;
#endif /* RSAREF */
	unsigned int blocksize;
	byte *p;
	int i;	/* Temporary, and holds error codes */

	blocksize = countbytes(N) - 1;	/* Bytes available for user data */

	p = (byte *)temp;

#ifdef RSAREF
	/* Fill in the R_RSA_PUBLIC_KEY structure as needed later. */
	i = make_RSA_PUBLIC_KEY(&PubKey, E, N);
	if (i < 0)
		return -1;
#else /* !RSAREF */
	/* If !RSAREF, we are building the mpi in place, except for a
	 * possible byte-order swap to little-endian at the end.  Thus,
	 * we need to fill the buffer with leading 0's in the unused
	 * most significant byte positions.
	 */
	for (i = units2bytes(global_precision) - blocksize; i > 0; --i)
		*p++ = 0;
#endif /* !RSAREF */

	/* Both the PKCS and PGP 2.0 key formats add a type byte, and a
	 * a framing byte of 0 to the user data.  The remaining space
	 * is filled with random padding.  (PKCS requires that there be
	 * at least 1 byte of padding.)
	 */
	i = blocksize - 2 - bytes;

	if (pkcs_compat) {
		if (i < 1)		/* Less than minimum padding? */
			return -1;
#ifndef RSAREF	/* Build the packet ourselves */
		*p++ = CK_ENCRYPTED_BYTE;	/* Type byte */
		while (i)			/* Non-zero random padding */
			if ((*p = idearand()))
				++p, --i;
		*p++ = 0;			/* Framing byte */
		memcpy(p, inbuf, bytes);	/* User data */
#else /* RSAREF */
		/* The RSAREF routines have their own random number generator
		 * to generate random padding.  The following code seeds it
		 * from PGP's random number generator.
		 */
		R_RandomInit(&Random);
		for (;;) {
			R_GetRandomBytesNeeded(&i, &Random);
			if (i <= 0)
				break;
			blocksize = i > sizeof(temp) ? sizeof(temp) : i;
			for (i = 0; i < blocksize; i++)
				((byte *)temp)[i] = idearand();
			R_RandomUpdate(&Random, (byte *)temp, blocksize);
		}
		/* Pad and encrypt */
		i = RSAPublicEncrypt((byte *)temp, &blocksize,
		                     inbuf, bytes, &PubKey, &Random);
		R_RandomFinal(&Random);	/* Clean up RSAREF's RNG */
		burn(Random);		/* Just to be sure */
#endif /* RSAREF */
	} else {	/* !pkcs_compat */
		if (i < 0)
			return -1;
		memcpy(p, inbuf, bytes);	/* User data */
		p += bytes;
		*p++ = 0;			/* Framing byte */
		while (i)			/* Non-zero random padding */
			if ((*p = idearand()))
				++p, --i;
		*p = CK_ENCRYPTED_BYTE;		/* Type byte */
#ifdef RSAREF
	/* Do the encryption */
	i = RSAPublicBlock((byte *)temp, &blocksize,
	                   (byte *)temp, blocksize, &PubKey);
#endif

	} /* !pkcs_compat */

#ifndef RSAREF
	mp_convert_order((byte *)temp);		/* Convert buffer to MPI */
	i = mp_modexp(outbuf, temp, E, N);	/* Do the encryption */
#else /* RSAREF */
	rsaref2reg(outbuf, (byte *)temp, blocksize);
#endif /* RSAREF */

Cleanup:
	mp_burn(temp);
#ifdef RSAREF
	burn(PubKey);
#endif
	return i < 0 ? i : 0;
} /* rsa_public_encrypt */

int
rsa_private_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N)
/* Encrypt a message digest with a private key.
 * Returns <0 on error.
 */
{
	unit temp[MAX_UNIT_PRECISION];
	unit DP[MAX_UNIT_PRECISION], DQ[MAX_UNIT_PRECISION];
#ifdef RSAREF
	R_RSA_PRIVATE_KEY PrivKey;
#else
	int byte_precision;
#endif
	unsigned int blocksize;
	byte *p;
	int i;

	/* PGP doesn't store these coefficents, so we need to compute them. */
	mp_move(temp,P);
	mp_dec(temp);
	mp_mod(DP,D,temp);
	mp_move(temp,Q);
	mp_dec(temp);
	mp_mod(DQ,D,temp);

	blocksize = countbytes(N) - 1;	/* Space available for data */

	p = (byte *)temp;

#ifdef RSAREF
	i = make_RSA_PRIVATE_KEY(&PrivKey, E, D, P, Q, DP, DQ, U, N);
	if (i < 0)
		goto Cleanup;
#else
	/* If !RSAREF, we are building the mpi in place, except for a
	 * possible byte-order swap to little-endian at the end.  Thus,
	 * we need to fill the buffer with leading 0's in the unused
	 * most significant byte positions.
	 */
	for (i = units2bytes(global_precision) - blocksize; i > 0; --i)
		*p++ = 0;
#endif
	i = blocksize - 2 - bytes;		/* Padding needed */

	if (pkcs_compat) {
#ifndef RSAREF	/* Pad the packet ourselves */
		i -= sizeof(asn_array);		/* Space for type encoding */
		if (i < 0) {
			i = -1;			/* Error code */
			goto Cleanup;
		}
		*p++ = MD_ENCRYPTED_BYTE;	/* Type byte */
		memset(p, ~0, i);		/* All 1's padding */
		p += i;
		*p++ = 0;			/* Zero framing byte */
#endif /* !RSAREF */
		memcpy(p, asn_array, sizeof(asn_array)); /* ASN data */
		p += sizeof(asn_array);
		memcpy(p, inbuf, bytes);	/* User data */
#ifdef RSAREF
		/* Pad and encrypt */
		RSAPrivateEncrypt((byte *)temp, &blocksize,
		                  (byte *)temp, bytes+sizeof(asn_array),
		                  &PrivKey);
#endif
	} else {	/* Not pkcs_compat */
		--i;				/* Space for type byte */
		if (i < 0) {
			i = -1;			/* Error code */
			goto Cleanup;
		}
		*p++ = MD5_ALGORITHM_BYTE;	/* Algorithm type byte */
		memcpy(p, inbuf, bytes);	/* User data */
		p += bytes;
		*p++ = 0;			/* Framing byte of 0 */
		memset(p, ~0, i);		/* All 1's padding */
		p += i;
		*p = MD_ENCRYPTED_BYTE;		/* Type byte */

#ifdef RSAREF
	/* Do the encryption */
	i = RSAPrivateBlock((byte *)temp, &blocksize,
			    (byte *)temp, blocksize, &PrivKey);
#endif /* RSAREF */

	} /* !pkcs_compat */

#ifndef RSAREF
	mp_convert_order((byte *)temp);
	i = mp_modexp_crt(outbuf, temp, P, Q, DP, DQ, U);	/* Encrypt */
#else /* RSAREF */
	rsaref2reg(outbuf, (byte *)temp, blocksize);
#endif /* RSAREF */

Cleanup:
	mp_burn(temp);
#ifdef RSAREF
	memset(&PrivKey, 0, sizeof(PrivKey));
#endif
	return i < 0 ? i : 0;
} /* rsa_private_encrypt */

/* Remove a signature packet from an MPI */
/* Thus, we expect constant padding and the MIC ASN sequence */
int
rsa_public_decrypt(byteptr outbuf, unitptr inbuf,
	unitptr E, unitptr N)
/* Decrypt a message digest using a public key.  Returns the number of bytes
 * extracted, or <0 on error.
 * -1: Corrupted packet.
 * -2: Unrecognized message digest algorithm.
 */
{
#ifdef RSAREF
	R_RSA_PUBLIC_KEY PubKey;
#endif
	unit temp[MAX_UNIT_PRECISION];
	unsigned int blocksize;
	int i;
	byte *front, *back;

#ifdef RSAREF
	make_RSA_PUBLIC_KEY(&PubKey, E, N);
	blocksize = countbytes(inbuf);
	reg2rsaref((byte *)temp, blocksize, inbuf);
	RSAPublicBlock((byte *)temp, &blocksize,
	                   (byte *)temp, blocksize, &PubKey);

	front = (byte *)temp;			/* The start of the block */
	back = front + blocksize;		/* The end */
	i = blocksize - countbytes(N) + 1;	/* Expected leading 0's */
#else
	i = mp_modexp(temp, inbuf, E, N);
	if (i < 0) {
		mp_burn(temp);
		return i;
	}
	mp_convert_order((byte *)temp);
	blocksize = countbytes(N) - 1;
	front = (byte *)temp;			/* Points to start of block */
	i = units2bytes(global_precision);
	back = front + i;			/* Points to end of block */
	i -= countbytes(N) - 1;			/* Expected leading 0's */
#endif

	/* This could be stricter, but the length returned by the RSAREF code
	   is not documented too well. */
	if (i < 0)
		goto ErrorReturn;
	while (i--)		/* Any excess should be 0 */
		if (*front++)
			goto ErrorReturn;
	/* front now points to the data */

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
		if (*front++ != MD5_ALGORITHM_BYTE) {
			mp_burn(temp);
			return -2;
		}
		while (*--back == 0xff)	/* Skip constant padding */
			;
		if (*back)		/* It should end with a zero */
			goto ErrorReturn;
	} else {
		/* PKCS format: padding at the beginning */
		if (*front++ != MD_ENCRYPTED_BYTE)
			goto ErrorReturn;
		while (*front++ == 0xff) /* Skip constant padding */
			;
		if (front[-1])	/* First non-FF byte should be 0 */
			goto ErrorReturn;
		/* Then comes the ASN header */
		if (memcmp(front, asn_array, sizeof(asn_array))) {
			mp_burn(temp);
			return -2;
		}
		front += sizeof(asn_array);
/* This is temporary - to be removed on release */
		if (back-front == 17 && *front == MD5_ALGORITHM_BYTE) {
			front++;
			fprintf(stderr, "PGP 2.2b signature bug!\n");
		}
	}
/* We're done - copy user data to outbuf */
	if (back < front)
		goto ErrorReturn;
	memcpy(outbuf, front, back-front);
	mp_burn(temp);
	return back-front;
ErrorReturn:
	mp_burn(temp);
	return -1;
} /* rsa_public_decrypt */

/* We expect to find random padding and an encryption key */
int
rsa_private_decrypt(byteptr outbuf, unitptr inbuf,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N)
/* Decrypt an encryption key using a private key.  Returns the number of bytes
 * extracted, or <0 on error.
 * -1: Corrupted packet.
 */
{
#ifdef RSAREF
	R_RSA_PRIVATE_KEY PrivKey;
#endif
	unsigned int blocksize;
	unit temp[MAX_UNIT_PRECISION];
	unit DP[MAX_UNIT_PRECISION], DQ[MAX_UNIT_PRECISION];
	byte *front, *back;
	int i;

	mp_move(temp,P);
	mp_dec(temp);
	mp_mod(DP,D,temp);
	mp_move(temp,Q);
	mp_dec(temp);
	mp_mod(DQ,D,temp);

#ifdef RSAREF
	make_RSA_PRIVATE_KEY(&PrivKey, E, D, P, Q, DP, DQ, U, N);
	blocksize = countbytes(inbuf);
	reg2rsaref((byte *)temp, blocksize, inbuf);
	i = RSAPrivateBlock((byte *)temp, &blocksize,
	                    (byte *)temp, blocksize, &PrivKey);
	if (i < 0)
		goto ErrorReturn;
	front = (byte *)temp;			/* Start of block */
	back = front + blocksize;		/* End of block */
	i = blocksize - countbytes(N) + 1;	/* Expected # of leading 0's */
#else
	i = mp_modexp_crt(temp, inbuf, P, Q, DP, DQ, U);
	if (i < 0)
		goto ErrorReturn;
	mp_convert_order((byte *)temp);
	front = (byte *)temp;			/* Start of block */
	i = units2bytes(global_precision);
	back = (byte *)front + i;		/* End of block */
	blocksize = countbytes(N) - 1;
	i -= blocksize;				/* Expected # of leading 0's */
#endif
	if (i < 0)				/* This shouldn't happen */
		goto ErrorReturn;
	while (i--)				/* Extra bytes should be 0 */
		if (*front++)
			goto ErrorReturn;

	/* How to distinguish old PGP from PKCS formats.
	 * PGP packets have a trailing type byte (CK_ENCRYPTED_BYTE),
	 * while PKCS formats have it leading.
	 */
	if (front[0] != CK_ENCRYPTED_BYTE && back[-1] == CK_ENCRYPTED_BYTE) {
		/* PGP 2.0 format  - padding at the end */
		if (back[-1] != CK_ENCRYPTED_BYTE)
			goto ErrorReturn;
		while (*--back)	/* Skip non-zero random padding */
			;
	} else {
		/* PKCS format - padding at the beginning */
		if (*front++ != CK_ENCRYPTED_BYTE)
			goto ErrorReturn;
		while (*front++)	/* Skip non-zero random padding */
			;
	}
	if (back < front)
		goto ErrorReturn;
	memcpy(outbuf, front, back-front);
	mp_burn(temp);
	mp_burn(DP);
	mp_burn(DQ);
	return back-front;

ErrorReturn:
	mp_burn(temp);
	mp_burn(DP);
	mp_burn(DQ);
	return -1;
} /* rsa_private_decrypt */
