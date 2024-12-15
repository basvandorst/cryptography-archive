/*
 *	NB - This file is a modified version of one by Eric Young.
 *  It was modifed by Systemics Ltd (http://www.systemics.com/)
 */

/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 */
#ifndef _DES_HEADER
#define _DES_HEADER

typedef unsigned char u_int8_t;
typedef unsigned long u_int32_t;

typedef u_int8_t  des_user_key[8];
typedef u_int32_t des_cblock[2];
typedef u_int32_t des_ks[32];

void des_crypt( des_cblock in, des_cblock out, des_ks key, int encrypt );
void des_expand_key( des_user_key userKey, des_ks key );

#endif

