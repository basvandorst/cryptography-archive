/*____________________________________________________________________________
	CEncryptTranslator.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CEncryptTranslator.h,v 1.3 1997/08/05 20:57:42 lloyd Exp $
____________________________________________________________________________*/
#pragma once


#include "CTranslator.h"

struct PGPKeySpec;

class CEncryptTranslator : public CTranslator
{
protected:
	CipherOutputType	mOutputType;
	Boolean				mDoSign;
	
public:
				CEncryptTranslator(
					PGPContextRef	context,
					emsProgress	progressHook,
					CipherOutputType outputType = kUseOldPGPFormat,
					Boolean doSign = false );
	
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMETypeHandle *	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage,
							long * resultCode);
};
