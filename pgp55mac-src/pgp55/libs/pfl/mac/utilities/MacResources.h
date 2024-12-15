/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacResources.h,v 1.6 1997/09/18 01:34:55 lloyd Exp $
____________________________________________________________________________*/

#pragma once

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



OSStatus	ForceResError(OSStatus defaultError);

OSStatus	AddWriteResource(Handle theResource,
				OSType resourceType, short resourceID,
				ConstStr255Param resourceName);
OSStatus	AddWriteResourcePtr(const void * resourceData,
				ByteCount resourceDataSize,
				OSType resourceType, short resourceID,
				ConstStr255Param resourceName);
						
OSStatus	CopyResource(short sourceResFile, OSType sourceResourceType,
				short sourceResourceID,
				short destResFile,
				OSType destResourceType, short destResourceID);
						
OSStatus	Remove1Resource(OSType resourceType, short resourceID,
						Boolean removeProtectedResources);
						
Boolean		ResourceExists(ResType resourceType, short resourceID,
				Boolean inCurResOnly);




#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS