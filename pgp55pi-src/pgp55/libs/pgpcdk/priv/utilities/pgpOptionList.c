/*____________________________________________________________________________
	pgpOptionList.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: pgpOptionList.c,v 1.50 1997/09/30 22:43:18 lloyd Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpContext.h"
#include "pgpOptionList.h"
#include "pgpEncodePriv.h"
#include "pgpErrors.h"

#if PGP_CLIENT_LIBRARY
#include "pgpClientLib.h"	/* No PFL dependencies for client library */
#else
#include "pgpDebug.h"
#include "pgpMem.h"
#endif

enum
{
	kPGPOptionList_Magic		= 0x4F4C7374,	/* 'OLst' */
	
	kPGPOptionList_MajorVersion	= 1,
	kPGPOptionList_MinorVersion	= 1,
	
	kPGPOptionList_NumGrowElements	= 5
};

enum
{
	kPGPOptionListFlag_PersistentList	= ( 1 << 0 )
};

struct PGPOptionList
{
	PGPUInt32		magic;			/* Always kPGPOptionList_Magic */
	PGPVersion		version;
	PGPContextRef	context;
	PGPError		err;			/* Important that this field is second */
	PGPFlags		flags;
	PGPUInt16		maxOptions;
	PGPUInt16		numOptions;
	
	PGPOption		*options;
};

	PGPBoolean
pgpOptionListIsReal(PGPConstOptionListRef optionList)
{
	return( optionList != kPGPEndOfArgsOptionListRef &&
			optionList != kPGPOutOfMemoryOptionListRef &&
			optionList != kPGPBadParamsOptionListRef &&
			optionList != kPGPNullOptionListRef );
}


	PGPError
pgpOptionListToError( PGPConstOptionListRef optionList )
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( optionList == kPGPEndOfArgsOptionListRef ||
		optionList == kPGPBadParamsOptionListRef ||
		optionList == kPGPNullOptionListRef )
	{
		err	= kPGPError_BadParams;
	}
	else if ( optionList == kPGPOutOfMemoryOptionListRef )
	{
		err	= kPGPError_OutOfMemory;
	}
	else
	{
		pgpAssert( pgpOptionListIsReal( optionList ) );
		err	= kPGPError_NoErr;
	}
	
	return( err );
}


#if 0
	static PGPBoolean
pgpOptionListIsPersistent(PGPConstOptionListRef optionList)
{
	PGPBoolean	isPersistent = FALSE;
	
	if( pgpOptionListIsReal( optionList ) )
	{
		if( ( optionList->flags & kPGPOptionListFlag_PersistentList ) != 0 )
			isPersistent = TRUE;
	}
	
	return( isPersistent );
}
#endif

	static PGPBoolean
pgpVerifyOptionList(PGPConstOptionListRef optionList)
{
	PGPBoolean	isValid = TRUE;
	
	if( pgpOptionListIsReal( optionList ) )
	{
		if( optionList->numOptions <= optionList->maxOptions )
		{
			PGPUInt32	optionIndex;
			PGPUInt32	prevOptionType = 0;
			
			for( optionIndex = 0; optionIndex < optionList->numOptions;
					++optionIndex )
			{
				const PGPOption	*curOption;
				
				curOption = &optionList->options[optionIndex];
				
				/* Verify the PGPOption record */
				if( (PGPUInt32) curOption->type < (PGPUInt32) prevOptionType ||
					curOption->type >= kPGPOptionType_LastType )
				{
					isValid = FALSE;
					break;
				}
				
				if( PGPRefIsValid( curOption->subOptions ) )
				{
					isValid = pgpVerifyOptionList( curOption->subOptions );
					if( ! isValid )
						break;
				}
				
				prevOptionType = curOption->type;
			}
		}
		else
		{
			isValid = FALSE;
		}
	}
			
	return( isValid );
}

/*____________________________________________________________________________
	This function validates a PGPOptionList data structure. It does not assert
	so it can be used within assert macros.
____________________________________________________________________________*/

	PGPBoolean
