/*____________________________________________________________________________	Copyright (C) 1997-1998 Network Associates, Inc. and its affiliates.	All rights reserved.	$Id: CPGPStDialogHandler.h,v 1.2.16.1 1998/11/12 03:18:59 heller Exp $____________________________________________________________________________*/#include <UModalDialogs.h>class CPGPStDialogHandler : public StDialogHandler{public:						CPGPStDialogHandler(ResIDT inDialogResID, 									LCommander *inSuper);	virtual				~CPGPStDialogHandler(void); 	virtual void		FindCommandStatus(CommandT inCommand,							Boolean &outEnabled, Boolean &outUsesMark,							Char16 &outMark, Str255 outName);};