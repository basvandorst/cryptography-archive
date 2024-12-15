/*____________________________________________________________________________	CSignTranslator.h		Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CSignTranslator.h,v 1.10 1999/04/26 09:40:12 heller Exp $____________________________________________________________________________*/#pragma once#include "TranslatorUtils.h"#include "CTranslator.h"class CSignTranslator : public CTranslator{protected:		CipherOutputType	mOutputType;			virtual void		GetErrorString( CComboError err, StringPtr msg);	public:	CSignTranslator( PGPContextRef context, PGPtlsContextRef tlsContext,					emsProgress progressHook,					CipherOutputType outputType = kUseOldPGPFormat );		virtual PluginError	TranslateFile(emsTranslatorP trans, emsDataFileP inFile,    							emsDataFileP outFile, emsResultStatusP transStatus);};