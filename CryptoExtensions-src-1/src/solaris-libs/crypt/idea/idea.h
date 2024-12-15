/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

#ifndef _IDEA_HEADER
#define _IDEA_HEADER

#define u_int16_t unsigned short 

#ifndef int32_t
#define int32_t long
#endif

#ifdef LITTLE_ENDIAN
#define SWAP_BYTES(us) ((((us)<<8)&0xFF00)|(((us)>>8)&0x00FF))
#else
#define SWAP_BYTES(us) (us)
#endif

#define IDEA_KS_SIZE 104

typedef u_int16_t idea_cblock[4];
typedef u_int16_t idea_user_key[8];
typedef u_int16_t idea_ks[52];

void idea_crypt( idea_cblock in, idea_cblock out, idea_ks key );
void idea_invert_key( idea_ks key, idea_ks inv_key );
void idea_expand_key( idea_user_key userKey, idea_ks key );

#endif

