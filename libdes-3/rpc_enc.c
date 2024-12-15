/* rpc_enc.c */
/* Copyright (C) 1994 Eric Young - see README for more details */
#include "rpc_des.h"
#include "des_locl.h"
#include "version.h"

int _des_crypt(buf,len,desp)
char *buf;
int len;
struct desparams *desp;
	{
	Key_schedule ks;
	int enc;
	register char *a,*b;

	des_set_key((des_cblock *)desp->des_key,ks);
	enc=(desp->des_dir == ENCRYPT)?DES_ENCRYPT:DES_DECRYPT;

	if (desp->des_mode == CBC)
		des_ecb_encrypt((des_cblock *)desp->UDES.UDES_buf,
				(des_cblock *)desp->UDES.UDES_buf,ks,enc);
	else
		{
		des_cbc_encrypt((des_cblock *)desp->UDES.UDES_buf,
				(des_cblock *)desp->UDES.UDES_buf,
				(long)len,ks,
				(des_cblock *)desp->des_ivec,enc);
		/* len will always be %8 if called from common_crypt
		 * in secure_rpc.
		 * Libdes's cbc encrypt does not copy back the iv,
		 * so we have to do it here. */
		a=&(desp->UDES.UDES_buf[len-8]);
		b=&(desp->des_ivec[0]);
		*(a++)= *(b++); *(a++)= *(b++);
		*(a++)= *(b++); *(a++)= *(b++);
		*(a++)= *(b++); *(a++)= *(b++);
		*(a++)= *(b++); *(a++)= *(b++);
		}
	return(1);	
	}

