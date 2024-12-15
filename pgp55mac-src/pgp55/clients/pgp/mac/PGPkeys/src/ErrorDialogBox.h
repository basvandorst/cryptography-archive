/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ErrorDialogBox.h,v 1.2 1997/09/17 19:36:42 wprice Exp $
____________________________________________________________________________*/
#include <LDialogBox.h>
#include <LCommander.h>

class	ErrorDialogBox:	public	LCommander
{
protected:
	enum	{	
				kMaxMessageLength	= 255, 
				kErrorWindowID 		= 128,
				kErrorTextID 		= 'dmsg',
				kPushButtonID 		= 'ok  '
			};
	
	char			mErrorMessage[kMaxMessageLength];
	UInt16			mErrorMessageLength;
	Boolean			mSmartAdd;
	LControl		*mDefaultButton;
	Boolean			mShowing;
	
	static	ErrorDialogBox	*sErrorHandler;
	
public:
		
			ErrorDialogBox( void );									

	static 	ErrorDialogBox*	CreateErrorDialogBoxStream( LStream *inStream );
	
	virtual Boolean	HandleKeyPress( const EventRecord &inKeyEvent );
	
	//Quick messages
	void	Report( const char *theError );
	void	Report( UInt32	number );
	void	Report( ResIDT	resource, UInt32 number );
	void	Report( const char *theError, UInt32 number );
	void	Report( const char *theError, UInt32 number,
					UInt32 number2 );
	void	Report( const char *theError, UInt32 number, UInt32 number2,
					UInt32 number3 );

	void	Report( const unsigned char *theError );
	void	Report( const unsigned char *theError, UInt32 number );
	void	Report( const unsigned char *theError, UInt32 number,
					UInt32 number2 );
	void	Report( const unsigned char *theError, UInt32 number,
					UInt32 number2, UInt32 number3 );
	
	//Build messages
	void	Reset( void );
	void	Add( const char *theError );
	void	Add( const unsigned char *theError );
	void	Add( Int32 number );
	void	Show( void );
	
	Boolean	GetSmartAdd( void ) const;
	void	SetSmartAdd(Boolean newSmartAdd);
	
	//Global error handler.  Always use this one.
	static	ErrorDialogBox&	ErrorHandler();
};

class SaveErrorDialogSmartAdd
{
private:
	Boolean	mValue;
public:
	SaveErrorDialogSmartAdd()
		{mValue = ErrorDialogBox::ErrorHandler().GetSmartAdd();}
	~SaveErrorDialogSmartAdd()
		{ErrorDialogBox::ErrorHandler().SetSmartAdd(mValue);}
};

