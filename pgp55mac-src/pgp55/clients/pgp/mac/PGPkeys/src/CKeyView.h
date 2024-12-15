/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyView.h,v 1.6 1997/08/31 12:52:21 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"

class CKeyTable;
class CKeyTableLabels;
class CGAScroller;

const ResIDT	kKeyViewResID		=	132;

class CKeyView	:	public LView
{
public:
	enum {	class_ID = 'keyV'	};
	

					CKeyView();
					CKeyView(LStream	*inStream);
					~CKeyView();
					
	virtual void	FinishCreateSelf();
	virtual void	DrawSelf();
	
	virtual void	CloseKeys();
	virtual void	SetKeyDBInfo(	PGPKeySetRef	keySet,
									Boolean			writable,
									Boolean			defaultRings
								);
	PGPKeySetRef	GetKeySet();
	CKeyTable		*GetKeyTable()	{	return mKeyTable;	}
	virtual void	Empty();
	void			ImportKeysFromHandle(Handle data);
	
protected:
	enum	{
				kStringListID 	= 1000,
				kUserIDStringID		= 1,
				kValidityStringID,
				kTrustStringID,
				kCreationStringID,
				kSizeStringID
			};

private:
	Boolean						mIsDefaultKeyrings;
	CKeyTable					*mKeyTable;
	CKeyTableLabels				*mKeyTableLabels;
	CGAScroller					*mKeyScroller;
	
	enum	{	
				kKeyTableLabels			= 'lKey',
				kKeyTable				= 'tKey',
				kKeyScroller			= 'sKey'
			};
};

