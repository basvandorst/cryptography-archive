/*
	Developer VISE 3.0
		Installer
			ExternCodeDefines.h
	
	This file contains the constants and structures for External Code calling.
	
	
	Copyright (C) 1994 MindVision Software. All Rights Reserved Worldwide.
*/

#ifndef __EXTERNCODEDEFINES__
#define __EXTERNCODEDEFINES__

#ifndef __INSTALLERDEFINES__
#include "InstallerDefines.h"
#endif

#ifndef __LISTSTRUCTS__
#include "ListStructs.h"
#endif

#if defined(powerc)
#pragma options align=mac68k
#endif


// VERSION
#define kExternPBVersion	4		// Version Number of External Code ParamBlock

// SELECTORS (ExternParmBlock.selector)
#define kSelectorLaunchTime		0	// External Code being called at launch time (install dialog is valid, but invisible 
#define kSelectorEvent			1	// External Code being called during main event loop, ExternParmBlock.itemHit will be filled in from DialogSelect
#define kSelectorBeforeInstall	2	// External Code being called at the beginning of the install routine
#define kSelectorAfterInstall	3	// External Code being called at the end of install routine
#define kSelectorBeforeFile		4	// External Code being called Before a file is to be installed
#define kSelectorAfterFile		5	// External Code being called After a file was installed
#define kSelectorCheckingSizes	6	// External Code being called to calculate sizes - see ExternParamBlock.flags for if on Custom or Easy Install
#define kSelectorDynamicTime	7	// External Code being called to set the localization language
#define kSelectorPackChanged	8	// External Code being called when package checkbox changes state


// FLAGS - ExternParmBlock.flags
#define kCancelInstall		(1L << 0)	// Cancel Install bit ; if (gPBExtern.flags & kCancelInstall) then quit install;
#define kWhichInstall		(1L << 1)	// 0 = Easy 1 = Custom - what the current install is (easy or custom)
#define kCreateMovedFolder	(1L << 2)	// Set this flag to tell the Installer to create the Moved Files Folder in the System Folder
#define kRemoveInstall		(1L << 4)	// If this flag is set, the user has selected Remove


// VALUES FOR INSTALL FORK
#define kDontInstallFile	0	// Don't Install File
#define kInstallDataFork	1	// install Data Fork Only
#define kInstallResFork		2	// install Resource Fork Only
#define kInstallFile		3	// install both

// EASY / CUSTOM
#define kGotoEasyInstall	1	// set ExternParmBlock.easyCustom to Easy Install (LaunchTime only)
#define kGotoCustomInstall	2	// set ExternParmBlock.easyCustom to Custom Install (LaunchTime only)


struct ExternParmBlock {
	short				version;			// current version of this ParmBlock
	short				selector;			// where in the installer your external code is being called from - see SELECTORS above
	long				refCon;				// user defined number (number assigned in Developer VISE External Codes dialog
	long				flags;				// bit flags - see FLAGS above
	Handle				userStorage;		// user storage handle - initialized to nil at start of Installer
	short				easyCustom;			// only set & changeable at when called with kSelectorLaunchTime
	short				numberOfPackages;	// number of packages
	IPackageArrayHdl	packages;			// package handle (see InstallerDefines.h)
	Handle				easyInstallText;	// handle to easy install text (displayed on the install window (easy install only))
	ArcFileType			theFile;			// information about the file to be installed (*WILL BE NIL* - if selector not kSelectorBeforeFile and AfterFile)
	EventRecord			theEvent;			// event record - used during event loop call
	DialogPtr			theDialog;			// Install Dialog
	short				itemHit;			// itemHit - Event loop processing
	short				installFork;		// which fork(s) of file to install (see VALUES FOR INSTALL FORK). Initialized to kInstallFile
	ArcListHdl			fileList;			// Handle to a list of files that are in the archive
	long				numberOfFiles;		// number of files in the ArcListHdl
	short				movedItemsVRefNum;	// vRefNum of the moved items folder 0 = not created
	long				movedItemsDirID;	// dirID of the moved items folder
	short				logRefNum;			// refnum of the Installer log file 0 = not open
	Handle				removeText;			// Handle to text to display if you include an Uninstall
};
typedef struct ExternParmBlock ExternParmBlock, *ExternParmBlockPtr;

typedef void (*ProcCallPtr)(ExternParmBlock *eInfo);


// ITEMS FOR INSTALL LOCATIONS INTERFACE
#define kInstallLocExternPBVersion		4	// external code is responsible for checking this value against ILE_Version in the ParamBlock below

// Type and Id of Resources Needed by the Install Location
typedef struct ResList {
	ResType		RL_Type;					// Resource Type
	short		RL_ID;						// Resource ID
} ResList;
typedef struct ResList	*ResListPtr;


// PARAMBLOCK FOR INSTALL LOCATIONS EXTERNAL CODE INTERFACE
typedef struct InstallLocExternPB {
	short				ILE_Version;		// current version of this ParamBlock
	ResType				ILE_FindFldrCode;	// find folder code assigned to the install location that is calling this external code - isn't necessarily used to call FindFolder
	Boolean				ILE_DoCreate;		// whether or not to create the folder if it isn't found or to just let it error out
	long				ILE_RefCon;			// currently unused
	long				ILE_Flags;			// bit flags - see FLAGS above - currently only the kCancelInstall bit is checked after returning
	long				ILE_SystemVersion;	// version of the system currently running on - obtained from call to Gestalt
	Handle				ILE_UserStorage;	// currently unused
	short				ILE_SuppResCount;	// number of supporting resources that this install location uses
	ResListPtr			ILE_SuppResListPtr;	// array of ResList items - 1 element for each supporting resource
	OSErr				ILE_ResultError;	// error that is returned from the call to the external code - i.e. fnfErr, nsvErr, etc.
	short				ILE_ResultVRefNum;	// vRefNum of the found folder - initially the vRefNum of the selected volume
	long				ILE_ResultDirID;	// dirID of the found folder - initially the current dirID
} InstallLocExternPB;
typedef struct InstallLocExternPB	*InstallLocExternPBPtr;

typedef void (*InstallLocProcCallPtr)(InstallLocExternPBPtr ILInfo);


#if defined(powerc)
#pragma options align=reset
#endif


#endif

