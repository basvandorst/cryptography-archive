/*____________________________________________________________________________
	MacResources.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacResources.c,v 1.8.10.1 1997/12/07 04:27:13 mhw Exp $
____________________________________________________________________________*/

#include <LowMem.h>
#include <Resources.h>

#include "MacDebug.h"
#include "MacMemory.h"
#include "MacResources.h"

/*____________________________________________________________________________
	AddWriteResource: Add a non-resource handle to a the current resource file
	and write the data to the file. The resource handle is detached if an error
	occurs. resourceName can be nil
____________________________________________________________________________*/

	OSStatus
AddWriteResource(
	Handle 				theResource,
	OSType 				resourceType,
	short 				resourceID,
	ConstStr255Param	resourceName)
{
	OSStatus	status;
	
	AssertHandleIsValid( theResource, "AddWriteResource" );
	AssertAddrNullOrValid( resourceName, ConstStr255Param, "AddWriteResource" );
	
	AddResource( theResource, resourceType, resourceID,
					IsntNull( resourceName ) ? resourceName : "\p" );

	status = ResError();
	AssertNoErr( status, nil );

	if( IsntErr( status ) )
	{
		WriteResource( theResource );
		
		status = ResError();
		AssertNoErr( status, nil );
		
		if( IsErr( status ) )
		{
			/*
			** Detach theResource in this case so the state of the Handle
			** is known upon function exit.
			*/
			
			DetachResource( theResource );
		}
	}
	
	return( status );
}

/*____________________________________________________________________________
	AddWriteResourcePtr: Add a data to a the current resource file
	and write the data to the file. 
____________________________________________________________________________*/

	OSStatus
AddWriteResourcePtr(
	const void *		resourceData,
	ByteCount			resourceDataSize,
	OSType 				resourceType,
	short 				resourceID,
	ConstStr255Param	resourceName)
{
	OSStatus	status;
	Handle		theResource;
	
	pgpAssertAddrValid( resourceData, char );
	AssertAddrNullOrValid( resourceName, ConstStr255Param, nil );
	
	theResource = pgpNewHandle( resourceDataSize, kMacMemory_UseCurrentHeap |
					kMacMemory_UseApplicationHeap | kMacMemory_UseSystemHeap );
	if( IsntNull( theResource ) )
	{
		BlockMoveData( resourceData, *theResource, resourceDataSize );
		
		status = AddWriteResource( theResource,
			resourceType, resourceID, resourceName );
		if( IsntErr( status ) )
		{
			UpdateResFile( CurResFile() );
			DetachResource( theResource );
		}
		
		pgpDisposeHandle( theResource );
	}
	else
	{
		status = memFullErr;
	}
	
	return( status );
}

/*____________________________________________________________________________
	ForceResError: GetResource can sometimes return a nil handle and set
	ResErr to noErr. This is a replacement function for ResError()
	which returns ResError() is is is not noErr and defaultError if it is.
____________________________________________________________________________*/

	OSStatus
ForceResError(OSStatus defaultError)
{
	OSStatus	status;
	
	status = ResError();
	if( IsntErr( status ) )
		status = defaultError;
		
	return( status );
}

/*____________________________________________________________________________
	Remove1Resource: Delete the specified resource from the current resource
	file.
____________________________________________________________________________*/

	OSStatus
Remove1Resource(
	OSType 	resourceType,
	short 	resourceID,
	Boolean removeProtectedResources)
{
	OSStatus	status;
	Handle		theResource;
	Boolean		saveResLoad;
	
	saveResLoad = LMGetResLoad();

	SetResLoad( FALSE );
		theResource = Get1Resource( resourceType, resourceID );
	SetResLoad( saveResLoad );
	
	if( IsntNull( theResource ) )
	{
		if( removeProtectedResources )
		{
			short	resourceAttributes;
	
			resourceAttributes = GetResAttrs( theResource );
			if( ( resourceAttributes & resProtected ) != 0 )
			{
				SetResAttrs( theResource, resourceAttributes &= ~resProtected );
			}
		}
		
		RemoveResource( theResource );

		status = ResError();
		AssertNoErr( status, nil );
		
		DisposeHandle( theResource );
	}
	else
	{
		status = ForceResError( resNotFound );
	}
		
	return( status );
}

/*____________________________________________________________________________
	CopyResource: Copy one resource from sourceResFile to destResFile.
	
	Note:
		* The copied resource may be decompressed if the source resource is
		compressed.
		* If a resource with the same type and ID already exists in the
		 destination,  it is blown away silently.
		* The resource will be detached from the source file only if it was
		in a purged  state. In this case, outstanding handles will be
		left dangling.
____________________________________________________________________________*/

	OSStatus
