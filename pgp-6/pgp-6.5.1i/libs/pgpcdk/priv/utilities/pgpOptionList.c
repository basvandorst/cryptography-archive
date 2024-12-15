/*____________________________________________________________________________
	pgpOptionList.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpOptionList.c,v 1.68 1999/03/10 02:53:58 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpFileSpec.h"
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
pgpOptionListIsReal(PGPOptionListRef optionList)
{
	return( optionList != kPGPEndOfArgsOptionListRef &&
			optionList != kPGPOutOfMemoryOptionListRef &&
			optionList != kPGPBadParamsOptionListRef &&
			optionList != kPGPNullOptionListRef );
}


	PGPError
pgpOptionListToError( PGPOptionListRef optionList )
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

	static PGPBoolean
pgpVerifyOptionList(PGPOptionListRef optionList)
{
	PGPBoolean	isValid = TRUE;
	
	if( pgpOptionListIsReal( optionList ) )
	{
		if( optionList->numOptions <= optionList->maxOptions )
		{
			PGPUInt32	optionIndex;
			
			for( optionIndex = 0; optionIndex < optionList->numOptions;
					++optionIndex )
			{
				const PGPOption	*curOption;
				
				curOption = &optionList->options[optionIndex];
				
				if( ( (PGPInt32) curOption->type >=
							kPGPOptionType_FirstSDKOption &&
					  curOption->type < kPGPOptionType_LastSDKOption ) ||
					( curOption->type >= kPGPOptionType_FirstUIOption &&
					  curOption->type < kPGPOptionType_LastUIOption ) ||
					( curOption->type >= kPGPOptionType_FirstNetOption &&
					  curOption->type < kPGPOptionType_LastNetOption ) )
				{
					/* OK */
				}
				else
				{
					isValid = FALSE;
					break;
				}
				
				if( PGPOptionListRefIsValid( curOption->subOptions ) )
				{
					isValid = pgpVerifyOptionList( curOption->subOptions );
					if( ! isValid )
						break;
				}
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
pgpOptionListIsValid(PGPOptionListRef optionList)
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
pgpGetOptionListError(PGPOptionListRef optionList)
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

	PGPUInt32
pgpGetOptionListCount(PGPOptionListRef optionList)
{
	pgpAssert( pgpOptionListIsValid( optionList ) );
	
	return( optionList->numOptions );
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

	PGPError
pgpCopyOption(
	PGPContextRef	context,
	const PGPOption	*sourceOption,
	PGPOption		*destOption)
{
	PGPError	err = kPGPError_NoErr;
	
	pgpAssertAddrValid( sourceOption, PGPOption );
	pgpAssertAddrValid( destOption, PGPOption );

	*destOption 			= *sourceOption;
	destOption->subOptions	= kInvalidPGPOptionListRef;
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

	if( IsntPGPError( err ) &&
		PGPOptionListRefIsValid( sourceOption->subOptions ) )
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
			PGPOption	addedOption;
			
			addedOption = *newOption;
			
			addedOption.flags &= ~(kPGPOptionFlag_Recognized |
									kPGPOptionFlag_Frozen );
			
			optionList->options[optionList->numOptions] = addedOption;
			++optionList->numOptions;
		}
	}
	
	pgpSetOptionListError( optionList, err );
	
	return( err );
}


	PGPOptionListRef
pgpCopyOptionList(PGPOptionListRef optionList) 
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
pgpGetIndexedOptionType(
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
	PGPOption	*curOption;
	
	if( IsntNull( numFoundOptions ) )
		*numFoundOptions = 0;
		
	PGPValidateParam( pgpOptionListIsValid( optionList ) );
	
	foundOptions 	= 0;
	err				= kPGPError_OptionNotFound;
	curOption		= optionList->options;
	
	for(optionIndex = 0; optionIndex < optionList->numOptions; optionIndex++)
	{
		if( curOption->type == optionType )
		{
			if( foundOptions == searchIndex )
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
			
			++foundOptions;
		}
		
		++curOption;
	}
			
	if( IsntNull( numFoundOptions ) )
	{
		pgpAssertAddrValid( numFoundOptions, PGPUInt32 );
		*numFoundOptions = foundOptions;
	}
	
	return( err );
}

