/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPGPPopupMenuOptionView.h,v 1.2.12.1 1998/11/12 03:20:41 heller Exp $____________________________________________________________________________*/#pragma once#include "CPGPOptionView.h"class LPopupButton;struct PGPOUIPopupListDesc;class CPGPPopupMenuOptionView : public CPGPOptionView{public:	enum { class_ID = 'POpt' };						CPGPPopupMenuOptionView(LStream *inStream);	virtual				~CPGPPopupMenuOptionView();	virtual	void		ListenToMessage(MessageT inMessage, void *ioParam);	static CPGPPopupMenuOptionView *CreatePopupMenuOptionView(										LView *inSuperView,										const PGPOUIPopupListDesc *info );protected:	virtual void		FinishCreateSelf();	private:	LPopupButton		*mPopupMenu;	PGPUInt32			*mResultPtr;		void				SetInfo(const PGPOUIPopupListDesc *info);};