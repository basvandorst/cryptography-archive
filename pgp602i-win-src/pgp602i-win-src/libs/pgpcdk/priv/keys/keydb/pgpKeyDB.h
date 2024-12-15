/*____________________________________________________________________________
	pgpKeyDB.h
	
	Copyright( C) 1996,1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	Private definitions for PGP KeyDB Library

	$Id: pgpKeyDB.h,v 1.38.12.1 1998/11/12 03:21:54 heller Exp $
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
PGPError	pgpAddAttributeInternal(PGPKeyRef key,
						PGPAttributeType attributeType,
						PGPByte const *attributeData, PGPSize attributeLength,
						PGPOptionListRef optionList);
PGPError	pgpRevokeCertInternal(PGPSigRef cert, PGPKeySetRef allkeys,
						PGPOptionListRef optionList);
PGPError	pgpRevokeKeyInternal(PGPKeyRef key, PGPOptionListRef optionList);
PGPError	pgpRevokeSubKeyInternal(PGPSubKeyRef subkey,
						PGPOptionListRef optionList);
PGPError	pgpGetFirstSubKey(PGPKeyRef key, PGPSubKeyRef *subKey);
PGPError	pgpChangePassphraseInternal(PGPKeyRef key,
						PGPOptionListRef optionList);
PGPError	pgpChangeSubKeyPassphraseInternal(PGPSubKeyRef subkey,
						PGPOptionListRef optionList);
PGPBoolean	pgpPassphraseIsValidInternal(PGPKeyRef key,
						PGPOptionListRef optionList);
PGPError	pgpSetKeyAxiomaticInternal(PGPKeyRef key,
						PGPOptionListRef optionList);
PGPError	pgpGetKeyPasskeyBufferInternal(PGPKeyRef key,
						void *passkeyBuffer, PGPOptionListRef optionList);
PGPError	pgpGetSubKeyPasskeyBufferInternal(PGPSubKeyRef subkey,
						void *passkeyBuffer, PGPOptionListRef optionList);

PGPError	pgpAddKeyOptionsInternal (PGPKeyRef key,
						PGPOptionListRef optionList);
PGPError	pgpRemoveKeyOptionsInternal (PGPKeyRef key,
						PGPOptionListRef optionList);
PGPError	pgpUpdateKeyOptionsInternal (PGPKeyRef key,
						PGPOptionListRef optionList);



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
