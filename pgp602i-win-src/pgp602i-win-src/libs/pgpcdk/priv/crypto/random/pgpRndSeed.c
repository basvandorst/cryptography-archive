/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpRndSeed.c,v 1.36.10.1 1998/11/12 03:21:32 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include "pgpDebug.h"
#include "pgpMem.h"

#include "pgpRnd.h"
#include "pgpCFB.h"
#include "pgpErrors.h"
#include "pgpFileUtilities.h"

#include "pgpRndSeed.h"
#include "pgpRandomPoolPriv.h"
#include "pgpRandomContext.h"
#include "pgpRandomX9_17.h"
#include "pgpUtilities.h"
#include "pgpEnv.h"
#include "pgpContext.h"
#include "pgpSDKPrefs.h"
#include "pgpFileRef.h"


/*
 * Save the state of the random number generator to "file" using "cfb"
 * as the washing mechanism.  If cfb is NULL, do not wash the output.
 *
 * Try to write "bytes", or a reasonable minimum size, whichever is the
 * greater.  Returns the number of bytes actually written.
 */
	static PGPError
pgpRandSeedWriteBytes (
	PGPIORef					io,
	PGPRandomContext const *	rc,
	PGPCFBContext *				cfb,
	PGPUInt32					requestCount,
	PGPSize *					writeCount)
{
	PGPUInt32					total	= 0;
	PGPUInt32					minsize;
	PGPRandomContext			randPoolContext;
	PGPByte						buf[ 512 ];
	PGPError					err	= kPGPError_NoErr;
	
	pgpAssertAddrValid( io, VoidAlign );
	if ( IsntNull( writeCount ) )
		*writeCount	= 0;

	pgpInitGlobalRandomPoolContext( &randPoolContext );

	/* If no randomcontext passed, default to the global pool */
	if ( IsNull( rc ) )
	{
		rc = (PGPRandomContext const *)&randPoolContext;
	}

	/* Figure out a "reasonable minimum size" */
	minsize = (PGPGlobalRandomPoolGetSize()+7)/8;
	if (minsize < PGP_SEED_MIN_BYTES)
		minsize = PGP_SEED_MIN_BYTES;
	else if (minsize > 512)
		minsize = 512;

	/* Always try to write at least this "reasonable minimum size" */
	if (requestCount < minsize)
		requestCount = minsize;

	/* Add entropy to the global random pool based on current time */
	(void)pgpRandomCollectEntropy( &randPoolContext );

	/* Stir, to hide patterns */
	pgpRandomStir(rc);

	total = 0;
	while (total < requestCount)
	{
		PGPUInt32	len;
		
		len	= requestCount - total;
		if ( len > sizeof( buf ) )
			len	= sizeof( buf );

		pgpRandomGetBytes(rc, buf, len);
		if( IsntNull( cfb ) )
			PGPCFBEncrypt (cfb, buf, len, buf);
		err = PGPIOWrite( io, len, buf );
		if ( IsPGPError( err ) )
			break;
			
		total += len;
	}
	/* is there any security reason to do this? */
	pgpClearMemory( buf, sizeof(buf) );
	
	if ( IsntNull( writeCount ) )
		*writeCount	= total;
	
	return( err );
}


/*
 * Save the "state" of the random number generator to "file" using
 * "cfb" as the washing mechanism.  If cfb is NULL, do not wash
 * the output.
 */
	static void
pgpRandSeedWrite(
	PGPIORef					io,
	PGPRandomContext const *	rc,
	PGPCFBContext *	cfb)
{
	pgpAssertAddrValid( io, VoidAlign );

	(void)pgpRandSeedWriteBytes( io, rc, cfb, 0, NULL );
}



/*
 * Load the RNG state from the file on disk (randseed.rnd).
 * Returns 0 on success, <0 on error.
 * Must read at least 24 bytes (the size of the X9.17 generator's
 * state) to be considered successful.  Any additional data is just
 * dumped into the "true" randpool.
 */
	static PGPError
pgpRandSeedRead(
	PGPIORef					io,
	PGPRandomContext const *	rc)
{
	PGPSize				total, rewriteCount;
	PGPRandomContext	randPoolContext;
	PGPError			err	= kPGPError_NoErr;
	
	pgpAssert( IsntNull( io ) );
	if ( IsNull( io ) )
		return kPGPError_BadParams;

	pgpInitGlobalRandomPoolContext( &randPoolContext );

	/* Dump the file into the random number generator */
	total = 0;
	while ( TRUE )
	{
		PGPByte				buf[ 512 ];
		PGPSize				actualCount;
		
		err	= PGPIORead( io, sizeof( buf ), buf, &actualCount );
		if ( IsPGPError( err ) )
			break;
			
		total += actualCount;
		/* Add the data both to the X9.17 layer and the underlying pool */
		if ( IsntNull( rc ) )
		{
			pgpRandomAddBytes(rc, buf, actualCount);
		}
		pgpRandomAddBytes( &randPoolContext, buf, actualCount );
	}

	/* Write it back out again */
	err	= PGPIOSetPos( io, 0 );
	if ( IsntPGPError( err ) )
	{
		err = pgpRandSeedWriteBytes( io, rc, NULL, total, &rewriteCount );
	}
	else
	{
		rewriteCount	= 0;
	}

	/*
	 * If we can't rewrite it, we may have the same seed as
	 * another run of the program, which means it's not entropy
	 * at all.  We try not to depend *totally* on the seed file,
	 * but paranoia is many levels deep.
	 */
	/* If we didn't read enough *or* rewrite enough, complain */
	if (total < PGP_SEED_MIN_BYTES || rewriteCount < PGP_SEED_MIN_BYTES)
		return kPGPError_RandomSeedTooSmall;
		
	return kPGPError_NoErr;
}



	static PGPError
