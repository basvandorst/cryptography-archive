/*
 * pgpKeyLib.c
 * Initialization and cleanup functions related to the keydb library
 *
 * Copyright (C) 1996,1997 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpKeyLib.c,v 1.105.10.1 1998/11/12 03:21:59 heller Exp $
 */

#include "pgpConfig.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif


#include "pgpContext.h"
#include "pgpKeyDB.h"
#include "pgpDebug.h"
#include "pgpKDBInt.h"
#include "pgpEncodePriv.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpFileNames.h"
#include "pgpFileRef.h"
#include "pgpFileSpec.h"
#include "pgpRandomX9_17.h"
#include "pgpRandomPoolPriv.h"
#include "pgpRndSeed.h"
#include "pgpRngRead.h"
#include "pgpSigSpec.h"
#include "pgpTrstPkt.h"
#include "pgpSDKPrefs.h"
#include "pgpOptionList.h"
#include "pgpUtilitiesPriv.h"


#if PGP_MACINTOSH
#include "MacStrings.h"
#include "MacFiles.h"
#endif


#include "pgpDEBUGStartup.h"

#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))


#if PGP_WIN32

/*  In Windows, the DllMain function is called when a process or 
    thread attaches to, or detaches from, the DLL. */

#include "windows.h"

BOOL WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved);

BOOL WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	BOOL		success	= TRUE;
	PGPError	err	= kPGPError_NoErr;
	
	(void)hInst;		/* Avoid warnings */
	(void)lpReserved;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		pgpLeaksBeginSession("PGPsdk");
		
		err	= PGPsdkInit();
		if ( IsPGPError( err ) )
			success	= FALSE;
		break;

	case DLL_PROCESS_DETACH:
		(void)pgpsdkCleanupForce();
		
		pgpLeaksEndSession();
		break;
	}
	return success;
}
#endif	/* PGP_WIN32 */


/* See if the newly opened keyring appears to needs sig checking */
	static PGPError
sIsSigCheckNeeded( PGPKeySetRef set, PGPBoolean *sigCheckNeeded )
{
	RingSet const *		rset;
	RingIterator *		riter;
	int					level;
	PGPBoolean			needSigCheck;

	*sigCheckNeeded = FALSE;
	needSigCheck = FALSE;

	rset = pgpKeyDBRingSet( set->keyDB );
	riter = ringIterCreate( rset );
	if ( !riter )
		return ringSetError( rset )->error;

	/*
	 * We look for a key with a revocation signature which has not
	 * been tried, such that the key's revoke bit is not set.  This is
	 * how some old version of PGP leave revocations.  We always set the
	 * key's revocation bit in the file, so this will not happen with
	 * keyrings written by this library.
	 */
	while ( (level = ringIterNextObjectAnywhere( riter ) ) > 0 )
	{
		if ( level == 2 )
		{
			RingObject *obj = ringIterCurrentObject( riter, level );
			if ( ringObjectType( obj ) == RINGTYPE_SIG  &&
				 ringSigType( rset, obj ) == PGP_SIGTYPE_KEY_REVOKE &&
				 ringSigTrust( rset, obj ) == PGP_SIGTRUST_UNTRIED )
			 {
				RingObject *parent = ringIterCurrentObject( riter, level - 1 );
				if ( ringObjectType( parent ) == RINGTYPE_KEY &&
					 !ringKeyRevoked( rset, parent ) )
				{
					/* Here we have our case we are looking for */
					needSigCheck = TRUE;
					break;
				}
			}
		}
	}
	ringIterDestroy( riter );

	*sigCheckNeeded = needSigCheck;
	return kPGPError_NoErr;
}

	static PGPError
