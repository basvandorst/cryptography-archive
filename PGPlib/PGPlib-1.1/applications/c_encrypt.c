/*********************************************************************
 * Filename:      c_encrypt.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:47:32 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

/* Conventionaly encrypt a file */
 

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
	    "usage: %s [-a] -p password file\n",
	    Progname);
    exit(1);
}

/*
 * Encrypt a file with IDEA.
 */

void
main(int argc, char *argv[])
{

    int         ch;
    extern char *optarg;
    u_char	digest[16];
    PGPliteral_t literal;
    int		do_compress, armor, outfile;
    char	*filename;
    u_char	*outbuf, *tmp;
    int		outbuflen;
    int 	i;

    Progname = argv[0];

    memset(digest, 0, 16);
    filename = NULL;
    armor = 0;
    do_compress = 0;

    while ((ch = getopt (argc, argv, "acp:")) != -1) {
	switch (ch) {
	case 'a':
	    armor = 1;
	    break;
	case 'c':
	    do_compress = 1;
	    break;
	case 'p':
            MD5(optarg, strlen(optarg), digest);
            break;
	case '?':
        default:
            usage();
	}
    }

    if ( digest[0] == 0 || argc == optind)
	usage();

    i = access(argv[optind], R_OK);
    if ( i < 0 ) {
	perror(argv[optind]);
	usage();
    }
    filename = calloc(1, strlen(argv[optind]) + 4 + 1);
    if ( filename == NULL ) {
	fprintf(stderr, "Out of memory\n");
	exit(1);
    }
    strcpy(filename, argv[optind]);

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

    i = conventional_key_encrypt(tmp,
				 packet_length(tmp),
				 digest,
				 do_compress,
				 &outbuflen,
				 &outbuf);
    if ( i != 0 ) {
	fprintf(stderr, "Could not encrypt\n");
	exit(1);
    }

    strcat(filename, ".pgp");
    outfile = open(filename, O_WRONLY|O_CREAT, 0600);
    if ( outfile < 0 ) {
        perror("creat");
        usage();
    }

    i = write(outfile, outbuf, outbuflen);
    if ( i < 0 ) {
        perror("write");
        fprintf(stderr, "Cound not write result\n");
	exit(1);
    }
    close(outfile);
    exit(0);
}
