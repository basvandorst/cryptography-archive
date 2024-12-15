/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "PGPldapLibUtils.h"
#include "pgpDebug.h"
#include "pgpMem.h"

	int
strncasecmp(const char *str1, const char *str2, unsigned nchars)
{
	// Taken from the MSL BeOS source
	
    int i;
    int diff;

    for (i = 0; i < nchars && *str1 && *str2; i++, str1++, str2++) {
        diff = toupper(*str1) - toupper(*str2);
        if (diff != 0) {
            return (diff);
        }
    }
    if (i == nchars) {
        return (0);
    }
    if (*str1 == 0) {
        return (-1);
    } else {
        return (1);
    }
}

// This implementation is from "getdn.c"

	char *
strdup(char *str)
{
	char	*p;

	if ( (p = (char *) malloc( strlen( str ) + 1 )) == NULL )
		return( NULL );

	strcpy( p, str );

	return( p );
}

// Replacements for Standard C memory allocation functions.
// Prototypes are inherently define in <stdlib.h> because of
// our macro substitution in the prefix file.

	void *
pgp_ldap_malloc(size_t size)
{
	return( pgpMemAlloc( size ) );
}

	void *
pgp_ldap_calloc(size_t nmemb, size_t size)
{
	void	*result;
	
	result = pgpMemAlloc( nmemb * size );
	if( IsntNull( result ) )
	{
		pgpClearMemory( result, nmemb * size );
	}
	
	return( result );
}

	void *
pgp_ldap_realloc(void * ptr, size_t size)
{
	return( pgpMemRealloc( ptr, size ) );
}
