/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PluginMain.h,v 1.10 1999/04/09 20:09:44 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_PluginMain_h	/* [ */
#define Included_PluginMain_h

#include "emssdk/ems-win.h"
#include "TranslatorIDs.h"
#include "resource.h"

#define InitPtrToNull( p )	{ if ( !( p ) )	*(p) = NULL; }

typedef struct _TransInfoStruct {
	char *description;
	long type;
	long subtype;
	unsigned long flags;
	unsigned long nIconID;
}TransInfoStruct;

static TransInfoStruct gTransInfo[] = {
	{
		"Decrypt PGP/MIME Message",
		EMST_PREPROCESS,
		kDecryptTranslatorID, 
		( EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_WHOLE_MESSAGE | 
			EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		IDI_PGP_DECRYPT
	},
	{
		"Verify PGP/MIME Message",
		EMST_SIGNATURE,
		kVerifyTranslatorID, 
		( EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_WHOLE_MESSAGE | 
			EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		IDI_PGP_VERIFY
	},
	{
		"PGP Encrypt",
		EMST_PREPROCESS,
		kEncryptTranslatorID, 
		( EMSF_Q4_COMPLETION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | 
			EMSF_GENERATES_MIME),
		IDI_PGP_ENCRYPT
	},
	{
		"PGP Sign",
		EMST_SIGNATURE,
		kSignTranslatorID, 
		( EMSF_Q4_COMPLETION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | 
			EMSF_GENERATES_MIME),
		IDI_PGP_SIGN
	},
	{
		"PGP Encrypt & Sign",
		EMST_COALESCED,
		kEncryptAndSignTranslatorID, 
		( EMSF_Q4_COMPLETION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | 
			EMSF_GENERATES_MIME ),
		0L //IDI_PGP_ENCRYPT_SIGN
	},
	{
		"PGP Encrypt",
		EMST_LANGUAGE,
		kManualEncryptTranslatorID, 
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		IDI_PGP_ENCRYPT
	},
	{
		"PGP Sign",
		EMST_LANGUAGE,
		kManualSignTranslatorID, 
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		IDI_PGP_SIGN
	},
	{
		"PGP Encrypt & Sign",
		EMST_LANGUAGE,
		kManualEncryptSignTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		IDI_PGP_ENCRYPT_SIGN
	},
	{
		"PGP Decrypt & Verify",
		EMST_LANGUAGE,
		kManualDecryptVerifyTranslatorID, 
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE | EMSF_GENERATES_MIME |
			EMSF_TOOLBAR_PRESENCE),
		IDI_PGP_DECRYPT
	}
};

static TransInfoStruct gSpecialInfo[] = {

	{
		"Launch PGPkeys",
		EMST_LANGUAGE,
		kSpecialLaunchKeysID,
		( EMSF_ON_REQUEST | EMSF_TOOLBAR_PRESENCE  ),
		IDI_PGP_KEYS
	}

};

static const long	kNumTrans = (sizeof(gTransInfo)/sizeof(*gTransInfo));

static const long	kNumSpecial = 
				(sizeof(gSpecialInfo)/sizeof(*gSpecialInfo));

// for future use we have 881 882 883 884 as well
static const short	kPluginID = 880; 

#endif /* ] Included_PluginMain_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


