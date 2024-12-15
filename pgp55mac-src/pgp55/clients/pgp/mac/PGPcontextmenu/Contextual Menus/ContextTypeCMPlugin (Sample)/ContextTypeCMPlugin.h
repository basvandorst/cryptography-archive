/*
	File:		ContextTypeCMPlugin.h

	Contains:	<contents>

	Written by:	Guy Fullerton

	Copyright:	<copyright>

	Change History (most recent first):

        <0+>     1/16/97    GBF     first checked in
*/


#pragma once


// Parent Class Include
#include <AbstractCMPlugin.h>


class ContextTypeCMPlugin : virtual AbstractCMPlugin {

#pragma SOMReleaseOrder (Initialize, ExamineContext, HandleSelection, PostMenuCleanup)

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

