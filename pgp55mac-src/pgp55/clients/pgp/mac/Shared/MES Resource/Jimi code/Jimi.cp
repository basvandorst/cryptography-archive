/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: Jimi.cp,v 1.2.10.1 1997/12/05 22:15:17 mhw Exp $
____________________________________________________________________________*/

// Turn on and off debugging
//#define Debug_Throw


#include <ShutDown.h>

#include <A4Stuff.h>
#include <SetUpA4.h>

#include <string.h>

#include <UDebugging.h>
#include <UException.h>

#include "StSaveCurResFile.h"
#include "sharedmem.h"
#include "plugin.h"
#include "init.h"
#include "Jimi.h"

#ifdef powerc

#define MacsBug_()     SysBreak()
#define MacsBugStr_(s) SysBreakStr(s)

#else // 68K

#define MacsBug_()     Debugger()
#define MacsBugStr_(s) DebugStr(s)

#endif


// Declarations
pascal OSErr SharedMemSelector(OSType selector,	long *response);

typedef struct FSSpecListS{
	UInt32					minFreeSpace;
	FSSpec					fsSpec;
	struct FSSpecListS *	link;
} FSSpecListS, *FSSpecListSPtr;

typedef struct CaptureListS{
	OSType					creator;
	FSSpecListSPtr			fsSpecListP;
	struct CaptureListS *	link;
} CaptureListS, *CaptureListSPtr;

typedef struct ProcessListS {
	ProcessSerialNumber		psn;
	struct ProcessListS *	link;
} ProcessListS, *ProcessListSPtr;

const	Str31	kCaptureListNamedBlock	=	"\pJimi_CaptureList";
const	Str31	kJimiVersionNamedBlock	=	"\pJimi_Version";
const	UInt16	kJimiVersion			=	0x0100;
const	OSType	kFinderSig				=	'MACS';


// Globals
Str31				gDirName;
UniversalProcPtr	gPreviousCall;
ProcessListSPtr		gProcessListP = nil;
Boolean				gInitialized = false;


pascal void MyShutDownProc();
#ifdef __cplusplus
extern "C" {
#endif
extern void	__InitCode__(void);
extern void	__destroy_global_chain(void);
#ifdef __cplusplus
}
#endif

	pascal void
