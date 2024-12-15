/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPassphraseEdit.cp,v 1.11.10.1 1997/12/07 04:27:04 mhw Exp $
____________________________________________________________________________*/

#include "MacEnvirons.h"
#include "MacMemory.h"
#include "pgpMem.h"

#include "CPassphraseEdit.h"

UInt32	CPassphraseEdit::mNumActiveFields = 0;
UInt8	CPassphraseEdit::mSpaceCountList[kMaxPassphraseLength];

const uchar	kSpaceCharacter = '\312';	// option-Space

CPassphraseEdit::CPassphraseEdit(LStream	*inStream)
		: LGAEditField(inStream)
{
	// This must be TRUE for GetDescriptor to work
	pgpAssert( kMaxPassphraseLength < sizeof( Str255 ) );
	
	mKeyFilter = PassphraseKeyFilter;
	
	mPassphraseHidden 		= TRUE;
	mPassphraseLength		= 0;
	mSpacePassphraseLength	= 0;

	pgpClearMemory( mPassphrase, sizeof( mPassphrase ) );

	if( VirtualMemoryIsOn() )
	{
		HoldMemory( mPassphrase, sizeof( mPassphrase ) );
	}
		
	// Pre-fill an array of space characters for simple showing and
	// hiding of the passphrase
	pgpFillMemory( mSpacePassphrase,
			sizeof( mSpacePassphrase ), kSpaceCharacter );
	
	SetMaxChars( kMaxPassphraseLength );
	
	++mNumActiveFields;
	if( mNumActiveFields == 1 )
	{
		// Generate a "random" amount of Space characters to represent the
		// entered character. This is stored so that backspacing appears to
		// work "correctly". The number of Spaces is derived using the crappy
		// Macintosh Random() routine.
			
		for( short index = 0; index < kMaxPassphraseLength; index++ )
		{
			mSpaceCountList[index] =
				( (ulong) Random() % kMaxSpacesPerCharacter ) + 1;
		}
	}
}

CPassphraseEdit::~CPassphraseEdit()
{
	// For security reasons, clear the passphrase items
	
	mPassphraseLength = 0;
	
	pgpAssert( mNumActiveFields > 0 );
	--mNumActiveFields;
	
	pgpClearMemory( mPassphrase, sizeof( mPassphrase ) );
	
	if( VirtualMemoryIsOn() )
	{
		UnholdMemory( mPassphrase, sizeof( mPassphrase ) );
	}
}

// ---------------------------------------------------------------------------
//	* SetHideStatus
// ---------------------------------------------------------------------------
//	Show or hide the passphrase text

	void
CPassphraseEdit::SetHideTyping(Boolean hide)
{
	if( mPassphraseHidden != hide )
	{
		FocusDraw();
		
		if( hide )
		{
			::TESetText( mSpacePassphrase, mSpacePassphraseLength, mTextEditH);
		}
		else
		{
			::TESetText( mPassphrase, mPassphraseLength, mTextEditH);
		}
		
		DrawSelf();

		mPassphraseHidden = hide;
	}
}

// ---------------------------------------------------------------------------
//	* FindCommandStatus
// ---------------------------------------------------------------------------
//	Disable the Edit menu options for the password field.

	void
CPassphraseEdit::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	Char16&		outMark,
	Str255		outName)
{
	switch (inCommand) {

		case cmd_Cut:
		case cmd_Copy:
		case cmd_Paste:
		case cmd_Clear:
		case cmd_SelectAll:
			outEnabled = FALSE;
			break;
			
		default:
			LGAEditField::FindCommandStatus(inCommand,
				outEnabled, outUsesMark, outMark, outName);
			break;
	}
}

// ---------------------------------------------------------------------------
//	* ClickSelf
// ---------------------------------------------------------------------------
//	Disallow mouse selection within the password field. The user can backspace,
//	but keyboard and mouse navigation are not allowed. Clicking in the field
//	will target it and set the insertion point to just beyond the last character
//	in the field.

	void
CPassphraseEdit::ClickSelf( const SMouseDownEvent &inMouseDown)
{
	#pragma unused( inMouseDown )

	BroadcastMessage('ckUI', (void *)mPaneID);

	if( !IsTarget() )
	{
		FocusDraw();
		::TESetSelect( max_Int16, max_Int16, mTextEditH );
		SwitchTarget( this );
	}
}

