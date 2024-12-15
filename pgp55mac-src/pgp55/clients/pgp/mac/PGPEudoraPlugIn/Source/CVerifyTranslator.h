/*____________________________________________________________________________
	CVerifyTranslator.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CVerifyTranslator.h,v 1.4 1997/08/05 20:57:46 lloyd Exp $
____________________________________________________________________________*/
#pragma once


#include "CTranslator.h"

class CVerifyTranslator : public CTranslator
{
protected:
	
	
public:
						CVerifyTranslator( PGPContextRef context,
							emsProgress progressHook)
							: CTranslator( context, progressHook)
							{
							}
							
	virtual PluginError	CanTranslate( long ransContext,
						emsMIMETypeHandle inMimeTypeHandle,
						const FSSpec * inFSp, StringHandle** addressesHandle,
						long * aprox_len, StringHandle* errorMessage,
						long * result_code);
						
						
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMETypeHandle *	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage, long * resultCode);
};
