/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CServerStatusDialog.h,v 1.2.10.1 1997/12/05 22:13:34 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <LPeriodical.h>
#include "CPGPLibGrafPortView.h"


class CServerStatusDialog : public CPGPLibGrafPortView,
							public LPeriodical {
public:
	enum				{ class_ID = 'StsD' };
	
						CServerStatusDialog(LStream * inStream);
	virtual				~CServerStatusDialog();
	virtual void		FinishCreateSelf();
	
	virtual void		SpendTime(const EventRecord & inMacEvent);

	void				SetServerCaption(ConstStringPtr inServer);
	void				SetStatusCaption(ConstStringPtr inStatus);
	
protected:
};

