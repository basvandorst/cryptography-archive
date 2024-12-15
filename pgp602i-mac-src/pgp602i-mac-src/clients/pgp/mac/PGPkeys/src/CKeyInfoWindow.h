/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: CKeyInfoWindow.h,v 1.15.8.1 1998/11/12 03:08:00 heller Exp $____________________________________________________________________________*/#pragma once#include <LListener.h>#include <LWindow.h>#include <LStaticText.h>#include "pgpKeys.h"class CKeyTable;class LMultiPanelView;const short	kMaxKeyInfoWindows	=	16;	// Prevent memory problemsclass CKeyInfoWindow : public LWindow{public:	enum { class_ID = 'KInf' };								CKeyInfoWindow(LStream *inStream);								~CKeyInfoWindow();	void						FinishCreateSelf();	virtual void				ApplyForeAndBackColors(void) const;	virtual void				AttemptClose(void);	virtual void				DoClose(void);										static Boolean				OpenKeyInfo(PGPKey *key, CKeyTable *table);	static void					CloseAll();		static	void				GetFingerprintString(									Str255 pstr, PGPKeyRef key);		PGPKeyRef					GetKey();	private:	enum	{		kMultiPanelViewPaneID 	= 'pMPV',		kTabsControlPaneID		= 'pTab'	};	LMultiPanelView				*mMultiView;	PGPKeyRef					mWindowKey;	CKeyTable					*mParentTable;	Boolean						HasChanged(void);	void						CleanupPanel(short panelIndex);	void						ProcessClose(void);	void						SetInfo(CKeyTable *table, PGPKeyRef theKey);		static CKeyInfoWindow		*sKeyInfoWindows[kMaxKeyInfoWindows];	static short				sNumKeyInfoWindows;		};