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


#include <stdio.h>
#include <rpc/rpc.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

cipher(k,s,d)
     cfskey *k;
     unsigned char *s;
     int d; /* "decrypting" flag */
{
	d=d&1;
	switch (k->cipher) {
	    case STD_DES:
		des_block_cipher(k->var.des.primary,s,d);
		break;
	    case THREE_DES:
		des_block_cipher(k->var.des3.primary1,s,d);
		des_block_cipher(k->var.des3.primary2,s,1-d);
		des_block_cipher(k->var.des3.primary1,s,d);
		break;
	    case TRUE_THREE_DES:
		if (d) {
			des_block_cipher(k->var.dest3.primary3,s,1);
			des_block_cipher(k->var.dest3.primary2,s,1);
			des_block_cipher(k->var.dest3.primary1,s,1);
		} else {
			des_block_cipher(k->var.dest3.primary1,s,0);
			des_block_cipher(k->var.dest3.primary2,s,0);
			des_block_cipher(k->var.dest3.primary3,s,0);
		}
		break;
	    case MCG:
		if (d)
			mcg_block_decrypt(s,&k->var.mcg.primary);
		else
			mcg_block_encrypt(s,&k->var.mcg.primary);
		break;
	    case BLOWFISH:
		if (d)
			BF_encrypt(s,&k->var.bf.primary,BF_DECRYPT);
		else
			BF_encrypt(s,&k->var.bf.primary,BF_ENCRYPT);
		break;
	    case SAFER_SK128:
		if (d)
			Safer_Decrypt_Block(s,k->var.safer.primary,s);
		else
			Safer_Encrypt_Block(s,k->var.safer.primary,s);
	    default:	/* just does nothing */
		break;
	}
}

mask_cipher(k,s,d)
     cfskey *k;
     unsigned char *s;
     int d;
{
	d=d&1;
	switch (k->cipher) {
	    case STD_DES:
		des_block_cipher(k->var.des.secondary,s,d);
		break;
	    case THREE_DES:
		des_block_cipher(k->var.des3.secondary1,s,d);
		des_block_cipher(k->var.des3.secondary2,s,1-d);
		des_block_cipher(k->var.des3.secondary1,s,d);
		break;
	    case TRUE_THREE_DES:
		if (d) {
			des_block_cipher(k->var.dest3.secondary3,s,1);
			des_block_cipher(k->var.dest3.secondary2,s,1);
			des_block_cipher(k->var.dest3.secondary1,s,1);
		} else {
			des_block_cipher(k->var.dest3.secondary1,s,0);
			des_block_cipher(k->var.dest3.secondary2,s,0);
			des_block_cipher(k->var.dest3.secondary3,s,0);
		}
		break;
	    case MCG:
		if (d)
			mcg_block_decrypt(s,&k->var.mcg.secondary);
		else
			mcg_block_encrypt(s,&k->var.mcg.secondary);
		break;
	    case BLOWFISH:
		if (d)
			BF_encrypt(s,&k->var.bf.secondary,BF_DECRYPT);
		else
			BF_encrypt(s,&k->var.bf.secondary,BF_ENCRYPT);
		break;
	    case SAFER_SK128:
		if (d)
			Safer_Decrypt_Block(s,k->var.safer.secondary,s);
		else
			Safer_Encrypt_Block(s,k->var.safer.secondary,s);
	    default:	/* just does nothing */
		break;
	}
}



copykey(key,k)
     cfs_admkey *key;
     cfskey *k;
{
	switch (key->cipher) {
	    case CFS_STD_DES:
		k->cipher=STD_DES;
		des_key_setup(key->cfs_admkey_u.deskey.primary,
		      k->var.des.primary);
		des_key_setup(key->cfs_admkey_u.deskey.secondary,
		      k->var.des.secondary);
		break;
	    case CFS_THREE_DES:
		k->cipher=THREE_DES;
		des_key_setup(key->cfs_admkey_u.des3key.primary1,
		      k->var.des3.primary1);
		des_key_setup(key->cfs_admkey_u.des3key.primary2,
		      k->var.des3.primary2);
		des_key_setup(key->cfs_admkey_u.des3key.secondary1,
		      k->var.des3.secondary1);
		des_key_setup(key->cfs_admkey_u.des3key.secondary2,
		      k->var.des3.secondary2);
		break;
	    case CFS_TRUE_THREE_DES:
		k->cipher=TRUE_THREE_DES;
		des_key_setup(key->cfs_admkey_u.tdes3key.primary1,
		      k->var.dest3.primary1);
		des_key_setup(key->cfs_admkey_u.tdes3key.primary2,
		      k->var.dest3.primary2);
		des_key_setup(key->cfs_admkey_u.tdes3key.primary3,
		      k->var.dest3.primary3);
		des_key_setup(key->cfs_admkey_u.tdes3key.secondary1,
		      k->var.dest3.secondary1);
		des_key_setup(key->cfs_admkey_u.tdes3key.secondary2,
		      k->var.dest3.secondary2);
		des_key_setup(key->cfs_admkey_u.tdes3key.secondary3,
		      k->var.dest3.secondary3);
		break;
	    case CFS_MACGUFFIN:
		k->cipher=MCG;
		mcg_keyset(key->cfs_admkey_u.mcgkey.primary,
			   &k->var.mcg.primary);
		mcg_keyset(key->cfs_admkey_u.mcgkey.secondary,
			   &k->var.mcg.secondary);
		break;
	    case CFS_BLOWFISH:
		k->cipher=BLOWFISH;
		BF_set_key(&k->var.bf.primary,16,
			   key->cfs_admkey_u.blowkey.primary);
		BF_set_key(&k->var.bf.secondary,16,
			   key->cfs_admkey_u.blowkey.secondary);
		break;
	    case CFS_SAFER_SK128:
		k->cipher=SAFER_SK128;
		Safer_Init_Module();
		Safer_Expand_Userkey(key->cfs_admkey_u.saferkey.primary,
				     &(key->cfs_admkey_u.saferkey.primary[8]),
				     SAFER_SK128_DEFAULT_NOF_ROUNDS,
				     1, /* for SK128 */
				     k->var.safer.primary);
		Safer_Expand_Userkey(key->cfs_admkey_u.saferkey.secondary,
				     &(key->cfs_admkey_u.saferkey.secondary[8]),
				     SAFER_SK128_DEFAULT_NOF_ROUNDS,
				     1, /* for SK128 */
				     k->var.safer.secondary);
	    default:
		break;
	}
}
