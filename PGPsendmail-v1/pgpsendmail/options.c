/*  options.c

    Source file for option processing for  PGPsendmail  (wrapper to sendmail).

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

    Updated by      Richard Gooch   1-JUN-1994: Created  translate_recipient  .

    Updated by      Richard Gooch   30-JUN-1994: Supported NULL recipient in
  translate_recipient  .

    Updated by      Richard Gooch   2-JUL-1994: Fixed bug in
  translate_recipient  when recipient is NULL and no config file exists.

    Last updated by Richard Gooch   29-OCT-1994: Fixed bug in
  translate_recipient  which only processed first option for recipient.


*/
#include <stdio.h>
#include <string.h>
#include "pgpsendmail.h"

flag process_flags (line, options)
/*  This routine will process flags in a line. The flags recognised are:
        always-secure       always use encryption
	secure              synonym for  always-secure
	always              synonym for  always-secure
	never-secure        never use encryption
	insecure            synonym for  never-secure
	never               synonym for  never-secure
	return-receipt      return a success message
	discard-receipt     do not return a success message
	no-receipt          synonym for  discard-receipt
	add-key             include public key in message
	no-advertising      do not show advertising message about PGPsendmail
	sign                sign messages
    The line must be pointed to by  line  .
    The flags will modify the storage pointed to by  options  thus:
      The value TRUE will be written to the  secure  field  if the
      "always-secure" flag is present. If the flag is not present, nothing is
      written here.
      The value TRUE will be written to the  insecure  field  if the
      "never-secure" flag is present. If the flag is not present, nothing is
      written here.
      The routine will not write TRUE into both  secure  and  insecure  .
      The value TRUE will be written to the  receipt  field if the
      "return-receipt". The value FALSE is written if the "discard-receipt"
      flag is present. The default must be set externally.
      The value TRUE will be written to the  add_key  field if the "add-key"
      flag is present. If the flag is not present, nothing is written here.
      The value TRUE will be written to the  no_advertising  field if the
      "no-advertising" flag is present. If the flag is not present, nothing is
      written here.
      The value TRUE will be written to the  sign  field if the "sign" flag is
      present. If the flag is not present, nothing is written here.
    The routine returns TRUE on success, else it returns FALSE.
*/
char *line;
option_type *options;
{
    char *ptr, *rest;
    extern FILE *debug_fp;

    for ( ptr = ex_str (line, &rest); ptr != NULL; ptr = ex_str (rest, &rest) )
    {
	if ( (st_icmp (ptr, "always-secure") == 0) ||
	    (st_icmp (ptr, "always") == 0) || (st_icmp (ptr, "secure") == 0) )
	{
	    if ( (*options).insecure )
	    {
		(void) fprintf (stderr,
				"\"always-secure\" and \"never-secure\" flags found\n");
		m_free (ptr);
		return (FALSE);
	    }
	    (*options).secure = TRUE;
	}
	else if ( (st_icmp (ptr, "never-secure") == 0) ||
		 (st_icmp (ptr, "never") == 0) ||
		 (st_icmp (ptr, "insecure") == 0) )
	{
	    if ( (*options).secure )
	    {
		(void) fprintf (stderr,
				"\"always-secure\" and \"never-secure\" flags found\n");
		m_free (ptr);
		return (FALSE);
	    }
	    (*options).insecure = TRUE;
	}
	else if (st_icmp (ptr, "return-receipt")
		 == 0) (*options).receipt = TRUE;
	else if ( (st_icmp (ptr, "discard-receipt") == 0) ||
		   (st_icmp (ptr, "no-receipt")
		    == 0) ) (*options).receipt = FALSE;
	else if (st_icmp (ptr, "add-key") == 0) (*options).add_key = TRUE;
	else if (st_icmp (ptr, "no-advertising")
		 == 0) (*options).no_advertising = TRUE;
	else if (st_icmp (ptr, "sign") == 0) (*options).sign = TRUE;
	else
	{
	    (void) fprintf (stderr, "Bad flag: \"%s\"\n", ptr);
	    m_free (ptr);
	    return (FALSE);
	}
	m_free (ptr);
    }
    return (TRUE);
}   /*  End Function check_for_header_flags  */

char *translate_recipient (recipient, pgppath, options)
/*  This routine will translate a recipient by searching the configuration
    file:  $PGPPATH/PGPsendmail.config
    The recipient must be pointed to by  recipient  .If this is NULL, only the
    "GLOBAL" option is processed, and a NULL is returned.
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .
    The storage pointed to by  options  may be modified.
    The routine returns a pointer to a dynamically allocated synonym on
    success, else it returns NULL. If the recipient has no synonym, a copy is
    made of the input.
*/
char *recipient;
char *pgppath;
option_type *options;
{
    char *name, *synonym, *rest;
    FILE *fp;
    char config_filename[STRING_LENGTH];
    char txt[STRING_LENGTH];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    (void) sprintf (config_filename, "%s/PGPsendmail.config", pgppath);
    if ( ( fp = fopen (config_filename, "r") ) == NULL )
    {
	if (recipient == NULL) return (NULL);
	if ( ( synonym = m_alloc (strlen (recipient) + 1) ) == NULL )
	{
	    return (NULL);
	}
	(void) strcpy (synonym, recipient);
	return (synonym);
    }
    /*  Read in lines  */
    while (fgets (txt, STRING_LENGTH - 1, fp) != NULL)
    {
	/*  Strip newline  */
	txt[strlen (txt) - 1] = '\0';
	if ( (int) strlen (txt) < 1 ) continue;
	if (txt[0] == '#') continue;
	if ( ( name = ex_str (txt, &rest) ) == NULL ) continue;
	if (strcmp (name, "GLOBAL") == 0)
	{
	    m_free (name);
	    if ( (*options).global_done )
	    {
		continue;
	    }
	    if ( !process_flags (rest, options) )
	    {
		(void) fclose (fp);
		return (NULL);
	    }
	    (*options).global_done = TRUE;
	    if (recipient == NULL)
	    {
		/*  Early return  */
		(void) fclose (fp);
		return (NULL);
	    }
	    continue;
	}
	if (recipient == NULL)
	{
	    m_free (name);
	    continue;
	}
	/*  Check if recipient name matches  */
	if (st_icmp (recipient, name) != 0)
	{
	    m_free (name);
	    continue;
	}
	/*  Name matches  */
	m_free (name);
	if ( ( synonym = ex_str (rest, &rest) ) == NULL )
	{
	    (void) fprintf (stderr, "Could not find synonym on line: \"%s\"\n",
			    txt);
	    (void) fclose (fp);
	    return (NULL);
	}
	/*  Process flags  */
	if ( !process_flags (rest, options) )
	{
	    m_free (synonym);
	    (void) fclose (fp);
	    return (NULL);
	}
	(void) fclose (fp);
	return (synonym);
    }
    (void) fclose (fp);
    if (recipient == NULL) return (NULL);
    if ( ( synonym = m_alloc (strlen (recipient) + 1) ) == NULL )
    {
	return (NULL);
    }
    (void) strcpy (synonym, recipient);
    return (synonym);
}   /*  End Function translate_recipient  */
