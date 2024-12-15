#include <stdio.h>
#include "gmp_fatal.h"

/*
 * Separated out into own file so users may shadow this with their
 * own error handler.  This function should not return.
 */

#if	__STDC__
void		gmp_fatal(int gmp_errno, const char *free_form_msg)
#else
void		gmp_fatal(gmp_errno, free_form_msg)
int		gmp_errno;
const char	*free_form_msg;
#endif
{
	fprintf(stderr,"gmp: %s %s\n",gmp_errstr[gmp_errno],free_form_msg);
	abort();
}
