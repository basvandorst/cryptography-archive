/*
** Copyright (C)1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/



#ifndef _PGP_PLUGIN_H 
#define _PGP_PLUGIN_H 

#include "emssdk/ems-win.h"
#include "PGPPlugTypes.h"
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
		( EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		IDI_PGP_DECRYPT
	},
	{
		"Verify PGP/MIME Message",
		EMST_SIGNATURE,
		kVerifyTranslatorID, 
		( EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		IDI_PGP_VERIFY
	},
	{
		"PGP plugin 1 (use toolbar)",
		EMST_PREPROCESS,
		kEncryptTranslatorID, 
		( EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		IDI_PGP_ENCRYPT
	}
	,
	{
		"PGP plugin 2 (use toolbar)",
		EMST_SIGNATURE,
		kSignTranslatorID, 
		( EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		IDI_PGP_ENCRYPT//IDI_PGP_SIGN
	},
	{
		"PGPmime Encrypt & Sign",
		EMST_COALESCED,
		kEncryptAndSignTranslatorID, 
		( EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		0L //IDI_PGP_ENCRYPT_SIGN
	},
	/*
	{
		"PGP Encrypt & Sign",
		EMST_LANGUAGE,
		kManualEncryptSignTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		IDI_PGP_ENCRYPT
	},
	{
		"PGP Decrypt & Verify",
		EMST_LANGUAGE,
		kManualDecryptVerifyTranslatorID, 
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE ),
		IDI_PGP_DECRYPT
	},
	
	{
		"Extract PGP Key From Message",
		EMST_LANGUAGE,
		kManualAddKeyTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		IDI_PGP_ADDKEY
	}*/
};

static const long	kNumTrans				= (sizeof(gTransInfo)/sizeof(*gTransInfo));
static const short	kPGPPluginID			= 880; // for future use we have 881 882 883 884 as well

static const char*	kPGPPluginDescription	= "PGP 5.5";

#endif _PGP_PLUGIN_H
