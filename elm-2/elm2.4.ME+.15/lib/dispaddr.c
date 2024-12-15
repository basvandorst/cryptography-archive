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

/* NOTICE: h->from includes Full name of Sender
 *          h->to and h->cc includes to and cc headers
 *
 *         If h->from contains address is is taken from "From " -line and
 *         it don't anyway then include sender!
 *
 *         - K E H <hurtta@dionysos.FMi.FI>
 */

#include "headers.h"
#include "me.h"

int
DisplayAddress (h, f, size)
     struct header_rec *h;
     char *f;
     int size;
{
  int i = 0, using_to = 0;

  if ('\0' == h->to[0]) {
    /* No To: address, must select From: address anyway.... */
    strfcpy (f, h->from, size);
    return using_to;
  }

  if (addr_matches_user (h->from, username) ||
      /* addr_matches_user isn't needed here. This does not still
       * take care from h->from is MIME encoded (RFC 1522) and
       * full_username isn't
       */
      0 == strincmp(h->from, full_username,STRING)) {

    /* I sent this message.  Use the TO address.  
     * (perhaps I have also receiver... hmm.)
     */
    
    get_real_name (h->to,f, size);
    using_to = TRUE;
  }
  /* This does not take care when h->from is fullname -- not address.... */
  else if (okay_address(h->to, h->from) && okay_address(h->cc, h->from)) {

    /* This message is not addressed to me. */

    if (showto) {
      get_real_name (h->to,f,size);
      using_to = 1;
    } else
      strfcpy (f, h->from,size);
  }
  else /* This message is addressed to me (or sender of mail). */
    strfcpy (f, h->from, size);

  return(using_to);
}

