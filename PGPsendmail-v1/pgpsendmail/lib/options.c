/*  options.c

    Source file for option processing for  PGPsendmail  (wrapper to sendmail).

    Copyright (C) 1994-1997  Richard Gooch

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

    Updated by      Richard Gooch   29-OCT-1994: Fixed bug in
  translate_recipient  which only processed first option for recipient.

    Updated by      Richard Gooch   25-SEP-1997: Used new ERRSTRING macro.

    Updated by      Richard Gooch   1-OCT-1997: Added "no-sign" option. Added
  automatic logging of email addresses.

    Updated by      Richard Gooch   2-OCT-1997: Added regular expression
  matching for email addresses.

    Updated by      Richard Gooch   3-OCT-1997: Configuration file now opened
  externally.

    Updated by      Richard Gooch   5-OCT-1997: Added support for "ALIAS"
  directive.

    Last updated by Richard Gooch   9-OCT-1997: Made use of new function
  <st_expr_expand>.


*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include "pgpsendmail.h"


/*  Private functions  */
STATIC_FUNCTION (void log_address, (CONST char *address, char *line,
				    FILE *debug_fp) );
STATIC_FUNCTION (flag process_alias_line,
		 (CONST char *recipient, CONST char *alias, FILE *config_fp,
		  option_type *options, FILE *debug_fp,
		  stringlist_type *recipient_list) );
STATIC_FUNCTION (flag process_list, (CONST char *list, FILE *config_fp,
				     option_type *options, FILE *debug_fp,
				     stringlist_type *recipient_list) );


/*  Public functions follow  */

flag process_flags (char *line, option_type *options)
/*  [SUMMARY] Process flags on a line.
    [PURPOSE] This routine will process flags in a line. The flags recognised
    are:
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
	no-sign             don't sign messages
    <line> The line buffer.
    <options> The flags will modify the contents thus:
      The value TRUE will be written to the <<secure>> field if the
      "always-secure" flag is present. If the flag is not present, nothing is
      written here.
      The value TRUE will be written to the <<insecure>> field if the
      "never-secure" flag is present. If the flag is not present, nothing is
      written here.
      The routine will not write TRUE into both <<secure>> and <<insecure>>.
      The value TRUE will be written to the <<receipt>> field if the
      "return-receipt". The value FALSE is written if the "discard-receipt"
      flag is present. The default must be set externally.
      The value TRUE will be written to the <<add_key>> field if the "add-key"
      flag is present. If the flag is not present, nothing is written here.
      The value TRUE will be written to the <<no_advertising>> field if the
      "no-advertising" flag is present. If the flag is not present, nothing is
      written here.
      The value TRUE will be written to the <<sign>> field if the "sign" flag
      is present. If the flag is not present, nothing is written here.
      The value FALSE will be written to the <<sign>> field if the "no-sign"
      flag is present. If the flag is not present, nothing is written here.
    [RETURNS] TRUE on success, else FALSE.
*/
{
    char *ptr, *rest;
    extern FILE *debug_fp;

    for ( ptr = ex_str (line, &rest); ptr != NULL; ptr = ex_str (rest, &rest) )
    {
	if ( (st_icmp (ptr, "always-secure") == 0) ||
	     (st_icmp (ptr, "always") == 0) || (st_icmp (ptr, "secure") == 0) )
	{
	    if (options->insecure)
	    {
		fprintf (stderr,
			 "\"always-secure\" and \"never-secure\" flags found\n");
		m_free (ptr);
		return (FALSE);
	    }
	    options->secure = TRUE;
	}
	else if ( (st_icmp (ptr, "never-secure") == 0) ||
		  (st_icmp (ptr, "never") == 0) ||
		  (st_icmp (ptr, "insecure") == 0) )
	{
	    if (options->secure)
	    {
		fprintf (stderr,
			 "\"always-secure\" and \"never-secure\" flags found\n");
		m_free (ptr);
		return (FALSE);
	    }
	    options->insecure = TRUE;
	}
	else if (st_icmp (ptr, "return-receipt")
		 == 0) options->receipt = TRUE;
	else if ( (st_icmp (ptr, "discard-receipt") == 0) ||
		  (st_icmp (ptr, "no-receipt")
		   == 0) ) options->receipt = FALSE;
	else if (st_icmp (ptr, "add-key") == 0) options->add_key = TRUE;
	else if (st_icmp (ptr, "no-advertising")
		 == 0) options->no_advertising = TRUE;
	else if (st_icmp (ptr, "sign") == 0) options->sign = TRUE;
	else if (st_icmp (ptr, "no-sign") == 0) options->sign = FALSE;
	else
	{
	    fprintf (stderr, "Bad flag: \"%s\"\n", ptr);
	    m_free (ptr);
	    return (FALSE);
	}
	m_free (ptr);
    }
    return (TRUE);
}   /*  End Function check_for_header_flags  */

