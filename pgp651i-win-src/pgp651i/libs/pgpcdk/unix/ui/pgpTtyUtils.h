/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpTtyUtils.h,v 1.2 1999/03/10 02:59:25 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpTtyUtils_h
#define Included_pgpTtyUtils_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

int pgpTtyGetPass(FILE *fp, char *buffer, int buflen);
int pgpTtyGetString(char *buf, int len, FILE *echo);

PGP_END_C_DECLARATIONS

#endif
