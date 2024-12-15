/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: PGPCL.h,v 1.2.10.1 1998/11/12 03:24:14 heller Exp $
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
