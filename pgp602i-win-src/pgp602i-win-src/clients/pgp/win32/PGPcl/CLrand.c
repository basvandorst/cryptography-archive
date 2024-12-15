/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLrand.c - post dialog to get entropy from mouse and keyboard
	

	$Id: CLrand.c,v 1.7 1998/08/11 14:43:13 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// pgp header files
#include "pgpRandomPool.h"
#include "pgpUserInterface.h"

//	______________________________________________
//
//  Entropy collection routine -- posts dialog for 
//  mouse/keyboard entropy collection.

PGPError PGPclExport 
PGPclRandom (
		PGPContextRef	context,
		HWND			hwnd, 
		UINT			uNeeded) 
{
	PGPError err	= kPGPError_NoErr;

	if ((uNeeded != 0) ||
		!PGPGlobalRandomPoolHasMinimumEntropy ())
	{
		err=PGPCollectRandomDataDialog (
			context,
			uNeeded,
			PGPOUIParentWindowHandle (context, hwnd),
			PGPOLastOption (context));
	}

	return err;
}