GetRandSeedFileRef(
	PGPContextRef		context,
	PFLFileSpecRef	*	outRef)
{
	PGPError			err	= kPGPError_NoErr;

	PGPValidateParam( outRef );
	*outRef	= NULL;
	PGPValidateContext( context );
	
	if ( IsNull( pgpContextGetPrefs( context ) ) )
	{
		/* loading prefs has the side effect of copying into env */
		err	= PGPsdkLoadDefaultPrefs( context );
	}
	if ( IsntPGPError( err ) )
	{
		err	= PGPsdkPrefGetFileSpec( context,
			kPGPsdkPref_RandomSeedFile, (PGPFileSpecRef *)outRef );
	}
	
	return err;
}



	static PGPError
sSeedRandomPool(
	PGPContextRef		context,
	PGPRandomContext *	randomContext)
{
	PFLFileSpecRef		randSeedRef	= NULL;
	PGPError		err	= kPGPError_NoErr;
	
	(void)pgpInitGlobalRandomPool();
	
	/* Read in the randseed file */
	err	= GetRandSeedFileRef( context, &randSeedRef);
	if ( IsntPGPError( err )  )
	{
		PGPFileIORef	fileIO	= NULL;
		
		err	= PGPOpenFileSpec( randSeedRef, kPFLFileOpenFlags_ReadWrite,
									&fileIO );
		PFLFreeFileSpec(randSeedRef);
		
		if ( IsntPGPError( err ) )
		{
			err	= pgpRandSeedRead( (PGPIORef)fileIO, randomContext );
			PGPFreeIO( (PGPIORef)fileIO );
			fileIO	= NULL;
		}
	}

	return( err );
}


/* evil, but the random pool IS global */
static PGPBoolean	sIsSeeded	= FALSE;

	PGPBoolean	
pgpGlobalRandomPoolIsSeeded( void )
{
	return( sIsSeeded );
}

	PGPError
pgpSeedGlobalRandomPool( PGPContextRef context )
{
	PGPError			err	= kPGPError_NoErr;
	PGPRandomContext *	randomContext	= NULL;

	if ( sIsSeeded ) 
		return( kPGPError_NoErr );
	
	randomContext	= pgpContextGetGlobalPoolRandomContext( context );
	if ( IsntNull( randomContext ) )
	{
		err	= sSeedRandomPool( context, randomContext );
	}
	
	/* be conservative, assume not seeded if error */
	sIsSeeded	= IsntPGPError( err );
	
	return( err );
}



/*____________________________________________________________________________
	Add file data into the pool.  Used to add existing random pool file data
	into the random pool before overwiting it.  This fixes a problem where
	the last client to quit wipes out contributions of previous clients to
	the random pool.
____________________________________________________________________________*/
	static void
sAddFileToPool(
	PGPIORef					io,
	PGPRandomContext const *	rc)
{
	PGPError		err	= kPGPError_NoErr;
	PGPFileOffset	startPos;
	
	err	= PGPIOGetPos( io, &startPos );
	if ( IsntPGPError( err ) )
		err	= PGPIOSetPos( io, 0 );
	
	while ( IsntPGPError( err ) )
	{
		PGPByte			buffer[ 512 ];
		PGPSize			actualCount;
		
		err	= PGPIORead( io, sizeof( buffer ), buffer, &actualCount );
		if ( IsntPGPError( err ) || err == kPGPError_EOF )
		{
			pgpRandomAddBytes( rc, buffer, actualCount);
		}
		else
		{
			pgpAssertNoErr( err );
		}
	}
	
	(void)PGPIOSetPos( io, startPos );
}


	PGPError
pgpSaveGlobalRandomPool( PGPContextRef context )
{
	PGPError	err	= kPGPError_NoErr;
	PFLFileSpecRef	randSeedRef	= NULL;
	PGPEnv		*pgpEnv		= NULL;

	pgpEnv = pgpContextGetEnvironment( context );

	err = GetRandSeedFileRef( context, &randSeedRef );
	if ( IsntPGPError( err ) )
	{
		PGPFileIORef	fileIO;

		err	= PGPOpenFileSpec( randSeedRef, kPFLFileOpenFlags_ReadWrite,
									&fileIO );

		if( err == kPGPError_FileNotFound )
		{
			pgpCreateFile( randSeedRef, kPGPFileTypeRandomSeed );
			err	= PGPOpenFileSpec( randSeedRef, kPFLFileOpenFlags_ReadWrite,
								   &fileIO );
		}

		PFLFreeFileSpec(randSeedRef);
		
		if ( IsntPGPError( err ) )
		{
			PGPRandomContext *	randomContext	= NULL;
			
			randomContext	= pgpContextGetGlobalPoolRandomContext( context );
		
			sAddFileToPool( (PGPIORef)fileIO, randomContext );

			pgpRandSeedWrite( (PGPIORef)fileIO, randomContext, NULL);
			PGPFreeIO( (PGPIORef)fileIO );
		}
	}
	
	return( err );
}









/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
