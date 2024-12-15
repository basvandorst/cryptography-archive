/*********************************************************************
 * Filename:      sign.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:52:54 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <md5.h>

#include <pgplib.h>


char *Progname;

void
usage()
{
    fprintf(stderr,
	    "usage: %s [-a] -f file -k keyring -p password [-u KeyID]\n", Progname);
    exit(1);
}

void
main(int argc, char *argv[])
{
    int		ch;
    extern char	*optarg;
    FILE	*secring;
    int		infile, outfile;
    char	*filename;
    u_char	passwdhash[16];
    int		digest_ok;
    u_char	*filebuf;
    int		armor;
    PGPKEYID	key_id;
    struct stat sbuf;
    PGPseckey_t	*key, armed_key;
    PGPsig_t	sig;
    int		armor_len, buflen;
    u_char	*sigbuf;
    char	*armor_buf;
    /* scratch */
    int		i, j;
    PGPKEYID	key_id2;

    Progname = argv[0];

    armor = 0;
    secring = NULL;
    filename = NULL;
    outfile = infile = -1;
    digest_ok = 0;
    PGPKEYID_ZERO(key_id);

    while ((ch = getopt (argc, argv, "ak:f:p:u:")) != -1) {
	switch (ch) {
	case 'a':
	    armor = 1;
	    break;
	case 'f':
	    i = access(optarg, R_OK);
	    if ( i < 0 ) {
		perror(optarg);
		usage();
	    }
	    infile = open(optarg, O_RDONLY);
	    if ( infile < 0 ) {
		perror(optarg);
		usage();
	    }

	    filename = calloc(1, strlen(optarg) + 4 + 1);
	    if ( filename == NULL ) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	    }
	    strcpy(filename, optarg);
	    strcat(filename, ".pgp");
	    break;

	case 'k':
	    secring = fopen(optarg, "r");
	    if ( secring == NULL ) {
		perror(optarg);
		usage();
	    }
	    break;

	case 'p':
	    MD5(optarg, strlen(optarg), passwdhash);
	    digest_ok = 1;
	    break;
	case 'u':
	    if (!PGPKEYID_FromAscii(optarg,&key_id))
	    {
		fprintf(stderr, "Error in keyid: %s\n", optarg);
		usage();
	    }
	    break;

	case '?':
	default:
	    usage();
	}
    }

    if ( secring == NULL || filename == NULL || digest_ok == 0)
	usage();

    while ( 1 ) {
	i = get_seckey_from_file(secring, &key);
	if ( i != 0 ) {
	    fprintf(stderr, "Could not find secret key\n");
	    exit(1);
	}
	if ( PGPKEYID_IsZero(key_id) )
	    break;
	else {
	    PGPKEYID_FromBN(&key_id2,key->N);
	    if ( !PGPKEYID_EQUAL(key_id,key_id2) )
		continue;
	    else
		break;
	}
    }

    /* If we get here, we're OK */

    i = fstat(infile, &sbuf);
    if ( i < 0 ) {
	perror("fstat");
	exit(1);
    }
    filebuf = malloc(sbuf.st_size);
    i = read(infile, filebuf, sbuf.st_size);
    (void) close(infile);

    /* Make the key ready */

    i = decrypt_seckey(key, passwdhash, &armed_key);
    if ( i != 0 ) {
	fprintf(stderr, "Password incorrect\n");
	exit(1);
    }

    memset(&sig, 0, sizeof(sig));
    i = sign_buffer(filebuf, sbuf.st_size, &armed_key, &sig);
    if ( i != 0 ) {
	fprintf(stderr, "Could not sign buffer\n");
	exit(1);
    }
    clear_seckey(&armed_key);

    /* convert */
    i = signature2buf(&sig, &sigbuf);
    if ( i != 0 ) {
	fprintf(stderr, "Could not marshall signature\n");
	exit(1);
    }
    /* store */
    outfile = open(filename, O_WRONLY|O_CREAT, 0600);
    if ( outfile < 0 ) {
	perror(filename);
	fprintf(stderr, "Eror open\n");
	usage();
    }

    buflen = packet_length(sigbuf);
    if (armor) {
	i = armor_buffer(sigbuf, buflen, &j, &armor_buf, &armor_len);
	if ( i != 0 ) {
	    fprintf(stderr, "Error armoring\n");
	    close(outfile);
	    unlink(filename);
	    exit(1);
	}
	free(sigbuf);
	sigbuf = armor_buf;
	buflen = armor_len;
    }
    i = write(outfile, sigbuf, buflen);
    if ( i != buflen) {
	fprintf(stderr, "Error writing\n");
	perror(filename);
	exit(1);
    }

    exit(0);
}
