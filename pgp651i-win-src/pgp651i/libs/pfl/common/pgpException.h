/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: pgpException.h,v 1.4 1999/03/25 18:02:29 melkins Exp $
____________________________________________________________________________*/

#ifndef Included_pgpException_h
#define Included_pgpException_h

#include "pgpPFLErrors.h"

#define ThrowPGPError_(x)	throw(((PGPError) (x)))
#define ThrowIfPGPError_(x)	if (IsPGPError((x))) ThrowPGPError_(x)

#endif
