/*____________________________________________________________________________
    pgpLanguage.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    $Id: pgpLanguage.c,v 1.2 1999/05/12 21:01:05 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include "pgpBase.h"

char *LANG(char *x) 
{ 
	return x; 
}

char * get_ext_c_ptr(void) 
{ 
	return NULL; 
}

void init_charset(void) 
{ 
	;
}

int set_CONVERSION() 
{ 
	return 0; 
}

int get_CONVERSION() 
{ 
	return 0; 
}

void CONVERT_TO_CANONICAL_CHARSET(char *s) 
{ 
	;
}

char EXT_C(char c) 
{ 
	return c; 
}

char INT_C(char c) 
{ 
	return c; 
}


