/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpSharePriv.h,v 1.4 1999/03/10 02:55:54 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpSharePriv_h	/* [ */
#define Included_pgpSharePriv_h

#include "pgpShare.h"

PGP_BEGIN_C_DECLARATIONS

PGPError pgpCreateShares(PGPContextRef context, PGPKeyID keyID,
				PGPShareID shareID, PGPUInt32 threshold, PGPUInt32 numShares, 
				PGPUInt32 totalShares, PGPSize shareSize,
				const PGPByte *shareData, PGPShareRef *shares);

/* The share data needs to be freed with PGPFreeData(shareData) */
PGPError pgpGetShareData(PGPShareRef share, PGPByte **shareData, 
				PGPSize *shareDataSize);

/* The share hash needs to be freed with PGPFreeData(shareHash) */
PGPError pgpGetShareHash(PGPShareRef share, PGPByte **shareHash, 
				PGPSize *shareHashSize);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpSharePriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