pgpOptionListIsValid(PGPConstOptionListRef optionList)
{
	PGPBoolean	isValid;
	
	if( pgpOptionListIsReal( optionList ) )
	{
		isValid = ( optionList != NULL &&
					optionList->magic == kPGPOptionList_Magic );

#if PGP_DEBUG	/* [ */
		if( isValid )
		{
			/* Perform more stringent check */
			isValid = pgpVerifyOptionList( optionList );
		}
#endif	/* ] */
	}
	else
	{
		isValid = TRUE;
	}
	
	return( isValid );
}

	PGPError
pgpGetOptionListError(PGPConstOptionListRef optionList)
{
	PGPError	err;
	
	PGPValidateParam( pgpOptionListIsValid( optionList ) );

	if( optionList == kPGPEndOfArgsOptionListRef ||
		optionList == kPGPNullOptionListRef )
	{
		err = kPGPError_NoErr;
	}
	else if( optionList == kPGPOutOfMemoryOptionListRef )
	{
		err = kPGPError_OutOfMemory;
	}
	else if( optionList == kPGPBadParamsOptionListRef )
	{
		err = kPGPError_BadParams;
	}
	else
	{
		err = optionList->err;
	}
	
	return( err );
}

	void
pgpSetOptionListError(
	PGPOptionListRef 	optionList,
	PGPError 			err)
{
	pgpAssert( pgpOptionListIsValid( optionList ) );

	if( pgpOptionListIsReal( optionList ) )
	{
		/* Set the err code iff there isn't already an err for the list */

		if( IsntPGPError( optionList->err ) )
		{
			optionList->err = err;
		}
	}
}

	static void
pgpFreeOptionListInternal(
	PGPOptionListRef 	optionList,
	PGPBoolean			freePersistentList)
{
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	if( pgpOptionListIsReal( optionList ) )
	{
		PGPBoolean	freeTheList = TRUE;
		
		if( ( optionList->flags & kPGPOptionListFlag_PersistentList ) != 0 )
		{
			freeTheList = freePersistentList;
		}

		if( freeTheList )
		{
			PGPUInt32	optionIndex;
			
			/*
			**	Free all of the allocations and suboption lists hanging
			**	off of the main option list
			*/
			
			for( optionIndex = 0; optionIndex < optionList->numOptions;
						optionIndex++ )
			{
				PGPOption	*curOption = &optionList->options[optionIndex];
				
				if( IsntNull( curOption->handlerProc ) )
				{
					(*curOption->handlerProc)(
								optionList->context,
								kPGPOptionHandler_FreeDataOperation,
								curOption->type,
								curOption->value, curOption->valueSize,
								NULL, NULL );
				}
			
				if( IsntNull( curOption->subOptions ) )
				{
					pgpAssert( ( curOption->subOptions->flags & 
							kPGPOptionListFlag_PersistentList ) == 0 );
							
					pgpFreeOptionListInternal( curOption->subOptions, TRUE );
				}
			}
			
			pgpContextMemFree( optionList->context, optionList->options );
			pgpContextMemFree( optionList->context, optionList );
		}
	}
}

	void
pgpFreeOptionList(PGPOptionListRef optionList)
{
	pgpFreeOptionListInternal( optionList, TRUE );
}

	PGPOptionListRef
