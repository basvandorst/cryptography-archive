/*  strings.c

    Source file for string operations for  PGPsendmail  (wrapper to sendmail).

    Copyright (C) 1994  Richard Gooch

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Richard Gooch may be reached by email at  rgooch@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.
*/

/*  This programme intercepts messages sent by user mail agents to the
    sendmail daemon and checks to see if messages can be encrypted using the
    recipient's PGP public keys.


    Written by      Richard Gooch   31-MAY-1994

    Updated by      Richard Gooch   31-MAY-1994: Extracted from  pgpsendmail.c

    Updated by      Richard Gooch   10-JUN-1994: Copied  st_icmp  from the
  Karma  source/lib/karma/level2/strings.c  file, since some platforms don't
  have  stricmp  .

    Updated by      Richard Gooch   23-JUN-1994: Removed CONST declaration for
  st_istr  .

    Updated by      Richard Gooch   30-JUN-1994: Fixed bug in  st_nupr  where
  test for null character was performed prior to count test. This caused SEGV
  if  string[str_len]  was an invalid address.

    Last updated by Richard Gooch   19-JUL-1994: Added  #include <ctype.h>
  so that  toupper(3)  does the right thing under BSDI 1.1
  A "known problem" that is not documented.


*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "pgpsendmail.h"


int st_nicmp (string1, string2, str_len)
/*  This routine will compare two strings pointed to by  string1  and
    string2  .The string comparison is performed up to  str_len  characters
    long. The operation of this routine is similar to that of  strncmp  ,
    except that the comparison is case insensitive.
    The comparison value is returned.
*/
CONST char *string1;
CONST char *string2;
int str_len;
{
    int ret_value;
    char *str1 = NULL;
    char *str2 = NULL;
    static char function_name[] = "st_nicmp";

    if (string1 == string2)
    {
	return (0);
    }
    if (string1 == NULL)
    {
	return (-1);
    }
    if (string2 == NULL)
    {
	return (1);
    }

    if ( ( str1 = m_alloc (str_len) ) == NULL )
    {
	m_abort (function_name, "partial copy of string1");
    }
    strncpy (str1, string1, str_len);
    st_nupr (str1, str_len);

    if ( ( str2 = m_alloc (str_len) ) == NULL )
    {
	m_abort (function_name, "partial copy of string2");
    }
    strncpy (str2, string2, str_len);
    st_nupr (str2, str_len);
    ret_value = strncmp (str1, str2, str_len);
    m_free (str1);
    m_free (str2);
    return (ret_value);
}   /*  End Function st_nicmp   */

char *st_nupr (string, str_len)
/*  This routine will convert the string pointed to by  string  to uppercase.
    The conversion stops at the first NULL terminator character or if  str_len
    characters have been converted.
    The routine returns the address of the string.
*/
char *string;
int str_len;
{
    int count = 0;
    static char function_name[] = "st_nupr";

    if (string == NULL)
    {
	(void) fprintf (stderr, "NULL pointer passed\n");
	a_prog_bug (function_name);
    }
    while ( (count < str_len) && (*string != '\0') )
    {
	if (islower (*string) != 0)
	{
	    *string = toupper (*string);
	}
        ++string;
        ++count;
    }
    return (string);
}   /*  End Function st_nupr */

char *st_istr (string, substring)
/*  This routine will search for the first occurrence of a substring within a
    string. The search is case insensitive.
    The string to be search must be pointed to by  string  .
    The substring to be searched for must be pointed to by  substring  .
    The routine returns a pointer to the first occurrence on success, else it
    returns NULL.
*/
CONST char *string;
CONST char *substring;
{
    int len;

    len = strlen (substring);
    while (*string != '\0')
    {
	if (st_nicmp (string, substring, len) == 0) return ( (char *) string );
	++string;
    }
    return (NULL);
}   /*  End Function st_istr  */