flag translate_recipient (CONST char *recipient, FILE *config_fp,
			  option_type *options, FILE *debug_fp,
			  stringlist_type *recipient_list)
/*  [SUMMARY] Translate a recipient by searching $PGPPATH/PGPsendmail.config
    <recipient> The recipient. If this is NULL, only the "GLOBAL" option is
    processed, and a NULL is returned.
    <config_fp> The configuration file. This may be NULL.
    <options> The options information. This may be modified.
    <debug_fp> Debugging information will be written here. If this is NULL no
    debugging information is written.
    <recipient_list> The list of translated recipients. This is modified.
    [RETURNS] TRUE on success, else FALSE.
*/
{
    int err;
    regex_t pbuf;
    regmatch_t mbuf;
    char *name, *synonym, *rest, *ptr;
    char txt[STRING_LENGTH];
    static char function_name[] = "translate_recipient";

    if (config_fp == NULL)
    {
	if (recipient == NULL) return (FALSE);
	if ( !add_string (recipient_list, recipient, TRUE) ) return (FALSE);
	if (debug_fp != NULL)
	{
	    fprintf (debug_fp, "    Recipient: \"%s\"\tsynonym: \"%s\"\n",
		     recipient,
		     recipient_list->strings[recipient_list->num_set - 1]);
	    fflush (debug_fp);
	}
	return (TRUE);
    }
    if (fseek (config_fp, 0, SEEK_SET) == -1)
    {
	if (debug_fp != NULL)
	    fprintf (debug_fp, "%s: error rewinding config file\t%s\n",
		     function_name, ERRSTRING);
	return (FALSE);
    }
    /*  Read in lines  */
    while (fgets (txt, STRING_LENGTH - 1, config_fp) != NULL)
    {
	/*  Strip newline  */
	txt[strlen (txt) - 1] = '\0';
	if ( (int) strlen (txt) < 1 ) continue;
	if (txt[0] == '#') continue;
	if ( ( name = ex_str (txt, &rest) ) == NULL ) continue;
	if (strcmp (name, "GLOBAL") == 0)
	{
	    m_free (name);
	    if (options->global_done)
	    {
		continue;
	    }
	    if ( !process_flags (rest, options) )
	    {
		return (FALSE);
	    }
	    options->global_done = TRUE;
	    if (recipient == NULL)
	    {
		/*  Early return  */
		return (FALSE);
	    }
	    continue;
	}
	if (recipient == NULL)
	{
	    m_free (name);
	    continue;
	}
	if (strcmp (name, "ADDRESSLOG") == 0)
	{
	    /*  Must log each new recipient  */
	    m_free (name);
	    log_address (recipient, rest, debug_fp);
	    continue;
	}
	if (strcmp (name, "ALIAS") == 0)
	{
	    m_free (name);
	    for (ptr = rest; !isspace (*ptr) && (*ptr != '\0'); ++ptr);
	    if (*ptr != '\0') *ptr++ = '\0';
	    if (st_icmp (recipient, rest) == 0)
	    {
		/*  Must translate recipient using system aliases  */
		return process_alias_line (recipient, ptr, config_fp,
					   options, debug_fp, recipient_list);
	    }
	    continue;
	}
	if (name[0] == '!')
	{
	    /*  Do regular expression matching  */
	    if ( ( err = regcomp (&pbuf, name + 1, REG_ICASE) ) != 0 )
	    {
		regerror (err, &pbuf, txt, STRING_LENGTH);
		if (debug_fp != NULL)
		    fprintf (debug_fp, "Error compiling regexp: \"%s\"\t%s\n",
			     name, txt);
		m_free (name);
		return (FALSE);
	    }
	    if (regexec (&pbuf, recipient, 1, &mbuf, 0) != 0)
	    {
		regfree (&pbuf);
		m_free (name);
		continue;
	    }
	    regfree (&pbuf);
	}
	else
	{
	    /*  Check if recipient name matches  */
	    if (st_icmp (recipient, name) != 0)
	    {
		m_free (name);
		continue;
	    }
	}
	/*  Name matches  */
	m_free (name);
	if ( ( synonym = ex_str (rest, &rest) ) == NULL )
	{
	    fprintf (stderr, "Could not find synonym on line: \"%s\"\n", txt);
	    return (FALSE);
	}
	/*  Process flags  */
	if ( !process_flags (rest, options) )
	{
	    m_free (synonym);
	    return (FALSE);
	}
	if ( !add_string (recipient_list, synonym, FALSE) ) return (FALSE);
	if (debug_fp != NULL)
	{
	    fprintf (debug_fp, "    Recipient: \"%s\"\tsynonym: \"%s\"\n",
		     recipient,
		     recipient_list->strings[recipient_list->num_set - 1]);
	    fflush (debug_fp);
	}
	return (TRUE);
    }
    if (recipient == NULL) return (FALSE);
    if ( !add_string (recipient_list, recipient, TRUE) ) return (FALSE);
    if (debug_fp != NULL)
    {
	fprintf (debug_fp, "    Recipient: \"%s\"\tsynonym: \"%s\"\n",
		 recipient,
		 recipient_list->strings[recipient_list->num_set - 1]);
	fflush (debug_fp);
    }
    return (TRUE);
}   /*  End Function translate_recipient  */


