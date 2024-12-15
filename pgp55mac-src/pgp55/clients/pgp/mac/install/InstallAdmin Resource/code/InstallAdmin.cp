/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: InstallAdmin.cp,v 1.3.10.1 1997/12/05 22:15:28 mhw Exp $
____________________________________________________________________________*/

#define PGP_MACINTOSH		1

#include "pgpBase.h"
#include "MacResources.h"

#include "ExternCodeDefines.h"

#define	kAdminType	'ADMN'


	pascal void
main(
	ExternParmBlock *	eInfo)
{
	if (eInfo->selector == kSelectorAfterFile) {
		OSStatus	err = resNotFound;
		FSSpec		theFile;
		
		// Create the FSSpec
		::FSMakeFSSpec(	eInfo->theFile.af_vRefNum,
						eInfo->theFile.af_DirID,
						eInfo->theFile.af_Name,
						&theFile);

		// Attempt to get the resource
		if (ResourceExists(kAdminType, eInfo->refCon, true)) {
			Handle		theAdminResource;
			SInt16		refNum;
			
			// Get a handle to the resource
			::SetResLoad(false);
			theAdminResource = ::Get1Resource(kAdminType, eInfo->refCon);
			::SetResLoad(true);
			
			// Open the new file
			err = ::FSpOpenDF(	&theFile,
								fsRdWrPerm,
								&refNum);
			if (err == noErr) {
				SInt32		total = ::GetResourceSizeOnDisk(theAdminResource);
				SInt32		offset = 0;
				SInt32		blockSize = 1024;
				Ptr			theData = ::NewPtr(blockSize);
				
				err = MemError();
				if (err == noErr) {
					::SetEOF(	refNum,
								0);
					
					// Copy the resource to the file
					while (offset < total) {
						blockSize = ((total - offset) > 1024) ?
										1024 : (total - offset);
						::ReadPartialResource(	theAdminResource,
												offset,
												theData,
												blockSize);
						err = ResError();
						if (err == noErr) {
							offset += blockSize;
							err = ::FSWrite(	refNum,
												&blockSize,
												theData);
						}
						if (err != noErr) {
							break;
						}
					}
				}
				::DisposePtr(theData);
			}
			::FSClose(refNum);
		}
		
		// Delete the file if anything failed
		if (err != noErr) {
			::FSpDelete(&theFile);
		}
	}
}
