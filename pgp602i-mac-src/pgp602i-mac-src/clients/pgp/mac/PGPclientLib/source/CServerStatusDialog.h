/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CServerStatusDialog.h,v 1.4.8.1 1998/11/12 03:07:17 heller Exp $____________________________________________________________________________*/#pragma once #include <LPeriodical.h>#include "CPGPModalGrafPortView.h"class CClientKSGrafPortView : public CPGPModalGrafPortView{public:	enum					{ class_ID = 'KGRP' };							CClientKSGrafPortView(LStream *inStream);	virtual					~CClientKSGrafPortView();	virtual void			DoIdle(const EventRecord &inMacEvent);};class CServerStatusDialog : public CClientKSGrafPortView{public:	enum				{ class_ID = 'StsD' };							CServerStatusDialog(LStream * inStream);	virtual				~CServerStatusDialog();		void				SetServerCaption(ConstStringPtr inServer);	void				SetStatusCaption(ConstStringPtr inStatus);	protected:};