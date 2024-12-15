/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: plugin.h,v 1.2.10.1 1997/12/05 22:15:20 mhw Exp $
____________________________________________________________________________*/

#pragma once 


// Main entry point declaration
#ifdef __cplusplus
extern "C" {
#endif
typedef SInt32 (*PlugInMain)(ProcessInfoRec *, FSSpec *);

#ifdef __cplusplus
}
#endif

enum {
	uppPlugInMainProcInfo = kThinkCStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(SInt32)))
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ProcessInfoRec *)))
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(FSSpec *)))
};

#if GENERATINGCFM
typedef UniversalProcPtr PlugInMainProcUPP;
#define CallPlugInMainProc(userRoutine, processInfoPtr, fsSpecPtr)	\
(SInt32) CallUniversalProc(	(UniversalProcPtr)(userRoutine),		\
							uppPlugInMainProcInfo,					\
							(processInfoPtr),						\
							(fsSpecPtr))							
#else
typedef PlugInMain PlugInMainProcUPP;
#define CallPlugInMainProc(userRoutine, processInfoPtr, fsSpecPtr)\
(SInt32)(*(userRoutine))((processInfoPtr), (fsSpecPtr))
#endif

// Initialization errors
enum {
	kNoErr			=	0x00000000,
	kUnloadPlug		=	0x00010000,	// Any error returned with bit 16 set
									// will unload the plugin.
	kUnknownErr		=	0x00010001,
	kPlugExpired	=	0x00010002,
	kOutOfMemoryErr	=	0x00010003
};

// Capture IDs list
enum {
	kPluginCLstID	=	1000	// The CLst that lists all creators this
								// plugin needs captured.
};

// For plugs that want their main called for every process
const OSType	kEveryProcess = '****';

// Plugin resource location for 68k machines
const ResType	kPlugResType	=	'MESP';
const SInt16	kPPCPlugResID	=	1000;
const SInt16	k68kPlugResID	=	1001;

// ICN# IDs for the icon to show at startup
const SInt16	kRunningIcon			=	1000;
const SInt16	kNotRunningIcon			=	1001;
	