pgpGetDefaultRingFileRefs(
	PGPContextRef	cdkContext,
	PFLFileSpecRef		*pubRefOut,
	PFLFileSpecRef		*privRefOut )
{
	PGPError		err = kPGPError_NoErr;

	/* set outputs to default */
	if ( IsntNull( privRefOut ) )
		*privRefOut = NULL;
	if ( IsntNull( pubRefOut ) )
		*pubRefOut = NULL;
		
	/* load preferences if not already loaded */
	if ( IsNull( pgpContextGetPrefs( cdkContext ) ) )
	{
		err	= PGPsdkLoadDefaultPrefs( cdkContext );
	}
	
	if ( IsntPGPError( err ) )
	{
		if ( IsntNull( privRefOut ) )
		{
			err = PGPsdkPrefGetFileSpec( cdkContext,
				kPGPsdkPref_PrivateKeyring, (PGPFileSpecRef *)privRefOut);
		}

		if ( IsntPGPError( err ) && IsntNull( pubRefOut ) )
		{
			err = PGPsdkPrefGetFileSpec( cdkContext,
				kPGPsdkPref_PublicKeyring, (PGPFileSpecRef *)pubRefOut);
			if ( IsPGPError( err ) )
			{
				PFLFreeFileSpec( *privRefOut );
				*privRefOut	= NULL;
			}
		}
	}
	
	return err;
}

/*
 * Open default keyrings for user, return keyset for it.
 * If isMutable is false, keyrings are read only.
 */
	PGPError
PGPOpenDefaultKeyRings(
	PGPContextRef		cdkContext,
	PGPKeyRingOpenFlags	openFlags,
	PGPKeySetRef *		keySetOut )
{
	PFLFileSpecRef	secFileRef = NULL;	/* File reference for secret keyring */
	PFLFileSpecRef	pubFileRef = NULL;	/* File reference for public keyring */
	PGPError		err = kPGPError_NoErr;
	PGPKeySetRef	set	= NULL;

	PGPValidatePtr( keySetOut );
	*keySetOut	= NULL;
	PGPValidateContext( cdkContext );

	err	= pgpGetDefaultRingFileRefs( cdkContext, &pubFileRef, &secFileRef);
	if ( IsntPGPError( err ) )
	{
		pgpAssert( IsntNull( pubFileRef ) );
		pgpAssert( IsntNull( secFileRef ) );
		
		err = PGPOpenKeyRingPair(cdkContext, openFlags,
					(PGPFileSpecRef)pubFileRef,
					(PGPFileSpecRef)secFileRef, &set);
			
		PFLFreeFileSpec( secFileRef );
		PFLFreeFileSpec( pubFileRef );
	}
		
	if ( IsPGPError( err ) && IsntNull( set ) )
	{
		PGPFreeKeySet( set );
		set	= NULL;
	}
	
	*keySetOut	= set;
	pgpAssertErrWithPtr( err, *keySetOut );
	return err;
}

/*
 * Open the specified keyrings for user, return keyset for it.
 * If isMutable is false, keyrings are read only.
 */
	PGPError
