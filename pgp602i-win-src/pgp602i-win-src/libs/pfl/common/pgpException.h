/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: pgpException.h,v 1.2.16.1 1998/11/12 03:17:52 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpException_h
#define Included_pgpException_h

#include "pgpPFLErrors.h"

#define ThrowPGPError_(x)	throw(((PGPError) (x)))
#define ThrowIfPGPError_(x)	if (IsPGPError((x))) ThrowPGPError_(x)

#endif