/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CmenuGetResourcePatch.cp,v 1.1.2.1.2.1 1998/11/12 03:08:37 heller Exp $____________________________________________________________________________*/#include "CmenuGetResourcePatch.h"CmenuGetResourcePatch::CmenuGetResourcePatch(	SGlobals &	inGlobals)		: mGlobals(inGlobals){}CmenuGetResourcePatch::~CmenuGetResourcePatch(){}	HandleCmenuGetResourcePatch::NewGetResource(	ResType	theType,	short	theID){	Handle	result = OldGetResource(theType, theID);		if ((result != nil) && mGlobals.mungeDialogs) {		UInt16 *	thePosition = nil;				switch (theType) {			case 'ALRT':			{				thePosition = (UInt16 *) ((*result) + 12);			}			break;									case 'DLOG':			{				Ptr	tempPosition = (*result) + 20;				thePosition = (UInt16 *) (tempPosition + *tempPosition + 1											+ ((*tempPosition + 1) % 2));			}			break;		}		if (thePosition != nil) {			switch (*thePosition) {				case kWindowCenterParentWindow:				case kWindowCenterParentWindowScreen:				{					*thePosition = kWindowCenterMainScreen;				}				break;												case kWindowAlertPositionParentWindow:				case kWindowAlertPositionParentWindowScreen:				{					*thePosition = kWindowAlertPositionMainScreen;				}				break;				case kWindowStaggerParentWindow:				case kWindowStaggerParentWindowScreen:				{					*thePosition = kWindowStaggerMainScreen;				}				break;			}		}		}		return result;}