// ---------------------------------------------------------------------------
//		* AdjustCursorSelf
// ---------------------------------------------------------------------------
//	Do not let LEditField change the cursor to the I-beam.

void
CPassphraseEdit::AdjustCursorSelf(Point inPortPt, const EventRecord &inMacEvent)
{
#pragma unused( inPortPt, inMacEvent )
}

// ---------------------------------------------------------------------------
//	* UserChangedText
// ---------------------------------------------------------------------------
//	Broadcast a message to the listeners that the text has changed

	void
CPassphraseEdit::UserChangedText(void)
{
	LGAEditField::UserChangedText();
	
	BroadcastValueMessage();
}

// ---------------------------------------------------------------------------
//	* GetPassphraseLength
// ---------------------------------------------------------------------------

	UInt32
CPassphraseEdit::GetPassphraseLength(void)
{
	return( mPassphraseLength );
}

// ---------------------------------------------------------------------------
//	* GetDescriptor
// ---------------------------------------------------------------------------

	StringPtr
CPassphraseEdit::GetDescriptor(Str255 outDescriptor) const
{
	BlockMoveData( mPassphrase, &outDescriptor[1], mPassphraseLength );
	
	outDescriptor[0] = mPassphraseLength;
	
	return outDescriptor;
}

// ---------------------------------------------------------------------------
//	* PassphraseKeyFilter
// ---------------------------------------------------------------------------

	EKeyStatus
CPassphraseEdit::PassphraseKeyFilter(
	const EventRecord&	inKeyEvent,
	Int16				inKeyPosition)
{
	EKeyStatus	keyStatus;
	
	if( UKeyFilters::IsNavigationKey( inKeyEvent.message ) ||
		UKeyFilters::IsExtraEditKey( inKeyEvent.message ))
	{
		keyStatus = keyStatus_Reject;
	}
	else
	{
		keyStatus =
			UNewKeyFilters::PrintingCharField( inKeyEvent, inKeyPosition );
		if( keyStatus == keyStatus_PassUp )
		{
			// Reject all non-pringing pass up characters except action keys
			if( ! UKeyFilters::IsActionKey( inKeyEvent.message ) )
			{
				keyStatus = keyStatus_Reject;
			}
		}
	}

	return( keyStatus );
}

// ---------------------------------------------------------------------------
//	* HandleKeyPress
// ---------------------------------------------------------------------------
//	This is the total logic (and most of the code) from
//	LBroadcasterEditField::HandleKeyPress and LBroadcasterEditField::
//	LEditField_HandleKeyPress. We copy the code here to address security
//	concerns. LBroadcasterEditField posts LTETypingAction objects to support
//	undo/redo. The objects contain copies of the current edit text and are
//	uncontrolled. Since we support no undo, this functionality is eliminated.
//	
//	Because we have copied the code from PowerPlant, it is important to keep
//	in sync with future developments of the LBroadcasterEditField and
//	LGAEditField classes. We do this by breaking compilation if the
//	PowerPlant version changes

#if __PowerPlant__ > 0x01708000	// Version 1.7
	!!! Recheck code copied from parent classes. See comment above!
#endif

	Boolean
