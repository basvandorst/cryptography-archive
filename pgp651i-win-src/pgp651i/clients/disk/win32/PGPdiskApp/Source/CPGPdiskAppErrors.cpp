//////////////////////////////////////////////////////////////////////////////
// CPGPdiskAppErrors.cpp
//
// Functions for error handling
//////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "PGPdiskPfl.h"
#include "UtilityFunctions.h"

#include "CPGPdiskApp.h"
#include "CPGPdiskAppErrors.h"
#include "Globals.h"
#include "PGPdisk.h"


////////////
// Constants
////////////

static LPCSTR kPGPdiskErrorDlgTitle = "PGPdisk";

static LPCSTR kPGPdiskNoMemForErrorString = 
	"PGPdisk is running out of memory - please quit now.";


///////////////////////////////////////
// Functions for PGPdisk error handling
///////////////////////////////////////

// HandleIOError handles an input/output error. This needs special treatment
// since we ask the user if he wants to emergency unmount the PGPdisk, then
// we tell the driver whether to unmount the PGPdisk or attempt to restart IO
// to the PGPdisk.

void 
CPGPdiskApp::HandleIOError(PGDMajorError perr, DualErr derr, PGPUInt8 drive)
{
	DualErr			localDerr;
	PGPdisk			*pPGD;
	UserResponse	button;

	pgpAssert(IsLegalDriveNumber(drive));
	pPGD = mPGPdisks.FindPGPdisk(drive);

	pgpAssertAddrValid(pPGD, PGPdisk);

	// Does user want to emergency unmount or attempt to restart I/O?
	button = ReportError(perr, derr, drive, kPMBS_YesNo);

	if (button == kUser_Yes)
	{
		AppCommandInfo	ACI;

		// Attempt an emergency unmount of the PGPdisk.
		ACI.op		= kOp_Unmount;
		ACI.flags	= kACF_EmergencyUnmount;
		ACI.drive	= pPGD->GetDrive();
		ACI.path[0]	= '\0';

		DispatchAppCommandInfo(&ACI);
	}
	else
	{
		AD_StartStopIO	SSIO;
		DualErr			ssioDerr;

		// Tell the driver to attempt to resume I/O to the PGPdisk.
		SSIO.code		= kAD_StartStopIO;
		SSIO.drive		= drive;				// drive concerned
		SSIO.enableIO	= TRUE;					// restart IO
		SSIO.pDerr		= &ssioDerr;			// error code

		localDerr = Comm->SendPacket((PPacket) &SSIO);

		if (localDerr.IsError())
		{
			ReportError(kPGDMajorError_DriverCommFailed, localDerr);
		}
	
		if (ssioDerr.IsError())
		{
			ReportError(kPGDMajorError_PGPdiskIOControlFailed, ssioDerr, 
				drive);
		}
	}
}

// ReportError is called to report an error message. It displays a dialog box
// with the appropriate text and data.

UserResponse 
CPGPdiskApp::ReportError(
	PGDMajorError	perr, 
	DualErr			derr, 
	PGPUInt8		drive, 
	PGDMessageBoxStyle	style, 
	PGDMessageBoxFocus	focus)
{
	CString			errorString, temp1, temp2, temp3;
	UserResponse	button;

	// Don't display dialog if we fail silently or if user canceled.
	if ((derr.perr == kPGDMinorError_FailSilently) ||
		(derr.perr == kPGDMinorError_UserCanceledOperation))
	{
		return kUser_Cancel;
	}

	Beep(0, 0);

	try
	{
		// There are four possibilities. We can either have both a
		// PGDMinorError and an OSError, a PGDMinorError alone, an OSError
		// alone, or neither.
		//
		// Each major error has the form of "text%s.text" or
		// "text %c:\\ text%s." %c represents an optional drive parameter, and
		// %s is the place where the PGDMinorError/OSError explanation string
		// goes.

		if (style == kPMBS_YesNo)
			derr.err = kOSError_NoErr;	// don't put stuff after question mark

		if (!errorString.LoadString(kPGDMajorErrorStrBase + perr))
		{
			errorString = kPGDMajorUnknownErrorStr;
		}
		else
		{
			if (DoesMajorErrTakeDrive(perr))		// using drive parameter?
			{
				temp1.Format(errorString, DriveNumToLet(drive), "%s");
				errorString = temp1;
			}

			if (derr.HasPGDMinorError() && derr.HasOSError())	// both errs?
			{
				if (!temp1.LoadString(kPGDMinorErrorStrBase + derr.perr))
					temp1 = kPGDMinorUnknownErrorStr;
				
				// Get the system error message.
				::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS |
					FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, derr.err, 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
					(LPSTR) temp2.GetBuffer(kMaxStringSize), kMaxStringSize, 
					NULL);

				temp2.ReleaseBuffer();
				temp3 += "because " + temp1;
				temp1.Format(errorString, temp3);
				errorString = temp1 + " " + temp2;
			}
			else if (derr.HasPGDMinorError())		// just a PGDMinorError?
			{
				if (!temp1.LoadString(kPGDMinorErrorStrBase + derr.perr))
					temp1 = kPGDMinorUnknownErrorStr;
				
				temp2 = "because " + temp1;
				temp1.Format(errorString, temp2);
				errorString = temp1;
			}
			else if (derr.HasOSError())				// just an OS Error?
			{
				// Get the system error message.
				::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS |
					FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, derr.err, 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
					(LPSTR) temp1.GetBuffer(kMaxStringSize), kMaxStringSize, 
					NULL);

				temp1.ReleaseBuffer();
				temp2.Format(errorString, "");
				errorString = temp2 + " " + temp1;
			}
		}

		// Stick a numeric error ID on the end for certain requests.
		if ((derr.perr != kPGDMinorError_NoErr) && 
			(style != kPMBS_YesNo))
		{
			temp1.Format(" (%d)", (PGPUInt32) derr.perr);

			// Tack extra space onto end if last char isn't already space.
			if (errorString[errorString.GetLength() - 1] != ' ')
				errorString += " ";
			
			errorString += temp1;
		}

		// Show the error message.
		button = PGPdiskMessageBox(errorString, kPGPdiskErrorDlgTitle, 
			style, focus);
	}
	catch (CMemoryException *ex)
	{
		// Guaranteed to succeed with these flags.
		::MessageBox(NULL, "PGPdisk", kPGPdiskNoMemForErrorString, 
			MB_ICONHAND | MB_SYSTEMMODAL);

		button = kUser_OK;
		ex->Delete();
	}

	// Return the button the user pressed.
	return button;
}