/*  Private functions follow  */

static void log_address (CONST char *address, char *line, FILE *debug_fp)
/*  [SUMMARY] Log email address.
    <address> The email address to log.
    <line> The rest of the configuration line after the "ADDRESSLOG". This is
    modified.
    <debug_fp> Debugging information will be written here. If this is NULL no
    debugging information is written.
    [RETURNS] Nothing.
*/
{
    flag found;
    int fd, len;
    char *ptr;
    char *logfile = line;
    CONST char *strings[2];
    char addr[STRING_LENGTH], filename[STRING_LENGTH];

    /*  Find end of filename  */
    for (ptr = line; !isspace (*ptr) && (*ptr != '\0'); ++ptr);
    if (*ptr != '\0') *ptr++ = '\0';
    line = ptr;
    strings[1] = NULL;
    while (*line != '\0')
    {
	/*  Find end of filename  */
	for (ptr = line; !isspace (*ptr) && (*ptr != '\0'); ++ptr);
	if (*ptr != '\0') *ptr++ = '\0';
	st_expr_expand (filename, STRING_LENGTH, line, debug_fp);
	line = ptr;
	strings[0] = address;
	if ( !search_file (filename, strings, debug_fp, &found) )
	{
	    if (debug_fp != NULL)
		fprintf (debug_fp, "Error searching file: \"%s\"\n",
			 filename);
	    return;
	}
	if (found) return;
    }
    st_expr_expand (filename, STRING_LENGTH, logfile, debug_fp);
    if (debug_fp != NULL)
	fprintf (debug_fp, "Logging new email address: \"%s\"\n", address);
    if ( ( fd = open (filename, O_WRONLY | O_APPEND, 0) ) == -1 )
    {
	if (debug_fp != NULL)
	    fprintf (debug_fp, "Error opening file: \"%s\"\t%s\n",
		     filename, ERRSTRING);
	return;
    }
    strcpy (addr, address);
    len = strlen (addr);
    addr[len++] = '\n';
    addr[len] = '\0';
    write (fd, addr, len);
    close (fd);
}   /*  End Function log_address  */