CPassphraseEdit::HandleKeyPress(
	const EventRecord& inKeyEvent)
{

	// Intercept Enter and Return key for immediate value change.

	EKeyStatus	theKeyStatus 	= keyStatus_Input;
	Boolean		keyHandled 		= TRUE;
	Int16		theKey 			= inKeyEvent.message & charCodeMask;
	
	if (mKeyFilter != nil)
	{
		theKeyStatus = (*mKeyFilter)(inKeyEvent, 0);
	}
	
	if ((theKeyStatus != keyStatus_Input)
	  && (UKeyFilters::IsActionKey(inKeyEvent.message)))
	{
		switch (inKeyEvent.message & charCodeMask)
		{
			case char_Return:
			case char_Enter: 
				BroadcastValueMessage();
				SelectAll();
		}
	}

	if (inKeyEvent.modifiers & cmdKey)
	{										// Always pass up when the command
		theKeyStatus = keyStatus_PassUp;	//   key is down
	}
	
	switch (theKeyStatus)
	{
		case keyStatus_Input:
		{
			UInt32	numSpaces;
			
			// Check if we are at the character limit
			// ### Not two-PGPByte char compatible
			
			if( TooManyCharacters( 1 ) )
			{
				SysBeep(1);
				break;
			}
			
			FocusDraw();
			
			numSpaces 				= mSpaceCountList[mPassphraseLength];
			mSpacePassphraseLength	+= numSpaces;
			
			mPassphrase[mPassphraseLength] = theKey;
			++mPassphraseLength;
			
			if( mPassphraseHidden )
			{
				UInt32	SpaceIndex;
				
				// If the passphrase is hidden, add the Spaces
				for( SpaceIndex = 0; SpaceIndex < numSpaces; SpaceIndex++ )
				{
					::TEKey(kSpaceCharacter, mTextEditH);
				}
			}
			else
			{
				::TEKey(theKey, mTextEditH);
			}
			
			UserChangedText();
			break;
		}
		
		case keyStatus_TEDelete:
		{
			if ((**mTextEditH).selEnd > 0)
			{
				UInt32	backspaceCount;
				UInt32	index;
				UInt32	numSpaces;
				
				FocusDraw();
				
				pgpAssert( mPassphraseLength > 0 );

				--mPassphraseLength;
				
				numSpaces = mSpaceCountList[mPassphraseLength];
				mSpacePassphraseLength	-= numSpaces;
				
				if( mPassphraseHidden )
				{
					backspaceCount = numSpaces;
				}
				else
				{
					backspaceCount = 1;
				}
				
				for( index = 0; index < backspaceCount; index++ )
				{
					::TEKey(char_Backspace, mTextEditH);
				}

				UserChangedText();
			}
			
			break;
		}
			
		case keyStatus_TECursor:
			pgpDebugMsg( "CPassphraseEdit::HandleKeyPress: "
				"Cursor key was not filtered!" );
			break;
			
		case keyStatus_ExtraEdit:
			pgpDebugMsg( "CPassphraseEdit::HandleKeyPress: "
				"Edit key was not filtered!" );
			break;
			
		case keyStatus_Reject:
			SysBeep(1);
			break;
			
		case keyStatus_PassUp:
			// Skip all parent classes up to LCommander
			keyHandled = LCommander::HandleKeyPress(inKeyEvent);
			break;
	}
	
	return keyHandled;

}

// ---------------------------------------------------------------------------
//	* SelectAll
// ---------------------------------------------------------------------------
//	Don't allow select all text. Instead, position the insertion point at the
//	end of the text.

	void
CPassphraseEdit::SelectAll(void)
{
	StFocusAndClipIfHidden	focus(this);
	::TESetSelect(max_Int16, max_Int16, mTextEditH);
}

// ---------------------------------------------------------------------------
//	* TooManyCharacters
// ---------------------------------------------------------------------------
//	The default implementation uses the onscreen representation to get the
//	length of the text. This does not work when Spaces are being displayed.


	Boolean
CPassphraseEdit::TooManyCharacters(Int32 inCharsToAdd)
{
	return( mPassphraseLength + inCharsToAdd > mMaxChars );
}

// ---------------------------------------------------------------------------
//	* SetDescriptor
// ---------------------------------------------------------------------------
//	Don't allow this call for passphrase fields. They always start out empty.

	void
CPassphraseEdit::SetDescriptor(ConstStr255Param inDescriptor)
{
	#pragma unused( inDescriptor )
	
	pgpDebugMsg( "CPassphraseEdit::SetDescriptor(): "
		"This should never be called" );
}

// ---------------------------------------------------------------------------
//	* ClearPassphrase
// ---------------------------------------------------------------------------
//	Clear out the passphrase and redraw.

	void
CPassphraseEdit::ClearPassphrase(void)
{
	mPassphraseLength		= 0;
	mSpacePassphraseLength	= 0;

	LGAEditField::SetDescriptor( "\p" );

	pgpClearMemory( mPassphrase, sizeof( mPassphrase ) );
	
	Draw( nil );
}

