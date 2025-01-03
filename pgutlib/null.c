#include "crypt.h"

/****************************************************************************
*																			*
*							Null En/Decryption Routines						*
*																			*
****************************************************************************/

int nullInit( CRYPT_INFO *cryptInfo )
	{
	/* Get rid of unused parameter warning in a compiler-independant manner */
	if( cryptInfo );

	return( CRYPT_OK );
	}

int nullInitEx( CRYPT_INFO *cryptInfo, void *cryptInfoEx )
	{
	/* Get rid of unused parameter warnings in a compiler-independant manner */
	if( cryptInfo || cryptInfoEx );

	return( CRYPT_OK );
	}

int nullEnd( CRYPT_INFO *cryptInfo )
	{
	/* Get rid of unused parameter warning in a compiler-independant manner */
	if( cryptInfo );

	return( CRYPT_OK );
	}

int nullInitKey( CRYPT_INFO *cryptInfo )
	{
	/* Get rid of unused parameter warning in a compiler-independant manner */
	if( cryptInfo );

	return( CRYPT_OK );
	}

int nullInitIV( CRYPT_INFO *cryptInfo )
	{
	/* Get rid of unused parameter warning in a compiler-independant manner */
	if( cryptInfo );

	return( CRYPT_OK );
	}

int nullEncrypt( CRYPT_INFO *cryptInfo, void *buffer, int length )
	{
	/* Get rid of unused parameter warning in a compiler-independant manner */
	if( cryptInfo || buffer || length );

	return( CRYPT_OK );
	}

int nullDecrypt( CRYPT_INFO *cryptInfo, void *buffer, int length )
	{
	/* Get rid of unused parameter warning in a compiler-independant manner */
	if( cryptInfo || buffer || length );

	return( CRYPT_OK );
	}
