/*____________________________________________________________________________
	pgpClientKeyDB.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Wrapper functions for KeyDB library
	

	$Id: pgpClientKeyDB.c,v 1.28 1997/09/26 19:53:55 lloyd Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pgpMem.h"
#include "pgpContext.h"
#include "pgpOptionList.h"
#include "pgpKeys.h"
#include "pgpKeyDB.h"
#include "pgpOpaqueStructs.h"
#include "pgpErrors.h"
#include "pgpKDBInt.h"


/*____________________________________________________________________________
	Generate a key
____________________________________________________________________________*/

	PGPError
PGPGenerateKey(
	PGPContextRef		context,
	PGPKeyRef			*key,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError	error	= kPGPError_NoErr;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( IsntNull(key ) );
	
	if (IsntNull( key ) )
		*key	= kPGPInvalidRef;
	
	if( pgpContextIsValid( context ) &&
			IsntNull(key ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;

		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpGenerateKeyInternal( context, key, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	pgpAssertErrWithPtr( error, *key );
	
	if ( IsntPGPError( error ) )
	{
		pgpAssert( (*key)->refCount == 1 );
		pgpIncKeyRefCount( *key );
	}
	
	return( error );
}


/*____________________________________________________________________________
	Generate a subkey
____________________________________________________________________________*/

	PGPError
PGPGenerateSubKey(
	PGPContextRef		context,
	PGPSubKeyRef		*subkey,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError	error	= kPGPError_NoErr;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssertAddrValid( subkey, PGPSubKeyRef );
	
	if (IsntNull( subkey ) )
		*subkey	= kPGPInvalidRef;
		
	if( pgpContextIsValid( context ) &&
		IsntNull( subkey ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		
		*subkey = NULL;

		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpGenerateSubKeyInternal( context, subkey, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	return( error );
}


/*____________________________________________________________________________
	Certify a userid, possibly with trust information
____________________________________________________________________________*/

	PGPError
PGPSignUserID(
	PGPUserIDRef		userid,
	PGPKeyRef			certifyingKey,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError	error;
	
	pgpAssertAddrValid( userid, PGPUserID );
	pgpAssert( pgpKeyIsValid( certifyingKey ) );
	
	if( IsntNull( userid ) && pgpKeyIsValid( certifyingKey ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		
		va_start( args, firstOption );
		context	= PGPGetKeyContext( certifyingKey );
	
		optionList = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpCertifyUserIDInternal( userid, certifyingKey,
											  optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	return( error );
}


/*____________________________________________________________________________
	Revoke a certificate
____________________________________________________________________________*/

	PGPError
PGPRevokeSig(
	PGPSigRef			sig,
	PGPKeySetRef		allkeys,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError		error = kPGPError_NoErr;
	
	pgpAssert( pgpKeySetIsValid( allkeys ) );
	pgpAssert( pgpSigIsValid( sig ) );
	
	if( pgpSigIsValid( sig ) &&
		pgpKeySetIsValid( allkeys ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		PGPKeyRef			sigParentKey;
	
		error = pgpGetCertKey( sig, TRUE, &sigParentKey );
		if( IsPGPError( error ) )
		{
			goto cleanup;
		}
		
		va_start( args, firstOption );
		context		= PGPGetKeyContext( sigParentKey );
		optionList = pgpBuildOptionListArgs( context, FALSE,
											 firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpRevokeCertInternal( sig, allkeys, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;

cleanup:

		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		if( IsntPGPError( error ) )
		{
			error = kPGPError_BadParams;
		}
	}
	
	return( error );
}


/*____________________________________________________________________________
	Revoke a key
____________________________________________________________________________*/

	PGPError
PGPRevokeKey(
	PGPKeyRef			key,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError		error;
	
	pgpAssert( pgpKeyIsValid( key ) );
	
	if( pgpKeyIsValid( key ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		
		va_start( args, firstOption );
		context		= PGPGetKeyContext( key );
		optionList = pgpBuildOptionListArgs( context, FALSE,
											 firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpRevokeKeyInternal( key, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	return( error );
}


/*____________________________________________________________________________
	Revoke a subkey
____________________________________________________________________________*/

	PGPError
PGPRevokeSubKey(
	PGPSubKeyRef		subkey,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError		error;
	
	pgpAssert( pgpSubKeyIsValid( subkey ) );
	
	if( pgpSubKeyIsValid( subkey ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		
		va_start( args, firstOption );
		context		= PGPGetKeyContext( subkey->key );
		optionList = pgpBuildOptionListArgs( context, FALSE,
											 firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpRevokeSubKeyInternal( subkey, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	return( error );
}


/*____________________________________________________________________________
	Add a new userid to a key
____________________________________________________________________________*/

	PGPError
PGPAddUserID(
	PGPKeyRef			key,
	char const *		userID,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError	error;
	
	pgpAssert( pgpKeyIsValid( key ) );
	pgpAssertAddrValid( userID, char );
	
	if( pgpKeyIsValid( key )  && IsntNull( userID )
		&& strlen( userID ) < 256 )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		PGPContextRef		context;
		
		va_start( args, firstOption );
		
		context		= PGPGetKeyContext( key );
		optionList = pgpBuildOptionListArgs( context, FALSE,
											 firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpAddUserIDInternal( key, userID, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	return( error );
}


/*____________________________________________________________________________
	Find out how much entropy is needed
____________________________________________________________________________*/

	PGPUInt32
PGPGetKeyEntropyNeeded(
	PGPContextRef		context,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError	error	= kPGPError_NoErr;
	PGPUInt32	result = ~0UL;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			result = pgpKeyEntropyNeededInternal( context, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
	}
	
	/* No mechanism to return an error at this time */
	return( result );
}


/*____________________________________________________________________________
	Export a keyset to a buffer or file
____________________________________________________________________________*/

	PGPError
PGPExportKeySet(
	PGPKeySetRef		keys,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError		error	= kPGPError_NoErr;
	
	pgpAssert( pgpKeySetIsValid( keys ) );
	
	if( pgpKeySetIsValid( keys ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		PGPContextRef		context;
	
		va_start( args, firstOption );
		
		context	= PGPGetKeySetContext( keys );
		optionList = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpExportKeySetInternal( keys, optionList );
		}
		va_end( args );

		/* Internal function frees optionlist */
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	return( error );
}

/*____________________________________________________________________________
	Import a keyset from a buffer or file
____________________________________________________________________________*/

	PGPError
PGPImportKeySet(
	PGPContextRef		context,
	PGPKeySetRef		*keys,
	PGPOptionListRef	firstOption,
	...
	)
{
	PGPError	error	= kPGPError_NoErr;
	
	if( IsntNull( keys ) )
		*keys	= kPGPInvalidRef;
		
	if( pgpContextIsValid( context ) && IsntNull( keys ) )
	{
		va_list				args;
		PGPOptionListRef	optionList;
		
		*keys	= kPGPInvalidRef;
	
		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpImportKeySetInternal( context, keys, optionList );
		}
		va_end( args );

		/* Internal function frees optionlist */
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		error = kPGPError_BadParams;
	}
	
	return( error );
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
