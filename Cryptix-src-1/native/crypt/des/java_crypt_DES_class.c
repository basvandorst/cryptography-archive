/*
 * This library includes software developed by Eric Young (eay@mincom.oz.au)
 *
 * Other parts are Copyright (C) 1995, 1996 Systemics Ltd
 * (http://www.systemics.com/). All rights reserved.
 *
 */

#include "config.h"
#include "des.h"
#include "java_crypt_DES.h"

void java_crypt_DES_set_ks( struct Hjava_crypt_DES * this, HArrayOfByte * userKey )
{
	des_expand_key( (u_int8_t *)( unhand( userKey )->body ), (u_int32_t *)( unhand( unhand( this )->ks )->body ) );
}

void java_crypt_DES_do_des( struct Hjava_crypt_DES * this,
						HArrayOfByte * in, long in_offset,
						HArrayOfByte * out, long out_offset,
						long encrypt )
{
	char * inPtr = unhand( in )->body + in_offset;
	char * outPtr = unhand( out )->body + out_offset;

	des_crypt(
				inPtr, outPtr,
				(u_int32_t *)( unhand( unhand( this )->ks )->body ),
				!!encrypt
			);
}

