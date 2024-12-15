//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInterfaceErrors.cpp
//
// Error handling functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInterfaceErrors.cpp,v 1.1.2.8 1998/07/06 23:26:58 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include <vdw.h>

#include "Required.h"
#include "DualErr.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"

#include "CPGPdiskInterface.h"
#include "CPGPdiskInterfaceErrors.h"
#include "Globals.h"
#include "KernelModeUtils.h"
#include "msglog.h"


///////////
// Constant
///////////

const LPCSTR kPGPdiskKernelErrString = 
	"Error code is (%d, %d, %d)";


///////////////////////////
// Error handling functions
///////////////////////////

// ReportError reports certain errors to the global event log..

void 
CPGPdiskInterface::ReportError(
	PGDMajorError	perr, 
	DualErr			derr, 
	PGPUInt8		drive)
{
	static char				errString[kMaxStringSize];
	KErrorLogEntry			errEntry;
	KUstring				uniErrString;
	PIO_ERROR_LOG_PACKET	pErrPacket;
	static PGPUInt32		uniqueCode	= 0;

	pErrPacket = errEntry;

	// Prepare the error log entry.
	pErrPacket->FinalStatus			= STATUS_SUCCESS;
	pErrPacket->ErrorCode			= PGPDISK_ERROR_CODE;
	pErrPacket->UniqueErrorValue	= uniqueCode++;

	// Prepare the error string and post it.
	sprintf(errString, kPGPdiskKernelErrString, (PGPUInt32) perr, 
		(PGPUInt32) derr.mMinorError, derr.mExternalError);

	if (AssignToUni(&uniErrString, errString).IsntError())
	{
		errEntry.InsertString(uniErrString.UnicodeString().Buffer);
		errEntry.Post();
	}
}