static flag process_alias_line (CONST char *recipient, CONST char *alias,
				FILE *config_fp, option_type *options,
				FILE *debug_fp,stringlist_type *recipient_list)
/*  [SUMMARY] Translate a recipient by searching $PGPPATH/PGPsendmail.config
    <recipient> The recipient. If this is NULL, only the "GLOBAL" option is
    processed, and a NULL is returned.
    <alias> The optional system alias.
    <config_fp> The configuration file. This may be NULL.
    <options> The options information. This may be modified.
    <debug_fp> Debugging information will be written here. If this is NULL no
    debugging information is written.
    <recipient_list> The list of translated recipients. This is modified.
    [RETURNS] TRUE on success, else FALSE.
*/
{
    flag eof;
    char ch;
    char *ptr1, *ptr2, *sysalias;
    char buffer[STRING_LENGTH];
    static FILE *aliases_fp = NULL;
    static char function_name[] = "process_alias_line";

    if (aliases_fp == NULL)
    {
	/*  HACK  */
	if ( ( aliases_fp = fopen ("/etc/aliases", "r") ) == NULL )
	{
	    if (debug_fp != NULL) fprintf (debug_fp,
					   "Error opening aliases file\t%s\n",
					   ERRSTRING);
	    return (FALSE);
	}
    }
    else
    {
	if (fseek (aliases_fp, 0, SEEK_SET) == -1)
	{
	    if (debug_fp != NULL)
		fprintf (debug_fp, "%s: error rewinding aliases file\t%s\n",
			 function_name, ERRSTRING);
	    return (FALSE);
	}
    }
    if (*alias == '\0') alias = recipient;
    /*  Search aliases file for the alias  */
    while (TRUE)
    {
	if ( !read_fp_line (aliases_fp, buffer, STRING_LENGTH, debug_fp,
			    &eof) ) return (FALSE);
	if (eof)
	{
	    if (debug_fp != NULL)
		fprintf (debug_fp, "Alias: \"%s\" not found in aliases file\n",
			 alias);
	    return (TRUE);
	}
	/*  Skip leading whitespace  */
	for (sysalias = buffer; isspace (*sysalias) && (*sysalias != '\0');
	     ++sysalias);
	if (*sysalias == '\0') continue;
	/*  Search for end of alias  */
	for (ptr1 = sysalias;
	     !isspace (*ptr1) && (*ptr1 != '\0') && (*ptr1 != ':');
	     ++ptr1);
	if (*ptr1 == '\0') continue;
	/*  Skip trailing whitespace  */
	for (ptr2 = ptr1; isspace (*ptr2) && (*ptr2 != '\0'); ++ptr2);
	if (*ptr2 != ':') continue;
	for (++ptr2; isspace (*ptr2) && (*ptr2 != '\0'); ++ptr2);
	if (*ptr2 == '\0') continue;
	*ptr1 = '\0';
	if (st_icmp (alias, sysalias) != 0) continue;
	/*  Found a match!  */
	break;
    }
    if (debug_fp != NULL) fprintf (debug_fp, "    alias line: \"%s\"\n", ptr2);
    fflush (debug_fp);
    /*  Start decoding comma separated entries  */
    ptr1 = ptr2;
    while (TRUE)
    {
	if (*ptr1 == '\0') return (TRUE);
	if ( isspace (*ptr1) )
	{
	    ++ptr1;
	    continue;
	}
	if (*ptr1 == ',')
	{
	    for (++ptr1;
		 ( isspace (*ptr1) || (*ptr1 == ',') ) && (*ptr1 != '\0');
		  ++ptr1);
	    if (*ptr1 != '\0') continue;
	    if ( !read_fp_line (aliases_fp, buffer, STRING_LENGTH, debug_fp,
				&eof) ) return (FALSE);
	    if (eof) return (TRUE);
	    if (debug_fp != NULL) fprintf (debug_fp,
					   "    alias line: \"%s\"\n", buffer);
	    ptr1 = buffer;
	}
	if (*ptr1 == '"')
	{
	    for (ptr2 = ptr1 + 1; (*ptr2 != '"') && (*ptr2 != '\0'); ++ptr2);
	    if (*ptr2 == '\0')
	    {
		if (debug_fp != NULL)
		    fprintf (debug_fp, "Illegal alias entry: \"%s\"\n", ptr1);
		return (TRUE);
	    }
	    ptr1 = ptr2 + 1;
	    continue;
	}
	if (strncmp (ptr1, ":include:", 9) == 0)
	{
	    /*  Have a mailing list  */
	    ptr1 += 9;
	    for (ptr2 = ptr1 + 1; (*ptr2 != ',') && (*ptr2 != '\0'); ++ptr2);
	    ch = *ptr2;
	    *ptr2 = '\0';
	    if ( !process_list (ptr1, config_fp, options, debug_fp,
				recipient_list) ) return (FALSE);
	    ptr1 = ptr2;
	    *ptr1 = ch;
	    continue;
	}
	/*  Assume it's an ordinary alias entry  */
	for (ptr2 = ptr1 + 1;
	     !isspace (*ptr2) && (*ptr2 != ',') && (*ptr2 != '\0');
	     ++ptr2);
	ch = *ptr2;
	if ( !translate_recipient (ptr1, config_fp, options, debug_fp,
				   recipient_list) ) return (FALSE);
	ptr1 = ptr2;
	*ptr1 = ch;
    }
    return (TRUE);
}   /*  End Function process_alias_line  */