main(
	Handle	inJimiResH)
{
	EnterCodeResource();
	PrepareCallback();

	__InitCode__();
	
	SInt32					err = noErr;
	CaptureListSPtr *		theCaptureListP;
	UInt16 *				theJimiVersionP;
	
	// Install this resource if it is the first one otherwise get shared
	// mem
	err = GetNamedBlock(	kCaptureListNamedBlock,
							(Ptr *) &theCaptureListP);
	if (err == gestaltUndefSelectorErr) {
		// Add Gestalt Selector
		err = ::NewGestalt(	kSharedMemSelector,
							NewSelectorFunctionProc(SharedMemSelector));
		if (err != noErr) {
			goto exitLabel;
		}
		err = NewNamedBlock(	kCaptureListNamedBlock,
								sizeof(CaptureListSPtr),
								(Ptr *) &theCaptureListP);
		if (err != noErr) {
			goto exitLabel;
		}
		*theCaptureListP = nil;
		
		err = NewNamedBlock(	kJimiVersionNamedBlock,
								sizeof(UInt16),
								(Ptr *) &theJimiVersionP);
		if (err != noErr) {
			goto exitLabel;
		}
		*theJimiVersionP = 0;
	} else if (err != noErr) {
		goto exitLabel;
	} else {
		err = GetNamedBlock(	kJimiVersionNamedBlock,
								(Ptr *) &theJimiVersionP);
		if (err != noErr) {
			goto exitLabel;
		}
	}
	
	// Detach ourselves if our version number is greater than existing
	if (kJimiVersion > *theJimiVersionP) {
		*theJimiVersionP = kJimiVersion;
		// Detach ourselves
		::DetachResource(inJimiResH);
		
		// Get the current trap
		gPreviousCall = ::GetToolTrapAddress(_InitMenus);
		
		// Install the new trap
		::SetToolTrapAddress(	(UniversalProcPtr) MyInitMenus,
								_InitMenus);
							
		// Install shutdown proc
		::ShutDwnInstall(	MyShutDownProc,
							sdOnDrivers);
	}
	
	// Add this resource file to the capture list
	FSSpec			theSpec;
	FCBPBRec		pb;
	Handle			theCLst;
	OSType			creator;
	UInt16			numItems;
	char *			curPos;
	CaptureListSPtr	curCaptureListP;
	FSSpecListSPtr	curFSSpecListP;
	
	pb.ioCompletion = nil;
	pb.ioNamePtr = theSpec.name;
	pb.ioVRefNum = 0;
	pb.ioRefNum = ::CurResFile();
	pb.ioFCBIndx = 0;
	
	err = ::PBGetFCBInfoSync(&pb);
	if (err != noErr) {
		goto exitLabel;
	}
	theSpec.vRefNum = pb.ioFCBVRefNum;
	theSpec.parID = pb.ioFCBParID;
	
	// Get the STR# resource
	theCLst = Get1Resource(	'CLst',
							kPluginCLstID);
	if (theCLst == nil)
		goto exitLabel;
	HLock(theCLst);

	// Walk the CLst resource adding this FSSpec to the capture list
	curPos = *theCLst + sizeof(UInt16);
	numItems = *(UInt16 *) curPos;
	curPos += sizeof(numItems);
	curCaptureListP = *theCaptureListP;
	while (numItems > 0){
		creator = *(OSType *)curPos;
		curPos += sizeof(creator);

		// Walk the capture list to add the FSSpec
		while (curCaptureListP != nil){
			// If the creator already exists, just link a new FSSpecList
			//	struct to it
			if (curCaptureListP->creator == creator){
				curFSSpecListP = (FSSpecListSPtr)
									::NewPtrSys(sizeof(FSSpecListS));
				if (curFSSpecListP == nil) {
					goto exitLabel;
				}
				curFSSpecListP->fsSpec = theSpec;
				curFSSpecListP->minFreeSpace = *(UInt32 *)curPos;
				curPos += sizeof(UInt32);
				curFSSpecListP->link = curCaptureListP->fsSpecListP;
				curCaptureListP->fsSpecListP = curFSSpecListP;
				break;
			}
			curCaptureListP = curCaptureListP->link;
		}
		
		// Otherwise create a new CaptureList struct and link it in
		if (curCaptureListP == nil){
			curCaptureListP = (CaptureListSPtr)
								::NewPtrSys(sizeof(CaptureListS));
			if (curCaptureListP == nil)
				goto exitLabel;
			curCaptureListP->creator = creator;
			curCaptureListP->link = *theCaptureListP;
			*theCaptureListP = curCaptureListP;
			curFSSpecListP = (FSSpecListSPtr)
								::NewPtrSys(sizeof(FSSpecListS));
			if (curFSSpecListP == nil)
				goto exitLabel;
			curFSSpecListP->fsSpec = theSpec;
			curFSSpecListP->minFreeSpace = *(UInt32 *)curPos;
			curPos += sizeof(UInt32);
			curFSSpecListP->link = nil;
			curCaptureListP->fsSpecListP = curFSSpecListP;
		}
		
		numItems--;
	}
	
exitLabel:
	ExitCodeResource();
}



	pascal void
MyShutDownProc()
{
	EnterCallback();
	
	OSErr		err;
	UInt16 *	theJimiVersionP;
	
	err = GetNamedBlock(	kJimiVersionNamedBlock,
							(Ptr *) &theJimiVersionP);
	if (err == noErr) {
		if (*theJimiVersionP == kJimiVersion) {
			__destroy_global_chain();
		}
	}
	
	ExitCallback();
}



	pascal void
