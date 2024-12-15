/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPmenuMESP.cp,v 1.5.8.1 1997/12/05 22:14:12 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>
#include <A4Stuff.h>

#include <CodeFragments.h>
#include <MixedMode.h>
#include <UDebugging.h>

union CFragUsage1Union {
	UInt32	appStackSize;
};

union CFragUsage2Union { 
	SInt16	appSubdirID;
};

typedef struct ScfrgResourceMember {
	CFragArchitecture	architecture;
	UInt16				reservedA;
	UInt8				reservedB;
	UInt8				updateLevel;
	NumVersion			currentVersion;
	NumVersion			oldDefVersion;
	CFragUsage1Union	uUsage1;
	CFragUsage2Union	uUsage2;
	CFragUsage			usage;
	CFragLocatorKind	where;
	UInt32				offset;
	UInt32				length;
	UInt32				reservedC;
	UInt32				reservedD;
	UInt16				memberSize;
	Str15				name;
} * ScfrgResourceMemberPtr;

typedef struct ScfrgResource {
	UInt32				reservedA;
	UInt32				reservedB;
	UInt16				reservedC;
	UInt16				version;
	UInt32				reservedD;
	UInt32				reservedE;
	UInt32				reservedF;
	UInt32				reservedG;
	UInt16				reservedH;
	UInt16				memberCount;
	ScfrgResourceMember	firstMember;
} * ScfrgResourcePtr, ** ScfrgResourceHandle;

#include "pflPrefTypes.h"
#include "pflContext.h"
#include "pgpClientPrefs.h"
#include "pgpOpenPrefs.h"
#include "pgpPubTypes.h"

#include "plugin.h"


enum {
	kFinderSignature = 'MACS'
};

struct SMenuApps {
	PGPBoolean	privateScrap;
	PGPBoolean	useOutputDialog;
	OSType		creator;
	Str31		name;
};

// Main entry point declaration
#ifdef __cplusplus
extern "C" {
#endif
typedef SInt32 (*PGPmenuMain)(ProcessInfoRec *, FSSpec *, Boolean, Boolean);

#ifdef __cplusplus
}
#endif

enum {
	uppPGPmenuMainProcInfo = kThinkCStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(SInt32)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ProcessInfoRec *)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(FSSpec *)))
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Boolean)))
};

#if GENERATINGCFM
typedef UniversalProcPtr PGPmenuMainProcUPP;
#define CallPGPmenuMainProc(userRoutine, processInfoPtr, fsSpecPtr,	\
							privateScrap, useOutputDialog)			\
(SInt32) CallUniversalProc(	(UniversalProcPtr)(userRoutine),		\
							uppPGPmenuMainProcInfo,					\
							(processInfoPtr),						\
							(fsSpecPtr),							\
							(privateScrap),							\
							(useOutputDialog))							
#else
typedef PGPmenuMain PGPmenuMainProcUPP;
#define CallPGPmenuMainProc(userRoutine, processInfoPtr, fsSpecPtr,	\
							privateScrap, useOutputDialog)			\
(SInt32)(*(userRoutine))((processInfoPtr), (fsSpecPtr),				\
						(privateScrap), (useOutputDialog))
#endif

Boolean	CaptureThisApp(OSType inCreator, Boolean * outPrivateScrap,
			Boolean * outUseOutputDialog);


	SInt32								// Error code
