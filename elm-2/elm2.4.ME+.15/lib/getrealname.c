/* $Id: getrealname.c,v 1.2 1995/08/02 17:52:27 elkins Exp $
 *
 * This code originally written by Michael Elkins <elkins@aero.org>
 *
 * $Log: getrealname.c,v $
 * Revision 1.2  1995/08/02  17:52:27  elkins
 * older BSD systems need <ctype.h>
 * From: Michael Finken <finken@conware.de>
 *
 * Revision 1.1  1995/07/13  18:07:32  elkins
 * Initial revision
 *
 */

#include "headers.h"
#include "me.h"

/* copies the "real name" part of s to d */

void
get_real_name (s, d, size)
     char *s, *d;
     int size;
{
  char *p;
  int i = 0;

  if ((p = (char*)strchr(s, '(')) != 0) {
    p++;
    while (*p && *p != ')' && i < size-1)
      d[i++] = *p++;
    d[i] = '\0';
  } else {
    while (*s && (isspace(*s) || *s == '"')) ++s;
    if (*s == '<') {
      /* with addresses like "<elkins@aero.org>", use the email address
         instead of a zero length string (no real name part). */
      s++;
      while (*s && *s!='>' && i < size-1)
        d[i++] = *s++;
    }
    else {
      while (*s && *s != '"' && *s != '<' && i < size-1)
        d[i++] = *s++;
    }
    d[i] = '\0';
    remove_possible_trailing_spaces(d);
  }
}
