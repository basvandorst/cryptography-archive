/*____________________________________________________________________________
	CManualTranslator.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CManualTranslator.h,v 1.4 1997/08/05 20:57:43 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include "CTranslator.h"

class CManualTranslator : public CTranslator
{
public:
	
protected:
	
public:
			CManualTranslator( PGPContextRef context,
				emsProgress progressHook)
				: CTranslator( context, progressHook)
				{
				}
};



class CManualDecryptVerifyTranslator : public CManualTranslator
{
protected:
	virtual CComboError	HandlePassphraseEvent(
							PGPEvent *event,
							EventHandlerData *data );				
public:
			CManualDecryptVerifyTranslator( PGPContextRef context,
				emsProgress progressHook)
				: CManualTranslator( context, progressHook)
				{
				}
				
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMEtype ***	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage, long * resultCode);
};



class CManualAddKeyTranslator : public CManualTranslator
{
public:
			CManualAddKeyTranslator( PGPContextRef context,
				emsProgress progressHook)
				: CManualTranslator( context, progressHook)
				{
				}
				
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMEtype ***	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage, long * resultCode);
};


class CManualEncryptSignTranslator : public CManualTranslator
{
protected:
	Boolean			mDoSign;
	
public:
		CManualEncryptSignTranslator( PGPContextRef context,
			emsProgress progressHook, Boolean sign );
					
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMEtype ***	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage, long * resultCode);
};


class CManualSignTranslator : public CManualTranslator
{
public:
			CManualSignTranslator( PGPContextRef context,
				emsProgress progressHook)
				: CManualTranslator( context, progressHook)
				{
				}
				
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMEtype ***	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage, long * resultCode);
};


