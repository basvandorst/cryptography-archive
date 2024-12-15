/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: BackupHandler.h,v 1.3 1997/10/22 07:29:45 wprice Exp $
____________________________________________________________________________*/

class	BackupHandler:	public	LCommander
{
protected:
	LWindow			*mWindow;
	LGAPushButton	*mOKButton;
	LGAPushButton	*mCancelButton;
	LGAPushButton	*mDontSaveButton;
	enum			{
						kBackupWindowID			= 135,
						kOKButtonID				= 'OK  ',
						kCancelButtonID			= 'Canc',
						kNoBackupButtonID		= 'Dont',
						kBackupStrings 			= 1008,
						kBackupOKTitleID		= 1,
						kBackupCancelTitleID	= 2,
						kBackupMessageID		= 3,
						kBackupErrorStringID	= 4,
						kBackupFileNameID		= 5,
						kPublicBackupMessageID	= 6,
						kPublicFileNameID		= 7,
						kBackupCreator			= '????',
						kBackupFileType			= 'TEXT'
					};

public:
	Boolean	Run( void );
	
	Boolean	HandleKeyPress( const EventRecord&	inKeyEvent );

};

