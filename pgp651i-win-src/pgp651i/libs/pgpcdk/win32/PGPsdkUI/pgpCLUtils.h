/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpCLUtils.h,v 1.2 1999/03/10 02:47:40 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpCLUtils_h
#define Included_pgpCLUtils_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

int pgpCLGetPass(FILE *fp, char *buffer, int buflen);
int pgpCLGetString(char *buf, int len, FILE *echo);

PGP_END_C_DECLARATIONS

#endif
