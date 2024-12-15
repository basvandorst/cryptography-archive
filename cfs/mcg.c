/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * macguffin block encrypt/decrypt
 * unrolled, constant mask (optimized) version
 * 
 * For a complete description of MacGuffin, see
 * M. Blaze & B. Schneier, "The MacGuffin Block Cipher Algorithm",
 * proc. 2nd Workshop on Cryptographic Algorithms, Leuven, Belgium, 1994.
 *
 * 10/4/94 matt blaze
 */

#include "mcg.h"

/*
 * codebook encrypt one block with given expanded key
 */
mcg_block_encrypt(blk,key)
     unsigned char *blk;
     mcg_key *key;

{
	unsigned short r0, r1, r2, r3, a, b, c;
	int i;
	unsigned short *ek;

	r0=blk[0]|(blk[1]<<8);
	r1=blk[2]|(blk[3]<<8);
	r2=blk[4]|(blk[5]<<8);
	r3=blk[6]|(blk[7]<<8);
	
	ek = &(key->val[0]);
	/*
	 * round loop, unrolled 4x
	 */
	for (i=0; i<8; i++) {
		a = r1 ^ *(ek++);
		b = r2 ^ *(ek++);
		c = r3 ^ *(ek++);
		r0 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
		a = r2 ^ *(ek++);
		b = r3 ^ *(ek++);
		c = r0 ^ *(ek++);
		r1 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
		a = r3 ^ *(ek++);
		b = r0 ^ *(ek++);
		c = r1 ^ *(ek++);
		r2 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
		a = r0 ^ *(ek++);
		b = r1 ^ *(ek++);
		c = r2 ^ *(ek++);
		r3 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
	}
	blk[0] = r0;
	blk[1] = r0>>8;
	blk[2] = r1;
	blk[3] = r1>>8;
	blk[4] = r2;
	blk[5] = r2>>8;
	blk[6] = r3;
	blk[7] = r3>>8;
}

/*
 * codebook decrypt one block with given expanded key
 */
mcg_block_decrypt(blk,key)
     unsigned char *blk;
     mcg_key *key;

{
	unsigned short r0, r1, r2, r3, a, b, c;
	int i;
	unsigned short *ek;

	r0=blk[0]|(blk[1]<<8);
	r1=blk[2]|(blk[3]<<8);
	r2=blk[4]|(blk[5]<<8);
	r3=blk[6]|(blk[7]<<8);

	ek = &(key->val[KSIZE]);
	/*
	 * round loop, unrolled 4x
	 */
	for (i=0; i<8; ++i) {
		c = r2 ^ *(--ek);
		b = r1 ^ *(--ek);
		a = r0 ^ *(--ek);
		r3 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
		c = r1 ^ *(--ek);
		b = r0 ^ *(--ek);
		a = r3 ^ *(--ek);
		r2 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
		c = r0 ^ *(--ek);
		b = r3 ^ *(--ek);
		a = r2 ^ *(--ek);
		r1 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
		c = r3 ^ *(--ek);
		b = r2 ^ *(--ek);
		a = r1 ^ *(--ek);
		r0 ^= ((OUT0 & stable[(a & LOOK00) |
				      (b & LOOK01) |
				      (c & LOOK02)])
		       | (OUT1 & stable[(a & LOOK10) |
					(b & LOOK11) |
					(c & LOOK12)])
		       | (OUT2 & stable[(a & LOOK20) |
					(b & LOOK21) |
					(c & LOOK22)])
		       | (OUT3 & stable[(a & LOOK30) |
					(b & LOOK31) |
					(c & LOOK32)]));
	}
	blk[0] = r0;
	blk[1] = r0>>8;
	blk[2] = r1;
	blk[3] = r1>>8;
	blk[4] = r2;
	blk[5] = r2>>8;
	blk[6] = r3;
	blk[7] = r3>>8;
}