PGPOpenKeyRingPair(
	PGPContextRef		cdkContext,
	PGPKeyRingOpenFlags	openFlags,
	PGPFileSpecRef		pubFileRefIn,
	PGPFileSpecRef		privFileRefIn,
	PGPKeySetRef *		keySetOut )
{
	PGPKeyDB	   *dbsec = NULL,		/* KeyDB for secret keyring */
				   *dbpub = NULL,		/* KeyDB for public keyring */
				   *dbunion = NULL;		/* KeyDB for union of both keyrings */
	RingPool	   *pgpRingPool;		/* RingPool from cdkContext */
	PGPKeySet	   *set = NULL;
	PGPBoolean		sigCheckNeeded;
	PGPError		err;
	PFLFileSpecRef	pubFileRef	= (PFLFileSpecRef)pubFileRefIn;
	PFLFileSpecRef	privFileRef	= (PFLFileSpecRef)privFileRefIn;

	PGPValidatePtr( keySetOut );
	*keySetOut	= NULL;
	PGPValidateContext( cdkContext );
	PFLValidateFileSpec( pubFileRef );
	PFLValidateFileSpec( privFileRef );
	
	pgpAssert( (openFlags & kPGPKeyRingOpenFlags_Reserved) == 0 );
	pgpAssert(0 == (openFlags & (kPGPKeyRingOpenFlags_Private |
								 kPGPKeyRingOpenFlags_Trusted)));

	pgpRingPool = pgpContextGetRingPool( cdkContext );

	/* Create key databases for these files.  Don't bother with keypool. 
	   Private keyring is not trusted (no trust packets) */
	if ((dbsec = pgpCreateFileKeyDB(cdkContext, privFileRef,
								(PGPKeyRingOpenFlags)
								(openFlags | kPGPKeyRingOpenFlags_Private),
								pgpRingPool, &err)) == NULL)
		goto error;
	if ((dbpub = pgpCreateFileKeyDB(cdkContext, pubFileRef,
								(PGPKeyRingOpenFlags)
								(openFlags | kPGPKeyRingOpenFlags_Trusted),
								pgpRingPool, &err)) == NULL)
		goto error;

	/* Create union database for these two files */
	if ((dbunion = pgpCreateUnionKeyDB(cdkContext, &err)) == NULL)
		goto error;
	
	err	= pgpUnionKeyDBAdd(dbunion, dbsec);
	if ( IsPGPError( err ) )
		goto error;
	dbsec = NULL;	/* dbunion now has responsibility for freeing dbsec */
	
	err	= pgpUnionKeyDBAdd(dbunion, dbpub);
	if ( IsPGPError( err ) )
		goto error;
	dbpub = NULL;	/* dbunion now has responsibility for freeing dbpub */

	/*
	 * Verify that we have sufficient ringsets to work with union.
	 * It is easier to check for this now than to check everywhere we
	 * ask for a ringset.
	 */
	if (dbunion->getRingSet(dbunion) == NULL) {
		/* Insufficient ringsets */
		err = kPGPError_OutOfRings;
		goto error;
	}

	err	= pgpBuildKeyPool(dbunion, 0);
	if ( IsPGPError( err ) )
		goto error;

	set = pgpKeyDBRootSet(dbunion);
	if ( IsNull( set ) )
	{
		err = kPGPError_OutOfMemory;	/* XXX Improve error */
		goto error;
	}

	/*
	 * Some earlier versions of PGP don't cache revocation info.  We will
	 * check signatures if the keyring has unchecked revocation signatures
	 * where the key does not have the revoke flag cached.
	 */
	if( IsPGPError( err = sIsSigCheckNeeded( set, &sigCheckNeeded ) ) )
		goto error;
	if( sigCheckNeeded )
	{
		if ( IsPGPError( err = PGPCheckKeyRingSigs( set, set, FALSE,
													NULL, NULL ) ) )
			goto error;
		if ( IsPGPError( err = PGPPropagateTrust( set ) ) )
			goto error;
		if ( PGPKeySetIsMutable( set ) )
		{
			if ( IsPGPError( err = PGPCommitKeyRingChanges( set ) ) )
				goto error;
		}
	}

	err = kPGPError_NoErr;
	
error:
	if (dbsec != NULL)
		pgpFreeKeyDB(dbsec);
	if (dbpub != NULL)
		pgpFreeKeyDB(dbpub);
	if (dbunion != NULL)
		pgpFreeKeyDB(dbunion);
	if (set != NULL && IsPGPError( err ))
	{
		PGPFreeKeySet(set);
		set = NULL;
	}
	*keySetOut	= set;
	
	pgpAssertErrWithPtr( err, *keySetOut );
	return err;
}

/*
 * Open a single specified keyring for user, return keyset for it.
 * If isMutable is false, keyrings are read only.
 * If isTrusted is false, trust packets are ignored.
 */
	PGPError
