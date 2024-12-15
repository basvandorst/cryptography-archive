/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once



const char *FindMatchingBytes( const char * buffer, ulong length,
				const char * findIt, ulong findItLength );
					
Boolean		FindBlock( const void * inBuffer, const ulong inBufferSize,
				ConstStr255Param startLine, ConstStr255Param endLine,
				ulong * encryptedBlockStart, ulong * encryptedBlockSize );
	
Boolean		FindSignedBlock( const void * inBuffer, const ulong inBufferSize,
				ulong * encryptedBlockStart, ulong * encryptedBlockSize );
Boolean		FindEncryptedBlock( const void * inBuffer,
				const ulong inBufferSize, ulong * encryptedBlockStart,
				ulong * encryptedBlockSize );
Boolean		FindPGPKeyBlock( const void * inBuffer, const ulong inBufferSize,
				ulong * encryptedBlockStart, ulong * encryptedBlockSize );
