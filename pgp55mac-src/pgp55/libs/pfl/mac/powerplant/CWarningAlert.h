/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CWarningAlert.h,v 1.3 1997/08/18 20:50:25 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#include <LGADialogBox.h>
#include <PP_Messages.h>

const ResIDT	kWarningAlertDialogResID = 31500;

#include "WarningAlertTypes.h"


class CWarningAlert : public LGADialogBox
{
public:

	enum { class_ID = 'Warn' };

					CWarningAlert();
	virtual			~CWarningAlert();
					CWarningAlert(LStream *inStream);

	void			HideCancelButton(void);
	void			SetIcon(ResIDT iconResID);
	void			SetMessage(ConstStr255Param message);
	void			SetButtonTitle(PaneIDT buttonPaneID,
						ConstStr255Param descriptor);

	Boolean			SetupDialog(
						WarningAlertType	alertType,
						WarningAlertStyle	alertStyle,
						ConstStr255Param	message,
						ConstStr255Param	str1 = NULL,
						ConstStr255Param	str2 = NULL,
						ConstStr255Param	str3 = NULL);
				

	static MessageT	Display(WarningAlertType alertType,
						WarningAlertStyle alertStyle, 
						ConstStr255Param message,
						ConstStr255Param str1 = NULL,
						ConstStr255Param str2 = NULL,
						ConstStr255Param str3 = NULL);
					
	static MessageT	Display(WarningAlertType alertType,
						WarningAlertStyle alertStyle, 
						ResIDT stringListResID,
						short stringListIndex,
						ConstStr255Param str1 = NULL,
						ConstStr255Param str2 = NULL,
						ConstStr255Param str3 = NULL);
					
	static MessageT	DisplayCustom(
						short dialogResID,
						WarningAlertType alertType,
						WarningAlertStyle alertStyle,
						ConstStr255Param message,
						ConstStr255Param str1 = NULL,
						ConstStr255Param str2 = NULL,
						ConstStr255Param str3 = NULL);
};














