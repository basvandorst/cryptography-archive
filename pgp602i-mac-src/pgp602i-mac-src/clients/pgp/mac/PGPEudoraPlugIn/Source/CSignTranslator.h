/*____________________________________________________________________________	CSignTranslator.h		Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: CSignTranslator.h,v 1.8.8.1 1998/11/12 03:06:35 heller Exp $____________________________________________________________________________*/#pragma once#include "TranslatorUtils.h"#include "CTranslator.h"class CSignTranslator : public CTranslator{protected:		CipherOutputType	mOutputType;			virtual void		GetErrorString( CComboError err, StringPtr msg);	public:	CSignTranslator( PGPContextRef context, PGPtlsContextRef tlsContext,					emsProgress progressHook,					CipherOutputType outputType = kUseOldPGPFormat );		virtual PluginError	TranslateFile( long transContext, 							ConstemsMIMETypeHandle InMIMEtypeHandle,							const FSSpec * inFSp,							StringHandle** addressesHandle,							emsMIMETypeHandle *	OutMIMEtypeHandlePtr,							const FSSpec * outFSp,							StringHandle* returnedMessage,							StringHandle* errorMessage, long * resultCode);};