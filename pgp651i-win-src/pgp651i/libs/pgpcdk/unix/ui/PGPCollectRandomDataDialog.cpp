/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPCollectRandomDataDialog.cpp,v 1.4.8.1 1999/06/17 21:32:19 heller Exp $
____________________________________________________________________________*/
#include <stdio.h>

// PGP SDK header files
#include "pgpDialogs.h"
#include "pgpRandomPool.h"
#include "pgpKB.h"

// local globals
static PGPBoolean			g_bEntropy = FALSE;

//	______________________________________________
//
//  Entropy collection routine -- posts dialog for 
//  mouse/keyboard entropy collection.

PGPError
pgpCollectRandomDataDialogPlatform(
	PGPContextRef					context,
	CPGPRandomDataDialogOptions 	*options)
{
	PGPInt32 lRandBits = PGPGlobalRandomPoolGetEntropy();

	(void) context;
	
	if (g_bEntropy) 
		return kPGPError_ItemAlreadyExists;

	g_bEntropy = TRUE;
	PGPBoolean bCollectionEnabled = TRUE;

	// if zero passed in, use default
	PGPInt32 lTotalNeeded;
	if (options->mNeededEntropyBits) 
		lTotalNeeded = options->mNeededEntropyBits;
	else 
		lTotalNeeded = PGPGlobalRandomPoolGetMinimumEntropy();

	// if we have enough already, then just return
	if (lTotalNeeded < lRandBits) 
		return kPGPError_NoErr;

	// collect the entropy
	if (options->mPrompt)
		printf("%s\n", options->mPrompt);
	else
		printf("\n\
PGP needs to generate some random data. This is done by measuring\n\
the time intervals between your keystrokes. Please enter some\n\
random text on your keyboard until the indicator reaches 100%%.\n\
Press ^D to cancel\n");

	// Go into CBreak Mode
	PGPInt32 lStillNeeded = lTotalNeeded - lRandBits;
    kbCbreak(0);
	
	double dPercent = 0;
	while (bCollectionEnabled) {

		if (lRandBits > 0)
			dPercent = ((double) lRandBits / lTotalNeeded) * 100;

		if (dPercent > 100.5)
			dPercent = 100;

		if (dPercent >= 99.5 && lStillNeeded > 0)
			dPercent = 99.0;

		printf("\r");
		printf("%.0f%% of required data   ", dPercent);
		fflush(stdout);
		
		if (lStillNeeded <= 0) {
			bCollectionEnabled = FALSE;
			printf("\n");
			break;
		}
		
		kbFlush(0);
		int c = kbGet();		// get the key and add to random pool
		if (c == 0x04) {
			g_bEntropy = FALSE;
			printf("\a\nUser cancelled collection. \n");
			kbFlush(1);
			kbNorm();
			return kPGPError_UserAbort;
		}
		
		lRandBits = PGPGlobalRandomPoolGetEntropy();
		lStillNeeded = lTotalNeeded - lRandBits;
	}
	
    printf("\aEnough, thank you.\n");;
	
	kbFlush(1);
    kbNorm();
	g_bEntropy = FALSE;

	return kPGPError_NoErr;
}

