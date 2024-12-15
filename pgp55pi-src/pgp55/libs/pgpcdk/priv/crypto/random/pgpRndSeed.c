/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpRndSeed.c,v 1.31 1997/10/06 18:01:08 lloyd Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpMem.h"

#include "pgpRnd.h"
#include "pgpCFB.h"
#include "pgpErrors.h"

#include "pgpRndSeed.h"
#include "pgpRandomPoolPriv.h"
#include "pgpRandomContext.h"
#include "pgpRandomX9_17.h"
#include "pgpUtilities.h"
#include "pgpEnv.h"
#include "pgpContext.h"
#include "pgpFileRef.h"
#include "pgpFileSpec.h"
#include "pgpSDKPrefs.h"

#if PGP_MACINTOSH
#include "MacStrings.h"
#include "MacFiles.h"
#endif

/*
 * Save the state of the random number generator to "file" using "cfb"
 * as the washing mechanism.  If cfb is NULL, do not wash the output.
 *
 * Try to write "bytes", or a reasonable minimum size, whichever is the
 * greater.  Returns the number of bytes actually written.
 */
static size_t
pgpRandSeedWriteBytes (
	FILE *						file,
	PGPRandomContext const *	rc,
	PGPCFBContext *	cfb,
	PGPUInt32					bytes)
{
	PGPByte						buf[128];	/* Additional bytes written out */
	PGPUInt32					total,
								written,
								minsize;
	PGPRandomContext			randPoolContext;
	
	pgpAssertAddrValid( file, FILE );

	pgpInitGlobalRandomPoolContext( &randPoolContext );

	/* If no randomcontext passed, default to the global pool */
	if (!rc)
		rc = (PGPRandomContext const *)&randPoolContext;

	/* Figure out a "reasonable minimum size" */
	minsize = (PGPGlobalRandomPoolGetSize()+7)/8;
	if (minsize < PGP_SEED_MIN_BYTES)
		minsize = PGP_SEED_MIN_BYTES;
	else if (minsize > 512)
		minsize = 512;

	/* Always try to write at least this "reasonable minimum size" */
	if (bytes < minsize)
		bytes = minsize;

	/* Add entropy to the global random pool based on current time */
	(void)pgpRandomCollectEntropy( &randPoolContext );

	/* Stir, to hide patterns */
	pgpRandomStir(rc);

	total = 0;
	while (total < bytes) {
		PGPUInt32 len;
		len = (PGPUInt32)(bytes-total < sizeof(buf) ? bytes-total
						  							: sizeof(buf));
		pgpRandomGetBytes(rc, buf, len);
		if( IsntNull( cfb ) )
			PGPCFBEncrypt (cfb, buf, len, buf);
		written = fwrite (buf, 1, len, file);
		if (!written)
			break;
		total += written;
	}
	pgpClearMemory( buf, sizeof(buf) );
	return total;
}


/*
 * Save the "state" of the random number generator to "file" using
 * "cfb" as the washing mechanism.  If cfb is NULL, do not wash
 * the output.
 */
	static void
pgpRandSeedWrite(
	FILE *						file,
	PGPRandomContext const *	rc,
	PGPCFBContext *	cfb)
{
	pgpAssertAddrValid( file, FILE );

	(void)pgpRandSeedWriteBytes (file, rc, cfb, 0);
}



/*
 * Load the RNG state from the file on disk (randseed.bin).
 * Returns 0 on success, <0 on error.
 * Must read at least 24 bytes (the size of the X9.17 generator's
 * state) to be considered successful.  Any additional data is just
 * dumped into the "true" randpool.
 */
	static PGPError
pgpRandSeedRead(
	FILE *						file,
	PGPRandomContext const *	rc)
{
	PGPByte				buf[128];
	int					len;
	size_t				total,
						rewrite;
	PGPRandomContext	randPoolContext;

	pgpAssertAddrValid( file, FILE );
	
	if ( IsNull( file ) )
		return kPGPError_CantOpenFile;

	pgpInitGlobalRandomPoolContext( &randPoolContext );

	/* Dump the file into the random number generator */
	total = 0;
	for (;;) {
		len = fread ((char *)buf, 1, sizeof(buf), file);
		if (len <= 0)
			break;
		total += len;
		/* Add the data both to the X9.17 layer and the underlying pool */
		if (rc)
			pgpRandomAddBytes(rc, buf, len);
		pgpRandomAddBytes(&randPoolContext, buf, len);
	}

	/* Write it back out again */
	rewind (file);
	rewrite = pgpRandSeedWriteBytes (file, rc, NULL, total);

	/*
	 * If we can't rewrite it, we may have the same seed as
	 * another run of the program, which means it's not entropy
	 * at all.  We try not to depend *totally* on the seed file,
	 * but paranoia is many levels deep.
	 */
	/* If we didn't read enough *or* rewrite enough, complain */
	if (total < PGP_SEED_MIN_BYTES || rewrite < PGP_SEED_MIN_BYTES)
		return kPGPError_RandomSeedTooSmall;
	return 0;	/* No error */
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
	FILE *			rsFILE	= NULL;
	
	(void)pgpInitGlobalRandomPool();
	
	/* Read in the randseed file */
	err	= GetRandSeedFileRef( context, &randSeedRef);
	if ( IsntPGPError( err )  )
	{
		err	= pgpFileRefStdIOOpen(randSeedRef, kPGPFileOpenReadWritePerm,
						kPGPFileTypeNone, &rsFILE);
		PFLFreeFileSpec(randSeedRef);
		
		if ( IsntPGPError( err ) )
		{
			err	= pgpRandSeedRead( rsFILE, randomContext );
			pgpStdIOClose( rsFILE );
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
		FILE		*rsFILE		= NULL;

		err	= pgpFileRefStdIOOpen(randSeedRef,
				kPGPFileOpenStdWriteFlags, kPGPFileTypeRandomSeed, &rsFILE);
		PFLFreeFileSpec(randSeedRef);
		
		if ( IsntPGPError( err ) )
		{
			PGPRandomContext *	randomContext	= NULL; 
			
			randomContext	= pgpContextGetGlobalPoolRandomContext( context );
			pgpRandSeedWrite( rsFILE, randomContext, NULL);
			err	= pgpStdIOClose( rsFILE );
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
