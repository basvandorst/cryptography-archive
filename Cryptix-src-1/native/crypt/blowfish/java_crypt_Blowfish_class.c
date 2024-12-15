/*
 * This library includes software developed by Eric Young (eay@mincom.oz.au)
 *
 * Other parts are Copyright (C) 1995, 1996 Systemics Ltd
 * (http://www.systemics.com/). All rights reserved.
 *
 */

#include "config.h"
#include "blowfish.h"
#include "java_crypt_Blowfish.h"

long java_crypt_Blowfish_set_ks( struct Hjava_crypt_Blowfish * this, HArrayOfByte * userKey )
{
	return blowfish_make_bfkey(
						unhand( userKey )->body,
						obj_length( userKey ),
						( BFkey_type *)( unhand( unhand( this )->ks )->body )
					);
}


void java_crypt_Blowfish_do_blowfish( struct Hjava_crypt_Blowfish * this,
						HArrayOfByte * in, long in_offset,
						HArrayOfByte * out, long out_offset,
						long encrypt )
{
	char * inPtr = unhand( in )->body + in_offset;
	char * outPtr = unhand( out )->body + out_offset;
	blowfish_crypt(
				inPtr, outPtr,
				(BFkey_type *)( unhand( unhand( this )->ks )->body ),
				!encrypt
			);
}

