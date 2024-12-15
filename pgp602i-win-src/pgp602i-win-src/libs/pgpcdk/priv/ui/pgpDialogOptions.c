/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpDialogOptions.c,v 1.26.10.1 1998/11/12 03:22:58 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpKeys.h"
#include "pgpKeyServer.h"
#include "pgpMem.h"
#include "pgpUserInterface.h"

#include "pgpOptionListPriv.h"

#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

	PGPOptionListRef
PGPOUIDialogPrompt(
	PGPContextRef 	context,
	const char		*prompt)
{
	PGPOptionListRef	optionList;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( prompt ) );
	
	optionList = pgpCreateBufferOptionList( context,
								kPGPOptionType_DialogPrompt,
								prompt, strlen( prompt ) + 1 );
								
	return( optionList );
}

	PGPOptionListRef
PGPOUIWindowTitle(
	PGPContextRef 	context,
	const char		*title)
{
	PGPOptionListRef	optionList;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( title ) );
	
	optionList = pgpCreateBufferOptionList( context,
								kPGPOptionType_WindowTitle,
								title, strlen( title ) + 1 );
								
	return( optionList );
}

	PGPOptionListRef
PGPOUIOutputPassphrase(
	PGPContextRef 	context,
	char			**passphrase)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionParam( IsntNull( passphrase ) );
	*passphrase = NULL;
	pgpValidateOptionContext( context );
	
	/* buffer size may be 0 */
	
	value.asPtr	= passphrase;
	optionList 	= pgpCreateStandardValueOptionList( context, 
						kPGPOptionType_OutputPassphrase,
						&value, sizeof( passphrase), NULL );

	return( optionList );
}

/* MS Windows only.*/
/* Option takes parent's HWND and uses for child windows */

#if PGP_WIN32
	PGPOptionListRef
PGPOUIParentWindowHandle(
	PGPContextRef 	context,
	HWND hwndParent)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	/* buffer size may be 0 */
	
	value.asUInt = (UINT)hwndParent;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_ParentWindowHandle,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}
#endif

	static void
FreePGPOUICheckboxDesc(
	PGPContextRef		context,
	PGPOUICheckboxDesc	*desc)
{
	pgpAssertAddrValid( desc, PGPOUICheckboxDesc );
	
	if( IsntNull( desc->title ) )
	{
		pgpContextMemFree( context, (void *) desc->title );
		desc->title = NULL;
	}

	if( IsntNull( desc->description ) )
	{
		pgpContextMemFree( context, (void *) desc->description );
		desc->description = NULL;
	}
	
	pgpContextMemFree( context, desc );
}

	static PGPError
CopyPGPOUICheckboxDesc(
	PGPContextRef				context,
	const PGPOUICheckboxDesc	*srcDesc,
	PGPOUICheckboxDesc			**destDesc)
{
	PGPError			err = kPGPError_OutOfMemory;
	PGPOUICheckboxDesc	*outDesc;
	
	pgpAssertAddrValid( srcDesc, PGPOUICheckboxDesc );
	pgpAssertAddrValid( destDesc, PGPOUICheckboxDesc * );
	
	*destDesc = NULL;
	
	outDesc = (PGPOUICheckboxDesc *) pgpContextMemAlloc( context,
						sizeof( *outDesc ), kPGPMemoryMgrFlags_Clear );
	if( IsntNull( outDesc ) )
	{
		*outDesc = *srcDesc;
		
		pgpAssertAddrValid( srcDesc->title, char );
		pgpAssertAddrValid( srcDesc->description, char );
		
		outDesc->title 			= pgpAllocCString( context, srcDesc->title );
		outDesc->description 	= pgpAllocCString( context,
											srcDesc->description );
		if( IsntNull( outDesc->title ) &&
			IsntNull( outDesc->description ) )
		{
			err = kPGPError_NoErr;
		}
		else
		{
			FreePGPOUICheckboxDesc( context, outDesc );
			outDesc = NULL;
		}
	}
	
	*destDesc = outDesc;

	return( err );
}

	static PGPError
CheckboxOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError			err = kPGPError_NoErr;
	PGPOUICheckboxDesc	*inputDesc;
	
	pgpAssert( inputValueSize == sizeof( *inputDesc ) );

	(void) type;
	(void) inputValueSize;
	
	inputDesc = (PGPOUICheckboxDesc *) inputValue.asPtr;
	pgpAssertAddrValid( inputDesc, PGPOUICheckboxDesc );
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			FreePGPOUICheckboxDesc( context, inputDesc );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			PGPOUICheckboxDesc	*outputDesc;

			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			err = CopyPGPOUICheckboxDesc( context, inputDesc, &outputDesc );
			if( IsntPGPError( err ) )
			{
				outputValue->asPtr 	= outputDesc;
				*outputValueSize 	= sizeof( *outputDesc );
			}

			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}

	PGPOptionListRef
PGPOUICheckbox(
	PGPContextRef 	context,
	PGPUInt32 		itemID,
	const char 		*title,
	const char 		*description,	/* Can be NULL */
	PGPUInt32 		initialValue,
	PGPUInt32 		*resultPtr)
{
	PGPOptionListRef	optionList;
	PGPOUICheckboxDesc	descriptor;
	PGPOUICheckboxDesc	*allocatedDesc;
	char				emptyString[] = "";
	
	pgpValidateOptionParam( IsntNull( resultPtr ) );
	*resultPtr = 0;
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( title ) );
	
	/* Booleanize value */
	if( initialValue > 0 )
		initialValue = 1;
	
	*resultPtr = initialValue;
	
	pgpClearMemory( &descriptor, sizeof( descriptor ) );
	
	descriptor.itemID		= itemID;
	descriptor.valuePtr 	= resultPtr;
	descriptor.title		= title;
	
	if( IsntNull( description ) )
	{
		descriptor.description = description;
	}
	else
	{
		descriptor.description = emptyString;
	}
	
	if( IsntPGPError( CopyPGPOUICheckboxDesc( context, &descriptor,
				&allocatedDesc ) ) )
	{
		PGPOptionValue		value;
		
		value.asPtr = allocatedDesc;
		
		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_Checkbox,
							&value, sizeof( *allocatedDesc ),
							CheckboxOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	static void
FreePGPOUIPopupListDesc(
	PGPContextRef		context,
	PGPOUIPopupListDesc	*desc)
{
	pgpAssertAddrValid( desc, PGPOUIPopupListDesc );
	
	if( IsntNull( desc->title ) )
	{
		pgpContextMemFree( context, (void *) desc->title );
		desc->title = NULL;
	}

	if( IsntNull( desc->description ) )
	{
		pgpContextMemFree( context, (void *) desc->description );
		desc->description = NULL;
	}

	if( IsntNull( desc->listItems ) )
	{
		PGPUInt32	listIndex;
		
		for( listIndex = 0; listIndex < desc->numListItems; listIndex++ )
		{
			if( IsntNull( desc->listItems[listIndex] ) )
			{
				pgpContextMemFree( context, 
							(void *) desc->listItems[listIndex] );
				desc->listItems[listIndex] = NULL;
			}
		}
		
		pgpContextMemFree( context, (void *) desc->listItems );
		desc->listItems = NULL;
	}
	
	pgpContextMemFree( context, desc );
}

	static char **
CopyPopupListItems(
	PGPContextRef				context,
	const PGPOUIPopupListDesc	*srcDesc)
{
	char		**listItems;
	PGPError	err = kPGPError_NoErr;
	
	pgpAssertAddrValid( srcDesc, PGPOUIPopupListDesc );
	pgpAssert( srcDesc->numListItems > 0 );
	pgpAssertAddrValid( srcDesc->listItems, char * );

	listItems = (char **) pgpContextMemAlloc( context, srcDesc->numListItems *
						sizeof( srcDesc->listItems[0] ),
						kPGPMemoryMgrFlags_Clear );
	if( IsntNull( listItems ) )
	{
		PGPUInt32	listIndex;
		
		for( listIndex = 0; listIndex < srcDesc->numListItems; listIndex++ )
		{
			if( IsntNull( srcDesc->listItems[listIndex] ) )
			{
				listItems[listIndex] = pgpAllocCString( context,
											srcDesc->listItems[listIndex] );
				if( IsNull( listItems[listIndex] ) )
				{
					err = kPGPError_OutOfMemory;
					break;
				}
			}
		}
	
		if( IsPGPError( err ) )
		{
			for( listIndex = 0; listIndex < srcDesc->numListItems; listIndex++ )
			{
				if( IsntNull( listItems[listIndex] ) )
					pgpContextMemFree( context, listItems[listIndex] );
			}
			
			pgpContextMemFree( context, listItems );
			listItems = NULL;
		}
	}
	
	return( listItems );	
}

	static PGPError
CopyPGPOUIPopupListDesc(
	PGPContextRef				context,
	const PGPOUIPopupListDesc	*srcDesc,
	PGPOUIPopupListDesc			**destDesc)
{
	PGPError			err = kPGPError_OutOfMemory;
	PGPOUIPopupListDesc	*outDesc;
	
	pgpAssertAddrValid( srcDesc, PGPOUIPopupListDesc );
	pgpAssertAddrValid( destDesc, PGPOUIPopupListDesc * );
	
	*destDesc = NULL;
	
	outDesc = (PGPOUIPopupListDesc *) pgpContextMemAlloc( context,
						sizeof( *outDesc ), kPGPMemoryMgrFlags_Clear );
	if( IsntNull( outDesc ) )
	{
		*outDesc = *srcDesc;
		
		pgpAssertAddrValid( srcDesc->title, char );
		pgpAssertAddrValid( srcDesc->description, char );
		
		outDesc->title 			= pgpAllocCString( context, srcDesc->title );
		outDesc->description 	= pgpAllocCString( context,
											srcDesc->description );
		outDesc->listItems		= CopyPopupListItems( context, srcDesc );
						
		if( IsntNull( outDesc->title ) &&
			IsntNull( outDesc->description ) &&
			IsntNull( outDesc->listItems ) )
		{
			err = kPGPError_NoErr;
		}
		else
		{
			FreePGPOUIPopupListDesc( context, outDesc );
			outDesc = NULL;
		}
	}
	
	*destDesc = outDesc;

	return( err );
}

	static PGPError
PopupListOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError			err = kPGPError_NoErr;
	PGPOUIPopupListDesc	*inputDesc;
	
	pgpAssert( inputValueSize == sizeof( *inputDesc ) );

	(void) type;
	(void) inputValueSize;
	
	inputDesc = (PGPOUIPopupListDesc *) inputValue.asPtr;
	pgpAssertAddrValid( inputDesc, PGPOUIPopupListDesc );
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			FreePGPOUIPopupListDesc( context, inputDesc );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			PGPOUIPopupListDesc	*outputDesc;

			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			err = CopyPGPOUIPopupListDesc( context, inputDesc, &outputDesc );
			if( IsntPGPError( err ) )
			{
				outputValue->asPtr 	= outputDesc;
				*outputValueSize 	= sizeof( *outputDesc );
			}

			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}

	PGPOptionListRef
