/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPadmin.h,v 1.3.10.1 1997/12/05 22:13:40 mhw Exp $
____________________________________________________________________________*/

#pragma once 



#include <LApplication.h>


class CAdminApp : public LApplication {
public:
						CAdminApp();
	virtual 			~CAdminApp();
	

	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);	
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean & outEnabled, Boolean & outUsesMark,
							Char16 & outMark, Str255 outName);
	virtual void		ShowAboutBox();

protected:
	void				MakeLicenseStr(CString & outLicenseStr);
	virtual void		StartUp();
};
