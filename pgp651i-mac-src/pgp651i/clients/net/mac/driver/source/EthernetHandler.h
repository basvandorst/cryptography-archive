/*____________________________________________________________________________	Copyright (C) 1999 Network Associates, Inc.	All rights reserved.	$Id: EthernetHandler.h,v 1.7 1999/02/19 10:05:29 jason Exp $____________________________________________________________________________*/#pragma once#include "PGPnetModuleCommon.h"PGPBoolean	EthernetHandleReadPutDATA(queue_t * inQ, mblk_t * inMessage,				SGlobals * inGlobals);PGPBoolean	EthernetHandleWritePutDATA(queue_t * inQ, mblk_t * inMessage,				SGlobals * inGlobals);