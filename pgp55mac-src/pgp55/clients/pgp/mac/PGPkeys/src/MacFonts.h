/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacFonts.h,v 1.1 1997/07/03 12:22:44 wprice Exp $
____________________________________________________________________________*/

class USystemFontUtils
{
	public:
		static void				FetchSystemFont(	Int16 &outFont,
													Int16 &outSize);
		static void				ResetSystemFont();
		static void				SetSystemFont(		Int16 inFont,
													Int16 inSize);
};

class StSystemFontState
{
	public:
		StSystemFontState(Int16 inNewFont, Int16 inNewSize);
		~StSystemFontState();

	protected:
		Int16					mSaveFont;
		Int16					mSaveSize;
};

