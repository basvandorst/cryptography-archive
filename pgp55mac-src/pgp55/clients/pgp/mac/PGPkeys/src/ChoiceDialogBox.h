/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ChoiceDialogBox.h,v 1.2 1997/09/17 19:36:41 wprice Exp $
____________________________________________________________________________*/

class	ChoiceDialogBox:	public	LCommander
{
protected:
	enum	{	
				kMaxMessageLength		= 255, 
				kChoiceWindowID 		= 130,
				kChoiceTextID 			= 'dmsg',
				kOKButtonID 			= 'ok  ',
				kCancelButtonID 		= 'canc',
				kStringListID			= 1003,
				kOKButtonTitleID		= 1,
				kCancelButtonTitleID	= 2
			};
	
	char			mChoiceMessage[ kMaxMessageLength ];
	unsigned char	mOKButtonTitle[ kMaxMessageLength ];
	unsigned char	mCancelButtonTitle[ kMaxMessageLength ];
	UInt16			mChoiceMessageLength;
	Boolean			mSmartAdd;
	LGAPushButton	*mOKButton;
	LGAPushButton	*mCancelButton;
	Boolean			mShowing;
	
	static	ChoiceDialogBox	*sChoiceHandler;
	
public:
		
			ChoiceDialogBox( void );									

	static 	ChoiceDialogBox*	CreateChoiceDialogBoxStream(
										LStream *inStream );
	
	virtual Boolean	HandleKeyPress( const EventRecord &inKeyEvent );
	
	//Quick messages
	Boolean	Ask( const char *theChoice );
	Boolean	Ask( UInt32	number );
	Boolean	Ask( ResIDT resource, UInt32 number );
	Boolean	Ask( const char *theChoice, UInt32 number );
	Boolean	Ask( const char *theChoice, UInt32 number, UInt32 number2 );
	Boolean	Ask( const char *theChoice, UInt32 number, UInt32 number2,
					UInt32 number3 );

	Boolean	Ask( const unsigned	char *theChoice );
	Boolean	Ask( const unsigned	char *theChoice, UInt32 number );
	Boolean	Ask( const unsigned	char *theChoice, UInt32 number,
					UInt32 number2 );
	Boolean	Ask( const unsigned	char *theChoice, UInt32 number,
					UInt32 number2, UInt32 number3 );
	
	//Build messages
	void	Reset( void );
	void	Add( const char *theChoice );
	void	Add( const unsigned	char *theChoice );
	void	Add( Int32 number );
	Boolean	Show( void );
	
	//Button messages
	void	SetOKButtonTitle( 		const char *okString );
	void	SetCancelButtonTitle(	const char *cancelString );
	
	void	SetOKButtonTitle( 		const unsigned	char *okString );
	void	SetCancelButtonTitle(	const unsigned	char *cancelString );
	
	void	SetOKButtonTitle( 		ResIDT resource, UInt32 number );
	void	SetCancelButtonTitle(	ResIDT resource, UInt32 number );
		
	Boolean	GetSmartAdd( void ) const;
	void	SetSmartAdd(Boolean newSmartAdd);
	
	//Global choice handler.  Always use this one.
	static	ChoiceDialogBox&	ChoiceHandler();
};

class SaveChoiceDialogSmartAdd
{
private:
	Boolean	mValue;
public:
	SaveChoiceDialogSmartAdd()
		{mValue = ChoiceDialogBox::ChoiceHandler().GetSmartAdd();}
	~SaveChoiceDialogSmartAdd()
		{ChoiceDialogBox::ChoiceHandler().SetSmartAdd(mValue);}
};

