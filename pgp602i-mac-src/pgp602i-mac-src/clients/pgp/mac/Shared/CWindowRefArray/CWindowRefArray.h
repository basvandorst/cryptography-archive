/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CWindowRefArray.h,v 1.4.8.1 1998/11/12 03:10:40 heller Exp $____________________________________________________________________________*/#pragma once #include <LArray.h>#include <LComparator.h>class LGrafPortView;typedef struct WindowRefS {	WindowRef		windowRef;	LGrafPortView *	grafPortView;} SWindowRef, * SWindowRefPtr;typedef const SWindowRef *	ConstSWindowRefPtr;class CWindowRefArray	:	public LArray {public:						CWindowRefArray();	virtual				~CWindowRefArray();		virtual ArrayIndexT	InsertSWindowRef(const SWindowRef & inItem)							{ return InsertItemsAt(1,1, &inItem,								sizeof(SWindowRef)); }								virtual ArrayIndexT	FetchIndexOfWindowRef(const WindowRef inWindowRef);	virtual ArrayIndexT	FetchIndexOfGrafPortView(							const LGrafPortView * inGrafPortView);		virtual Boolean		FetchSWindowRefAt(ArrayIndexT inAtIndex,							SWindowRef & outItem) const							{ 	return FetchItemAt(inAtIndex, &outItem); }		virtual void		RemoveSWindowRefAt(ArrayIndexT inAtIndex)							{ RemoveItemsAt(1, inAtIndex); }	protected:	class CWindowRefComparator	:	public LComparator {	public:						CWindowRefComparator(){ }		virtual			~CWindowRefComparator(){ }								virtual Int32	Compare(const void * inItemOne,							const void * inItemTwo,							UInt32 inSizeOne, UInt32 inSizeTwo) const							{ (void) inSizeOne; (void) inSizeTwo;							  return ! (static_cast<ConstSWindowRefPtr>							  (inItemOne)->windowRef ==							  static_cast<ConstSWindowRefPtr>							  (inItemTwo)->windowRef); }	};	class CGrafPortViewComparator	:	public LComparator {	public:						CGrafPortViewComparator(){ }		virtual			~CGrafPortViewComparator(){ }								virtual Int32	Compare(const void * inItemOne,							const void * inItemTwo,							UInt32 inSizeOne, UInt32 inSizeTwo) const							{ (void) inSizeOne; (void) inSizeTwo;							  return ! (static_cast<ConstSWindowRefPtr>							  (inItemOne)->grafPortView ==							  static_cast<ConstSWindowRefPtr>							  (inItemTwo)->grafPortView); }	};};