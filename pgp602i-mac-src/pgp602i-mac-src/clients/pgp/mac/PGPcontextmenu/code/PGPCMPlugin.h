/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: PGPCMPlugin.h,v 1.2.20.1 1998/11/12 03:07:31 heller Exp $____________________________________________________________________________*/#pragma once // Parent Class Include#include "AbstractCMPlugin.h"class PGPCMPlugin : virtual AbstractCMPlugin {#pragma SOMReleaseOrder(	Initialize,				\							ExamineContext,			\							HandleSelection,		\							PostMenuCleanup)		\public:	virtual	OSStatus Initialize(							Environment*,							FSSpec *inFileSpec);	virtual	OSStatus ExamineContext(							Environment*,							AEDesc *inContextDescriptor,							SInt32 inTimeOutInTicks,							AEDescList* ioCommands,							Boolean* outNeedMoreTime);	virtual	OSStatus HandleSelection(							Environment*,							AEDesc *inContextDescriptor,							SInt32 inCommandID);	virtual	OSStatus PostMenuCleanup(							Environment*);};