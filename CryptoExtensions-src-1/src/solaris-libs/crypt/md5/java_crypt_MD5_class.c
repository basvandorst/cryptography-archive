/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

#include "md5.h"
#include "java_crypt_MD5.h"

void java_crypt_MD5_init( struct Hjava_crypt_MD5 * this )
{
	native_MD5Init( (MD5_CTX *)unhand( unhand( this )->contextBuf )->body );
}

void java_crypt_MD5_update( struct Hjava_crypt_MD5 * this,
						HArrayOfByte * buffer, long offset, long length )
{
	native_MD5Update( (MD5_CTX *)unhand( unhand( this )->contextBuf )->body, ( unhand( buffer )->body ) + offset, length );
}

void java_crypt_MD5_finish( struct Hjava_crypt_MD5 * this, HArrayOfByte * outputBuffer )
{
	native_MD5Final( unhand( outputBuffer )->body, (MD5_CTX *)unhand( unhand( this )->contextBuf )->body );
}

