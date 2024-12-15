/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDialogs.h,v 1.5 1997/09/18 01:34:42 lloyd Exp $
____________________________________________________________________________*/
#pragma once

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

Handle			GetIndDialogItem(DialogRef dlg, short item);

void			SetUserItemProc(DialogRef dlg, short item, UserItemUPP theProc);

	
void			GetDialogItemRect(DialogRef dlg, short item, Rect *itemRect);

void			SetDialogItemRect(DialogRef dlg, short item,
					const Rect *itemRect);

void			MoveDialogItem(DialogRef dlg, short item, short h, short v);

void			OffsetDialogItem(DialogRef dlg, short item,
					short hOffset, short vOffset);

void			InvalDialogItem(DialogRef dlg, short item, short inset);

void			EraseDialogItem(DialogRef dlg, short item, short inset);

void			ShowHideDialogItem(DialogRef dlg, short item, Boolean show);

short			GetDialogItemType(DialogRef dlg, short item);


ControlHandle	GetDialogControlItem(DialogRef dlg, short item);

short			GetDialogControlValue(DialogRef dlg, short item);

void			SetDialogControlValue(DialogRef dlg, short item, short value);

Boolean			ToggleDialogCheckbox(DialogRef dlg, short item);




void		GetIndDialogItemText(DialogRef dlg, short item, uchar *itemText);

void		SetIndDialogItemText(DialogRef dlg, short item,
				const uchar *itemText);




#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS