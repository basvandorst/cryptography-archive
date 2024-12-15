/*  keylist.c

    Source file for keylist operations for  PGPsendmail  (wrapper to sendmail).

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

    Updated by      Richard Gooch   16-JUN-1994: Added more explicit
  information about recipient processing and discard revoked keys.

    Updated by      Richard Gooch   18-JUN-1994: Fixed bug in  freshen_keylist
  where missing public keyring was not trapped.

    Updated by      Richard Gooch   23-JUN-1994: Changed  freshen_keylist  to
  allow more flexibility in output of  pgp -kv  because of longer keyID
  display in PGP-2.6ui

    Updated by      Richard Gooch   25-SEP-1997: Used new ERRSTRING macro.

    Last updated by Richard Gooch   3-OCT-1997: Add +language=en option to PGP.


*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pgpsendmail.h"


flag freshen_keylist (char *pgppath)
/*  This routine will create a keylist file from the PGP public keyring. If the
    keylist is newer than the public keyring, no new list is generated.
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .
    The routine returns TRUE on success, else it returns FALSE.
*/
{
    flag dump_key;
    int in_fd = -1;
    int out_fd = -1;
    int err_fd = -1;
    int child;
    char *key_id, *user_id, *ptr;
    FILE *pgp_fp, *list_fp;
    char listfile[STRING_LENGTH];
    char keyring[STRING_LENGTH];
    char txt[STRING_LENGTH];
    struct stat statbuf_list, statbuf_keyring;
    extern char *sys_errlist[];
    static char *argv[] = {"pgp", "-kv", "+language=en", NULL};

    sprintf (listfile, "%s/.keylist", pgppath);
    sprintf (keyring, "%s/pubring.pgp", pgppath);
    /*  First check file ages  */
    if (stat (keyring, &statbuf_keyring) != 0)
    {
	fprintf (stderr, "Could not stat keyring file: \"%s\"\t%s\n",
		 keyring, ERRSTRING);
	return (FALSE);
    }
    if (stat (listfile, &statbuf_list) != 0)
    {
	if (errno != ENOENT)
	{
	    fprintf (stderr, "Could not stat keylist file: \"%s\"\t%s\n",
		     listfile, ERRSTRING);
	    return (FALSE);
	}
    }
    else
    {
	if (statbuf_keyring.st_mtime < statbuf_list.st_mtime) return (TRUE);
    }
    /*  Generate a new list  */
    fprintf (stderr, "Generating keylist...\n");
    if ( ( list_fp = fopen (listfile, "w") ) == NULL )
    {
	fprintf (stderr, "Could not open keylist file for writing\n");
	return (FALSE);
    }
    if ( ( in_fd = open ("/dev/null", O_RDONLY, 0) ) < 0 )
    {
	fprintf (stderr, "Could not open  /dev/null\n");
	return (FALSE);
    }
    err_fd = ERROR_FD;
    if ( ( child = spawn_job ("pgp", argv, &in_fd, &out_fd, &err_fd) ) < 0 )
    {
	fprintf (stderr, "Could not spawn PGP\n");
	return (FALSE);
    }
    if ( ( pgp_fp = fdopen (out_fd, "r") ) == NULL )
    {
	fprintf (stderr, "Could not create FILE from descriptor\n");
	return (FALSE);
    }
    /*  Start reading lines  */
    dump_key = FALSE;
    while (fgets (txt, STRING_LENGTH - 1, pgp_fp) != NULL)
    {
	/*  Strip newline  */
	txt[strlen (txt) - 1] = '\0';
	if ( (int) strlen (txt) < 1 ) continue;
	if (st_nicmp (txt, "key ring:", 9) == 0) continue;
	if (st_nicmp (txt, "type", 4) == 0) continue;
	if (strncmp (txt, "pub ", 4) == 0)
	{
	    /*  New key  */
	    dump_key = FALSE;
	    if ( ( key_id = find_field (txt, 1) ) == NULL ) continue;
	    if ( ( key_id = strchr (key_id, '/') ) == NULL ) continue;
	    ++key_id;
	    if ( ( user_id = find_field (txt, 3) ) == NULL ) continue;
	    if (strcmp (user_id, "*** KEY REVOKED ***") == 0)
	    {
		dump_key = TRUE;
		continue;
	    }
	    fprintf (list_fp, "%s\n", user_id);
	    for (ptr = key_id; (*ptr != '\0') && !isspace (*ptr); ++ptr);
	    *ptr = '\0';
	    fprintf (list_fp, " 0x%s\n", key_id);
	    /*  Don't search string after here since it's been changed  */
	    continue;
	}
	if (strncmp (txt, "                                  ", 29) == 0)
	{
	    /*  Extra ID for key  */
	    if (dump_key) continue;
	    if ( ( user_id = find_field (txt, 0) ) == NULL ) continue;
	    fprintf (list_fp, " %s\n", user_id);
	    continue;
	}
	/*  Discard anything else  */
    }
    /*  Clean up  */
    fclose (list_fp);
    waitpid (child, NULL, 0);
    fclose (pgp_fp);
    close (in_fd);
    fprintf (stderr, "Generated keylist.\n\n");
    return (TRUE);
}   /*  End Function freshen_keylist  */

unsigned int check_recipients (char **recipients, char *pgppath,
			       char **bad_recipient)
/*  This routine will check all recipients in a list and will determine if the
    message should be encrypted.
    The list of recipients must be pointed to by  recipients  .
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .
    The first recipient for which encryption is not possible will be written
    to the storage pointed to by  bad_recipient  .NULL is written here if all
    recipients can be encrypted for.
    The routine returns CHECK_CAN_ENCRYPT if the message should be encrpyted,
    else it returns something else.
*/
{
    flag all_have_key = TRUE;
    int key_number, index;
    FILE *fp;
    char *ptr;
    char txt[STRING_LENGTH];
    char listfile[STRING_LENGTH];

    sprintf (listfile, "%s/.keylist", pgppath);
    *bad_recipient = NULL;
    if ( ( fp = fopen (listfile, "r") ) == NULL )
    {
	fprintf (stderr, "Could not open keylist file for writing\n");
	return (CHECK_NO_KEYLIST);
    }
    for (; *recipients != NULL; ++recipients)
    {
	ptr = *recipients;
	rewind (fp);
	index = -2;
	key_number = -1;
	while (fgets (txt, STRING_LENGTH - 1, fp) != NULL)
	{
	    /*  Strip newline  */
	    txt[strlen (txt) - 1] = '\0';
	    if ( (int) strlen (txt) < 1 ) continue;
	    if (txt[0] == ' ')
	    {
		if (index == key_number) continue;
		if (st_istr (txt + 1, ptr) == NULL) continue;
		/*  Have a match!  */
		if (index >= 0)
		{
		    /*  Matched with a previous key  */
		    fclose (fp);
		    *bad_recipient = ptr;
		    return (CHECK_AMBIGUOUS_KEY);
		}
		index = key_number;
		continue;
	    }
	    ++key_number;
	    if (st_istr (txt, ptr) != NULL)
	    {
		if (index >= 0)
		{
		    /*  Matched with a previous key  */
		    fclose (fp);
		    *bad_recipient = ptr;
		    return (CHECK_AMBIGUOUS_KEY);
		}
		index = key_number;
	    }
	}
	if (index < 0)
	{
	    fclose (fp);
	    *bad_recipient = ptr;
	    return (CHECK_NO_KEY);
	}
	/*  By this time, we know that this recipient has a public key  */
	fprintf (stderr, "Recipient: \"%s\" should have a public key\n", ptr);
    }
    fclose (fp);
    return (CHECK_CAN_ENCRYPT);
}   /*  End Function check_recipients  */

flag include_mykey (CONST char *pgppath, FILE *sendmail_fp)
/*  This routine will include the users PGP public key in a message.
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .
    The output to the real  sendmail  must be pointed to by  sendmail_fp  .
    The routine returns TRUE on success, else it returns FALSE.
*/
{
    int ch;
    FILE *in_fp;
    char keyfile[STRING_LENGTH];
    extern char *sys_errlist[];

    sprintf (keyfile, "%s/mypubkey.asc", pgppath);
    if ( ( in_fp = fopen (keyfile, "r") ) == NULL )
    {
	fprintf (stderr, "\n\nError opening file: \"%s\"\t%s\n",
		 keyfile, ERRSTRING);
	return (FALSE);
    }
    fprintf (sendmail_fp,
	     "\nMy PGP public key (automatically included) follows:\n\n");
    while ( ( ch = getc (in_fp) ) != EOF ) putc (ch, sendmail_fp);
    fflush (sendmail_fp);
    fclose (in_fp);
    fprintf (stderr, "\n\nIncluded keyfile: \"%s\"\n", keyfile);
    return (TRUE);
}   /*  End Function include_mykey  */

char *find_field (CONST char *string, int number)
/*  This routine will search for the Nth whitespace separated field in a
    string. Fields may be separated by one or more consecutive whitespace
    characters.
    The string must be pointed to by  string  .
    The field number to find must be given by  number  .This must be 0 for the
    first field. The first field may be preceeded by whitespace.
    The routine returns a pointer to the desired field on succes,
    else it returns NULL if the field does not exist.
*/
{
    /*  Skip leading whitespace  */
    while ( (*string != '\0') && isspace (*string) ) ++string;
    if (*string == '\0') return (NULL);
    while (number > 0)
    {
	/*  Skip non-whitespace  */
	while ( (*string != '\0') && !isspace (*string) ) ++string;
	if (*string == '\0') return (NULL);
	/*  Skip leading whitespace  */
	while ( (*string != '\0') && isspace (*string) ) ++string;
	if (*string == '\0') return (NULL);
	--number;
    }
    return ( (char *) string );
}   /*  End Function find_field  */