pgpNewOptionList(
	PGPContextRef	context,
	PGPBoolean		createPersistentList)
{
	PGPOptionListRef	optionList;
	
	optionList = (PGPOptionListRef)
		pgpContextMemAlloc( context, sizeof( *optionList ), 0);
	if( IsntNull( optionList ) )
	{
		pgpClearMemory( optionList, sizeof( *optionList ) );

		optionList->options = (PGPOption *)
			pgpContextMemAlloc( context,  sizeof( PGPOption ), 0);
		if( IsntNull( optionList->options ) )
		{
			pgpClearMemory( optionList->options, sizeof( PGPOption ) );

			optionList->magic 		= kPGPOptionList_Magic;
			optionList->maxOptions	= 1;
			optionList->context		= context;

			optionList->version.majorVersion	= kPGPOptionList_MajorVersion;
			optionList->version.minorVersion	= kPGPOptionList_MinorVersion;
			
			if( createPersistentList )
				optionList->flags |= kPGPOptionListFlag_PersistentList;
		}
		else
		{
			pgpContextMemFree( optionList->context, optionList );
			optionList = kPGPOutOfMemoryOptionListRef;
		}
	}
	else
	{
		/*
		** We ran out of memory allocating the PGPOptionList (very rare
		** case). Point at the preallocated option list in the context
		** to handle this case.
		*/
		
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

/*____________________________________________________________________________
	Grow an existing PGPOptionList by a specified number of PGPOption
	elements. The new elements are cleared out.
____________________________________________________________________________*/

	static PGPError
pgpGrowOptionList(
	PGPOptionListRef	optionList,
	PGPUInt32			numNewOptions)
{
	PGPError	err;
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	pgpAssert( numNewOptions > 0 );

	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		PGPUInt32		existingListSize;
		PGPUInt32		newListSize;
		
		existingListSize 	= sizeof( PGPOption ) * optionList->maxOptions;
		newListSize 		= existingListSize + ( sizeof( PGPOption ) *
									numNewOptions );
		
		err = pgpContextMemRealloc( optionList->context,
					(void **)&optionList->options, newListSize, 0);
		if( IsntPGPError( err ) )
		{
			/*
			** Delete existing option list direct allocation only. Sub-option
			** lists are still valid.
			*/
			
			pgpClearMemory( ((char *)optionList->options) + existingListSize,
				newListSize - existingListSize );
						
			optionList->maxOptions += numNewOptions;
		}
	}
	
	return( err );
}

/*____________________________________________________________________________
	Returns the index of the insert location/first occurrence of an option
	type in an option list.
____________________________________________________________________________*/

	static PGPUInt32
pgpFindOptionListType(
	PGPOptionListRef	optionList,
	PGPOptionType		searchType)
{
	PGPUInt32	searchIndex;
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	pgpAssert( searchType > 0 );
	
	for( searchIndex = 0; searchIndex < optionList->numOptions; searchIndex++)
	{
		if( optionList->options[searchIndex].type >= searchType )
		{
			break;
		}
	}
	
	return( searchIndex );
}

	static PGPError
pgpCopyOption(
	PGPContextRef	context,
	const PGPOption	*sourceOption,
	PGPOption		*destOption)
{
	PGPError	err = kPGPError_NoErr;
	
	pgpAssertAddrValid( sourceOption, PGPOption );
	pgpAssertAddrValid( destOption, PGPOption );

	*destOption 			= *sourceOption;
	destOption->subOptions	= kPGPInvalidRef;
	destOption->handlerProc = NULL;
	
	if( IsntNull( sourceOption->handlerProc ) )
	{
		/* We have a handler proc, so a deep copy is possibly needed. */

		err = (*sourceOption->handlerProc)(
					context,
					kPGPOptionHandler_CopyDataOperation,
					sourceOption->type,
					sourceOption->value, sourceOption->valueSize,
					&destOption->value, &destOption->valueSize );
		if( IsntPGPError( err ) )
		{
			destOption->handlerProc = sourceOption->handlerProc;
		}
	}

	if( IsntPGPError( err ) && PGPRefIsValid( sourceOption->subOptions ) )
	{
		destOption->subOptions = pgpCopyOptionList( sourceOption->subOptions );
		err = pgpGetOptionListError( destOption->subOptions );
	}
	
	return( err );
}

/*____________________________________________________________________________
	Add a single PGPOption to a PGPOptionList. The option is added such that
	the options in the list are kept in ascending type order. No policy is
	enforced at this point about which options are duplicates.
____________________________________________________________________________*/

	static PGPError
pgpAddOptionToPGPOptionList(
	PGPOptionListRef	optionList,
	const PGPOption		*newOption)
{
	PGPError	err;

	pgpAssert( pgpOptionListIsValid( optionList ) );
	pgpAssertAddrValid( newOption, PGPOption );

	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		/* Do we need to grow the list? */
		if( optionList->numOptions == optionList->maxOptions )
		{
			err = pgpGrowOptionList( optionList,
								kPGPOptionList_NumGrowElements );
		}
	
		if( IsntPGPError( err ) )
		{
			PGPUInt32	insertIndex;
			PGPUInt32	bytesToMove;
			PGPOption	addedOption;
			
			addedOption = *newOption;
			
			addedOption.flags &= ~(kPGPOptionFlag_Recognized |
									kPGPOptionFlag_Frozen );
			
			insertIndex = pgpFindOptionListType( optionList,
						addedOption.type );
			bytesToMove = ( optionList->numOptions - insertIndex ) *
						sizeof( optionList->options[0] );
			if( bytesToMove != 0 )
			{		
				pgpCopyMemory( &optionList->options[insertIndex],
						&optionList->options[insertIndex + 1],
						bytesToMove );
			}
			
			optionList->options[insertIndex] = addedOption;
			++optionList->numOptions;
		}
	}
	
	pgpSetOptionListError( optionList, err );
	
	return( err );
}


	PGPOptionListRef
