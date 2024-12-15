/*
 * pgpRegExp.h - regular expression package (see copyright info below)
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpRegExp.h,v 1.1 1997/08/13 01:32:51 hal Exp $
 */


/*
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 */

#include "pgpContext.h"


PGP_BEGIN_C_DECLARATIONS

/* Private context structure */
struct regexp;
#ifndef TYPE_regexp
#define TYPE_regexp 1
typedef struct regexp regexp;
#endif

/* Compile a regular expression */
PGPError pgpRegComp(PGPContextRef context, char const *exp, regexp **pregexp);

/* Match a regular expression against a string */
int pgpRegExec(regexp *prog, char const *string);


PGP_END_C_DECLARATIONS






/*__Editor_settings____

	Local Variables:
	tab-width: 4
	comment-column: 40
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
