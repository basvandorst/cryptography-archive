/* 3cbc_enc.c */
/* Copyright (C) 1993 Eric Young - see README for more details */
#include "des_locl.h"

int des_3cbc_encrypt(input,output,length,ks1,ks2,iv1,iv2,encrypt)
des_cblock *input;
des_cblock *output;
long length;
des_key_schedule ks1,ks2;
des_cblock *iv1,*iv2;
int encrypt;
	{
	int off=length/8-1;
	des_cblock niv1,niv2;

	if (encrypt == DES_ENCRYPT)
		{
		des_cbc_encrypt(input,output,length,ks1,iv1,encrypt);
		if (length >= sizeof(des_cblock))
			bcopy(output[off],niv1,sizeof(des_cblock));
		des_cbc_encrypt(output,output,length,ks2,iv1,!encrypt);
		des_cbc_encrypt(output,output,length,ks1,iv2, encrypt);
		if (length >= sizeof(des_cblock))
			bcopy(output[off],niv2,sizeof(des_cblock));
		bcopy(niv1,*iv1,sizeof(des_cblock));
		}
	else
		{
		if (length >= sizeof(des_cblock))
			bcopy(input[off],niv1,sizeof(des_cblock));
		des_cbc_encrypt(input,output,length,ks1,iv1,encrypt);
		des_cbc_encrypt(output,output,length,ks2,iv2,!encrypt);
		if (length >= sizeof(des_cblock))
			bcopy(output[off],niv2,sizeof(des_cblock));
		des_cbc_encrypt(output,output,length,ks1,iv2, encrypt);
		}
	bcopy(niv1,iv1,sizeof(des_cblock));
	bcopy(niv2,iv2,sizeof(des_cblock));
	return(0);
	}
