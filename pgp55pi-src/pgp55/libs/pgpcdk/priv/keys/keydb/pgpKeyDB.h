/*____________________________________________________________________________
	pgpKeyDB.h
	
	Copyright( C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Private definitions for PGP KeyDB Library

	$Id: pgpKeyDB.h,v 1.33 1997/08/21 23:57:46 mhw Exp $
____________________________________________________________________________*/
#ifndef Included_pgpKeyDB_h	/* [ */
#define Included_pgpKeyDB_h

#include "pgpConfig.h"

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#include "pgpKeys.h"
#include "pgpTrust.h"
#include "pgpRngPub.h"

/* private entry points */

PGP_BEGIN_C_DECLARATIONS


/* always returns PGPTRUST0 in this release */
PgpTrustModel 	PGPGetTrustModel( PGPContextRef context );

PGPError		pgpDearmorKey(PGPEnv *env, int fFilein,
						PFLFileSpecRef InputFileRef, PGPFileRead *pfrin,
						PGPByte const *InputBuffer, PGPSize InputBufferLen,
						PGPByte **OutputBuffer, PGPSize *OutputBufferLen);

PGPUInt32 	pgpKeyEntropyNeededInternal(PGPContextRef context,
						PGPOptionListRef optionList);
PGPError 	pgpGenerateKeyInternal(PGPContextRef context,
						PGPKeyRef *key, PGPOptionListRef optionList);
PGPError 	pgpGenerateSubKeyInternal(PGPContextRef context,
						PGPSubKeyRef *subkey, PGPOptionListRef optionList);
PGPError 	pgpExportKeySetInternal(PGPKeySet *keys,
						PGPOptionListRef optionList);
PGPError 	pgpImportKeySetInternal(PGPContextRef context,
						PGPKeySetRef *keys, PGPOptionListRef optionList);
PGPError 	pgpCertifyUserIDInternal(PGPUserID *userid,
						PGPKey *certifyingKey, PGPOptionListRef optionList);
PGPError	pgpImportKeyBinary(PGPContextRef cdkContext, PGPByte *buffer,
						size_t length, PGPKeySetRef *outRef);
PGPError	pgpAddUserIDInternal(PGPKeyRef key, char const *userID,
						PGPOptionListRef optionList);
PGPError	pgpRevokeCertInternal(PGPSigRef cert, PGPKeySetRef allkeys,
						PGPOptionListRef optionList);
PGPError	pgpRevokeKeyInternal(PGPKeyRef key, PGPOptionListRef optionList);
PGPError	pgpRevokeSubKeyInternal(PGPSubKeyRef subkey,
						PGPOptionListRef optionList);
PGPError	pgpGetFirstSubKey(PGPKeyRef key, PGPSubKeyRef *subKey);


PGP_END_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

#endif /* ] Included_pgpKeyDB_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
