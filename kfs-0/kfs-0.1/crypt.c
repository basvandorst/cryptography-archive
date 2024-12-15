/* crypt.c - encryption functions for kfs
 *
 * Copyright (c) 1993 by William Dorsey
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "crypt.h"
#include "des.h"
#include "md5.h"


/* Initializes encryption routines.  Stores key and initialization vector
 * in memory for use later.  The first eight bytes of both key and iv
 * are used.
 */
void
xfs_init_crypt(unsigned long keys[64], const unsigned char *asc_key)
{
	MD5_CTX	context;
	unsigned char digest[16];

	MD5Init(&context);
	MD5Update(&context, asc_key, strlen(asc_key));
	MD5Final(digest, &context);
	deskey(keys, digest);	/* permute & store key */
}


/* Create a random 8-byte initialization vector to be stored with other
 * filesystem parameters.  Seeds the srand48 Unix random number generator
 * with the tv_usec member of the timeval returned by the system call
 * gettimeofday().
 */
void
xfs_make_iv(const unsigned long *keys, const unsigned char *rnd, unsigned long *ivec)
{
	bcopy(rnd, ivec, 8);
	desfunc(ivec, keys);
}


/* DES-CBC encryption function.  Takes an initialization vector pointed to
 * by the first argument and uses it and the previously supplied key (see
 * xfs_init_crypt() for more info) to encrypt the buffer pointed to by the
 * second argument.  Writes the result to the address pointed to by the
 * third argument.  The fourth argument is the length of the buffer to be
 * encrypted and must be a multiple of 8.
 */
void
xfs_encrypt(const unsigned long *keys, const unsigned long *iv, const unsigned long *src, unsigned long *dst, int buflen)
{
	unsigned int i;

	if (src == dst) {
		for (i=0; i<buflen; i+=8) {
			dst[0] ^= iv[0];
			dst[1] ^= iv[1];
			desfunc(dst, keys);
			iv = dst;
			dst += 2;
		}
	}
	else {
		for (i=0; i<buflen; i+=8) {
			dst[0] = src[0] ^ iv[0];
			dst[1] = src[1] ^ iv[1];
			desfunc(dst, keys);
			iv = dst;
			src += 2;
			dst += 2;
		}
	}
}


/* DES-CBC decryption function.  Takes an initialization vector pointed to
 * by the first argument and uses it and the previously supplied key (see
 * xfs_init_crypt() for more info) to decrypt the buffer pointed to by the
 * second argument.  Writes the result to the address pointed to by the
 * third argument.  The fourth argument is the length of the buffer to be
 * decrypted and must be a multiple of 8.
 */
void
xfs_decrypt(const unsigned long *keys, const unsigned long *iv, const unsigned long *src, unsigned long *dst, int buflen)
{
	unsigned int i;

	if (src == dst) {
		unsigned long cbc_vec[2];
		unsigned long ctx_buf[2];

		cbc_vec[0] = iv[0];
		cbc_vec[1] = iv[1];
		for (i=0; i<buflen; i+=8) {
			ctx_buf[0] = dst[0];
			ctx_buf[1] = dst[1];
			desfunc(dst, &keys[32]);
			dst[0] ^= cbc_vec[0];
			dst[1] ^= cbc_vec[1];
			dst += 2;
			cbc_vec[0] = ctx_buf[0];
			cbc_vec[1] = ctx_buf[1];
		}
	}
	else {
		dst[0] = src[0];
		dst[1] = src[1];
		desfunc(dst, &keys[32]);
		dst[0] ^= iv[0];
		dst[1] ^= iv[1];
		dst += 2;
		src += 2;
		for (i=8; i<buflen; i+=8) {
			dst[0] = src[0];
			dst[1] = src[1];
			desfunc(dst, &keys[32]);
			dst[0] ^= src[-2];
			dst[1] ^= src[-1];
			dst += 2;
			src += 2;
		}
	}
}
