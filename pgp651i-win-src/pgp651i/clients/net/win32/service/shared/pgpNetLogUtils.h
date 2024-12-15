/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetLogUtils.h,v 1.4.6.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetLogUtils_h
#define _Included_pgpNetLogUtils_h

#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "pgpTypes.h"

// global logging utility functions
void 		voutputToDebugger(const char *fmt, 
							  va_list args,  
							  PGPBoolean eol = TRUE,
							  const char *prepend = NULL);
void 		outputToDebugger(const char *args, ...);
PGPError 	myStrerror(PGPInt32 errnum, char *buf, PGPInt32 *len);
void 		vprogerr(const char *fmt,
					 va_list args,
					 FILE *stream,
					 PGPBoolean eol = TRUE,
					 const char *prepend = NULL);


#endif	// _Included_pgpNetLogUtils_h
