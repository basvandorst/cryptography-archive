/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPCMPlugin.h,v 1.2.10.1 1997/12/05 22:13:46 mhw Exp $
____________________________________________________________________________*/

#pragma once 

// Parent Class Include
#include "AbstractCMPlugin.h"


class PGPCMPlugin : virtual AbstractCMPlugin {

#pragma SOMReleaseOrder(	Initialize,				\
							ExamineContext,			\
							HandleSelection,		\
							PostMenuCleanup)		\

public:

	virtual	OSStatus Initialize(
							Environment*,
							FSSpec *inFileSpec);
	virtual	OSStatus ExamineContext(
							Environment*,
							AEDesc *inContextDescriptor,
							SInt32 inTimeOutInTicks,
							AEDescList* ioCommands,
							Boolean* outNeedMoreTime);
	virtual	OSStatus HandleSelection(
							Environment*,
							AEDesc *inContextDescriptor,
							SInt32 inCommandID);
	virtual	OSStatus PostMenuCleanup(
							Environment*);
};

