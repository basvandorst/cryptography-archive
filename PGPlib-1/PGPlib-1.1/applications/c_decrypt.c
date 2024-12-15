/*********************************************************************
 * Filename:      c_decrypt.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:46:17 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

/* Conventionaly decrypt a file */

#include <sys/types.h>
#include <sys/stat.h>
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
	    "usage: %s -p password file\n",
	    Progname);
    exit(1);
}

/*
 * Decrypt a file with IDEA.  We assume that it contains something
 * that is decrypted.  To verify encryption.
 */

void
main(int argc, char *argv[])
{

    int         ch;
    extern char *optarg;
    u_char	digest[16];
    int		do_compress, armor, outfile, infile;
    char	*filename;
    struct stat sbuf;
    u_char	*outbuf, *tmp;
    int		outbuflen;
    int 	i;

    Progname = argv[0];

    memset(digest, 0, 16);
    filename = NULL;
    armor = 0;
    do_compress = 0;

    while ((ch = getopt (argc, argv, "p:")) != -1) {
	switch (ch) {
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

    infile = open(argv[optind], O_RDONLY);
    if ( infile < 0 ) {
	perror(argv[optind]);
	exit(-1);
    }
    i = fstat(infile, &sbuf);
    if ( i < 0 ) {
	perror(argv[optind]);
	exit(-1);
    }
    tmp = calloc(1, sbuf.st_size);
    if ( tmp == NULL ) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
    }
    i = read(infile, tmp, sbuf.st_size);
    if ( i < sbuf.st_size) {
	perror("read");
	exit(-1);
    }
    close(infile);
    i = conventional_key_decrypt(tmp,
				 digest,
				 &outbuflen,
				 &outbuf);
    if ( i != 0 ) {
	fprintf(stderr, "Could not decrypt\n");
	exit(1);
    }

    strcat(filename, ".dec");
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
