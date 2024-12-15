//////////////////////////////////////////////////////////////////////////////
// CPGPdiskCmdLine.cpp
//
// Implementation of class CPGPdiskCmdLine.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskCmdLine.cpp,v 1.1.2.19.2.2 1998/10/22 22:27:48 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "StringAssociation.h"

#include "CPGPdiskCmdLine.h"
#include "Globals.h"


////////////////////////////////////////////////
// Class CPGPdiskCmdLine public member functions
////////////////////////////////////////////////

// The CPGPdiskCmdLine default constructor.

CPGPdiskCmdLine::CPGPdiskCmdLine()
{
	ClearData();
}

// ParseCommandLine parses the entire application's command line. It would
// been nice to use MFC's ParseCommandLine but in their infinite wisdom they
// made that function take a reference to a CCommandLineInfo structure,
// making it impossible to use derived classes due to 'slicing'.

DualErr 
CPGPdiskCmdLine::ParseCommandLine()
{
	DualErr 	derr;
	LPCTSTR 	param;
	PGPBoolean	isFlag;
	PGPBoolean	commandReady	= FALSE;
	PGPBoolean	stopParsing 	= FALSE;

	// Iterate through all the arguments and construct the ACI packet.
	for (PGPInt32 i=1; i < __argc; i++)
	{
		param = __targv[i];

		isFlag = FALSE;

		if (param[0] == '-' || param[0] == '/')
		{
			isFlag = TRUE;
			++param;			// remove flag specifier
		}

		// Parse the parameter. Stop if we are told to.
		derr = ParseParam(param, isFlag, &stopParsing, &commandReady);

		// Process the ACI only if refCommandReady is TRUE.
		if (derr.IsntError())
		{
			if (commandReady)
			{
				App->DispatchAppCommandInfo(&ACI);

				ClearData();
				commandReady = FALSE;
			}

			if (stopParsing)
				break;
		}

		if (derr.IsError())
		{
			break;
		}
	}

	return derr;
}


/////////////////////////////////////////////////
// Class CPGPdiskCmdLine private member functions
/////////////////////////////////////////////////

// ClearData clears the object's request data.

void 
CPGPdiskCmdLine::ClearData()
{
	ACI.op		= kAppOp_InvalidRequest;
	ACI.flags	= NULL;
	ACI.drive	= kInvalidDrive;
	ACI.path[0] = NULL;

	mEmergencyUnmount = FALSE;
	mSilentWiping = FALSE;
}

// ParseParam is called once for every parameter on the command line. We
// construct the ACI packet as we process each parameter, signalling
// refCommandReady when the packet is ready or refStopParsing when the
// command line is not going to result in an operation.

DualErr 
CPGPdiskCmdLine::ParseParam(
	LPCSTR		param, 
	PGPBoolean	isFlag, 
	PGPBoolean	*stopParsing, 
	PGPBoolean	*commandReady)
{
	DualErr derr;

	pgpAssertAddrValid(stopParsing, PGPBoolean);
	pgpAssertAddrValid(commandReady, PGPBoolean);

	try
	{
		// Each parameter can either be a flag, a command, or a pathname.
		// Flags can appear anywhere but the command has to come before the
		// pathname.

		if (isFlag)		// it's a flag
		{
			char		curFlag;
			PGPUInt32	i	= 0;

			curFlag = param[i];

			while (IsntNull(curFlag))
			{
				switch (curFlag)
				{

				case '?':							// print out help
					DisplayMessage(kPGPdiskCmdLineHelpString);
					break;

				case 'e':							// emergency unmount
					mEmergencyUnmount = TRUE;
					break;

				case 's':							// silent wiping
					mSilentWiping = TRUE;
					break;

				default:							// invalid option
					(* stopParsing) = TRUE;
					DisplayMessage(kPGPdiskBadCmdLineString);
					break;
				}

				curFlag = param[i++];
			}
		}
		else if (ACI.op == kAppOp_InvalidRequest)		// it's a command
		{
			CString command = param;

			// Figure out which command it is and prepare the ACI accordingly.

			if (command == GetCommonString(kPGPdiskOpenCmdString))
			{
				ACI.op = kAppOp_Mount;
			}
			else if (command == 
				GetCommonString(kPGPdiskGlobalConvertCmdString))
			{
				ACI.op = kAppOp_GlobalConvert;
				(* commandReady) = TRUE;	// doesn't require a path
			}
			else if (command == GetCommonString(kPGPdiskCreateCmdString))
			{
				ACI.op = kAppOp_Create;
			}
			else if (command == GetCommonString(kPGPdiskMountCmdString))
			{
				ACI.op = kAppOp_Mount;
			}
			else if (command == GetCommonString(kPGPdiskUnmountCmdString))
			{
				ACI.op = kAppOp_Unmount;

				if (mEmergencyUnmount)
					ACI.flags |= kACF_EmergencyUnmount;
			}
			else if (command == GetCommonString(kPGPdiskUnmountAllCmdString))
			{
				ACI.op = kAppOp_UnmountAll;

				if (mEmergencyUnmount)
					ACI.flags |= kACF_EmergencyUnmount;

				(* commandReady) = TRUE;	// doesn't require a path
			}
			else if (command == GetCommonString(kPGPdiskAddPassCmdString))
			{
				ACI.op = kAppOp_AddPassphrase;
			}
			else if (command == GetCommonString(kPGPdiskChangePassCmdString))
			{
				ACI.op = kAppOp_ChangePassphrase;
			}
			else if (command == GetCommonString(kPGPdiskRemovePassCmdString))
			{
				ACI.op = kAppOp_RemovePassphrase;
			}
			else if (command == GetCommonString(kPGPdiskRemoveAltsCmdString))
			{
				ACI.op = kAppOp_RemoveAlternates;
			}
			else if (command == GetCommonString(kPGPdiskPubKeysCmdString))
			{
				ACI.op = kAppOp_AddRemovePublicKeys;
			}

		#if PGPDISK_PUBLIC_KEY && PGPDISK_WIPE_FUNCS

			else if (command == GetCommonString(kPGPdiskWipeAllCmdString))
			{
				ACI.op = kAppOp_WipePassesAllMounted;

				if (mSilentWiping)
					ACI.flags |= kACF_SilentWiping;

				(* commandReady) = TRUE;	// doesn't require a path
			}

			else if (command == GetCommonString(kPGPdiskWipeIndCmdString))
			{
				ACI.op = kAppOp_WipePassesThisDisk;

				if (mSilentWiping)
					ACI.flags |= kACF_SilentWiping;
			}

		#endif // PGPDISK_PUBLIC_KEY && PGPDISK_WIPE_FUNCS

			else if (command == GetCommonString(kPGPdiskAboutCmdString))
			{
				ACI.op = kAppOp_ShowAboutBox;
				(* commandReady) = TRUE;	// doesn't require a path
			}
			else if (command == GetCommonString(kPGPdiskHelpCmdString))
			{
				ACI.op = kAppOp_ShowHelp;
				(* commandReady) = TRUE;	// doesn't require a path
			}
			else if (command == GetCommonString(kPGPdiskPrefsCmdString))
			{
				ACI.op = kAppOp_ShowPrefs;
				(* commandReady) = TRUE;	// doesn't require a path
			}
			else
			{
				// It's not a recognized command, assume "mount" and that
				// this is a path to a PGPdisk.

				ACI.op = kAppOp_Mount;
				strcpy(ACI.path, param);

				(* commandReady) = TRUE;
			}
		}
		else		// it's a pathname
		{
			strcpy(ACI.path, param);

			(* commandReady) = TRUE;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return derr;
}
