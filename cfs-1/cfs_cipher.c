/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.  You may
 * not export it, in whole or in part, or cause or allow such export,
 * through act or omission, without prior authorization from the United
 * States government and written permission from AT&T.  In particular,
 * you may not make any part of this software available for general or
 * unrestricted distribution to others, nor may you disclose this software
 * to persons other than citizens and permanent residents of the United
 * States and Canada. 
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
     int d;
{
	d=d&1;
	switch (k->cipher) {
	    case STD_DES:
		block_cipher(k->var.des.primary,s,d);
		break;
	    case THREE_DES:
		block_cipher(k->var.des3.primary1,s,d);
		block_cipher(k->var.des3.primary2,s,1-d);
		block_cipher(k->var.des3.primary1,s,d);
		break;
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
		block_cipher(k->var.des.secondary,s,d);
		break;
	    case THREE_DES:
		block_cipher(k->var.des3.secondary1,s,d);
		block_cipher(k->var.des3.secondary2,s,1-d);
		block_cipher(k->var.des3.secondary1,s,d);
		break;
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
		bcopy(key->cfs_admkey_u.deskey.primary,
		      k->var.des.primary,8);
		bcopy(key->cfs_admkey_u.deskey.secondary,
		      k->var.des.secondary,8);
		break;
	    case CFS_THREE_DES:
		k->cipher=THREE_DES;
		bcopy(key->cfs_admkey_u.des3key.primary1,
		      k->var.des3.primary1,8);
		bcopy(key->cfs_admkey_u.des3key.primary2,
		      k->var.des3.primary2,8);
		bcopy(key->cfs_admkey_u.des3key.secondary1,
		      k->var.des3.secondary1,8);
		bcopy(key->cfs_admkey_u.des3key.secondary2,
		      k->var.des3.secondary2,8);
		break;
	    default:
		break;
	}
}