/*____________________________________________________________________________
	Get the Nth option in a list and return its value. Returns
	kPGPError_OptionNotFound if the index is out of range
____________________________________________________________________________*/

	PGPError
pgpGetIndexedOption(
	PGPOptionListRef	optionList,
	PGPUInt32			searchIndex,
	PGPBoolean			markAsRecognized,
	PGPOption			*optionData)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateParam( pgpOptionListIsValid( optionList ) );
	
	if( searchIndex < optionList->numOptions )
	{
		if( markAsRecognized )
		{
			optionList->options[searchIndex].flags |=
						kPGPOptionFlag_Recognized;
		}

		*optionData = optionList->options[searchIndex];
	}
	else
	{
		err = kPGPError_OptionNotFound;
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
	PGPOptionListRef	optionList,
	PGPOptionType const	*optionSet,
	PGPUInt32			optionSetSize)
{
	PGPUInt32	optionIndex;
	PGPUInt32	setIndex;
	PGPError	err	= kPGPError_NoErr;
	
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

/* Read an option, marking it read, and only allow one instance of it */

	PGPError
pgpSearchOptionSingle(
	PGPOptionListRef	 optionList,
	PGPOptionType		 optionType,
	PGPOption			*optionData)
{
	PGPUInt32			 numFound;
	PGPError			 err = kPGPError_NoErr;

	pgpa( pgpaAddrValid( optionData, PGPOption ) );
	optionData->type = kPGPOptionType_InvalidOption;

	numFound = 0;
	if( IsntNull( optionList ) ) {
		if( IsPGPError( err = pgpGetIndexedOptionType( optionList,
						   optionType, 0, TRUE, optionData, &numFound ) ) ) {
			if (err != kPGPError_OptionNotFound)
				return err;
			err = kPGPError_NoErr;
		}
	}
	if( numFound > 1 ) {
		err = kPGPError_RedundantOptions;
	}
	if ( numFound < 1 ) {
		optionData->type = kPGPOptionType_InvalidOption;
	}
	return err;
}

/* Read the nth (searchIndex'th) instance of an option, marking it read */

	PGPError
pgpSearchOption(
	PGPOptionListRef	 optionList,
	PGPOptionType		 optionType,
	PGPUInt32			 searchIndex,
	PGPOption			*optionData
	)
{
	PGPUInt32			 numFound;
	PGPError			 err = kPGPError_NoErr;

	pgpa( pgpaAddrValid( optionData, PGPOption ) );
	optionData->type = kPGPOptionType_InvalidOption;

	numFound = 0;
	if( IsntNull( optionList ) ) {
		if( IsPGPError( err = pgpGetIndexedOptionType( optionList,
				  optionType, searchIndex, TRUE, optionData, &numFound ) ) ) {
			if (err != kPGPError_OptionNotFound)
				return err;
			err = kPGPError_NoErr;
		}
	}
	if( searchIndex >= numFound ) {
		optionData->type = kPGPOptionType_InvalidOption;
	}
	return err;
}


/* Find an option and return its arguments if any.
 *
 * Takes a parameter string consisting of "%d%p%t" values, where %d means
 * a 32-bit scalar, %p means a pointer, and %t means a timeinterval.
 * These are packed in a special way in the optionlist code, which this
 * routine has to know about.  A special case is "%p%l", meaning a pointer
 * and length pair.
 * If the first two characters are %b then we return a boolean telling whether
 * the option was found.
 * If the fMandatory flag is set, returns an error if the option doesn't
 * exist.
 */

	PGPError
pgpFindOptionArgs(
	PGPOptionListRef	 optionList,
	PGPOptionType		 optionType,
	PGPBoolean			 fMandatory,
	const char			*str,
	...
	)
{
	va_list				args;
	PGPOption			op;
	PGPInt32			*ptrInt32;
	void				**ptrPtr;
	PGPTime				*ptrTime;
	PGPTimeInterval		*ptrTimeInterval;
	PGPSize				*ptrSize;
	PGPBoolean			*ptrBool;
	PFLFileSpecRef		*ptrFileSpec;
	PGPByte				*ptr;
	PGPError			err = kPGPError_NoErr;

	pgpa( pgpaAddrValid( str, char ) );

	if( IsPGPError( err = pgpSearchOptionSingle( optionList,
												 optionType, &op ) ) )
		goto error;

	va_start( args, str );

	/* Handle %b */
	if( str[0] == '%' && str[1] == 'b' ) {
		ptrBool = va_arg( args, PGPBoolean * );
		*ptrBool = IsOp( op );
		str += 2;
	}
	/* Handle other options */
	if( IsOp( op ) ) {
		if( strlen( str ) == 2 ) {
			/* Single options get stored in simple form */
			if (str[0] != '%') {
				err = kPGPError_BadParams;
				goto error;
			}
			switch( str[1] ) {
			case 'd':
				ptrInt32 = va_arg( args, PGPInt32 * );
				*ptrInt32 = op.value.asInt;
				break;
			case 'p':
				ptrPtr = va_arg( args, void ** );
				*ptrPtr = op.value.asPtr;
				break;
			case 't':
				ptrTimeInterval = va_arg( args, PGPTimeInterval * );
				*ptrTimeInterval = op.value.asInterval;
				break;
			case 'T':
				ptrTime = va_arg( args, PGPTime * );
				*ptrTime = op.value.asTime;
				break;
			case 'f':
				ptrFileSpec = va_arg( args, PFLFileSpecRef * );
				*ptrFileSpec = op.value.asFileRef;
				break;
			default:
				err = kPGPError_BadParams;
				goto error;
			}
			va_end( args );
		} else if( strcmp( str, "%p%l" ) == 0 ) {
			/* Special case, still uses simple form */
			ptrPtr = va_arg( args, void ** );
			*ptrPtr = op.value.asPtr;
			ptrSize = va_arg( args, PGPSize * );
			*ptrSize = op.valueSize;
			va_end( args );
		} else {
			/* Get data out of second-order block */
			ptr = ( PGPByte * ) op.value.asPtr;
			while( *str++ == '%' ) {
				switch( *str++ ) {
				case 'd':
					ptrInt32 = va_arg( args, PGPInt32 * );
					*ptrInt32 = *( ( PGPInt32 * ) ptr );
					ptr += sizeof( PGPInt32 );
					break;
				case 'p':
					ptrPtr = va_arg( args, void ** );
					*ptrPtr = *( ( void ** ) ptr );
					ptr += sizeof( void * );
					break;
				case 't':
					ptrTimeInterval = va_arg( args, PGPTimeInterval * );
					*ptrTimeInterval = *( ( PGPTimeInterval * ) ptr );
					ptr += sizeof( PGPTimeInterval );
					break;
				case 'T':
					ptrTime = va_arg( args, PGPTime * );
					*ptrTime = *( ( PGPTime * ) ptr );
					ptr += sizeof( PGPTime );
					break;
				case 'f':
					ptrFileSpec = va_arg( args, PFLFileSpecRef * );
					*ptrFileSpec = *( ( PFLFileSpecRef * ) ptr );
					ptr += sizeof( PFLFileSpecRef );
					break;
				default:
					err = kPGPError_BadParams;
					goto error;
				}
			}
			va_end( args );
		}
	} else {
		/* Missing op, fill in parameters */
		ptr = ( PGPByte * ) op.value.asPtr;
		while( *str++ == '%' ) {
			switch( *str++ ) {
			case 'd':
				ptrInt32 = va_arg( args, PGPInt32 * );
				*ptrInt32 = (PGPInt32) 0;
				break;
			case 'l':
				ptrSize = va_arg( args, PGPSize * );
				*ptrSize = (PGPSize) 0;
				break;
			case 'p':
				ptrPtr = va_arg( args, void ** );
				*ptrPtr = NULL;
				break;
			case 't':
				ptrTimeInterval = va_arg( args, PGPTimeInterval * );
				*ptrTimeInterval = (PGPTimeInterval) 0;
				break;
			case 'T':
				ptrTime = va_arg( args, PGPTime * );
				*ptrTime = (PGPTime) 0;
				break;
			case 'f':
				ptrFileSpec = va_arg( args, PFLFileSpecRef * );
				*ptrFileSpec = kInvalidPFLFileSpecRef;
				break;
			default:
				err = kPGPError_BadParams;
				goto error;
			}
		}
		if( fMandatory ) {
		
			pgpDebugFmtMsg(( pgpaFmtPrefix,
				"Error: Missing required PGPO option #%ld",
				(long)optionType ));

			err = kPGPError_OptionNotFound;
			goto error;
		}
	}
	return kPGPError_NoErr;
error:
	return err;
}


/* Delete option list and null out the pointer to it */

	void
pgpCleanupOptionList(PGPOptionListRef *optionList)
{
	/* XXX check to make sure we have no critical unused options */
	if( IsntNull( *optionList ) ) {
		pgpFreeOptionList( *optionList );
		*optionList = NULL;
	}
}

/********************* Access functions for option data *********************/


	PGPError
pgpOptionInt(
	PGPOption			*op,
	PGPInt32			*pInt
	)			 
{
	*pInt = op->value.asInt;
	return kPGPError_NoErr;
}

	PGPError
pgpOptionUInt(
	PGPOption			*op,
	PGPUInt32			*pUInt
	)			 
{
	*pUInt = op->value.asInt;
	return kPGPError_NoErr;
}

	PGPError
pgpOptionPtr(
	PGPOption			 *op,
	void				**pPtr
	)			 
{
	*pPtr = op->value.asPtr;
	return kPGPError_NoErr;
}

	PGPError
pgpOptionInterval(
	PGPOption			*op,
	PGPTimeInterval		*pInterval
	)			 
{
	*pInterval = op->value.asInterval;
	return kPGPError_NoErr;
}

/* Access option data consisting of a pointer and a length */
/* This is a special case; the length is in the option itself */
	PGPError
pgpOptionPtrLength(
	PGPOption			 *op,
	void				**pPtr,
	PGPSize				 *pLength
	)			 
{
	*pPtr = op->value.asPtr;
	*pLength = op->valueSize;
	return kPGPError_NoErr;
}

/* Access option data consisting of a pointer then a pointer */
	PGPError
pgpOptionPtrPtr(
	PGPOption			 *op,
	void				**pPtr,
	void				**pPtr2
	)			 
{
	PGPByte *ptr = ( PGPByte * ) op->value.asPtr;
	*pPtr = *( ( void ** ) ptr );
	ptr += sizeof( void * );
	*pPtr2 = *( ( void ** ) ptr );
	return kPGPError_NoErr;
}

/* Access option data consisting of a pointer, length, then pointer */
	PGPError
pgpOptionPtrLengthPtr(
	PGPOption			 *op,
	void				**pPtr,
	PGPSize				 *pLength,
	void				**pPtr2
	)			 
{
	PGPByte *ptr = ( PGPByte * ) op->value.asPtr;
	*pPtr = *( ( void ** ) ptr );
	ptr += sizeof( void * );
	*pLength = *( ( PGPSize * ) ptr );
	ptr += sizeof( PGPSize );
	*pPtr2 = *( ( void ** ) ptr );
	return kPGPError_NoErr;
}


/* Allocate and make a copy of a C String */

	char *
pgpAllocCString(
	PGPContextRef	 	 context,
	char const			*string
	)
{
	char				*ptr;
	PGPSize				 length;

	length = strlen( string );
	ptr = (char *)pgpContextMemAlloc( context, length+1, 0 );
	if( IsntNull( ptr ) ) {
		strcpy( ptr, string );
	}
	return ptr;
}


/*
**	This is the handler proc for sensitive options like passphrase
*/

	PGPError
pgpSensitiveOptionHandlerProc(
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
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			/* This wipes as it frees since it is a secure buffer */
			PGPFreeData( inputValue.asPtr );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			outputValue->asPtr = PGPNewSecureData(
					PGPGetContextMemoryMgr( context ), inputValueSize, 0);
			if( IsNull( outputValue->asPtr ) ) {
				err = kPGPError_OutOfMemory;
				break;
			}

			pgpCopyMemory( inputValue.asPtr, outputValue->asPtr,
						   inputValueSize );
			*outputValueSize = inputValueSize;
			
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

/*
**	This is the handler proc for options which take buffers but aren't
**	sensitive.
*/

	PGPError
pgpBufferOptionHandlerProc(
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
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			pgpContextMemFree( context, inputValue.asPtr );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			outputValue->asPtr = pgpContextMemAlloc( context,
													 inputValueSize, 0 );
			if( IsNull( outputValue->asPtr ) ) {
				err = kPGPError_OutOfMemory;
				break;
			}

			pgpCopyMemory( inputValue.asPtr, outputValue->asPtr,
						   inputValueSize );
			*outputValueSize = inputValueSize;
			
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

/*
**	This is the handler proc for options containing PGPFileRefs.
*/

	PGPError
pgpFileRefOptionHandlerProc(
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
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			err = PFLFreeFileSpec( (PFLFileSpecRef) inputValue.asFileRef );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			err = PFLCopyFileSpec( (PFLFileSpecRef) inputValue.asFileRef,
						(PFLFileSpecRef *) &outputValue->asFileRef );
			if( IsntPGPError( err ) )
			{
				*outputValueSize = inputValueSize;
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
pgpCreateCustomValueOptionList(
	PGPContextRef			context,
	PGPOptionType			type,
	PGPOptionFlags			flags,
	const PGPOptionValue	*value,
	PGPSize					valueSize,
	PGPOptionListRef		subOptions,
	PGPOptionHandlerProcPtr	handlerProc)
{
	PGPOptionListRef	optionList;
	PGPOption			option;
	
	pgpValidateOptionContext( context );
	
	option.type			= type;
	option.flags		= flags;
	option.valueSize	= valueSize;
	option.subOptions	= subOptions;
	option.handlerProc	= handlerProc;
	
	if( IsNull( value ) )
	{
		pgpAssert( option.valueSize == 0 );
		
		option.value.asPtr = NULL;
	}
	else
	{
		option.value = *value;
	}
	
	optionList = pgpNewOneOptionList( context, &option );
	
	if( pgpOptionListIsValid( subOptions ) )
	{
		/* Fetch the err from the subOptions list, if any */
		pgpSetOptionListError( optionList,
					pgpGetOptionListError( subOptions ) );
	}
	
	return( optionList );
}

	PGPOptionListRef
pgpCreateStandardValueOptionList( 
	PGPContextRef			context,
	PGPOptionType			type,
	const PGPOptionValue	*value,
	PGPSize					valueSize,
	PGPOptionHandlerProcPtr	handlerProc)
{
	return( pgpCreateCustomValueOptionList( context,
				type, kPGPOptionFlag_Default,
				value, valueSize, NULL, handlerProc ) );
}

	PGPOptionListRef
pgpCreateFileRefOptionList( 
	PGPContextRef	context,
	PGPOptionType	type,
	PGPFileSpecRef	fileRef)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pflFileSpecIsValid( (PFLFileSpecRef)fileRef ) );
	
	if( IsntPGPError( PFLCopyFileSpec(
			(PFLConstFileSpecRef) fileRef, (PFLFileSpecRef *) &value.asFileRef ) ) )
	{
		optionList = pgpCreateStandardValueOptionList( context,  
							type,
							&value, sizeof( fileRef ),
							pgpFileRefOptionHandlerProc );
	}
	else
	{
		/* Inability to copy a ref means we're out of memory */
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
pgpCreateSensitiveOptionList( 
	PGPContextRef		context,
	PGPOptionType		type,
	void const			*sensitiveData,
	PGPSize				sensitiveDataSize)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( sensitiveData ) );
	
	value.asPtr = PGPNewSecureData( PGPGetContextMemoryMgr( context ),
					sensitiveDataSize, 0 );
	if( IsntNull( value.asPtr ) )
	{
		pgpCopyMemory( sensitiveData, value.asPtr, sensitiveDataSize );
		optionList = pgpCreateStandardValueOptionList( context,  
							type,
							&value, sensitiveDataSize,
							pgpSensitiveOptionHandlerProc );
	}
	else
	{
		/* Inability to copy a ref means we're out of memory */
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

/* This is like pgpCreateSensitiveOptionList, but for non-sensitive data */
	PGPOptionListRef
pgpCreateBufferOptionList( 
	PGPContextRef		context,
	PGPOptionType		type,
	void const			*bufferData,
	PGPSize				bufferDataSize)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( bufferData ) );
		
	value.asPtr = pgpContextMemAlloc( context, bufferDataSize, 0 );
	if( IsntNull( value.asPtr ) )
	{
		pgpCopyMemory( bufferData, value.asPtr, bufferDataSize );
		optionList = pgpCreateStandardValueOptionList( context,  
							type,
							&value, bufferDataSize,
							pgpBufferOptionHandlerProc );
	}
	else
	{
		/* Inability to copy a ref means we're out of memory */
		optionList = kPGPOutOfMemoryOptionListRef;
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
