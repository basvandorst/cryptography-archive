/*____________________________________________________________________________
	TranslatorFactory.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorFactory.h,v 1.2 1997/08/02 21:08:16 lloyd Exp $
____________________________________________________________________________*/

#pragma once


class CTranslator;

#include "PGPUtilities.h"
#include "TranslatorIDs.h"

#include "emsapi-mac.h"

CTranslator *	CreateTranslator( PGPContextRef context,
					emsProgress progressHook, TranslatorID id );