/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CAdminDialog.h,v 1.6.8.1 1997/12/05 22:13:37 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <LGADialogBox.h>

class CAdminDialog : public LGADialogBox {
public:
	enum	{ class_ID = 'AdmD' };

							CAdminDialog(LStream * inStream);
	virtual					~CAdminDialog();

	virtual void			ListenToMessage(MessageT inMessage,
									void * ioParam );

protected:
	Boolean					mDirty;
	PaneIDT					mCurrentPane;
	FSSpec					mBaseInstallerSpec;
	
	virtual Boolean			AttemptQuitSelf(SInt32 inSaveOption);
	virtual	void			FinishCreateSelf();

	Boolean					VerifyEditFieldValues();
	void					UpdateSummaryPane();
	
	Boolean					Save();
	OSStatus				CopyFileToResource(FSSpec * inFileSpec,
									FSSpec * inResourceFileSpec,
									ResType inType, ResIDT inID);

	static pascal Boolean	InstallerFileFilter(CInfoPBPtr inPB, Ptr inDataP);
};