pgpCopyOptionList(PGPConstOptionListRef optionList) 
{
	PGPOptionListRef	newOptionList;
	PGPError			err;
	
	newOptionList = pgpNewOptionList( optionList->context, FALSE );

	err = pgpGetOptionListError( newOptionList );	
	if( IsntPGPError( err ) )
	{
		err = pgpGetOptionListError( optionList );
		if( IsntPGPError( err ) )
		{
			PGPUInt32	optionIndex;
			
			for( optionIndex = 0; optionIndex < optionList->numOptions;
					optionIndex++)
			{
				PGPOption	newOption;
				
				err = pgpCopyOption( optionList->context,
							&optionList->options[optionIndex],
							&newOption );
				if( IsntPGPError( err ) )
				{
					err = pgpAddOptionToPGPOptionList( newOptionList,
								&newOption );
				}
				
				if( IsPGPError( err ) )
					break;
			}
		}
	}
	
	pgpSetOptionListError( newOptionList, err );
	
	return( newOptionList );
}

/*____________________________________________________________________________
	Merge all of the OptionList records from sourceOptionList into
	destOptionList and dispose of sourceOptionList if it is not a persistent
	list. Assumes both option lists are real.
____________________________________________________________________________*/

	static PGPError
pgpMergeOptionLists(
	PGPOptionListRef	sourceOptionList,
	PGPOptionListRef	destOptionList)
{
	PGPError	err = kPGPError_NoErr;
	
	pgpAssert( pgpOptionListIsValid( sourceOptionList ) );
	pgpAssert( pgpOptionListIsReal( sourceOptionList ) );
	pgpAssert( pgpOptionListIsValid( destOptionList ) );
	pgpAssert( pgpOptionListIsReal( destOptionList ) );
	
	err = pgpGetOptionListError( sourceOptionList );
	if( IsntPGPError( err ) )
	{
		err = pgpGetOptionListError( destOptionList );
		if( IsntPGPError( err ) )
		{
			PGPUInt32	optionIndex;
			
			if((sourceOptionList->flags & kPGPOptionListFlag_PersistentList)
					!= 0)
			{
				/* Create a non-persistent copy of the source list */
				sourceOptionList = pgpCopyOptionList(
							sourceOptionList );
				err = pgpGetOptionListError( sourceOptionList );
			}
			
			if( IsntPGPError( err ) )
			{
				for( optionIndex = 0;
						optionIndex < sourceOptionList->numOptions;
						++optionIndex )
				{
					PGPOption	*curSourceOption;
					
					curSourceOption = &sourceOptionList->options[optionIndex];
					
					err = pgpAddOptionToPGPOptionList( destOptionList,
								curSourceOption );
					if( IsntPGPError( err ) )
					{
					/*
					**	We have "moved" the option to the destination list.
					**	At this point we need to clear the option in the
					** 	source list so that allocated resources are not
					**	disposed when the list is freed. Those resources
					**	are now owned by the destination list.
					*/
						
						pgpClearMemory( curSourceOption,
							sizeof(*curSourceOption));
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	
	pgpFreeOptionListInternal( sourceOptionList, FALSE );
	pgpSetOptionListError( destOptionList, err );
	
	return( err );
}

/*____________________________________________________________________________
	Compose a new option list which is built from one or more component lists.
	The component lists are deleted.
____________________________________________________________________________*/

	PGPError
pgpAppendOptionListArgs(
	PGPOptionListRef	optionList,
	PGPOptionListRef	firstOption,
	va_list 			remainingOptions) 
{
	PGPError			err;
	PGPOptionListRef	curOptionList;
	
	/* Initialize err to the initial state of the list */
	err = pgpGetOptionListError( optionList );

	/* Always process all arguments even though the list may already be
	   in error. We do this because we may need to dispose of an input */
		
	curOptionList = firstOption;
	while( curOptionList != kPGPEndOfArgsOptionListRef )
	{
		if( curOptionList != kPGPNullOptionListRef )
		{
			pgpAssert( pgpOptionListIsValid( curOptionList ) );
			
			if( pgpOptionListIsValid( curOptionList ) )
			{
				PGPBoolean	freeCurOptionList = TRUE;
				
				if( IsntPGPError( err ) )
				{
					err = pgpGetOptionListError( curOptionList );
					if( IsntPGPError( err ) )
					{
						err = pgpMergeOptionLists(
									curOptionList,
									optionList );
						
						freeCurOptionList = FALSE;
					}
				}
				
				if( freeCurOptionList )
				{
					pgpFreeOptionListInternal( curOptionList, FALSE );
				}
			}
			else if( IsntPGPError( err ) )
			{
				err = kPGPError_BadParams;
			}
		}
		
		curOptionList = va_arg( remainingOptions, PGPOptionListRef );
	}
	
	pgpSetOptionListError( optionList, err );
	
	return( err );
}


/*____________________________________________________________________________
	Compose a new option list which is built from one or more component lists.
	The component lists are deleted.
____________________________________________________________________________*/
	PGPError
pgpFreeVarArgOptionList(
	PGPOptionListRef	firstOption,
	va_list				args) 
{
	PGPError			err	= kPGPError_NoErr;
	PGPOptionListRef	curOptionList;

	/* Always process all arguments even though the list may already be
	   in error. We do this because we may need to dispose of an input */
		
	curOptionList = firstOption;
	while( curOptionList != kPGPEndOfArgsOptionListRef )
	{
		if( curOptionList != kPGPNullOptionListRef )
		{
			pgpAssert( pgpOptionListIsValid( curOptionList ) );
			if( pgpOptionListIsValid( curOptionList ) )
			{
				pgpFreeOptionListInternal( curOptionList, FALSE );
			}
			else if( IsntPGPError( err ) )
			{
				if ( IsntPGPError( err ) )
					err = kPGPError_BadParams;
			}
		}
		
		curOptionList = va_arg( args, PGPOptionListRef );
	}
	
	return( err );
}


/*____________________________________________________________________________
	Compose a new option list which is built from one or more component lists.
	The component lists are deleted.
____________________________________________________________________________*/

	PGPOptionListRef
pgpBuildOptionListArgs(
	PGPContextRef		context,
	PGPBoolean			createPersistentList,
	PGPOptionListRef	firstOption,
	va_list 			remainingOptions) 
{
	PGPOptionListRef	newOptionList;
	
	/* Note that pgpNewOptionList can never return NULL */
	newOptionList = pgpNewOptionList( context, createPersistentList );
	
	pgpAppendOptionListArgs( newOptionList, firstOption, remainingOptions );
	
	return( newOptionList );
}




/*____________________________________________________________________________
	Find the first occurrence of an option in a list and return its value
	and how many options exist of that type.
____________________________________________________________________________*/

	PGPError
pgpGetIndexedOption(
	PGPContextRef		context,
	PGPOptionListRef	optionList,
	PGPOptionType		optionType,
	PGPUInt32			searchIndex,
	PGPBoolean			markAsRecognized,
	PGPOption			*optionData,
	PGPUInt32			*numFoundOptions)
{
	PGPUInt32	optionIndex;
	PGPError	err;
	PGPUInt32	foundOptions;
	
	(void) context;
	
	if( IsntNull( numFoundOptions ) )
		*numFoundOptions	= 0;
		
	PGPValidateParam( pgpOptionListIsValid( optionList ) );
	
	foundOptions 	= 0;
	err 			= kPGPError_OptionNotFound;
	
	optionIndex = pgpFindOptionListType( optionList, optionType );
	if( optionList->options[optionIndex].type == optionType )
	{
		PGPOption	*curOption;
		
		/* Found first occurrence of this type */
		
		curOption = &optionList->options[optionIndex];
		while( optionIndex < optionList->numOptions &&
				curOption->type == optionType )
		{
			if( searchIndex == foundOptions )
			{
				if( IsntNull( optionData ) )
				{
					pgpAssertAddrValid( optionData, PGPOption );
			
					if( markAsRecognized )
					{
						curOption->flags |= kPGPOptionFlag_Recognized;
					}
					
					*optionData = *curOption;
				}
				
				err = kPGPError_NoErr;
			}
			
			++optionIndex;
			++foundOptions;
			++curOption;
		}
	}
			
	if( IsntNull( numFoundOptions ) )
	{
		pgpAssertAddrValid( numFoundOptions, PGPUInt32 );
		*numFoundOptions = foundOptions;
	}
	
	return( err );
}


/*____________________________________________________________________________
    Test an option list and make sure all options are in the given set.
    The option set is an array of option numbers.  Returns kPGPError_BadParams
	if an option is not in the set.
____________________________________________________________________________*/

	PGPError
pgpCheckOptionsInSet(
	PGPContextRef		context,
	PGPOptionListRef	optionList,
	PGPOptionType const	*optionSet,
	PGPUInt32			optionSetSize)
{
	PGPUInt32	optionIndex;
	PGPUInt32	setIndex;
	PGPError	err	= kPGPError_NoErr;
	
	(void) context;
	
	PGPValidateParam( pgpOptionListIsValid( optionList ) );
	
	for( optionIndex = 0; optionIndex < optionList->numOptions; optionIndex++)
	{
		PGPOptionType type;

		type = optionList->options[optionIndex].type;

		for( setIndex = 0; setIndex < optionSetSize; setIndex++ )
		{
			if (type == optionSet[setIndex])
				break;
		}

		/* didn't find option in the list */
		if( setIndex == optionSetSize )
		{
			pgpDebugFmtMsg(( pgpaFmtPrefix,
				"Error: Illegal PGPO option #%ld",
				(long)type ));
		
			err	= kPGPError_BadParams;
			break;
		}
	}

	return err;
}



	PGPError
pgpAddJobOptionsArgs(
	PGPJobRef			job,
	PGPOptionListRef	firstOption,
	va_list				remainingOptions)
{
	PGPError			error;
	PGPOptionListRef	optionList;
	
	pgpAssert( IsntNull( job ) );
	
	optionList = pgpBuildOptionListArgs( job->context,
					FALSE, firstOption, remainingOptions );
	
	error = pgpGetOptionListError( optionList );
	if( IsntPGPError( error ) )
	{
		job->newOptionList = optionList;
	}
	
	return( error );
}

	PGPOptionListRef
pgpNewOneOptionList(
	PGPContextRef		context,
	const PGPOption	*	option)
{
	PGPOptionListRef	optionList;
	
	pgpAssertAddrValid( option, PGPOption );
	
	optionList = pgpNewOptionList( context, FALSE );
	if( IsntPGPError( pgpGetOptionListError( optionList ) ) )
	{
		(void) pgpAddOptionToPGPOptionList( optionList, option );
	}
		
	return( optionList );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
