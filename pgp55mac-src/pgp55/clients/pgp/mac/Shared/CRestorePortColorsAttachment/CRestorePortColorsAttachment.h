/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CRestorePortColorsAttachment.h,v 1.3.8.1 1997/12/05 22:15:04 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <LAttachment.h>

class	CRestorePortColorsAttachment : public LAttachment {
public:
	enum { class_ID = 'RPCA' };
	
					CRestorePortColorsAttachment(
							Boolean inForeColor = true,
							Boolean inBackColor = true,
							Boolean		inExecuteHost = true);
					CRestorePortColorsAttachment(LStream * inStream);
					
protected:
	Boolean			mForeColor;
	Boolean			mBackColor;
	
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};
