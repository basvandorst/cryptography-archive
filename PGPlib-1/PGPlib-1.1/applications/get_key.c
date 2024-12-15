/*********************************************************************
 * Filename:      get_key.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:04:35 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/



/* **************************************************************
 * Format: get [0x01234576 | 0x0123456789012345 | foo bar]
 * ************************************************************** */


#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <pgplib.h>


extern char Arg[];

void
get_key( int argc, char *argv[])
{
    u_long	short_keyid;
    PGPKEYID	key_id, *keyptr;
    int		num_keys;
    int		key_len;
    int		ret;
    char	*string;
    char	*endptr;
    u_char 	*buf;
    int		buflen;
    /* scratch */
    int		i;

    if ( argc != 2 && argc != 3) {
	fprintf(stdout, "[0x12345767 | 0x1234567890123456 | Foo Bar]\n");
	return;
    }

    PGPKEYID_ZERO(key_id);
    short_keyid = 0L;

    if ( argv[1][0] == '0' && toupper(argv[1][1]) == 'X') {
	key_len = strlen(argv[1]);
	switch ( key_len ) {
	case 10:
	    ret = sscanf(argv[1], "%lx", &short_keyid);
	    break;

	case 18:
	    if (PGPKEYID_FromAscii(argv[1],&key_id))
		ret = 1;
	    else
		ret = 0;
	    break;

	default:
	    ret = 0;
	    break;
	}

	if ( ret != 1 ) {
	    fprintf(stdout,
		    "[0x01234576 | 0x0123456789012345 | FooBar]\n");
	    return;
	}
	if ( short_keyid != 0 ) {
            /* Find the real key */
            int         i;
            PGPKEYID    tmpkey;

            fprintf(stdout,"shortid %s\n",argv[1]);

            i = find_shortid(argv[1], &tmpkey);
            if ( i == -1 ) {
                fprintf(stdout, "Found no matching key - %s\n",
			pgplib_strerror(pgplib_errno));
                return;
            }
            if ( PGPKEYID_IsZero(tmpkey) ) {
                fprintf(stdout, "Found no matching key\n");
                return;
            }
            PGPKEYID_COPY(key_id,tmpkey);
	}
	i = get_pubkey_rec(key_id, &buflen, &buf);
	switch ( i ) {
	case -1:
	    fprintf(stdout, "Some error\n");
	    return;
	case 1:
	    fprintf(stdout, "No (correct) key found\n");
	    return;
	case 0:
	    break;
	default:
	    assert(0);
	}
	/* Print what we found */
	print_keyrec(buf, buflen, stdout);

	return;
    }
    else {
	/*
	 * Start at the beginning of the command.  Skip the first word
	 * and any following white space.  Take it from there.
	 */
	string = strchr(Arg, ' ');
	assert(string != NULL);
	while ( isspace(*string) )
	    string += 1;

	/* First, see if the string has been saved exactly */

	i = keys_from_string(string, 0, &num_keys, &keyptr);

	if ( i  == -1) {
	    fprintf(stdout,  "Some error in find_userid\n");
	    return;
	}
	if ( num_keys == 0 ) {
	    fprintf(stdout, "Found no keys that matched\n");
	    return;
	}
	fprintf(stdout, "Found these keys:\n");
	for(i=0; i < num_keys; i++ )
	    fprintf(stdout, "\t%s\n",PGPKEYID_ToStr(&(keyptr[i])));
	free(keyptr);
	return;
    }
}
