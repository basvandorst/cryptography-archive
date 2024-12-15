//////////////////////////////////////////////////////////////////////////////
// CPGPdiskCmdLine.h
//
// Declaration of class CPGPdiskCmdLine.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskCmdLine.h,v 1.4 1998/12/14 18:57:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGpdiskCmdLine_h	// [
#define Included_CPGPdiskCmdLine_h

#include "DualErr.h"

#include "CPGPdiskAppACI.h"


////////////////////////
// Class CPGPdiskCmdLine
////////////////////////

// CPGPdiskCmdLine is responsible for parsing the PGPDisk command line. It is
// NOT derived from CCommandLineInfo due to some MFC idiocy. It parses only
// one command.

class CPGPdiskCmdLine
{
public:
			CPGPdiskCmdLine();
			~CPGPdiskCmdLine()	{ };

	DualErr	ParseCommandLine();

private:
	AppCommandInfo	ACI;				// we fill this in as we parse
	PGPBoolean		mEmergencyUnmount;	// emergency unmount?
	PGPBoolean		mSilentWiping;		// don't confirm wiping?

	void	ClearData();
	DualErr	ParseParam(LPCSTR param, PGPBoolean isFlag, 
				PGPBoolean *stopParsing, PGPBoolean *commandReady);
};

#endif	// ] Included_CPGPdiskCmdLine_h
