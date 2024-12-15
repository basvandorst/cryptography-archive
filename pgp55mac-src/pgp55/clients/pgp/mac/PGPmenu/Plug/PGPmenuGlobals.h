/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPmenuGlobals.h,v 1.7.10.1 1997/12/05 22:14:33 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "pgpUtilities.h"

typedef struct GlobalsS {
#if BETA || PGP_DEMO
	Boolean				disableEncryptAndSign;
#endif
#if ! PGP_FINDERMENU
	PGPContextRef		pgpContext;
	Boolean				privateScrap;
	Boolean				useOutputDialog;
#endif
	Str32				processName;
	ProcessInfoRec		processProcInfo;
	FSSpec				processSpec;
	FSSpec				plugSpec;
	MenuHandle			menuH;
	Handle				menuIconH;
	Boolean				insertMenu;
} SGlobals, * SGlobalsPtr;

const OSType		kFinderCreator						=	'MACS';

// Constants
const SInt16		MENU_PGPmenu						=	17359;
const SInt16		ICNx_PGPmenu						=	17359;

const ResIDT		STRx_Strings						=	10000;
const SInt16		kKeysNotFoundID						=	1;
const SInt16		kSignID								=	2;
const SInt16		kEncryptID							=	3;
const SInt16		kEncryptAndSignID					=	4;
const SInt16		kDecryptVerifyID					=	5;
const SInt16		kSignAsID							=	6;
const SInt16		kEncryptAsID						=	7;
const SInt16		kEncryptAndSignAsID					=	8;
const SInt16		kDecryptVerifyAsID					=	9;
const SInt16		kSignatureBadID						=	10;
const SInt16		kSignatureNotVerifiedID				=	11;
const SInt16		kBeginPGPSignedSectionID			=	12;
const SInt16		kEndPGPSignedSectionID				=	13;
const SInt16		kAddKeysID							=	14;

const ResIDT		STRx_Errors							=	10001;
const SInt16		kErrorStringID						=	1;



// Menu items
const SInt16	kEncrypt					=	1;
const SInt16	kSign						=	2;
const SInt16	kEncryptAndSign				=	3;
const SInt16	kDecryptVerify				=	4;
const SInt16	kWipe						=	5;
const SInt16	kPGPkeys					=	7;