char *ex_str (str, rest)
/*  This routine will extract a sub-string from a string. The sub-string may be
    delimited by any number of whitespace characters.
    The input string must be pointed to by  str. The routine will write a
    pointer to the next field in the input string to  rest  . If this is NULL,
    nothing is written here.
    The double quote character may appear anywhere in the sub-string, and will
    force all whitespace characters except '\n' into the output string. A
    second double quote character unquotes the previous quote. These double
    quote characters are not copied, unless they are consecutive.
    The routine returns a pointer to a copy of the sub-string.
*/
char *str;
char **rest;
{
    flag finished = FALSE;
    char quote = '\0';
    char *return_value;
    char *out_ptr;
    static char function_name[] = "ex_str";

    if ( (str == NULL) || (*str == '\0') )
    {
	if (rest != NULL)
	{
	    *rest = NULL;
	}
	return (NULL);
    }
    /*  Skip leading whitespace  */
    while ( isspace (*str) && (*str != '\0' ) )
    {
	++str;
    }
    if ( ( return_value = m_alloc (strlen (str) + 1) ) == NULL )
    {
	m_abort (function_name, "sub-string copy");
	return (NULL);
    }
    out_ptr = return_value;
    while (finished == FALSE)
    {
	switch (*str)
	{
	  case '\0':
	    *out_ptr = *str;
	    finished = TRUE;
	    break;
	  case ' ':
	  case '\t':
	    if (quote != '\0')
	    {
		*out_ptr++ = *str++;
	    }
	    else
	    {
		*out_ptr = '\0';
		finished = TRUE;
	    }
	    break;
	  case '"':
	  case '\'':
	    if ( (quote != '\0') && (*str != quote) )
	    {
		/*  Quote is already open and character is not the quote
		    character  */
		*out_ptr++ = *str++;
		continue;
	    }
	    if (str[1] == str[0])
	    {
		/*  Two quote characters: copy only one of them  */
		*out_ptr++ = *str++;
		++str;
		continue;
	    }
	    /*  Open quote if not open; close quote if already open  */
	    quote = (quote == '\0') ? *str : '\0';
	    ++str;
	    break;
	  default:
	    *out_ptr++ = *str++;
	    break;
	}
    }
    if (quote != '\0')
    {
	(void) fprintf (stderr, "Warning: closing quote not found\n");
	return (NULL);
    }
    if (rest != NULL)
    {
	/*  Skip trailing whitespace  */
	while ( isspace (*str) && (*str != '\0' ) )
	{
	    ++str;
	}
	*rest = (*str == '\0') ? '\0' : str;
    }
    return (return_value);
}   /*  End Function ex_str  */

int st_icmp (string1, string2)
/*  This routine will compare two strings pointed to by  string1  and
    string2  .The operation of this routine is similar to that of  strcmp
    ,except that the comparison is case insensitive.
    The comparison value is returned.
*/
CONST char *string1;
CONST char *string2;
{
    int ret_value;
    char *str1 = NULL;
    char *str2 = NULL;
    static char function_name[] = "st_icmp";

    if (string1 == string2)
    {
	return (0);
    }
    if (string1 == NULL)
    {
	return (-1);
    }
    if (string2 == NULL)
    {
	return (1);
    }

    if ( ( str1 = m_alloc (strlen (string1) + 1) ) == NULL )
    {
	m_abort (function_name, "copy of string1");
    }
    strcpy (str1, string1);
    st_upr (str1);

    if ( ( str2 = m_alloc (strlen (string2) + 1) ) == NULL )
    {
	m_abort (function_name, "copy of string2");
    }
    strcpy (str2, string2);
    st_upr (str2);
    ret_value = strcmp (str1, str2);
    m_free (str1);
    m_free (str2);
    return (ret_value);
}   /*  End Function st_icmp   */

char *st_upr (string)
/*  This routine will convert the string pointed to by  string  to uppercase.
    The routine returns the address of the string.
*/
char *string;
{
    static char function_name[] = "st_upr";

    if (string == NULL)
    {
	(void) fprintf (stderr, "NULL pointer passed\n");
	a_prog_bug (function_name);
    }
    while (*string != '\0')
    {
	if (islower (*string) != 0)
	{
	    *string = toupper (*string);
	}
        ++string;
    }
    return (string);
}   /*  End Function st_upr */
