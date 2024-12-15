/*
	File:		AbstractCMPlugin.h

	Contains:	<contents>

	Written by:	Guy Fullerton

	Copyright:	<copyright>

	Change History (most recent first):

         <2>     1/16/97    GBF     The command ID passed to HandleSelection
         							is an SInt32. Add the
                                    PostMenuCleanup method. Making methods
                                    virtual.
        <0+>     1/14/97    GBF     first checked in
*/


#pragma once

// SOM Includes
#include <somobj.hh>


// Mac OS Includes
#include <Types.h>
#include <Files.h>
#include <AppleEvents.h>


class AbstractCMPlugin : virtual SOMObject {

#pragma SOMReleaseOrder (	Initialize,					\
							ExamineContext,				\
							HandleSelection,			\
							PostMenuCleanup)

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

