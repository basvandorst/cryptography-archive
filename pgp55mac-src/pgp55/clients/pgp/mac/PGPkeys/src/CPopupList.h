/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPopupList.h,v 1.3 1997/08/09 10:02:22 wprice Exp $
____________________________________________________________________________*/

const short kMaxPopupListItems		=	32;

typedef struct PopupListItem
{
	Str255			item;
} PopupListItem;

class CPopupList :	public LControl
{
public:
	enum { class_ID = 'popL' };	
							CPopupList(LStream *inStream);
							~CPopupList();
							
	void					DrawSelf();
	void					ClickSelf(const SMouseDownEvent &inMouseDown);
	
	Int32					GetValue() const;
	void					SetValue(Int32 inValue);
	
	void					AddItem(ConstStringPtr name);
	void					GetItem(Int16 item, Str255 name);
	void					ClearItems();
	virtual	void			EnableSelf();
	virtual	void			DisableSelf();
	virtual void			ActivateSelf();
	virtual void			DeactivateSelf();
private:
	void					DrawSelf1(Int16 state);
	void					SetGrayFore(Int16 index);
	void					SetGrayBack(Int16 index);
	
	void					DrawItem(	Rect		trect,
										Int16		vertical,
										uchar		*name,
										Boolean		checked,
										Int16		ascent);
	
	Rect					mListRect;		//coords of popuplist
	Int16					mMaxWidth;
	ResIDT					mTextTraitsID;
	Int16					mNumItems;
	PopupListItem			*mItems;
};

