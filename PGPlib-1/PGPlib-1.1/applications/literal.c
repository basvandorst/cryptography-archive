/*********************************************************************
 * Filename:      literal.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:53:05 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pgplib.h>


char *Progname;

void
usage()
{
    fprintf(stderr,
	    "usage: %s -c file\n",
	    Progname);
    exit(1);
}

int
main(int argc, char *argv[])
{
    int         ch;
    extern char *optarg;
    extern int	optind;
    int 	i;
    int		len;
    int		fd;
    int		do_compress;
    u_char	*buf, *tmp;
    char	*filename;
    PGPliteral_t literal;

    Progname = argv[0];
    do_compress = 0;
    if ( argc == 1 )
	usage();
    while ((ch = getopt (argc, argv, "c")) != -1) {
	switch (ch) {
	case 'c':
	    do_compress = 1;
	    break;
	case '?':
	default:
	    usage();

	}
    }

    if ( argc < optind )
	usage();

    argc -= optind;
    argv += optind;
    if ( argc != 1 )
	usage();
    i = create_literal(argv[0], &literal);
    if ( i != 0 ) {
	fprintf(stderr, "Cound not create literal struct\n");
	exit(1);
    }

    i = literal2buf(&literal, &buf);
    if ( i != 0 ) {
	fprintf(stderr, "Cound not create literal packet\n");
	exit(1);
    }

    if ( ! do_compress )
	len = literal.length;
    else {
	tmp = buf;
	i = compress_buffer(tmp,
			    packet_length(buf),
			    &buf,
			    &len);
	if ( i != 0 ) {
	    fprintf(stderr, "Could not compress\n");
	    exit(1);
	}
	else
	    free(tmp);
    }
    i = strlen(argv[0]);
    filename = calloc(1, i+5);
    if ( filename == NULL ) {
	fprintf(stderr, "Out of memory\n");
	exit(1);
    }
    strcat(filename, argv[0]);
    strcat(filename, ".pgp");

    fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if ( fd < 0 ) {
	perror(filename);
	return(1);
    }
    write(fd, buf, len);
    close(fd);

    exit(0);
}



