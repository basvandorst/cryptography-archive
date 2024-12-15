/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: sdacast.c,v 1.3 1999/03/10 03:02:28 heller Exp $
____________________________________________________________________________*/

#include "DecodeStub.h"

// CAST5decryptCFBdbl decrypts the specified number of blocks in CFB mode.

	void 
CAST5decryptCFBdbl(
	const PGPUInt32*	xkey, 
	PGPUInt32			iv0, 
	PGPUInt32			iv1, 
	PGPUInt32			iv2, 
	PGPUInt32			iv3, 
	const PGPUInt32*	src, 
	PGPUInt32*			dest, 
	PGPUInt32			len)
{
	PGPUInt32	iv[4]	= {iv0, iv1, iv2, iv3};
	PGPUInt32	out[4];
	while (len--)
	{
		(cipherCAST5.encrypt)((void *)xkey,(void const *) iv, (void *) out);
		*dest++	= out[0] ^ (iv[0] = *src++);
		*dest++	= out[1] ^ (iv[1] = *src++);

		(cipherCAST5.encrypt)((void *)xkey,(void const *) (iv+2), (void *) (out+2));
		*dest++	= out[2] ^ (iv[2] = *src++);
		*dest++	= out[3] ^ (iv[3] = *src++);
	}
}

	void
DecryptBlock512(
	SDAHEADER				*SDAHeader,
	PGPUInt32				*ExpandedKey,
	PGPUInt32				blockNumber,
	const PGPUInt32 *		src,
	PGPUInt32 *				dest)
{
	const int kSplitPointInUInt32 = kBlockSizeInUInt32/2;
	const size_t kBytesBeforeSplit = kSplitPointInUInt32 * sizeof(PGPUInt32);
	const size_t kBytesAfterSplit = (kBlockSizeInUInt32 - kSplitPointInUInt32) *
					sizeof(PGPUInt32);
	int	i;
	PGPUInt32	iv0, iv1;

	/* This must be done in two halves to recover the IV */	
	CAST5decryptCFBdbl(ExpandedKey,
		src[ kSplitPointInUInt32 - 4 ], src[ kSplitPointInUInt32 - 3 ],
		src[ kSplitPointInUInt32 - 2 ], src[ kSplitPointInUInt32 - 1 ],
		src + kSplitPointInUInt32, dest + kSplitPointInUInt32,
				kBytesAfterSplit/(8 + 8));
	iv0 = dest[ kBlockSizeInUInt32 - 2 ];
	iv1 = dest[ kBlockSizeInUInt32 - 1 ];
	CAST5decryptCFBdbl(ExpandedKey, iv0, iv1, ~iv0,
			~iv1, src, dest, kBytesBeforeSplit/(8 + 8) );

	/* Undo the checksum */
	iv0	= SDAHeader->Salt.saltLongs[ 0 ] + blockNumber;
	iv1	= SDAHeader->Salt.saltLongs[ 1 ] + blockNumber;
	for (i = 0; i < kBlockSizeInUInt32 - 2; ++i)
	{
		iv1		+= dest[ i ];
		iv0		+= iv1;
	}

	dest[ kBlockSizeInUInt32 - 2 ]	^= iv0;
	dest[ kBlockSizeInUInt32 - 1 ]	^= iv1;
}

	
/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
