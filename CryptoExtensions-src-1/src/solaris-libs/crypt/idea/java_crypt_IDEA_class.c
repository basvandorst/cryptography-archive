/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

#include "java_crypt_IDEA.h"
#include "idea.h"

void java_crypt_IDEA_set_ks( struct Hjava_crypt_IDEA * this, HArrayOfByte * userKey )
{
	idea_expand_key( (u_int16_t *)( unhand( userKey )->body ), (u_int16_t *)( unhand( unhand( this )->ks )->body ) );
}

void java_crypt_IDEA_set_dks( struct Hjava_crypt_IDEA * this )
{
	idea_invert_key( (u_int16_t *)( unhand( unhand( this )->ks )->body ), (u_int16_t *)( unhand( unhand( this )->dks )->body ) );
}

void java_crypt_IDEA_do_idea( struct Hjava_crypt_IDEA * this,
								HArrayOfByte * in, long in_offset,
								HArrayOfByte * out, long out_offset,
								HArrayOfByte * key )
{
	char * inPtr = ( unhand( in )->body ) + in_offset;
	char * outPtr = ( unhand( out )->body ) + out_offset;
	idea_crypt( (u_int16_t *)inPtr, (u_int16_t *)outPtr, (u_int16_t *)( unhand( key )->body) );
}

