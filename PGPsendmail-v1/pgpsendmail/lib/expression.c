/*LINTLIBRARY*/
/*  expression.c

    This code provides shell-like expression expansion.

    Copyright (C) 1997  Richard Gooch

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Richard Gooch may be reached by email at  karma-request@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.
*/

/*  This file contains shell-like expression expansion routines.


    Written by      Richard Gooch   9-OCT-1997

    Last updated by Richard Gooch   9-OCT-1997


*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <karma.h>
#ifdef __KARMA__
#  include <karma_st.h>
#  include <karma_r.h>
#endif

#define BUF_SIZE 16384
#ifndef __KARMA__
#  define r_getenv getenv
#endif

/*  Public functions follow  */

/*EXPERIMENTAL_FUNCTION*/
flag st_expr_expand (char *output, unsigned int length, CONST char *input,
		     FILE *errfp)
/*  [SUMMARY] Expand an expression using shell-like unquoted rules.
    <output> The output expanded expression is written here.
    <length> The size of the output buffer.
    <input> The input expression. This may equal <<output>>.
    <errfp> Diagnostic messages are written here. If this is NULL the global
    stderr is used instead.
    [RETURNS] TRUE on success, else FALSE.
*/
{
    char ch;
    unsigned int len;
    unsigned int out_pos = 0;
    char *env;
    CONST char *ptr;
    struct passwd *pwent;
    char buffer[BUF_SIZE], tmp[STRING_LENGTH];
    static char function_name[] = "st_expr_expand";

    if (errfp == NULL) errfp = stderr;
    if (length > BUF_SIZE) length = BUF_SIZE;
    for (; TRUE; ++input)
    {
	switch (ch = *input)
	{
	  case '$':
	    /*  Variable expansion  */
	    ch = input[1];
	    if (ch == '$')
	    {
		/*  PID  */
		++input;
		sprintf ( tmp, "%d", getpid () );
		len = strlen (tmp);
		if (len + out_pos >= length)
		{
		    fprintf (errfp, "%s: output buffer too small\n",
			     function_name);
		    return (FALSE);
		}
		memcpy (buffer + out_pos, tmp, len + 1);
		out_pos += len;
		continue;
	    }
	    if (ch == '{') ch = (++input)[1];
	    for (ptr = ++input; isalnum (ch) || (ch == '_'); ch = *++ptr);
	    len = ptr - input;
	    if (len >= sizeof tmp)
	    {
		fprintf (errfp, "%s: username buffer too small\n",
			 function_name);
		return (FALSE);
	    }
	    memcpy (tmp, input, len);
	    tmp[len] = '\0';
	    if (ch == '}') ++ptr;
	    input = ptr - 1;
	    if ( ( env = r_getenv (tmp) ) == NULL )
	    {
		fprintf (errfp, "%s: environment variable: \"%s\" not found\n",
			 function_name, tmp);
		return (FALSE);
	    }
	    len = strlen (env);
	    if (len + out_pos >= length)
	    {
		fprintf (errfp, "%s: output buffer too small\n",
			 function_name);
		return (FALSE);
	    }
	    memcpy (buffer + out_pos, env, len + 1);
	    out_pos += len;
	    break;
	  case '~':
	    /*  Home directory expansion  */
	    ch = input[1];
	    if ( isspace (ch) || (ch == '/') || (ch == '\0') )
	    {
		/* User's own home directory: leave separator for next time */
		if ( ( env = r_getenv ("HOME") ) == NULL )
		{
		    fprintf (errfp,
			     "%s: environment variable: \"HOME\" not found\n",
			     function_name);
		    return (FALSE);
		}
		len = strlen (env);
		if (len + out_pos >= length)
		{
		    fprintf (errfp, "%s: output buffer too small\n",
			     function_name);
		    return (FALSE);
		}
		memcpy (buffer + out_pos, env, len + 1);
		out_pos += len;
		continue;
	    }
	    /*  Someone else's home directory  */
	    for (ptr = ++input; !isspace (ch) && (ch != '/') && (ch != '\0');
		 ch = *++ptr);
	    len = ptr - input;
	    if (len >= sizeof tmp)
	    {
		fprintf (errfp, "%s: username buffer too small\n",
			 function_name);
		return (FALSE);
	    }
	    memcpy (tmp, input, len);
	    tmp[len] = '\0';
	    input = ptr - 1;
	    if ( ( pwent = getpwnam (tmp) ) == NULL )
	    {
		fprintf (errfp, "%s: error getting pwent for user: \"%s\"\n",
			 function_name, tmp);
		return (FALSE);
	    }
	    len = strlen (pwent->pw_dir);
	    if (len + out_pos >= length)
	    {
		fprintf (errfp, "%s: output buffer too small\n",function_name);
		return (FALSE);
	    }
	    memcpy (buffer + out_pos, pwent->pw_dir, len + 1);
	    out_pos += len;
	    break;
	  case '\0':
	  default:
	    if (out_pos >= length)
	    {
		fprintf (errfp, "%s: output buffer too small\n",function_name);
		return (FALSE);
	    }
	    buffer[out_pos++] = ch;
	    if (ch == '\0')
	    {
		memcpy (output, buffer, out_pos);
		return (TRUE);
	    }
	    break;
	}
    }
    return (FALSE);
}   /*  End Function st_expr_expand  */
