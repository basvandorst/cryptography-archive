/*____________________________________________________________________________	TranslatorFactory.h		Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: TranslatorFactory.h,v 1.3.8.1 1998/11/12 03:06:49 heller Exp $____________________________________________________________________________*/#pragma once#include "pgpTLS.h"class CTranslator;#include "PGPUtilities.h"#include "TranslatorIDs.h"#include "emsapi-mac.h"CTranslator *	CreateTranslator( PGPContextRef context,						PGPtlsContextRef tlsContext, emsProgress progressHook,						TranslatorID id );