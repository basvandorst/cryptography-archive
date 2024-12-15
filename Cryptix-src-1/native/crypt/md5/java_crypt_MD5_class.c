/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

#include "config.h"
#include "md5.h"
#include "java_crypt_MD5.h"

void java_crypt_MD5_init( struct Hjava_crypt_MD5 * this )
{
	MD5_md_Init( (MD5_CTX *)unhand( unhand( this )->contextBuf )->body );
}

void java_crypt_MD5_update( struct Hjava_crypt_MD5 * this,
						HArrayOfByte * buffer, long offset, long length )
{
	MD5_md_Update( (MD5_CTX *)unhand( unhand( this )->contextBuf )->body, ( unhand( buffer )->body ) + offset, length );
}

void java_crypt_MD5_finish( struct Hjava_crypt_MD5 * this, HArrayOfByte * outputBuffer )
{
	MD5_md_Final( unhand( outputBuffer )->body, (MD5_CTX *)unhand( unhand( this )->contextBuf )->body );
}

