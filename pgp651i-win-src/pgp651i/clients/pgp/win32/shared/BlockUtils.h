/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: BlockUtils.h,v 1.3 1999/03/10 02:31:36 heller Exp $
____________________________________________________________________________*/
#ifndef Included_BlockUtils_h	/* [ */
#define Included_BlockUtils_h

#include "PGPTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


const char *	
FindMatchingBytes(	const char * buffer, 
					ulong length,
					const char * findIt, 
					ulong findItLength );
					
PGPBoolean	
FindBlock(	const void * inBuffer,  
			ulong inBufferSize,
			const char* startLine, 
			const char* endLine,
			ulong * encryptedBlockStart, 
			ulong * encryptedBlockSize );
	
PGPBoolean	
FindSignedBlock(const void * inBuffer,  
				ulong inBufferSize,
				ulong * encryptedBlockStart, 
				ulong * encryptedBlockSize );

PGPBoolean	
FindEncryptedBlock(	const void * inBuffer,  
					ulong inBufferSize,
					ulong * encryptedBlockStart, 
					ulong * encryptedBlockSize );

PGPBoolean	
FindPGPKeyBlock(const void * inBuffer,  
				ulong inBufferSize,
				ulong * encryptedBlockStart, 
				ulong * encryptedBlockSize );


#ifdef __cplusplus
}
#endif



#endif /* ] Included_BlockUtils_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