PGPOUIPopupList(
	PGPContextRef 	context,
	PGPUInt32 		itemID,
	const char 		*title,
	const char 		*description,	/* Can be NULL */
	PGPUInt32		numListItems,
	const char		*listItems[],
	PGPUInt32 		initialValue,
	PGPUInt32 		*resultPtr)
{
	PGPOptionListRef	optionList;
	PGPOUIPopupListDesc	descriptor;
	PGPOUIPopupListDesc	*allocatedDesc;
	char				emptyString[] = "";
	
	pgpValidateOptionParam( IsntNull( resultPtr ) );
	*resultPtr = 0;
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( title ) );
	pgpValidateOptionParam( numListItems > 0 );
	pgpValidateOptionParam( IsntNull( listItems ) );
	pgpValidateOptionParam( initialValue <= numListItems );
	
	*resultPtr = initialValue;
	
	pgpClearMemory( &descriptor, sizeof( descriptor ) );
	
	descriptor.itemID		= itemID;
	descriptor.valuePtr 	= resultPtr;
	descriptor.title		= title;
	descriptor.numListItems	= numListItems;
	descriptor.listItems	= listItems;
	
	if( IsntNull( description ) )
	{
		descriptor.description = description;
	}
	else
	{
		descriptor.description = emptyString;
	}

	if( IsntPGPError( CopyPGPOUIPopupListDesc( context, &descriptor,
				&allocatedDesc ) ) )
	{
		PGPOptionValue		value;
		
		value.asPtr = allocatedDesc;
		
		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_PopupList,
							&value, sizeof( *allocatedDesc ),
							PopupListOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

static const PGPOptionType dialogOptionSet[] =
{
	kPGPOptionType_Checkbox,
	kPGPOptionType_PopupList
};

	PGPOptionListRef
PGPOUIDialogOptions(
	PGPContextRef		context,
	PGPOptionListRef 	firstOption,
	...)
{
	PGPOptionListRef	optionList;
	PGPOptionListRef	subOptions;
	va_list				args;
	
	pgpAssert( pgpContextIsValid( context ) );

	if ( pgpContextIsValid( context ) )
	{
		va_start( args, firstOption );
			subOptions = pgpBuildOptionListArgs( context,
						FALSE, firstOption, args );
		va_end( args );

		if( IsntPGPError( pgpCheckOptionsInSet( subOptions,
				dialogOptionSet, elemsof( dialogOptionSet ) ) ) )
		{
			optionList = pgpCreateCustomValueOptionList(
							context,  
							kPGPOptionType_DialogOptions,
							kPGPOptionFlag_Default, NULL,
							0, subOptions, NULL);
		}
		else
		{
			optionList = kPGPBadParamsOptionListRef;
			
			PGPFreeOptionList( subOptions );
		}
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		optionList = kPGPBadParamsOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIMinimumPassphraseLength(
	PGPContextRef 	context,
	PGPUInt32		minimumPassphraseLength)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asUInt = minimumPassphraseLength;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_MinPassphraseLength,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIMinimumPassphraseQuality(
	PGPContextRef 	context,
	PGPUInt32		minimumPassphraseQuality)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( minimumPassphraseQuality <= 100 );
	
	value.asUInt = minimumPassphraseQuality;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_MinPassphraseQuality,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIShowPassphraseQuality(
	PGPContextRef 	context,
	PGPBoolean		showPassphraseQuality)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asUInt = showPassphraseQuality;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_ShowPassphraseQuality,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIVerifyPassphrase(
	PGPContextRef 	context,
	PGPBoolean		verifyPassphrase)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asUInt = verifyPassphrase;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_VerifyPassphrase,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIFindMatchingKey(
	PGPContextRef 	context,
	PGPBoolean		findMatchingKey)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asUInt = findMatchingKey;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_FindMatchingKey,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

PGPOptionListRef
PGPOUITextUI(
	PGPContextRef 	context,
	PGPBoolean		textUI)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asUInt = textUI;
	
	optionList = pgpCreateStandardValueOptionList( context,  
												   kPGPOptionType_TextUI,
												   &value,
												   sizeof( PGPUInt32 ),
												   NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIDefaultKey(
	PGPContextRef 	context,
	PGPKeyRef		defaultKey)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asKeyRef = defaultKey;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_DefaultKey,
						&value, sizeof( PGPKeyRef ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIDefaultRecipients(
	PGPContextRef 			context,
	PGPUInt32 				numRecipients,
	const PGPRecipientSpec 	recipients[])
{
	PGPOptionListRef		optionList;
	PGPBoolean				badList = FALSE;
	PGPUInt32				recipientIndex;
	const PGPRecipientSpec	*curRecipient;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( numRecipients > 0 );
	pgpValidateOptionParam( IsntNull( recipients ) );
	
	curRecipient = &recipients[0];
	
	for( recipientIndex = 0; recipientIndex < numRecipients; recipientIndex++ )
	{
		switch( curRecipient->type )
		{
			case kPGPRecipientSpecType_Key:
			{
				if( ! PGPKeyRefIsValid( curRecipient->u.key ) )
				{
					pgpDebugMsg( "PGPOUIDefaultRecipients() Invalid key ref" );
					badList = TRUE;
				}
				
				break;
			}

			case kPGPRecipientSpecType_UserID:
			{
				if( curRecipient->u.userIDStr[0] == 0 )
				{
					pgpDebugMsg( "PGPOUIDefaultRecipients() Empty user ID" );
					badList = TRUE;
				}
				
				break;
			}
			
			case kPGPRecipientSpecType_KeyID:
				break;
				
			default:
			{
				pgpDebugMsg( "PGPOUIDefaultRecipients(): Unknown spec type" );
			
				badList = TRUE;
				break;
			}
		}
	
		if( badList )
			break;
			
		++curRecipient;
	}
	
	if( ! badList )
	{
		optionList = pgpCreateBufferOptionList( context,
							kPGPOptionType_DefaultRecipients,
							recipients, numRecipients *
							sizeof( recipients[0] ) );
	}
	else
	{
		optionList = kPGPBadParamsOptionListRef;
	}
	return( optionList );
}

	PGPOptionListRef
PGPOUIDisplayMarginalValidity(
	PGPContextRef 	context,
	PGPBoolean		displayMarginalValidity)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asUInt = displayMarginalValidity;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_DisplayMarginalValidity,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIIgnoreMarginalValidity(
	PGPContextRef 	context,
	PGPBoolean		ignoreMarginalValidity)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asUInt = ignoreMarginalValidity;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_IgnoreMarginalValidity,
						&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	static PGPError
GroupSetOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError	err = kPGPError_NoErr;
	
	(void) context;
	(void) type;
	(void) inputValueSize;
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			PGPFreeGroupSet( inputValue.asGroupSetRef );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			err = PGPCopyGroupSet( inputValue.asGroupSetRef,
						&outputValue->asGroupSetRef );
			if( IsntPGPError( err ) )
			{
				*outputValueSize = sizeof( outputValue->asGroupSetRef );
			}

			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}

	PGPOptionListRef
PGPOUIRecipientGroups(
	PGPContextRef 	context,
	PGPGroupSetRef	groupSet)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPGroupSetRefIsValid( groupSet ) );
	
	if( IsntPGPError( PGPCopyGroupSet( groupSet, &value.asGroupSetRef ) ) )
	{
		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_RecipientGroups,
							&value, sizeof( groupSet ),
							GroupSetOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIEnforceAdditionalRecipientRequests(
	PGPContextRef 								context,
	PGPAdditionalRecipientRequestEnforcement 	enforcement,
	PGPBoolean									alwaysDisplayDialogWithARRs)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	
	value.asARRParams.enforcement 	= enforcement;
	value.asARRParams.displayDialog = ( alwaysDisplayDialogWithARRs != 0 );

	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_ARREnforcement,
						&value, sizeof( PGPOUIARRParamsDesc ), NULL );
	
	return( optionList );
}

	static PGPError
KeyServerOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError	err = kPGPError_NoErr;
	
	(void) context;
	(void) inputValueSize;
	(void) outputValueSize;
	(void) outputValue;
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			switch( type )
			{
				case kPGPOptionType_KeyServerSearchFilter:
					PGPFreeFilter( inputValue.asFilterRef );
					break;
				
				case kPGPOptionType_KeyServerSearchKeySet:
					PGPFreeKeySet( inputValue.asKeySetRef );
					break;
				
				default:
					break;
			}
			
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			switch( type )
			{
				case kPGPOptionType_KeyServerSearchFilter:
					PGPIncFilterRefCount( inputValue.asFilterRef );
					break;
				
				case kPGPOptionType_KeyServerSearchKeySet:
					PGPIncKeySetRefCount( inputValue.asKeySetRef );
					break;
				
				default:
					break;
			}
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}

	PGPOptionListRef
PGPOUIKeyServerSearchFilter(
	PGPContextRef	context,
	PGPFilterRef 	filter)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPFilterRefIsValid( filter ) );
	
	value.asFilterRef = filter;

	PGPIncFilterRefCount( filter );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerSearchFilter,
							&value, sizeof( filter ),
							KeyServerOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIKeyServerSearchKeySet(
	PGPContextRef	context,
	PGPKeySetRef 	keySet)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPKeySetRefIsValid( keySet ) );
	
	value.asKeySetRef = keySet;

	PGPIncKeySetRefCount( keySet );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerSearchKeySet,
							&value, sizeof( keySet ),
							KeyServerOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIKeyServerSearchKeyIDList(
	PGPContextRef	context,
	PGPUInt32 		keyIDCount,
	const PGPKeyID 	keyIDList[])
{
	PGPOptionListRef	optionList;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( keyIDList ) );
	pgpValidateOptionParam( keyIDCount > 0 );
	
	optionList = pgpCreateBufferOptionList( context,
							kPGPOptionType_KeyServerSearchKeyIDList,
							keyIDList, keyIDCount * sizeof( keyIDList[0] ) );
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIKeyServerSearchKey(
	PGPContextRef	context,
	PGPKeyRef 		key)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPKeyRefIsValid( key ) );
	
	value.asKeyRef = key;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerSearchKey,
							&value, sizeof( key ), NULL );
	
	return( optionList );
}

	static void
