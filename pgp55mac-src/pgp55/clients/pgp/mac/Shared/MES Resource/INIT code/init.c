/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: init.c,v 1.2.10.1 1997/12/05 22:15:15 mhw Exp $
____________________________________________________________________________*/

#include <Types.h>

#include <SetupA4.h>
#include <A4Stuff.h>

#include "ShowInitIcon.h"
#include "Jimi.h"
#include "plugin.h"
#include "init.h"

#ifdef powerc

#define MacsBug_()     SysBreak()
#define MacsBugStr_(s) SysBreakStr(s)

#else // 68K

#define MacsBug_()     Debugger()
#define MacsBugStr_(s) DebugStr(s)

#endif

void main(void)
{
	// Remember A4
	EnterCodeResource();
	
	{
		THz					savedZone;
		SInt32				theResponse;
		OSErr				err = noErr;
		Boolean				canRun = true;
		Handle				patchCodeH = NULL;
		CFragConnectionID	connID = 0;
		Ptr					symDirNameAddr = nil;
		
		// Set the current heap to the system heap
		savedZone = GetZone();
		SetZone(SystemZone());
		
		// Show starting icon
		ShowInitIcon(kRunningIcon, false);

		// Check to see if we are running System 7.5 or greater
		err = Gestalt(gestaltSystemVersion, &theResponse);
		if ((err != noErr) || (theResponse < kMinimumSystemVersion)) {
			canRun = false;
		}
			
		// Patch trap
		if (canRun) {
			// Get the patch code and detach it
			patchCodeH = Get1Resource(	kPatchResType,
										k68KPatchResID);
			if (patchCodeH == NULL) {
				canRun = false;
			} else {
				// JSR to our code
				JimiInitProcPtr	JimiInit = (JimiInitProcPtr) *patchCodeH;
				
				JimiInit(patchCodeH);
			}
		}
		
		// Display the correct icon
		if (canRun) {
			ShowInitIcon(kRunningIcon, true);
		} else {
			ShowInitIcon(kNotRunningIcon, true);
		}
		
		// Restore the previous heap
		SetZone(savedZone);
	}
	
	// Restore A4
	ExitCodeResource();
}
