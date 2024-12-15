/*
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#ifndef _BLOCKUTILS_H
#define _BLOCKUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

const char *	FindMatchingBytes( const char * buffer, ulong length,
					const char * findIt, ulong findItLength );
					
Boolean	FindBlock( const void * inBuffer,  ulong inBufferSize,
			const char* startLine, const char* endLine,
			ulong * encryptedBlockStart, ulong * encryptedBlockSize );
	
Boolean	FindSignedBlock( const void * inBuffer,  ulong inBufferSize,
			ulong * encryptedBlockStart, ulong * encryptedBlockSize );

Boolean	FindEncryptedBlock( const void * inBuffer,  ulong inBufferSize,
			ulong * encryptedBlockStart, ulong * encryptedBlockSize );

Boolean	FindPGPKeyBlock( const void * inBuffer,  ulong inBufferSize,
			ulong * encryptedBlockStart, ulong * encryptedBlockSize );

unsigned long WrapBuffer(char **WrappedBuffer,
						 char *OriginalBuffer,
						 unsigned short WordWrapThreshold);

#ifdef __cplusplus
}
#endif


#endif
