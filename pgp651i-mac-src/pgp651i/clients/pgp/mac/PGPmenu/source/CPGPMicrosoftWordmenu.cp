/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: CPGPMicrosoftWordmenu.cp,v 1.2 1999/04/19 09:24:39 jason Exp $____________________________________________________________________________*/#include <MacWindows.h>#include "CPGPMicrosoftWordmenu.h"CPGPMicrosoftWordmenu::CPGPMicrosoftWordmenu(	FSSpec *			inFSSpec,	SPGPmenuMESPData *	inMESPData)		:	CPGPAppmenu(inFSSpec, inMESPData), mPatchGetResource(true),			mMungeDialogs(false){}CPGPMicrosoftWordmenu::~CPGPMicrosoftWordmenu(){}	voidCPGPMicrosoftWordmenu::Patch(){	if (mPatchGetResource) {		new CPGPMicrosoftWordmenuGetResourcePatch;	}	CPGPAppmenu::Patch();}	voidCPGPMicrosoftWordmenu::HandleKeyDown(){	StMungeDialogs	munger(this);		CPGPAppmenu::HandleKeyDown();}	HandleCPGPMicrosoftWordmenu::CPGPMicrosoftWordmenuGetResourcePatch::NewGetResource(	ResType	theType,	short	theID){	Handle					result = OldGetResource(theType, theID);	try {		CPGPMicrosoftWordmenu *	pgpmenu = dynamic_cast<CPGPMicrosoftWordmenu *>(											CPGPmenu::Getmenu());				if ((result != nil) && pgpmenu->mMungeDialogs) {			UInt16 *	thePosition = nil;						switch (theType) {				case 'ALRT':				{					thePosition = (UInt16 *) ((*result) + 12);				}				break;												case 'DLOG':				{					Ptr	tempPosition = (*result) + 20;					thePosition = (UInt16 *) (tempPosition + *tempPosition + 1												+ ((*tempPosition + 1) % 2));				}				break;			}			if (thePosition != nil) {				switch (*thePosition) {					case kWindowCenterParentWindow:					case kWindowCenterParentWindowScreen:					{						*thePosition = kWindowCenterMainScreen;					}					break;															case kWindowAlertPositionParentWindow:					case kWindowAlertPositionParentWindowScreen:					{						*thePosition = kWindowAlertPositionMainScreen;					}					break;					case kWindowStaggerParentWindow:					case kWindowStaggerParentWindowScreen:					{						*thePosition = kWindowStaggerMainScreen;					}					break;				}			}			}	}		catch (...) {	}		return result;}