CopyResource(
	short		sourceResFile,
	OSType		sourceResourceType,
	short		sourceResourceID,
	short		destResFile,
	OSType		destResourceType,
	short		destResourceID)
{
	short		saveResFile;
	Boolean		saveResLoad;
	OSStatus	status;
	Handle		theResource;
	
	AssertFileRefNumIsValid( sourceResFile, "CopyResource" );
	AssertFileRefNumIsValid( destResFile, "CopyResource" );
	
	saveResFile = CurResFile();
	saveResLoad = LMGetResLoad();
	status		= noErr;

	UseResFile( sourceResFile );	

	SetResLoad( FALSE );
		theResource = Get1Resource( sourceResourceType, sourceResourceID );
	SetResLoad( TRUE );
	
	if( IsntNull( theResource ) )
	{
		Boolean	wasLoaded;
		
		/*
		** Check to see if the resource was already loaded. If it was, we will
		** not release the original resource.
		*/
		
		if( IsNull( *theResource ) )
		{
			wasLoaded = FALSE;
			
			LoadResource( theResource );
		}
		else
		{
			wasLoaded = TRUE;
		}
		
		if( IsntNull( *theResource ) )
		{	
			OSType	dummyResType;
			short	dummyResID;
			Str255	resourceName;
			SInt8	saveHandleState;
			
			saveHandleState = HGetState( theResource );	
			HNoPurge( theResource );

			GetResInfo( theResource, &dummyResID, &dummyResType, resourceName );
			
			status = ResError();
			AssertNoErr( status, nil );
			
			if( IsntErr( status ) )
			{
				Handle		resourceCopy;
				ByteCount	resourceSize;
				short		resourceAttributes;
				
				resourceSize 		= GetHandleSize( theResource );
				resourceAttributes	= GetResAttrs( theResource );
				
				resourceCopy = pgpNewHandle( resourceSize,
					kMacMemory_UseCurrentHeap |
					kMacMemory_UseApplicationHeap | kMacMemory_UseSystemHeap );
				if( IsntNull( resourceCopy ) )
				{
					BlockMoveData( *theResource, *resourceCopy, resourceSize );
				
					UseResFile( destResFile );
					
					(void) Remove1Resource( destResourceType,
							destResourceID, TRUE );
					
					status = AddWriteResource( resourceCopy,
							destResourceType, destResourceID, resourceName );
					if( IsntErr( status ) )
					{
						SetResAttrs( resourceCopy, resourceAttributes );
						
						status = ResError();
						AssertNoErr( status, nil );
						
						ReleaseResource( resourceCopy );
					}
					else
					{
						/*
						** If AddWriteResource returns an error,
						** then the handle is not in the  resource map.
						*/
						
						pgpDisposeHandle( resourceCopy );
					}
				}
				else
				{
					status = memFullErr;
				}
			}

			HSetState( theResource, saveHandleState );
		}
		else
		{
			status = ForceResError( memFullErr );
		}
		
		if( ! wasLoaded )
		{
			ReleaseResource( theResource );
		}
	}
	else
	{
		status = ForceResError( resNotFound );
	}	
	
	UseResFile( saveResFile );
	SetResLoad( saveResLoad );

	return( status );
}



/*____________________________________________________________________________

	ResourceExists: Checks to see if a resource exists in the current chain		
____________________________________________________________________________*/
	Boolean	
ResourceExists(
	ResType	resourceType,
	short	resourceID,
	Boolean	inCurResOnly)
{
	Boolean	result = false;
	Boolean	resLoad = LMGetResLoad();
	short	totalResources = 0;
	short	index = 1;
	Handle	resource;
	short	theID;
	ResType	theType;
	Str255	theName;
	
	SetResLoad( false );
	
	if ( inCurResOnly )
	{
		totalResources = Count1Resources( resourceType );
		while ( index <= totalResources )
		{
			resource = Get1IndResource( resourceType, index );
			if ( resource != nil )
			{
				GetResInfo( resource, &theID, &theType, theName );
				ReleaseResource( resource );
				if ( theID == resourceID )
				{
					result = true;
					break;
				}
			}
			index++;
		}
	}
	else
	{
		totalResources = CountResources( resourceType );
		while ( index <= totalResources )
		{
			resource = GetIndResource( resourceType, index );
			if ( resource != nil )
			{
				GetResInfo( resource, &theID, &theType, theName );
				ReleaseResource( resource );
				if ( theID == resourceID )
				{
					result = true;
					break;
				}
			}
			index++;
		}
	}
	
	SetResLoad( resLoad );
	
	return result;
}
