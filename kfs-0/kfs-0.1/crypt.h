/* crypt.h -- include file for xfs software.
 *
 * Copyright (c) 1993 by William W. Dorsey
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
 */

/* Machine-dependent */
typedef	unsigned char	WORD8;
typedef unsigned short	WORD16;
typedef unsigned long	WORD32;

void xfs_make_iv(const unsigned long *keys, const unsigned char *rnd, unsigned long *ivec);
/* creates a random 8-byte initialization vector which should be stored
 * with other filesystem parameters.  The first argument points to a
 * previously-initialized buffer containing internal state information.
 * The second argumernt points to 8 bytes containing random seed values.
 * The third argument points to an 8-byte buffer into which the initialization
 * vector will be placed.
 */

void xfs_init_crypt(unsigned long keys[64], const unsigned char *asc_key);
/* initializes cryptographic routines.  The first argument points to a
 * buffer big enough to hold 64 longs (256 bytes).  This will contain
 * internal state information needed by the encryption and decryption
 * functions.  The second argument is the key as typed by the user.  It
 * can be any length and should be null-terminated.
 */

void xfs_encrypt(const unsigned long *keys, const unsigned long *iv, const unsigned long *src, unsigned long *dst, int len);
void xfs_decrypt(const unsigned long *keys, const unsigned long *iv, const unsigned long *src, unsigned long *dst, int len);
/* DES-CBC encryption/decryption functions.  The first argument points to
 * a previously-initialized buffer containing internal state information
 * used by these functions.  The second argument points to the initialization
 * vector to be used to encrypt the block of data pointed to by the third
 * argument.  The fourth argument contains the address in memory at which to
 * write the encrypted data.  Unless the source and destination addresses
 * are identical, no overlapping is allowed.  The fifth and final argument
 * is the length of the block of data to be encrypted, and must be a multiple
 * of eight.
 */
