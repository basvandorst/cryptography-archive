/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CFM-MESP.cp,v 1.2.10.1 1997/12/05 22:15:13 mhw Exp $
____________________________________________________________________________*/

#include <CodeFragments.h>
#include <MixedMode.h>
#include <UDebugging.h>

#include "plugin.h"


enum {
	kFinderSignature = 'MACS'
};


	SInt32								// Error code
main (
	ProcessInfoRec *	inProcInfo,	// Proc info for captured browser
	FSSpec *			inFSSpec)	// FSSpec for this plugin
{
	OSErr				err;
	SInt32				result;

	// Check for CFM
	SInt32	response;
	
	err = ::Gestalt(gestaltCFMAttr, &response);
	if ((err != noErr) || (((response >> gestaltCFMPresent) & 1) != 1)) {
		result = kUnknownErr;
	} else {
		// Load the library
		CFragConnectionID	connID;
		Ptr					mainAddr;
		Str255				errName;
		THz					savedZone = ::GetZone();

		// If the app is the finder, we need to load in the system zone
		if (inProcInfo->processSignature == kFinderSignature) {
			::SetZone(::SystemZone());
		}
		err = ::GetDiskFragment(	inFSSpec,
									0L,
									kCFragGoesToEOF,
									inProcInfo->processName,
									kLoadCFrag,
									&connID,
									(Ptr*)&mainAddr,
									errName);
		::SetZone(savedZone);
		if (err != noErr) {
			result = kUnknownErr;
		} else {	
			// Create the UPP
			PlugInMainProcUPP	mainUPP;

			mainUPP = (PlugInMainProcUPP) ::NewRoutineDescriptorTrap(
											(ProcPtr) mainAddr,
											uppPlugInMainProcInfo,
											CFM_MESPTarget_);
			if (mainUPP == nil) {
				::CloseConnection(& connID);
				result = kOutOfMemoryErr;
			} else {
				savedZone = ::GetZone();
				::SetZone(::SystemZone());
				result = CallPlugInMainProc(mainUPP, inProcInfo, inFSSpec);
				::SetZone(savedZone);
				::DisposeRoutineDescriptorTrap((UniversalProcPtr) mainUPP);
				
				// Close the connection if we are not patching
				if (result != noErr) {
					::CloseConnection(& connID);
				}
			}
		}
	}
	
	return result | kUnloadPlug;
}
