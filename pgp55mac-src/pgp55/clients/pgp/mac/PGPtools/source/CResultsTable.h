/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <LHierarchyTable.h>

#include "PGPtools.h"
#include "PGPSharedEncryptDecrypt.h"

struct ResultsSingleItem;
struct ResultsSummaryItem;

class CResultsTable : public LHierarchyTable
{
public:
	enum 				{ class_ID = 'ResT' };

						CResultsTable(LStream *inStream);
	virtual				~CResultsTable(void);
	
	virtual void		AdaptToSuperFrameSize(Int32 inSurrWidthDelta,
									Int32 inSurrHeightDelta,
									Boolean inRefresh);
	TableIndexT			AddSummaryRow(void);
	void				AddProcessedItemRow(ConstStr255Param itemName,
									PGPtoolsSignatureData *signatureData);
	virtual void		DrawSelf(void);
	void				IncrementVerifiedCount(void);
		
protected:

		
	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);
	virtual void		FinishCreateSelf(void);
	
private:

	FontInfo			mSummaryItemFontInfo;
	FontInfo			mSingleItemFontInfo;
	PGPBoolean			mShowMarginalValidity;
	PGPBoolean			mMarginalIsInvalid;
	
	void				DrawSingleItemCell(const ResultsSingleItem *data,
									const Rect *cellRect);
	void				DrawSummaryCell(const ResultsSummaryItem *data,
									const Rect *cellRect);
	void				DrawValidity(PGPValidity validity,
									const Rect *columnRect);
};