FreePGPOUIKeyServerParamsDesc(
	PGPContextRef 				context,
	PGPOUIKeyServerParamsDesc 	*desc)
{
	pgpAssertAddrValid( desc, PGPOUIKeyServerParamsDesc );
	
	if( IsntNull( desc->serverList ) )
	{
		PGPUInt32	specIndex;

		for( specIndex = 0; specIndex < desc->serverCount; specIndex++)
		{
			PGPKeyServerSpec	*spec = &desc->serverList[specIndex];
			
			if( IsntNull( spec->serverName ) )
				pgpContextMemFree( context, (char *) spec->serverName );
				
			if( PGPKeyServerRefIsValid( spec->server ) )
				PGPFreeKeyServer( spec->server );
		}
		
		PGPFreeData( (PGPKeyServerSpec *) desc->serverList );
		desc->serverList = NULL;
	}
	
	PGPFreeData( desc );
}

	static PGPError
CopyPGPOUIKeyServerParamsDesc(
	PGPContextRef					context,
	const PGPOUIKeyServerParamsDesc	*srcDesc,
	PGPOUIKeyServerParamsDesc		**destDesc)
{
	PGPError					err = kPGPError_NoErr;
	PGPOUIKeyServerParamsDesc	*outDesc;
	
	pgpAssertAddrValid( srcDesc, PGPOUIKeyServerParamsDesc );
	pgpAssertAddrValid( destDesc, PGPOUIKeyServerParamsDesc * );
	
	*destDesc = NULL;
	
	outDesc = (PGPOUIKeyServerParamsDesc *) PGPNewData(
						PGPGetContextMemoryMgr( context ),
						sizeof( *outDesc ), kPGPMemoryMgrFlags_Clear );
	if( IsntNull( outDesc ) )
	{
		PGPSize	dataSize;
		
		pgpAssertAddrValid( srcDesc->serverList, PGPKeyServerSpec );
		
		*outDesc = *srcDesc;
		dataSize = srcDesc->serverCount * sizeof( PGPKeyServerSpec );
		
		outDesc->serverList = (PGPKeyServerSpec *) PGPNewData(
										PGPGetContextMemoryMgr( context ),
										dataSize, 0 );
		if( IsntNull( outDesc->serverList ) )
		{
			PGPUInt32	specIndex;
			
			pgpCopyMemory( srcDesc->serverList,
						(PGPKeyServerSpec *) outDesc->serverList, dataSize );
			
			/* Need to copy server names and domains */
			for(specIndex = 0; specIndex < srcDesc->serverCount; specIndex++)
			{
				PGPKeyServerSpec	*spec = &outDesc->serverList[specIndex];
				
				spec->serverName = NULL;
				
				if( PGPKeyServerRefIsValid( spec->server ) )
				{
					PGPIncKeyServerRefCount( spec->server );
				}
			}

			for(specIndex = 0; specIndex < srcDesc->serverCount; specIndex++)
			{
				PGPKeyServerSpec	*srcSpec;
				PGPKeyServerSpec	*destSpec;
				
				srcSpec 	= &srcDesc->serverList[specIndex];
				destSpec 	= &outDesc->serverList[specIndex];

				if( IsntNull( srcSpec->serverName ) )
				{
					destSpec->serverName = pgpAllocCString( context,
												srcSpec->serverName );
					if( IsNull( destSpec->serverName ) )
						err = kPGPError_OutOfMemory;
				}
				
				if( IsPGPError( err ) )
					break;
			}
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
		
		if( IsPGPError( err ) )
		{
			FreePGPOUIKeyServerParamsDesc( context, outDesc );
			outDesc = NULL;
		}
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}
	
	*destDesc = outDesc;

	return( err );
}

	static PGPError
KeyServerParamsOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError					err = kPGPError_NoErr;
	PGPOUIKeyServerParamsDesc	*inputDesc;
	
	pgpAssert( inputValueSize == sizeof( *inputDesc ) );

	(void) type;
	(void) inputValueSize;
	
	inputDesc = (PGPOUIKeyServerParamsDesc *) inputValue.asPtr;
	pgpAssertAddrValid( inputDesc, PGPOUIKeyServerParamsDesc );
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			FreePGPOUIKeyServerParamsDesc( context, inputDesc );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			PGPOUIKeyServerParamsDesc	*outputDesc;

			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			err = CopyPGPOUIKeyServerParamsDesc( context, inputDesc,
								&outputDesc );
			if( IsntPGPError( err ) )
			{
				outputValue->asPtr 	= outputDesc;
				*outputValueSize 	= sizeof( *outputDesc );
			}

			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}

	PGPOptionListRef
