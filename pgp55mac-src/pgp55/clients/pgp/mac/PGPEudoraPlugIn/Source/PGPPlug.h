/*____________________________________________________________________________
	PGPPlug.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPPlug.h,v 1.1 1997/07/25 23:47:33 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#include "usertrans.h"
#include "emsapi-mac.h"

#include "PGPPlugTypes.h"

extern "C" {

PluginError	PlugInFinish( emsUserGlobals **	globalsHandle );

PluginError	PlugInInit( emsUserGlobals ** globalsHandle,
				short* module_count, StringHandle *	descriptionHandlePtr,
				short* module_id, Handle* iconHandlePtr);
			
PluginError	GetTranslatorInfo( emsUserGlobals ** globalsHandle,
				short trans_id,
				long * type, long * subtype, unsigned long * flags,
				StringHandle* trans_desc, Handle * iconHandlePtr);
			
PluginError	CanTranslateFile( emsUserGlobals ** globalsHandle,
				long transContext,
				short trans_id, emsMIMEtype ** inMimeTypeHandle,
				const FSSpec * inFSp,
				StringHandle** addressesHandle, long * aprox_len,
				StringHandle* errorMessage, long * result_code);
				
PluginError	TranslateFile( emsUserGlobals ** globalsHandle,
				long transContext, short trans_id,
				emsMIMEtype ** InMIMEtypeHandle, const FSSpec * inFSp,
				StringHandle** addressesHandle, emsProgress progress,
				emsMIMEtype ***	OutMIMEtypeHandlePtr, const FSSpec * outFSp,
				Handle * returnedIcon, StringHandle* returnedMessage,
				StringHandle* errorMessage, long * resultCode);
}