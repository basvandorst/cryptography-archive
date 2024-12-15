/* $Id: dispaddr.c,v 1.3 1995/07/13 18:06:01 elkins Exp $
 *
 * $Log: dispaddr.c,v $
 * Revision 1.3  1995/07/13  18:06:01  elkins
 * DisplayAddress() now returns the "real name" part of the address if
 * it is availible.
 *
 * Revision 1.2  1995/07/13  02:36:15  elkins
 * fixed problem where messages in the =sent folder didn't print who the
 * message was TO.
 *
 * Revision 1.1  1995/06/01  23:13:15  elkins
 * Initial revision
 *
 */

#include "headers.h"

int
DisplayAddress (h, f)
struct header_rec *h;
char *f;
{
  char *tmpc, tmpstr[STRING];
  int i = 0, using_to = 0;

  if (addr_matches_user (h->from, username)) {

    /* I sent this message.  Use the TO address.  */

    tmpc = h->to;
    while (*tmpc && *tmpc!=',')
      tmpstr[i++] = *tmpc++;
    tmpstr[i] = '\0';
    using_to = TRUE;
  }
  else if (okay_address(h->to, h->from) && okay_address(h->cc, h->from)) {

    /* This message is not addressed to me. */

    if (showto) {
      char *tmpc;
      int i = 0;
      
      tmpc = h->to;
      while (*tmpc && *tmpc != ',')
        tmpstr[i++] = *tmpc++;
      tmpstr[i] = '\0';
      using_to = 1;
    } else
      strcpy (tmpstr, h->from);
  }
  else /* This message is addressed to me. */
    strcpy (tmpstr, h->from);

  get_real_name (tmpstr, f);

  return(using_to);
}
