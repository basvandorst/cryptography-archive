/*********************************************************************
 * Filename:      p_encrypt.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:09:36 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <fcntl.h>
#include <md5.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pgplib.h>

char *Progname;

void
usage()
{
    fprintf(stderr,
	    "usage: %s [-a] -d database -f file -p password -u user\n",
	    Progname);
    exit(1);
}

void
main(int argc, char *argv[])
{
    int		ch;
    extern char	*optarg;
    char 	*database;
    int		infile, outfile;
    char	*filename;
    u_char	digest[16];
    int		digest_ok;
    char 	*username;
    PGPKEYID	*keys;
    PGPpubkey_t	*pubkey;
    PGPliteral_t literal;
    int		armor;
    int		num_keys;
    int		buflen;
    /* scratch */
    int		i,j, k;
    u_char 	*tmp, *ptr;

    Progname = argv[0];

    armor = 0;
    database = NULL;
    filename = NULL;
    username = NULL;
    outfile = infile = -1;
    digest_ok = 0;
    while ((ch = getopt (argc, argv, "ad:f:p:u:")) != -1) {
	switch (ch) {
	case 'a':
	    armor = 1;
	    break;
	case 'd':
	    database = optarg;
	    i = keydb_init(optarg, READER);
	    if ( i != 0 ) {
		fprintf(stderr, "database error\n");
		usage();
	    }
	    break;
	case 'f':
	    i = access(optarg, R_OK);
	    if ( i < 0 ) {
		perror(optarg);
		usage();
	    }
	    filename = calloc(1, strlen(optarg) + 4 + 1);
	    if ( filename == NULL ) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	    }
	    strcpy(filename, optarg);
	    break;
	case 'p':
	    MD5(optarg, strlen(optarg), digest);
	    digest_ok = 1;
	    break;

	case 'u':
	    username = optarg;
	    break;
	case '?':
	default:
	    fprintf(stderr, "Unknown option\n");
	    usage();
	}
    }

    if ( database == NULL || filename == NULL || digest_ok == 0) {
	fprintf(stderr, "Missing something ?\n");
	usage();
    }

    i = keys_from_string(username, 0, &num_keys, &keys);
    if ( i != 0 ) {
	fprintf(stderr, "Error finding keys\n");
	exit(1);
    }
    if ( num_keys > 1 ) {
	PGPuserid_t	**uids;

	fprintf(stdout, "Found more than one key\n");
	for( ; num_keys-1 >= 0; num_keys -= 1) {
	    i = uids_from_keyid(keys[num_keys], &uids);
	    if ( i == -1 ) {
		fprintf(stderr, "Error while finding uids\n");
		exit(1);
	    }
	    if ( i != 0 ) {
		j=0;
		while (uids[j] != NULL ) {
		    fprintf(stdout, "\t%s\n", uids[j]->name);
		    free_uid(uids[j]);
		    j += 1;
		}
		free(uids);

		continue;
	    }
	}

	exit(1);
    }

    /* OK, only one key. */

    i = get_only_pubkey(keys[0], &pubkey);
    if ( i != 0 ) {
	fprintf(stderr, "Didn't find key\n");
	exit(1);
    }
    free(keys);

    i = create_literal(filename, &literal);
    if ( i != 0 ) {
	fprintf(stderr, "Failed to create literal from %s\n", filename);
	exit(1);
    }
    i = literal2buf(&literal, &tmp);
    if ( i != 0 ) {
	fprintf(stderr, "Failed to create literal buffer\n");
	exit(1);
    }

    i = pub_encrypt_buffer(pubkey, tmp, literal.length, 1,
			   digest, &buflen, &ptr);
    if ( i != 0 ) {
	fprintf(stderr, "Failed to encrypt\n");
	fprintf(stderr, "%s\n", pgplib_errlist[pgplib_errno]);
	exit(1);
    }
    free_literal(&literal);
    free(tmp);

    if ( armor == 1 ) {
	i = armor_buffer(ptr, buflen, &k, (char **)&tmp, &j);
	if ( i < 0 ) {
	    fprintf(stderr, "Could not armor\n");
	    exit(1);
	}
	free(ptr);
	ptr = tmp;
	buflen = j;
    }

    strcat(filename, ".pgp");
    outfile = open(filename, O_WRONLY|O_CREAT, 0600);
    if ( outfile < 0 ) {
	perror("creat");
	usage();
    }

    i = write(outfile, ptr, buflen);
    if ( i < 0 ) {
	perror("write");
	fprintf(stderr, "Cound not write result\n");
	exit(1);
    }

    exit(0);
}