PGPOUIKeyServerUpdateParams(
	PGPContextRef 			context,
	PGPUInt32 				serverCount,
	const PGPKeyServerSpec 	serverList[],
	PGPtlsContextRef		tlsContext,
	PGPBoolean 				searchBeforeDisplay,
	PGPKeySetRef 			*foundKeys,
	PGPOptionListRef 		firstOption,
	...)
{
	PGPOptionListRef	optionList;
	va_list				args;
	
	if( IsntNull( foundKeys ) )
		*foundKeys = kInvalidPGPKeySetRef;
	
	pgpAssert( IsntNull( foundKeys ) );
	pgpAssert( PGPContextRefIsValid( context ) );
	pgpAssert( IsntNull( serverList ) );
	pgpAssert( serverCount > 0 );
	
	if( PGPContextRefIsValid( context ) &&
		IsntNull( foundKeys ) &&
		IsntNull( serverList ) &&
		serverCount > 0 )
	{
		PGPOptionListRef			subOptions;
		PGPOUIKeyServerParamsDesc	descriptor;
		PGPOUIKeyServerParamsDesc	*allocatedDesc;

		va_start( args, firstOption );
			subOptions = pgpBuildOptionListArgs( context,
				FALSE, firstOption, args );
		va_end( args );

		pgpClearMemory( &descriptor, sizeof( descriptor ) );
		
		descriptor.serverList			= (PGPKeyServerSpec *) serverList;
		descriptor.serverCount 			= serverCount;
		descriptor.tlsContext			= tlsContext;
		descriptor.searchBeforeDisplay	= searchBeforeDisplay;
		descriptor.foundKeys			= foundKeys;
		
		if( IsntPGPError( CopyPGPOUIKeyServerParamsDesc( context, &descriptor,
					&allocatedDesc ) ) )
		{
			PGPOptionValue		value;
			
			value.asPtr = allocatedDesc;

			optionList = pgpCreateCustomValueOptionList( context, 
									kPGPOptionType_KeyServerUpdateParams,
									kPGPOptionFlag_Default, &value,
									sizeof( *allocatedDesc ), subOptions,
									KeyServerParamsOptionHandlerProc );
		}
		else
		{
			PGPFreeOptionList( subOptions );
			optionList = kPGPOutOfMemoryOptionListRef;
		}
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		optionList = kPGPBadParamsOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPOUIRecipientList(
	PGPContextRef 		context,
	PGPUInt32			*recipientCount,
	PGPRecipientSpec	**recipientList)
{
	PGPOUIRecipientListDesc	desc;
	PGPOptionListRef		optionList;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( recipientCount ) );
	pgpValidateOptionParam( IsntNull( recipientList ) );
	
	desc.recipientCount	= recipientCount;
	desc.recipientList	= recipientList;
	
	optionList = pgpCreateBufferOptionList( context,
								kPGPOptionType_RecipientList,
								&desc, sizeof( desc ) );
								
	return( optionList );
}