static flag process_list (CONST char *list, FILE *config_fp,
			  option_type *options, FILE *debug_fp,
			  stringlist_type *recipient_list)
/*  [SUMMARY] Translate a recipient by searching $PGPPATH/PGPsendmail.config
    <config_fp> The configuration file. This may be NULL.
    <options> The options information. This may be modified.
    <debug_fp> Debugging information will be written here. If this is NULL no
    debugging information is written.
    <recipient_list> The list of translated recipients. This is modified.
    [RETURNS] TRUE on success, else FALSE.
*/
{
    flag eof;
    FILE *fp;
    char buffer[STRING_LENGTH];
    static char function_name[] = "process_list";

    if ( ( fp = fopen (list, "r") ) == NULL )
    {
	fprintf (stderr, "%s: error opening file: \"%s\"\t%s\n",
		 function_name, list, ERRSTRING);
	if (debug_fp == NULL) return (FALSE);
	fprintf (debug_fp, "%s: error opening file: \"%s\"\t%s\n",
		 function_name, list, ERRSTRING);
	return (FALSE);
    }
    if (debug_fp != NULL) fprintf (debug_fp,
				   "    Processing list: \"%s\"\n", list);
    while (TRUE)
    {
	if ( !read_fp_line (fp, buffer, STRING_LENGTH, debug_fp, &eof) )
	{
	    fclose (fp);
	    return (FALSE);
	}
	if (eof)
	{
	    fclose (fp);
	    return (TRUE);
	}
	if ( !translate_recipient (buffer, config_fp, options, debug_fp,
				   recipient_list) ) return (FALSE);
    }
    return (TRUE);
}   /*  End Function process_list  */
