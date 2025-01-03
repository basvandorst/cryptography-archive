/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpPassphraseUtils.h,v 1.6.14.1 1998/11/12 03:18:42 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpPassphraseUtils_h	/* [ */
#define Included_pgpPassphraseUtils_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPUInt32	pgpEstimatePassphraseQuality(const char *passphrase );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpPassphraseUtils_h */
