/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPEmailerGlobals.h,v 1.3.10.1 1997/12/05 22:14:00 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CWindowRefArray.h"
#include "pgpUserInterface.h"

typedef struct GlobalsS {
#if BETA || PGP_DEMO
	Boolean			disableEncryptAndSign;
#endif
	SInt16			appResForkRef;
	NumVersion		versInfo;
	PGPContextRef	pgpContext;
	Str32			processName;
	ProcessInfoRec	processProcInfo;
	FSSpec			processSpec;
	FSSpec			plugSpec;
	CWindowRefArray	windowRefArray;
} SGlobals, * SGlobalsPtr;
