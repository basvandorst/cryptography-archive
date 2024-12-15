/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: CShareholderList.h,v 1.1.8.1 1998/11/12 03:10:35 heller Exp $____________________________________________________________________________*/#pragma once#include <LTableView.h>#include <LBroadcaster.h>#include "pgpPubTypes.h"const MessageT	kSLSelectionChangedMessageID	= 'cdSC';const PGPUInt16	kMaxShareholderUserIDLength		= 255;typedef struct ShareholderListEntry{	PGPBoolean				hasKey;	PGPKeyID				keyID;	PGPPublicKeyAlgorithm	algorithm;	char					name[kMaxShareholderUserIDLength + 1];	char					*phrase;	PGPUInt32				numShares;} ShareholderListEntry;class CShareholderList	:	public LTableView,							public LBroadcaster{public:	enum { class_ID = 'shaL' };								CShareholderList(LStream *inStream);	void					DrawSelf();	void					DrawCell(								const STableCell	&inCell,								const Rect			&inLocalRect);	void					ClickSelf(								const SMouseDownEvent &inMouseDown);	void					DeactivateSelf();	void					ActivateSelf();	void					SelectionChanged();};