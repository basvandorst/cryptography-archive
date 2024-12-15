/*********************************************************************
 * Filename:      p_decrypt.c
 * Description:
 * Author:        Thomas Roessler <roessler@guug.de>
 * Modified at:	  Mon Jan 19 10:09:18 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

/* based on p_encrypt.c */

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
	    "usage: %s [-a] -f file -p password -u user\n",
	    Progname);
    exit(1);
}

void
main(int argc, char *argv[])
{
    int		ch;
    extern char	*optarg;
    FILE	*infile;
    char	*filename;
    u_char	digest[16];
    int		digest_ok;
    PGPseckey_t	*key, armed_key;
    PGPpke_t	pke;
    FILE 	*secring;
    PGPliteral_t literal;
    int		armor;
    u_char	*pke_packet;
    u_char	*cke_packet;
    /* scratch */
    u_char	*outbuf;
    int		outlen;
    int 	outfile;
    int		i;
    PGPKEYID	key_id;

    Progname = argv[0];

    armor = 0;
    filename = NULL;
    secring = NULL;
    digest_ok = 0;
    while ((ch = getopt (argc, argv, "a:f:p:u:k:")) != -1) {
	switch (ch) {
	case 'f':
	    i = access(optarg, R_OK);
	    if ( i < 0 ) {
		perror(optarg);
		usage();
	    }
	    filename = strdup(optarg);
	    break;
	case 'k':
	    secring = fopen(optarg, "r");
	    if (secring == NULL) {
		perror(optarg);
		usage();
	    }
	    break;
	case 'p':
	    MD5(optarg, strlen(optarg), digest);
	    digest_ok = 1;
	    break;

	case '?':
	default:
	    usage();
	}
    }

    if ( filename == NULL || digest_ok == 0 || secring == NULL )
	usage();

    if((infile = fopen(filename, "r")) == NULL) {
	perror("filename");
	exit(1);
    }

    if(read_packet(infile, &pke_packet) ||
       read_packet(infile, &cke_packet))
    {
	fprintf(stderr, "bad packet(s).\n");
	exit(1);
    }

    fclose(infile);

    i = buf2pke(pke_packet, packet_length(pke_packet), &pke);
    if ( i != 0 ) {
	fprintf(stderr, "bad pke packet: %s\n", pgplib_errlist[pgplib_errno]);
	exit(1);
    }

    do {
	if(get_seckey_from_file(secring, &key)) {
	    fprintf(stderr, "Could not find secret key\n");
	    exit(1);
	}
	PGPKEYID_FromBN(&key_id,key->N);
    } while( ! PGPKEYID_EQUAL(pke.key_id,key_id));

    fclose(secring);

    if (decrypt_seckey(key, digest, &armed_key)) {
	fprintf(stderr, "Password incorrect\n");

	exit(1);
    }

    if((i = sec_decrypt_packet(&armed_key, pke_packet,
			       packet_length(pke_packet),
			       cke_packet, packet_length(cke_packet),
			       &outbuf, &outlen)))
    {
	fprintf(stderr, "sec_dec_packet: %d\n", i);
	exit(1);
    }
    clear_seckey(&armed_key);

    if(is_ctb(outbuf[0]) && is_ctb_type(outbuf[0], CTB_COMPRESSED_TYPE)) {
	u_char *ob;
	int ol;

	decompress_buffer(outbuf, outlen, &ob, &ol);
	free(outbuf);
	outbuf = ob;
	outlen = ol;
    }

    if(buf2literal(outbuf, outlen, &literal)) {
	fprintf(stderr, "Not a literal packet.\n");
	exit(1);
    }


    if((outfile = open(literal.filename, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1)
    {
	perror(literal.filename);
	exit(1);
    }

    write(outfile, literal.data, literal.datalen);
    close(outfile);


    free_literal(&literal);

    exit(0);
}