MyInitMenus(void)
{
	EnterCallback();

	{
		SInt32				err;
		UInt16 *			theJimiVersionP;
		
		err = GetNamedBlock(	kJimiVersionNamedBlock,
								(Ptr *) &theJimiVersionP);
		if (err != noErr) {
			goto errorLabel;
		}
		
		if (*theJimiVersionP == kJimiVersion) {
			StSaveCurResFile	saveCurRes;
			ProcessInfoRec		theProcessInfo;
			ProcessSerialNumber	theCurrentProcess = {0, kCurrentProcess};
			SInt32				gestaltReturn;
			Boolean				sameProcess = true;
			CaptureListSPtr		curCaptureListP;
			FSSpecListSPtr		curFSSpecListP;
			SInt32				plugResult;
			FSSpec				processFSSpec;
			Str31				processName;
			ProcessListSPtr		currProcessStructP;


			// Make sure we have process manager
			err = ::Gestalt(gestaltOSAttr, &gestaltReturn);

			if (err != noErr) {
				goto errorLabel;
			}


			// Get info for the current process
			memset(	&theProcessInfo,
					0,
					sizeof(theProcessInfo));
			theProcessInfo.processInfoLength = sizeof(theProcessInfo);
			theProcessInfo.processAppSpec = &processFSSpec;
			theProcessInfo.processName = processName;
			::GetProcessInformation(	&theCurrentProcess,
										&theProcessInfo);
			
			// Check to see if we are getting a duplicate call
			currProcessStructP = gProcessListP;
			while (currProcessStructP != nil) {	
				::SameProcess(	&currProcessStructP->psn,
								&theProcessInfo.processNumber, &sameProcess);
				if (sameProcess) {
					break;
				}
				currProcessStructP = currProcessStructP->link;
			}
			if (currProcessStructP != nil) {
				goto errorLabel;
			}
			

			// Everything worked, create a new process list struct for this
			currProcessStructP = (ProcessListSPtr)
									::NewPtrSys(sizeof(ProcessListS));
			if (currProcessStructP == nil) {
				goto errorLabel;
			}
			currProcessStructP->psn = theProcessInfo.processNumber;
			currProcessStructP->link = gProcessListP;
			gProcessListP = currProcessStructP;


			// Walk the list and see if we need to load a library
			CaptureListSPtr *		theCaptureListP;
			SInt16					plugID;

			err = GetNamedBlock(	kCaptureListNamedBlock,
									(Ptr *) &theCaptureListP);
			if (err != noErr) {
				goto errorLabel;
			}
			
			// Determine our machine type
			SInt32				theResponse;
			err = Gestalt(gestaltSysArchitecture, &theResponse);
			if (err != noErr) {
				goto errorLabel;
			}
			if (theResponse == gestalt68k) {
				plugID = k68kPlugResID;
			} else if (theResponse == gestaltPowerPC) {
				plugID = kPPCPlugResID;
			} else {
				goto errorLabel;
			}


			for (curCaptureListP = *theCaptureListP; curCaptureListP != nil;
			curCaptureListP = curCaptureListP->link){
				if ((curCaptureListP->creator
				== theProcessInfo.processSignature)
				|| (curCaptureListP->creator == kEveryProcess)){
					for (curFSSpecListP = curCaptureListP->fsSpecListP;
					curFSSpecListP != nil;
					curFSSpecListP = curFSSpecListP->link){
						// Make sure that we have enough memory
						SInt32	total;
						SInt32	contig;
						
						::PurgeSpace(&total, &contig);
						total += ::GetApplLimit() - ::LMGetHeapEnd() - 32;
						if ((theProcessInfo.processSignature == kFinderSig)
						|| (total >= curFSSpecListP->minFreeSpace)) {
							// Load and execute the plugin
							short	fileRef;
							Handle	mainAddrH;
							
							fileRef = ::FSpOpenResFile(	&curFSSpecListP->
															fsSpec,
														fsRdPerm);
							if (::ResError() == noErr) {
								mainAddrH = ::Get1Resource(	kPlugResType,
															plugID);
								if (mainAddrH != nil) {

									::DetachResource(mainAddrH);
								}

								::CloseResFile(fileRef);
								if (mainAddrH != nil) {
									plugResult = (*((PlugInMain) *mainAddrH))(
													&theProcessInfo,
													&curFSSpecListP->fsSpec);
									if (plugResult & kUnloadPlug) {
										// This also disposes of the fake
										// plugin for CFM68k code
										::DisposeHandle(mainAddrH);
									}
								}
							}
						}
					}
				}
			}
		}

errorLabel:

		// Call through to original function
		((InitMenusProc) gPreviousCall)();

	}
	
	ExitCallback();
}




/////////// Implement Shared Memory  ////////////////////////////////////////

NamedBlockP		gNamedBlocks = nil;

	pascal OSErr
SharedMemSelector(
	OSType selector,
	long *response)
{
	EnterCallback();
	
	*response = (long) &gNamedBlocks;
	
	ExitCallback();

	return noErr;
}