main (
	ProcessInfoRec *	inProcInfo,	// Proc info for captured browser
	FSSpec *			inFSSpec)	// FSSpec for this plugin
{
	// Remember A4
	EnterCodeResource();

	SInt32				result;

	{
		OSErr				err;
		Boolean				privateScrap;
		Boolean				useOutputDialog;
		SInt16				fileRef;

		fileRef = ::FSpOpenResFile(	inFSSpec,
									fsRdPerm);
									
		// Check for CFM
		SInt32	response;
		
		err = ::Gestalt(gestaltCFMAttr, &response);
		if ((::ResError() != noErr) || (err != noErr)
		|| (((response >> gestaltCFMPresent) & 1) != 1)) {
			result = kUnknownErr;
		} else if ((inProcInfo->processSignature == kFinderSignature)
		|| (CaptureThisApp(inProcInfo->processSignature, &privateScrap,
		&useOutputDialog))) {
			// Load the library
			CFragConnectionID	connID;
			Ptr					mainAddr;
			Str255				errName;
			THz					savedZone = ::GetZone();
			ScfrgResourceHandle	theCFragResourceH = nil;
			
			theCFragResourceH = (ScfrgResourceHandle)
					::Get1Resource(kCFragResourceType, kCFragResourceID);

			if (theCFragResourceH == nil) {
				result = kOutOfMemoryErr;
			} else {
				ScfrgResourceMemberPtr	memberPtr;
				
				::HLock((Handle) theCFragResourceH);
				memberPtr = &(**theCFragResourceH).firstMember;
				
				// If the app is the finder, we need to load in the system
				// zone otherwise move to the next cfrg
				if (inProcInfo->processSignature == kFinderSignature) {
					::SetZone(::SystemZone());
				} else {
					((Ptr) memberPtr) += memberPtr->memberSize;
				}
				err = ::GetDiskFragment(	inFSSpec,
											memberPtr->offset,
											memberPtr->length,
											memberPtr->name,
											kLoadCFrag,
											&connID,
											(Ptr*)&mainAddr,
											errName);
				::SetZone(savedZone);
				::ReleaseResource((Handle) theCFragResourceH);
				if (err != noErr) {
//					if (result == kOutOfMemoryErr) {
//					}
					result = kUnknownErr;
				} else {	
					// Create the UPP
					PGPmenuMainProcUPP	mainUPP;

					mainUPP = (PGPmenuMainProcUPP) 
								::NewRoutineDescriptorTrap(
												(ProcPtr) mainAddr,
												uppPGPmenuMainProcInfo,
												CFM_MESPTarget_);
					if (mainUPP == nil) {
						::CloseConnection(& connID);
						result = kOutOfMemoryErr;
					} else {
						savedZone = ::GetZone();
						::SetZone(::SystemZone());
						result = CallPGPmenuMainProc(	mainUPP,
														inProcInfo,
														inFSSpec,
														privateScrap,
														useOutputDialog);
						::SetZone(savedZone);
						::DisposeRoutineDescriptorTrap(
								(UniversalProcPtr) mainUPP);
						
						// Close the connection if we are not patching
						if (result != noErr) {
							::CloseConnection(& connID);
						}
					}
				}
			}
		}

		::CloseResFile(fileRef);
	}
	
	// Restore A4
	ExitCodeResource();

	return result | kUnloadPlug;
}



	Boolean
CaptureThisApp(
	OSType		inCreator,
	Boolean *	outPrivateScrap,
	Boolean *	outUseOutputDialog)
{
	Boolean			result = false;
	SMenuApps *		apps = nil;
	PGPPrefRef		prefRef = kPGPInvalidRef;
	PFLContextRef	pflContext = kPGPInvalidRef;
	PGPSize			len;
	UInt32			numApps;
	PGPError		pgpErr;

	pgpErr = PFLNewContext(&pflContext);
	if (pgpErr == kPGPError_NoErr) {
		pgpErr = PGPOpenClientPrefs(pflContext, &prefRef);
		if (pgpErr == kPGPError_NoErr) {
			pgpErr = PGPGetPrefData(	prefRef, 
										kPGPPrefPGPmenuMacAppSignatures, 
										&len, 
										&apps);
			if (pgpErr == kPGPError_NoErr) {
				numApps = len / sizeof(SMenuApps);
				for (UInt32 x = 0; x < numApps; x++) {
					if (apps[x].creator == inCreator) {
						*outPrivateScrap = apps[x].privateScrap;
						*outUseOutputDialog = apps[x].useOutputDialog;
						result = true;
						break;
					}
				}
			}
		}
	}
	
	if (apps != nil) {
		PGPDisposePrefData(	prefRef, 
							apps);

	}

	if (prefRef != kPGPInvalidRef) {
		PGPClosePrefFile(prefRef);
	}
	
	if (pflContext != kPGPInvalidRef) {
		PFLFreeContext(pflContext);
	}
	
	return result;
}