PGPOpenKeyRing(
	PGPContextRef		cdkContext,
	PGPKeyRingOpenFlags	openFlags,
	PGPFileSpecRef		fileRefIn,
	PGPKeySetRef *		keySetOut )
{
	PGPKeyDB	   *db = NULL;
	PGPKeySet	   *set = NULL;
	RingPool	   *pgpRingPool;
	PGPBoolean		sigCheckNeeded;
	PGPError		err = kPGPError_NoErr;
	PFLFileSpecRef	fileRef	= (PFLFileSpecRef)fileRefIn;

	PGPValidatePtr( keySetOut );
	*keySetOut	= NULL;
	PGPValidateContext( cdkContext );
	PFLValidateFileSpec( fileRef );
	
	pgpAssert( (openFlags & kPGPKeyRingOpenFlags_Reserved) == 0 );
	

	pgpRingPool = pgpContextGetRingPool( cdkContext );

	/* Create key database for this files.  Don't bother with keypool. */
	if ((db = pgpCreateFileKeyDB(cdkContext, fileRef, openFlags,
								 pgpRingPool, &err)) == NULL)
		goto error;

	err	= pgpBuildKeyPool(db, 0);
	if ( IsPGPError( err ) )
		goto error;

	set = pgpKeyDBRootSet(db);
	if ( IsNull( set ) )
	{
		err = kPGPError_OutOfMemory;	/* XXX Improve error */
		goto error;
	}

	/*
	 * Some earlier versions of PGP don't cache revocation info.  We will
	 * check signatures if the keyring has unchecked revocation signatures
	 * where the key does not have the revoke flag cached.
	 */
	if( IsPGPError( err = sIsSigCheckNeeded( set, &sigCheckNeeded ) ) )
		goto error;
	if( sigCheckNeeded )
	{
		if ( IsPGPError( err = PGPCheckKeyRingSigs( set, set, FALSE,
													NULL, NULL ) ) )
			goto error;
		if ( IsPGPError( err = PGPPropagateTrust( set ) ) )
			goto error;
		if ( PGPKeySetIsMutable( set ) )
		{
			if ( IsPGPError( err = PGPCommitKeyRingChanges( set ) ) )
				goto error;
		}
	}
	err = kPGPError_NoErr;
	
error:
	if (db != NULL)
		pgpFreeKeyDB(db);
	if (set != NULL && IsPGPError( err ))
	{
		PGPFreeKeySet(set);
		set = NULL;
	}
	*keySetOut	= set;
	pgpAssertErrWithPtr( err, *keySetOut );
	return err;
}

/*
 * Add keys to a keyset from a dynamically allocated binary key buffer.
 * Makes a copy of the binary key buffer data, so caller can dispose of
 * it after this call.
 */
	PGPError
pgpImportKeyBinary (
	PGPContextRef	cdkContext,
	PGPByte		   *buffer,
	size_t			length,
	PGPKeySetRef *	outRef
	)
{
	PGPKeyDBRef		kdb;
	PGPKeySetRef	set	= NULL;
	RingPool	   *pgpRingPool;
	PGPError		err	= kPGPError_NoErr;

	*outRef	= NULL;
	
	/* Create a file type KeyDB from the buffer */
	pgpRingPool = pgpContextGetRingPool( cdkContext );
	kdb = pgpCreateMemFileKeyDB (cdkContext, buffer, length, pgpRingPool,
								 &err);
	if ( IsNull( kdb ) )
	{
		pgpAssert( IsPGPError( err ) );
	}
	else
	{
		err = pgpBuildKeyPool (kdb, 0);
		if ( IsntPGPError( err ) )
		{
			set = pgpKeyDBRootSet (kdb);
		}
		pgpFreeKeyDB (kdb);
	}
	
	*outRef	= set;
	
	return err;
}


static const PGPOptionType impkeyOptionSet[] = {
	kPGPOptionType_InputFileRef,
	kPGPOptionType_LocalEncoding,
	kPGPOptionType_InputBuffer,
	kPGPOptionType_EventHandler,
	kPGPOptionType_SendNullEvents,
	kPGPOptionType_X509Encoding
};

/* Frees optionList, unlike most other internal functions */
	PGPError
pgpImportKeySetInternal (PGPContextRef context, PGPKeySetRef *keys,
	PGPOptionListRef optionList)
{
	PGPError		err = kPGPError_NoErr;
	PGPKeySetRef	keyset;
	PGPUInt32		fDo509;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						impkeyOptionSet, elemsof( impkeyOptionSet ) ) ) )
		return err;

	pgpAssertAddrValid( keys, PGPKeySetRef );
	*keys = NULL;
	
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_X509Encoding, FALSE,
						 "%d", &fDo509 ) ) )
		goto error;

	if( fDo509 ) {
		err = pgpImportX509Certificate( context, keys, optionList );
		PGPFreeOptionList( optionList );
		goto error;
	}

	if( IsPGPError( err = PGPNewKeySet( context, &keyset ) ) )
		goto error;

	if( IsPGPError( err = PGPDecode( context, optionList,
									 PGPODiscardOutput(context, TRUE),
									 PGPOImportKeysTo(context, keyset),
									 PGPOLastOption(context) ) ) ) {
		PGPFreeKeySet( keyset );
		goto error;
	}
	*keys = keyset;
	
