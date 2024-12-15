/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

#include "config.h"
#include "sha.h"
#include "java_crypt_SHA.h"

/* function to make the digest buffer the correct way around
   and copy it into return buffer.*/
static void finishHash( SHA_INFO * infoBlock, char * buffer )
{
#ifdef LITTLE_ENDIAN
	char * from = (char *)infoBlock->digest;
	int x;

	/* this is only 20 byte could be unwound.*/
	for ( x = 0; x < java_crypt_SHA_HASH_LENGTH; x += 4 )
	{
		*buffer++ = from[3 + x];		
		*buffer++ = from[2 + x];		
		*buffer++ = from[1 + x];		
		*buffer++ = from[0 + x];		
	}
#else /* LITTLE_ENDIAN */
	memcpy( buffer, infoBlock->digest, java_crypt_SHA_HASH_LENGTH );
#endif /* LITTLE_ENDIAN */
}

void java_crypt_SHA_init( struct Hjava_crypt_SHA * this )
{
	sha_init( (SHA_INFO *)( unhand( unhand( this )->contextBuf )->body ) );
}

void java_crypt_SHA_update(struct Hjava_crypt_SHA * this, HArrayOfByte * buffer, long offset, long length )
{
	sha_update( (SHA_INFO *)( unhand( unhand( this )->contextBuf )->body ), unhand(buffer)->body + offset, length );
}

void java_crypt_SHA_finish( struct Hjava_crypt_SHA * this, HArrayOfByte * buffer )
{
	SHA_INFO * infoPtr = (SHA_INFO *)( unhand( unhand( this )->contextBuf )->body );

	sha_final( infoPtr );
	finishHash( infoPtr, unhand( buffer )->body );
}


