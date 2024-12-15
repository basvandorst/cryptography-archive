/*********************************************************************
 * Filename:      armor.c<2>
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:35:48 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


#include <pgplib.h>

#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>


static char *progname;

void
usage()
{
    fprintf(stderr, "usage: %s [-d] infile outfile\n", progname);
    exit(1);
}

void
main(int argc, char *argv[])
{
    FILE	*fin, *fout;
    extern char *optarg;
    extern int 	optind;
    int		deflag;
    int		ch;
    long	file_size;
    char	*ibuf;
    u_char	*buffer;
    int		buf_len;
    int		used;
    /* scratch */
    int		i;

    progname = argv[0];

    if ( argc < 3 )
	usage();

    file_size = 0;

    fout = stdout;
    deflag = 0;
    while ((ch = getopt(argc, argv, "d")) != -1)
	switch(ch) {
	case 'd':
	    deflag = 1;
	    break;
	case '?':
	default:
	    usage();
	}

    if ( (fin = fopen(argv[1+deflag], "r")) == NULL) {
	perror("fopen");
	exit(1);
    }
    fseek(fin, 0L, SEEK_END);
    file_size = ftell(fin);
    ibuf = calloc(file_size, 1);
    if ( ibuf == NULL ) {
	fprintf(stderr, "Out of memory\n");
	exit(1);
    }
    rewind(fin);
    i = fread(ibuf, file_size, 1, fin);
    if ( i != 1 ) {
	fprintf(stderr, "Error: fread\n");
	exit(1);
    }
    fclose(fin);

    if (( fout = fopen(argv[2+deflag], "w+")) == NULL) {
	perror("open");
	exit(1);
    }

    if ( deflag ) {
	i = dearmor_buffer(ibuf, file_size, &used, &buffer, &buf_len);
	switch ( i ) {
	case -1:
	    fprintf(stderr, "Internal Error\n");
	    exit(-1);
	case 1:
	    fprintf(stderr, "Incorrect format\n");
	    exit(-1);
	default:
	    i = fwrite(buffer, buf_len, 1, fout);
	    if ( i != 1) {
		fprintf(stderr, "Could not write result\n");
		exit(-1);
	    }
	    fflush(fout);
	}
    }
    else {
	i = armor_buffer((u_char*)ibuf, (int)file_size, &i, (char **)&buffer,
			 &buf_len);
	if ( i < 0 ) {
	    fprintf(stderr, "Some error while armor'ing\n");
	    fclose(fout);
	    unlink(argv[2+deflag]);
	    exit(-1);
	}
	i = fwrite(buffer, buf_len, 1, fout);
	exit(0);
    }
}
