/*____________________________________________________________________________
	CSignTranslator.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSignTranslator.h,v 1.6 1997/08/05 20:57:44 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#include "TranslatorUtils.h"
#include "CTranslator.h"

class CSignTranslator : public CTranslator
{
protected:
	CipherOutputType	mOutputType;
	
	
	virtual void		GetErrorString( CComboError err, StringPtr msg);
	

public:
	CSignTranslator( PGPContextRef context, emsProgress progressHook,
					CipherOutputType outputType = kUseOldPGPFormat );
	
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMETypeHandle *	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage, long * resultCode);
};
