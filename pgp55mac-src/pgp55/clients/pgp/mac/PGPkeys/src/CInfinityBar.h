/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CInfinityBar.h,v 1.1 1997/08/05 12:04:01 wprice Exp $
____________________________________________________________________________*/
#pragma once

class	CInfinityBar: public LView
{
public:
	enum	{	class_ID	= 'Gkth'	};

					CInfinityBar( LStream	*inStream );
	
	virtual	void	DrawSelf( void );
	void			IncrementState(	void 	);
private:
	UInt32		mState;

	enum	{
				kMaxState	= 16,
				kIconWidth	= 16,
				kIconHeight	= 12,
				kIconID		= 4000
			};
};

