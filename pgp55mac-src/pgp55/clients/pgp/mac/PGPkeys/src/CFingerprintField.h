/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CFingerprintField.h,v 1.2 1997/09/17 19:36:35 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "CRecessedCaption.h"

class CFingerprintField :	public CRecessedCaption,
							public LCommander
{
private:
	typedef CRecessedCaption Inherited;
public:
	enum { class_ID = 'Fing' };
	static CFingerprintField * CreateFromStream(LStream * inStream);

						CFingerprintField();
						CFingerprintField(
							const CFingerprintField & inOriginal );
						CFingerprintField(	LStream * inStream );

	virtual Boolean		ObeyCommand(
								CommandT			inCommand,
								void *				ioParam);
	virtual void		FindCommandStatus(
								CommandT			inCommand,
								Boolean &			outEnabled,
								Boolean &			outUsesMark,
								Char16 &			outMark,
								Str255				outName);
	virtual void		DrawSelf();
	virtual void		FinishCreateSelf();
	virtual void		ClickSelf(
							const SMouseDownEvent & inMouseDown);
	virtual void		AdjustCursorSelf(
							Point				inPortPt,
							const EventRecord &	inMacEvent);
	virtual void		BeTarget();
	virtual void		DontBeTarget();
	
protected:
	virtual void		DrawBackground(	const Rect & localFrame);
	virtual void		DrawBox(		const Rect & localFrame);
	virtual void		DrawText(		const Rect & localFrame);
};

