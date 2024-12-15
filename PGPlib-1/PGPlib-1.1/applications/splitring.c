/*********************************************************************
 * Filename:      splitring.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:51:06 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <stdlib.h>
#include <string.h>

#include <pgplib.h>

static char *progname;
void
usage(void)
{
    fprintf(stderr, "usage : %s -f file -n num\n", progname);
    exit(1);
}

int
main(int argc, char *argv[])
{

    extern char	*optarg;
    int		ch;
    char	*pp;
    FILE *infile, *outfile;
    int		num;
    int		key_count, file_count;
    int		done;
    char	fname[15];
    u_char	*p_buf;
    int		p_len;
    /* scratch */
    int		i;

    infile = NULL;
    progname = argv[0];
    num = 0;
    while ((ch = getopt (argc, argv, "f:n:")) != -1) {
	switch (ch) {
	case 'f':
	    infile = fopen(optarg, "r");
	    if ( infile == NULL ) {
		perror("fopen");
		exit(1);
	    }
	    break;
	case 'n':
	    i = (int) strtol(optarg, &pp, 10);
	    if ( pp == NULL || *pp != 0)
		usage();
	    num = i;
	    break;
	case '?':
	default:
	    usage();
	}
    }
    if ( num == 0 || fname == NULL )
	usage();

    memset(fname, 0, 15);
    strcpy(fname, "split00000.pgp");
    outfile = fopen(fname, "w+");
    if ( outfile == NULL ) {
	perror(fname);
	exit(1);
    }

    done = 0;
    file_count = 0;
    key_count  = 0;
    while ( ! done ) {
	i = read_packet(infile, &p_buf);
	if ( i != 0 ) {
	    fclose(outfile);
	    if ( feof(infile) )
		exit(0);
	    else {
		fprintf(stderr, "Error reading file\n");
		exit(1);
	    }
	}
	p_len = packet_length(p_buf);
	if ( is_ctb_type(*p_buf, CTB_CERT_PUBKEY_TYPE) ) {
	    key_count += 1;
	    if ( key_count % num == 0 ) {
		fclose(outfile);
		file_count += 1;
		sprintf (fname, "split%05d.pgp", file_count);
		file_count += 1;
		outfile = fopen(fname, "w+");
		if ( outfile == NULL ) {
		    perror(fname);
		    exit(1);
		}
	    }
	}
	i = fwrite(p_buf, p_len, 1, outfile);
	free(p_buf);
	if ( i != 1 ) {
	    fprintf(stderr, "Error while writing\n");
	    exit(1);
	}
    }

    return (EXIT_SUCCESS);
}
