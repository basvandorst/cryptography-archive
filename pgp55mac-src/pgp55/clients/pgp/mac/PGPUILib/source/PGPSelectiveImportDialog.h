/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPSelectiveImportDialog.h,v 1.7.8.1 1997/12/05 22:13:35 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <LTableView.h>
#include <LBroadcaster.h>
#include "CModalDialogGrafPortView.h"
#include "pgpEncode.h"

class CSelectiveImportTable;

class CSelectiveImportDialog : public CModalDialogGrafPortView
{
public:

	enum				{ class_ID = 'ImpD' };
	
						CSelectiveImportDialog(LStream * inStream);
	virtual				~CSelectiveImportDialog();
	
	void				Init(PGPContextRef inContext, const char * inPrompt,
								PGPKeySetRef inFromSet,
								PGPKeySetRef inValiditySet,
								PGPKeySetRef * outSelectedSet);
	
	virtual Boolean		HandleKeyPress(const EventRecord & inKeyEvent);
	virtual void		ListenToMessage(MessageT inMessage, void * ioParam);
		
protected:

	virtual void		FinishCreateSelf();
	
private:

	CSelectiveImportTable	*mUserIDTable;
	
	void				AdjustButtons(void);
};


class CSelectiveImportTable :	public LTableView,
								public LBroadcaster
{
public:
	enum 				{ class_ID = 'ImpT' };

						CSelectiveImportTable(LStream * inStream);
	virtual				~CSelectiveImportTable();
	
	void				SetTableInfo(PGPContextRef inContext,
								PGPKeySetRef inFromSet,
								PGPKeySetRef inValiditySet,
								PGPKeySetRef * outSelectedSet);
								
	void				ImportKeys();

protected:

	PGPKeySetRef		mFromSet;
	PGPKeySetRef *		mSelectedSet;
	PGPKeyIterRef		mKeyIter;
	PixPatHandle		mBarberPixPat;
	PGPBoolean			mShowMarginalValidity;
	PGPBoolean			mMarginalIsInvalid;
	
	virtual void		HiliteCellActively(const STableCell & inCell,
								Boolean inHilite);
	virtual void		HiliteCellInactively(const STableCell & inCell,
								Boolean inHilite);
	
	virtual void		ClickSelf(const SMouseDownEvent & inMouseDown);
	
	virtual void		DrawSelf();
	virtual void		DrawCell(const STableCell & inCell,
								const Rect & inLocalRect);

	static PGPError		EventHandler(PGPContextRef context,
								PGPEvent * event, PGPUserValue userValue);	
};