error:
	return err;
}


/* 
 * Filter function for extraction.  Remove any secret objects.
 * If addarrs is true, also add any additional decryption key objects to
 * the set.  This will cause ADK's to be extracted with the keys
 * that use them.
 * XXX AUTOMATIC ADK EXTRACTION DOES NOT YET WORK.
 * There is no RingSet available in which
 * to look for ADK's.  The export functions typically are called with
 * just a memory RingSet.  We need to add versions which take an extra
 * PGPKeySet to flag that ADK's should be looked for there.
 */
static RingSet *
filterPubRingSet (RingSet const *rset, PGPKeySet *keys,
	PGPBoolean exportmastersecrets, PGPBoolean exportsubsecrets,
	PGPBoolean addarks, PGPBoolean includeattributes)
{
	RingSet		   *rsetnew;	/* Set of recipients */
	RingSet		   *adkeyset;	/* Set of additional decryption keys */
	RingIterator   *riter;		/* Iterator over adding sets */
	int				level;
	PGPBoolean		isMember = FALSE;
	PGPBoolean		exportsecrets = exportmastersecrets;
	PGPError		err	= kPGPError_NoErr;

	if (!rset)
		return NULL;
	adkeyset = NULL;
	rsetnew = ringSetCreate (ringSetPool (rset));
	if (!rsetnew)
		return NULL;
	riter = ringIterCreate (rset);
	if (!riter) {
		ringSetDestroy (rsetnew);
		return NULL;
	}
	/* 
	 * Copy objects in PGPKeySet to rsetnew except secret objects.
	 * At the same time, accumulate any additional decryption keys into
	 * adkeyset.
	 */
	while ((level = ringIterNextObjectAnywhere(riter)) > 0) {
		RingObject *obj = ringIterCurrentObject (riter, level);
		if (ringObjectType(obj) == RINGTYPE_KEY) {
			exportsecrets = ringKeyIsSubkey(rset, obj) ? exportsubsecrets :
														 exportmastersecrets;
		}
		/* Possibly skip secret objects */
		if (!exportsecrets && ringObjectType (obj) == RINGTYPE_SEC)
			continue;
		/* Skip signatures if exporting secret keys */
		if (exportsecrets && ringObjectType (obj) == RINGTYPE_SIG)
			continue;
		if (!includeattributes && ringObjectType (obj) == RINGTYPE_NAME
			&& ringNameIsAttribute (rset, obj))
			continue;
		if (!includeattributes && ringObjectType (obj) == RINGTYPE_SIG) {
			RingObject *parent = ringIterCurrentObject (riter, level-1);
			if (ringObjectType(parent) == RINGTYPE_NAME &&
				ringNameIsAttribute (rset, parent))
				continue;
		}
		/* Skip non-members */
		if (ringObjectType(obj) == RINGTYPE_KEY
								&& !ringKeyIsSubkey(rset, obj)) {
			/* Can only call isMember on top level key objects */
			isMember = (*keys->isMember)(keys, obj);
		}
		if (!isMember)
			continue;
		ringSetAddObject (rsetnew, obj);
		/* For key objects, look for additional decryption keys */
		if (addarks && ringObjectType (obj) == RINGTYPE_KEY)
		{
			RingObject	   *rkey;		/* Decryption key */
			unsigned		nrkeys;		/* Number of decryption keys */
			
			if (ringKeyAdditionalRecipientRequestKey (obj, rset, 0, NULL,
										NULL, NULL, &nrkeys, &err) != NULL )
			{
				/* Add to special set for additional decryption keys */
				while (nrkeys-- > 0)
				{
					rkey = ringKeyAdditionalRecipientRequestKey (obj, rset, 0,
											NULL, NULL, NULL, &nrkeys, &err);
					pgpAssert (rkey);
					if (!adkeyset)
					{
						adkeyset = ringSetCreate (ringSetPool(rset));
						if (!adkeyset)
						{
							ringIterDestroy (riter);
							ringSetDestroy (rsetnew);
							return NULL;
						}
					}
					ringSetAddObjectChildren (adkeyset, rset, rkey);
				}
			}
		}
	}
	ringIterDestroy (riter);

	/* Last, merge adkeyset into rsetnew, also stripping secrets */
	if (adkeyset) {
		ringSetFreeze (adkeyset);
		riter = ringIterCreate (adkeyset);
		if (!riter) {
			ringSetDestroy (adkeyset);
			ringSetDestroy (rsetnew);
			return NULL;
		}
		/* Loop over rsetnew iterator, adding non-secret objects */
		/* We will always strip ADK secrets, he can export those explicitly */
		while ((level = ringIterNextObjectAnywhere(riter)) > 0) {
			RingObject *obj = ringIterCurrentObject (riter, level);
			if (ringObjectType (obj) == RINGTYPE_SEC)
				continue;
			ringSetAddObject (rsetnew, obj);
		}
		ringIterDestroy (riter);
		ringSetDestroy (adkeyset);
	}

	/* Return new set in frozen form */
	ringSetFreeze (rsetnew);
	return rsetnew;
}



