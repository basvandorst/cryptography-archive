/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPCL.h,v 1.3 1999/03/10 02:47:25 heller Exp $
____________________________________________________________________________*/

#ifndef Included_PGPCL_h
#define Included_PGPCL_h

PGPError
pgpSigningPassphraseCL(PGPContextRef, CPGPSigningPassphraseDialogOptions *);

PGPError
pgpCollectRandomDataCL(PGPContextRef, CPGPRandomDataDialogOptions *);

PGPError
pgpKeyPassphraseCL(PGPContextRef, CPGPKeyPassphraseDialogOptions *);

PGPError
pgpPassphraseCL(PGPContextRef, CPGPPassphraseDialogOptions *);
#endif
