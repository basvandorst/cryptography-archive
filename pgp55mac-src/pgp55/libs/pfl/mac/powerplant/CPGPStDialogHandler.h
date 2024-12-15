/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#include <UModalDialogs.h>

class CPGPStDialogHandler : public StDialogHandler
{
public:
						CPGPStDialogHandler(ResIDT inDialogResID, 
									LCommander *inSuper);
	virtual				~CPGPStDialogHandler(void);

 	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
};