static const PGPOptionType expkeyOptionSet[] = {
	kPGPOptionType_ExportPrivateKeys,
	kPGPOptionType_ExportPrivateSubkeys,
	kPGPOptionType_ExportFormat,
	kPGPOptionType_OutputFileRef,
	kPGPOptionType_OutputBuffer,
	kPGPOptionType_OutputAllocatedBuffer,
	kPGPOptionType_DiscardOutput,
	kPGPOptionType_ArmorOutput,
	kPGPOptionType_CommentString,
	kPGPOptionType_VersionString,
	kPGPOptionType_EventHandler,
	kPGPOptionType_SendNullEvents,
	kPGPOptionType_OutputLineEndType
};

#if PGP_MACINTOSH
#pragma global_optimizer on
#endif

/* Frees optionList, unlike most other internal functions */
PGPError
pgpExportKeySetInternal (PGPKeySet *keys, PGPOptionListRef optionList)
{
	PGPUInt32			fExportSecrets;
	PGPUInt32			fExportSubSecrets;
	PGPBoolean			fExportAttributes;
	PGPBoolean			fExportFormat;
	PGPByte				*buf;
	PGPSize				bufSize;
	PGPSize				bufSizeRead;
	PGPContextRef		context;
	PGPFile				*pfile;
	RingSet const		*rset;
	RingSet const		*rsetpub;
	PGPOptionListRef	optList;
	PGPExportFormat		exportFormat = kPGPExportFormat_Complete;
	PGPUInt32			wExportFormat;
	PGPBoolean			armorop;
	PGPError			err = kPGPError_NoErr;

	context = PGPGetKeySetContext( keys );
	pgpAssert( pgpContextIsValid( context ) );

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						expkeyOptionSet, elemsof( expkeyOptionSet ) ) ) )
		return err;

	buf = NULL;
	pfile = NULL;
	rsetpub = NULL;
	rset = NULL;
	optList = NULL;

	/* Read optional options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_ExportPrivateKeys, FALSE,
						 "%d", &fExportSecrets ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_ExportPrivateSubkeys, FALSE,
						 "%d", &fExportSubSecrets ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_ExportFormat, FALSE,
						 "%b%d", &fExportFormat, &wExportFormat ) ) )
		goto error;

	if (fExportFormat)
		exportFormat = (PGPExportFormat) wExportFormat;
	fExportAttributes = (exportFormat > kPGPExportFormat_Basic);

	/* Create ringset to extract from */
	if( IsPGPError( err = pgpKeySetRingSet( keys, TRUE, &rset ) ) )
		goto error;
	/* Output public or private portion */
	rsetpub = filterPubRingSet (rset, keys, (PGPBoolean)fExportSecrets,
								(PGPBoolean)(fExportSubSecrets|fExportSecrets),
								FALSE, fExportAttributes);
	if (!rsetpub) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	/* Create memory buffer to write to */
	pfile = pgpFileMemOpen( context, NULL, 0 );
	if( IsNull( pfile ) ) {
		err = kPGPError_OutOfMemory;
		goto error;
	}

	/* Output data to memory buffer */
	if( IsPGPError( err = ringSetWrite (rsetpub, pfile, NULL, PGPVERSION_3,
										0) ) )
		goto error;
	ringSetDestroy( (RingSet *)rsetpub );
	rsetpub = NULL;

	/* Read data we just wrote */
	bufSize = pgpFileTell (pfile );
	buf = (PGPByte *)pgpContextMemAlloc( context, bufSize, 0 );
	if( IsNull( buf ) ) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	(void)pgpFileSeek( pfile, 0, SEEK_SET );
	bufSizeRead = pgpFileRead( buf, bufSize, pfile );
	pgpAssert( bufSizeRead == bufSize );
	pgpFileClose( pfile );
	pfile = NULL;

	/* Do ascii armoring */
	/* If user specified an ascii armor option, use his, else use TRUE */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_ArmorOutput, FALSE,
						"%b", &armorop ) ) )
		goto error;

	/* This next call frees optionList */
	if( IsPGPError( err = PGPBuildOptionList( context, &optList, optionList, 
									 (armorop ?
									  PGPONullOption(context) :
									  PGPOArmorOutput(context, TRUE)),
									 PGPORawPGPInput(context, TRUE),
									 PGPOCompression(context, FALSE),
									 PGPOLastOption(context) ) ) )
		goto error;
	optionList = NULL;

	if( IsPGPError( err = PGPEncode( context, optList,
									 PGPOInputBuffer(context, buf, bufSize),
									 PGPOLastOption(context) ) ) )
		goto error;

	pgpContextMemFree( context, buf );
	buf = NULL;

	/*
	 * If exporting private keys, append public keys to the buffer.
	 * We can't export them both together, as we end up with an output
	 * which has private keys followed by names and sigs.  This can not
	 * be safely imported, because of the "version bug".  The private key
	 * may have the incorrect version.  So we instead output the public
	 * part with names and sigs, after the private part with private keys
	 * and names.
	 */
	if( fExportSecrets || fExportSubSecrets ) {

		/* Get ringset for public portion */
		rsetpub = filterPubRingSet (rset, keys, FALSE, FALSE, FALSE,
									fExportAttributes);
		if (!rsetpub) {
			err = kPGPError_OutOfMemory;
			goto error;
		}

		/* Create memory buffer to write to */
		pfile = pgpFileMemOpen( context, NULL, 0 );
		if( IsNull( pfile ) ) {
			err = kPGPError_OutOfMemory;
			goto error;
		}

		/* Output data to memory buffer */
		if( IsPGPError( err = ringSetWrite (rsetpub, pfile, NULL,
											PGPVERSION_3, 0) ) )
			goto error;
		ringSetDestroy( (RingSet *)rsetpub );
		rsetpub = NULL;

		/* Read data we just wrote */
		bufSize = pgpFileTell (pfile );
		buf = (PGPByte *)pgpContextMemAlloc( context, bufSize, 0 );
		if( IsNull( buf ) ) {
			err = kPGPError_OutOfMemory;
			goto error;
		}
		(void)pgpFileSeek( pfile, 0, SEEK_SET );
		bufSizeRead = pgpFileRead( buf, bufSize, pfile );
		pgpAssert( bufSizeRead == bufSize );
		pgpFileClose( pfile );
		pfile = NULL;

		/* Do ascii armoring, append to existing output */
		PGPAppendOptionList( optList,
							 PGPOAppendOutput(context, TRUE),
							 PGPOLastOption(context) );
		if( IsPGPError( err = PGPEncode( context, optList,
									PGPOInputBuffer(context, buf, bufSize),
									PGPOLastOption(context) ) ) )
			goto error;

		pgpContextMemFree( context, buf );
		buf = NULL;
	}

	err = kPGPError_NoErr;

	/* Fall through */
error:
	if( IsntNull( optList ) )
		PGPFreeOptionList( optList );
	if( IsntNull( optionList ) )
		PGPFreeOptionList( optionList );
	if( IsntNull( pfile ) )
		pgpFileClose( pfile );
	if( IsntNull( rsetpub ) )
		ringSetDestroy ( (RingSet *)rsetpub);
	if( IsntNull( rset ) )
		ringSetDestroy ( (RingSet *)rset);
	if( IsntNull( buf ) )
		pgpContextMemFree( context, buf );
	return err;
}

#if PGP_MACINTOSH
#pragma global_optimizer reset
#endif



/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
