/*********************************************************************
 * Filename:      parser.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:10:12 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <md5.h>

#include <pgplib.h>


#define PASSWORD "Test"

char 	*progname;
int 	verbose;
int	more_to_do;

void
usage(void)
{
    fprintf(stderr, "Usage: %s [-v] [-p password] [-k pubring] file\n",
	    progname);
    exit(1);
}

#ifdef 0
void
print_seckey(int        verbose,
             FILE       *fd,
             PGPseckey_t *key)
{
    PGPKEYID    q;

    fprintf(fd, "Length  \t%d\n", (int)key->length);
    fprintf(fd, "Version \t%d\n", (int)key->version);
    fprintf(fd, "Created \t%ld\n", key->created);
    fprintf(fd, "Valid  \t%d\n", key->valid);
    if ( verbose ) {
        fprintf(fd, "N          ");
        BN_print_fp(fd, key->N);
        fprintf(fd, "\nE \t\t");
        BN_print_fp(fd, key->E);
        fprintf(fd, "\n");
    }
    else {
        PGPKEYID_FromBN(&q, key->N);
        fprintf(fd, "KeyID \t\t%s\n", PGPKEYID_ToStr(&q));
    }
    fprintf(fd, "Alg \t\t%s\n", key->algorithm ==  RSA_ALGORITHM_BYTE ?
        "RSA\n" : "Unknown\n");
}
#endif

int
main(int argc, char *argv[])
{
    /* Used by getopt */
    extern char	*optarg;
    extern int 	optind;

    /* USed to keep context between calls */
    char 	password[32];
    MD5_CTX 	context;
    unsigned char digest[16];
    unsigned char CTB;
    int 	length;
    int 	count, used;
    int 	fd;
    unsigned char *file_buf;
    char 	*pubring;
    struct stat s_buf;
    /* scratch variables */
    int 	i, j,ch;
    unsigned char *data;

    PGPliteral_t	literal;


    progname = argv[0];
    password[0] = 0;
    pubring = NULL;
    fd = 0;
    verbose = 0;
    count = 0;

    while ((ch = getopt(argc, argv, "vp:k:")) != -1) {
	switch(ch) {
	case 'k':
	    i = strlen(optarg);
	    if ( i > MAXPATHLEN) {
		fprintf(stderr, "%s: Pubring too long\n", progname);
		exit(1);
	    }	    else
		pubring = strdup(optarg);
	    break;
	case 'p':
	    if ( strlen(optarg) > 31 ) {
		(void) fprintf(stderr, "%s: Password too long\n", progname);
		exit(1);
	    }
	    strcpy(password, optarg);
	    break;
	case 'v':
	    verbose = 1;
	    break;
	default:
	    usage();
	}
    }
    argc -= optind;
    argv += optind;

    if ((fd = open(argv[0], O_RDONLY, 0)) < 0) {
	usage();
    }
    if ( password[0] == 0 )
	strcpy(password, PASSWORD);

    /* Get the entire file into memory for easy access */

    i = fstat(fd, &s_buf);
    if ( i < 0 ) {
	perror(progname);
	exit(1);
    }
    length = s_buf.st_size;
    file_buf = (unsigned char *)calloc(length, 1);
    if (file_buf == NULL ) {
	fprintf(stderr, "Out of memory on %s:%d\n", __FILE__, __LINE__);
	exit(1);
    }
    i = read(fd, file_buf, length);
    if ( i != length ) {
	perror(progname);
	exit(1);
    }
    /* Since PGP has many layers of "objects", we must loop over them
     * and do whatever we have to do.
     */

    more_to_do = 0;
    do {
	CTB = *file_buf;

	/* This test is _very_ weak */
	if ( ! is_ctb(CTB)) {
	    fprintf(stderr, "First byte definitely not CBT, but %d\n", (int)CTB);
	    exit(1);
	}

	/* Find the type */
	switch ( (CTB & CTB_TYPE_MASK) >> 2) {

	case CTB_USERID_TYPE: {
	    PGPuserid_t uid;

	    if ( verbose )
		fprintf(stdout, "CTB is UID package\n");
	    used = buf2uid(file_buf, length, &uid);
	    fprintf(stdout, "Name = %s\n", uid.name);
	    free_uid(&uid);
	    if ( length - used > 0 ) {
		data = malloc(length - used);
		memcpy(data, file_buf+used, length-used);
		free(file_buf);
		file_buf = data;
		length -= used;
		more_to_do = 1;
	    }
	    else
		more_to_do = 0;
	    break;
	}
	case CTB_CERT_SECKEY_TYPE : {
	    PGPseckey_t	*seckey;

	    if ( verbose )
		fprintf(stdout, "CTB is secret key package\n");
	    used = get_seckey_from_buf(file_buf, length, &seckey);
	    if ( used == -1 ) {
		fprintf(stderr, "Error in buffer (no seckey)\n");
		exit(1);
	    }
	    if ( used == 0 ) {
		if ( verbose )
		    fprintf(stdout, "No more\n");
		more_to_do = 0;
		break;
	    }
	    /*print_seckey(verbose, stdout, seckey);*/
	    free_seckey(seckey);
	    free(seckey);

	    if ( length - used > 0 ) {
		data = malloc(length - used);
		memcpy(data, file_buf+used, length-used);
		free(file_buf);
		file_buf = data;
		length -= used;
		more_to_do = 1;
	    }
	    break;
	}
	case CTB_COMPRESSED_TYPE:
	    if ( verbose )
		fprintf(stdout, "CTB is Compressed Data Packet\n");
	    i = decompress_buffer(file_buf, length, &data, &j);
	    if ( i != 0 ) {
		fprintf(stderr, "Didn't decompress properly\n");
		exit(1);
	    }
	    free(file_buf);
	    file_buf = data;
	    length = j;
	    more_to_do = 1;
	    break;

	case CTB_LITERAL_TYPE:
	    fprintf(stderr, "Ha, Ha\n");
	    exit(1);
	case CTB_LITERAL2_TYPE:
	    fprintf(stdout, "CTB is Raw Literal plaintext data, with filename and mode\n");

	    i = buf2literal(file_buf, length, &literal);
	    if ( i != 0 ) {
		fprintf(stderr, "Didn't find literal\n");
		exit(1);
	    }
	    /* create the file */
	    errno = 0;
	    fd = open(literal.filename, O_WRONLY|O_CREAT|O_EXCL, 0600);
	    if ( fd < 0 ) {
		perror(literal.filename);
		exit(1);
	    }
	    write(fd, literal.data, literal.datalen);
	    close(fd);
	    more_to_do = 0;
	    break;

	case CTB_CKE_TYPE:
	    if (verbose )
		fprintf(stdout,
			"CTB is type Conventional-key-encrypted data\n");
	    /* Convert the password to a 16-byte key by means of MD5 */
	    MD5_Init(&context);
	    MD5_Update(&context, password, strlen(password));
	    MD5_Final(digest, &context);
	    if ( verbose ) {
		fprintf(stdout, "The MD5 hash of the password is :");
		for (i = 0; i < 16; i++)
		    fprintf(stdout, "%02x", digest[i]);
		fprintf(stdout, "\n");
	    }
	    i = conventional_key_decrypt(file_buf, digest, &length, &data);
	    switch ( i ) {
	    case -1:
		fprintf(stderr, "Decrypt returned -1\n");
		exit(-1);
	    case 1:
		fprintf(stderr, "Syntax error\n");
		exit(1);
	    case 2:
		fprintf(stderr, "Wrong key\n");
		exit(-1);
	    case 0:
		break;
	    default:
		assert(0);
	    }
	    free(file_buf);
	    file_buf = data;
	    more_to_do = 1;

	    break;

	default:
	    fprintf(stderr, "Some strange type (%d)\n",
		    (CTB & CTB_TYPE_MASK) >> 2);
	    exit(1);
	}
	if ( more_to_do )
	    fprintf(stdout, "----------------\n");
    }while (more_to_do);

    return (EXIT_SUCCESS);
}
