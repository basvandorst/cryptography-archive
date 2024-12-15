/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: CAdminDialog.h,v 1.11 1999/03/14 12:31:56 wprice Exp $____________________________________________________________________________*/#pragma once #include <LGADialog.h>class CAdminDialog : public LGADialog {public:	enum	{ class_ID = 'AdmD' };							CAdminDialog(LStream * inStream);	virtual					~CAdminDialog();	virtual void			ListenToMessage(MessageT inMessage,									void * ioParam );	virtual Boolean			HandleKeyPress(const EventRecord & inKeyEvent );protected:	Boolean					mDirty;	FSSpec					mBaseInstallerSpec;	PGPAttributeValue *		mAVList;	PGPUInt32				mNumAVs;		virtual Boolean			AttemptQuitSelf(SInt32 inSaveOption);	virtual	void			FinishCreateSelf();	Boolean					VerifyEditFieldValues();	void					UpdateSummaryPane();		Boolean					Save();	PGPError				SaveCAData( PGPPrefRef		prefRef );	OSStatus				AddCompanyNameToInstaller(FSSpec * inFileSpec,								StringPtr inCompanyName);	OSStatus				CopyFileToResource(FSSpec * inFileSpec,									FSSpec * inResourceFileSpec,									ResType inType, ResIDT inID);	static pascal Boolean	InstallerFileFilter(CInfoPBPtr inPB, Ptr inDataP);};