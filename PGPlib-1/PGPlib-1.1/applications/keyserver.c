/*********************************************************************
 * Filename:      keyserver.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 11:01:18 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pgplib.h>

#ifndef HAVE_DAEMON
#  include "daemon.c"
#endif

#ifndef HAVE_STRERROR
#  include "strerror.c"
#endif

/* Not all OS'es declare this. */
extern int h_errno;


void
print_info(FILE *fd)
{

    char *s =
	"This keyserver is build on a library that makes it possible to \n"
	"use public keys in software (and not only on the command line by \n"
	"means of PGP).  There are routines to search PGP keyrings for \n"
	"keys, to generate packets that are fully compatible with those \n"
	"PGP writes, and to parse (and unarmor, uncompress and decrypt) \n"
	"packets written by PGP.  With it you can also create files \n"
	"(literal, compressed and armored) that can be manipulated by PGP.\n"
	"\n"
	"If the server turns out to be useful for the PGP community, that \n"
	"would be nice since it is a spin-off rather than a target.  The \n"
	"server handles one connection at the time (rather than fork'ing) \n"
	"to avoid having to reload the database for every request.\n"

	"The library has been written as part of the PASTA project; the \n"
	"security effort of PASTA is undertaken by Arne Helme (arne@acm.org)\n"
	" and myself, with the usual cast of students.  We are investigating\n"
	" the effects on system design when _personal_ machines are \n"
	"introduced.\n"
	"\n"
	"The cryptographic engine is SSLeay\n"
	" * ftp://ftp.psy.uq.oz.au/pub/Crypto/SSL/ - SSLeay source\n"
	" * ftp://ftp.psy.uq.oz.au/pub/Crypto/SSLapps/ - SSL applications\n"
	" * http://www.psy.uq.oz.au/~ftp/Crypto/ssl.html - SSLeay Programmer\n"
	"						   Reference\n"
	" * ftp://ftp.psy.uq.oz.au/pub/Crypto/SSLapps/PORT4-5 porting notes\n"
	"I have used the BN library, RSA-code and IDEA code.\n"
	"\n"
	"If you have any comments or suggestions, feel free to send me an\n"
	"e-mail.\n"
	"\n"
	"Tage Stabell-Kulø  (tage@acm.org)\n"
	"Department of Computer Science\n"
	"University of Tromsø\n"
	"Norway\n";
    fprintf(fd, "%s\n", s);

}


FILE	*logfile;


void
get_key( char *arg, FILE *fd)
{
    u_long	short_keyid;
    PGPKEYID	key_id, *keyptr;
    PGPuserid_t	**uid;
    int		num_keys;
    int		key_len;
    int		ret;
    char	*string;
    u_char 	*buf, *armor_buf;
    int		buflen, armor_len;
    /* scratch */
    int		index,i,j;

    PGPKEYID_ZERO(key_id);
    short_keyid = 0L;
    ret = -1;

    if ( ! strncmp(arg, "ARMOR ", 6)) {
	if ( ! PGPKEYID_FromAscii(arg+6,&key_id))
	{
	    fprintf(fd,
"You should type \"ARMOR KeyID\" (without quotes) where KeyID is a real\n"
"KeyID in the format 0x033367ab71fbd469.  You can find the real KeyID\n"
"of any key with this server.\n");
	    return;
	}
	i = get_pubkey_rec(key_id, &buflen, &buf);
	if ( i != 0 ) {
	    if ( i == -1 ) {
		fprintf(logfile,
			"Error from get_pub_key_record on key %#.16" PRIx64 \
			"\n", key_id);
		fprintf(fd, "Some error from get_pub_key_record\n");
		return;
	    }
	    else {
		fprintf(fd,
"No such key (are you sure you provided a real KeyID (64 bits long)).\n");
		return;
	    }
	    assert(0);
	}
	/* Found a key */
	i = armor_buffer(buf, buflen, &j, (char **)&armor_buf, &armor_len);
	fprintf(fd, "%s\n", armor_buf);
	free(buf);
	free(armor_buf);
	return;
    } /* if ARMOR */

    if ( arg[0] == '0' && toupper(arg[1]) == 'X') {
	key_len = strlen(arg);
	switch ( key_len ) {
	case 10:
	    ret = sscanf(arg, "%lx", &short_keyid);
	    break;

	case 18:
	    key_id = strtouq (arg, &endptr, 0);
	    if ( *endptr == '\0' && endptr != arg)
		ret = 1;
	    else
		ret = 0;
	    break;

	default:
	    ret = 0;
	    break;
	}

	if ( ret != 1 ) {
	    fprintf(fd,
		    "[0x01234576 | 0x0123456789012345 | FooBar]\n");
	    return;
	}

	if ( short_keyid != 0 ) {
	    /* Find the real key */
	    int		i;
	    u_quad_t	tmpkey;

	    i = find_shortid(arg, &tmpkey);
	    if ( i == -1 )
		return;
	    if ( tmpkey == 0 ) {
		fprintf(fd, "Found no matching key\n");
		return;
	    }
	    key_id = tmpkey;
	}
	/* A real key */
	i = get_pubkey_rec(key_id, &buflen, &buf);
	switch ( i ) {
	case -1:
	    fprintf(fd, "Some error\n");
	    return;
	case 1:
	    fprintf(fd, "No (correct) key found\n");
	    return;
	case 0:
	    break;
	default:
	    assert(0);
	}
	/* Print what we found */
	print_keyrec(buf, buflen, fd);
	free(buf);
	return;
    }
    else {
	/* Not a key but a string */
	string = arg;
	while ( isspace(*string) )
	    string += 1;

	if ( strlen(string) < 3 ) {
	    fprintf(fd,
"Sorry, that is a little too short.  I will get exhausted if I am to find\n"
"all keys that matches such a sort pattern.\n");
	    return;
	}
	if ( !strcasecmp(string, "com") || !strcasecmp(string, "edu") ||
	     !strcasecmp(string, "org") || !strcasecmp(string, "acm"))
	{
	    fprintf(fd,
"Can it be necessary to check whether there are UserID's with e-mail\n"
"addresses within them ?\n"
"If you _really_, _really_ need to torment me like this, try looking for\n"
".com (or .edu or .org) instead.\n");
	    return;
 	}
	if ( !strcasecmp(string, ".com") || !strcasecmp(string, ".edu") ||
	     !strcasecmp(string, ".org")) {
	    fprintf(fd,
"Well, sorry.  If you just want to be a pain-in-the-back I am sure you\n"
"can find other means.  If you have any (any at all) useful reason for this\n"
"request, I will be very happy to hear from you.  Try \"INFO\" instead.\n");
	    return;
 	}

	/* See if the string has been saved exactly */

	i = keys_from_string(string, 0, &num_keys, &keyptr);

	if ( i == -1) {
	    fprintf(fd,  "Some error in find_userid\n");
	    return;
	}
	if ( num_keys == 0 ) {
	    fprintf(fd, "Found no keys that matched\n");
	    return;
	}
	fprintf(fd, "Found these keys:\n");
	for(index=0; index < num_keys; index++ ) {
	    fprintf(fd, "\t%#.16" PRIx64 "\n", keyptr[index]);
	    uid = NULL;
	    i = uids_from_keyid(keyptr[index], &uid);
	    if ( i == -1 ) {
		fprintf(fd, "Some internal error (code 0x56345)\n");
		fprintf(logfile, "Some internal error\n");
		return;
	    }
	    if ( i == 1 ) {
		fprintf(fd, "No matching UID\n");
		continue;
	    }
	    assert(i==0);
	    for(j=0; uid[j] != NULL; j++) {
		fprintf(fd, "\t\t%s\n", uid[j]->name);
		free_uid(uid[j]);
	    }
	    assert(uid != NULL);
	    free(uid);
	}
	free(keyptr);
	return;
    }
}



void
usage(void)
{
    fprintf(stderr, "keyserver [-d ] -p port -f file [-l file]\n");
    exit(1);
}

#define LINELEN  80

char Arg[LINELEN];

int
main(int argc, char *argv[])
{

    int		ch;
    extern char	*optarg;
    int		port;
    char 	*database;
    int		server;
    int		s;
    struct sockaddr_in	sin;

    /* scratch */
    char	*pp;
    int		i;

    port = 0;
    server = 0;
    logfile  = NULL;
    database = NULL;
    while ((ch = getopt (argc, argv, "dp:f:l:")) != -1) {
	switch (ch) {
	case 'd':
	    server = 1;
	    break;
	case 'f':
	    database = optarg;
	    i = keydb_init(optarg, READER);
	    if ( i != 0 )
		usage();
	    break;
	case 'l':
	    logfile = fopen(optarg, "a+");
	    if ( logfile == NULL ) {
		perror("fopen");
		usage();
	    }
	    i = setvbuf(logfile, (char *)NULL, _IOLBF, 0);
	    if ( i == EOF ) {
		fprintf(stderr, "Could not set vbuf\n");
		exit(1);
	    }
	    break;
	case 'p':
	    errno = 0;
	    i = (int) strtol(optarg, &pp, 10);
	    if ( i > 0xffff) {
		fprintf(stderr, "Portnumber to large\n");
		usage();
	    }
	    if ( pp == NULL || *pp != 0)
		usage();
	    port = i;
	    break;
	case '?':
	default:
	    usage();
	}
    }

    if ( port == 0 )
	usage();
    if ( logfile == NULL )
	logfile = stderr;
    if ( database == NULL )
	usage();
    s = socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 ) {
	perror("socket");
	exit(1);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    i = bind(s, (struct sockaddr *) &sin, sizeof(sin));
    if ( i < 0 ) {
	perror("bind");
	exit(1);
    }
    listen(s, 5);

    if ( server == 1 ) {
	if ( daemon(1, 0) < 0 ) {
	    perror("daemon");
	    exit(1);
	}
    }

    /*
     * If you didn't specify -l but did use -d, your error messages
     * are lost at this point
     */


/* **************************************************************
 * OK, here is the keyserver.  We accept a single connection,
 * read the request, and try to do something with it.
 * ************************************************************** */


    while ( 1 ) {
	int 			sock, len;
	struct sockaddr_in 	from;
	struct hostent		*hp;
	char  		what_time[26];	/* Local representaion of the time */
	struct timeval 	started, now;	/* To get the above */
	double		total_cpu, sess_cpu;
	int		total_sec;
	fd_set		selfd;
	struct timeval	tval;
	FILE		*fd;
	char		*p;

	char        line[LINELEN];

	len = sizeof(from);

	sock = accept(s, (struct sockaddr *)&from, &len);
	if ( sock < 0 ) {
	    fprintf(logfile, "%s : %s\n", what_time, strerror(errno));
	    continue;
	}

	gettimeofday(&started, (struct timezone *) 0);
	sprintf(what_time, "%s", ctime( (time_t*)&started.tv_sec));

	total_cpu = clock();

	hp = gethostbyaddr((char *)&from.sin_addr, len, AF_INET);
	if ( hp != NULL ) {
	    fprintf(logfile, "%s [%s] %s", hp->h_name,
		    inet_ntoa(from.sin_addr), what_time);
	}
	else {
#ifdef HAVE_HSTRERROR
	    fprintf(logfile, "Error from gethostbyaddr: %s\n",
		    hstrerror(h_errno));
#else
	    fprintf (logfile, "Error from gethostbyaddr: h_errno = %d\n",
		     h_errno);
#endif /* HAVE_HSTRERROR */
	}

	fd = fdopen(sock, "a+");
	if ( fd == NULL ) {
	    fprintf(logfile, "fdopen: %s\n", strerror(errno));
	    close(sock);
	    continue;
	}

	fprintf(fd, "\nWelcome to the PASTA keyserver.  Local time is %s\n",
		what_time);

	fprintf(fd,
"Type a string (\"Tage Stabell-Kulø\") which will be searched for\n"
"among the available UserIDs (not found due to the Norvegian character),\n"
"a UserID (\"Tage Stabell-Kulo <tage@acm.org>\") which will be looked\n"
"up (or, failing that, searched for), a short keyID the way PGP\n"
"prints them (\"0x71FBD469\") or a real keyID the way PGP actually\n"
"stores them  (\"0x033367ab71fbd469\") or the word \"INFO\" (IN CAPITALS).\n"
"Do not expect the result to look like what you get from PGP.\n"
"\n"
"If you do ARMOR 0x033367ab71fbd469 (or some other KeyID) you will\n"
"get that key with associated UserID and sertificates armored (PGP -kxa).\n"
"The usefulness of this was pointed out by Peter N. Wan.\n"
"\n"
"The server is running on an old HP9000/433 (that is MC68040).\n\n\n"
">");
	fflush(fd);

	FD_ZERO(&selfd);
	FD_SET(sock, &selfd);
	/* Let the other end have 30 sec's to read this text. */
	tval.tv_sec = 30;
	tval.tv_usec = 0;
	i = select(32, &selfd, NULL, NULL, &tval);
	if ( i == -1 ) {
	    fprintf(logfile, "Error from select\n");
	    perror("select");
	    exit(1);
	}
	if ( i == 0 ) {
	    fprintf(fd, "Timeout - Sorry\n");
	    fclose(fd);			/* fclose fflush'es */
	    close(sock);
	    continue;
	}
	if ( fgets(line, LINELEN, fd) == NULL) {
	    fclose(fd);
	    close(sock);
	    continue;
	}

	fprintf(logfile, "\t%s", line);

	if ( strncmp(line, "INFO\r\n", 6)) {
	    /* Not INFO */
	    p = strrchr(line, '\r');
	    if ( p != NULL)
		*p = 0;
	    /* Then, just to be sure */
	    p = strrchr(line, '\n');
	    if ( p != NULL )
		*p = 0;

	    /* Keep a copy of the arguments on the command line */
	    strncpy(Arg, line, LINELEN);

	    fprintf(fd, "----------------\n");
	    get_key(line, fd);
	    fprintf(fd, "----------------");
	}
	else {
	    /* Asked for INOF */
	    print_info(fd);
	}

	/* Tell him how much time we used */
	gettimeofday(&now, (struct timezone  *)0);

	if (started.tv_usec > now.tv_usec) {
	    now.tv_usec += 1000000;
	    now.tv_sec--;
	}
	total_sec =  (now.tv_sec  - started.tv_sec);


	sess_cpu = clock() - total_cpu;

	fprintf(logfile, "\t%d seconds (of which %.2f CPU seconds)\n",
		total_sec, sess_cpu / (double)CLOCKS_PER_SEC);

	fprintf(fd, "\n\tYou spent %d seconds, (of which %.2f sec CPU)\n",
		total_sec, sess_cpu / (double)CLOCKS_PER_SEC);
	fprintf(fd, "\tIn total, %.2f seconds CPU consumed by this server\n",
		clock()/(double)CLOCKS_PER_SEC);
	fprintf(fd,
"\n\tThank you for using this experimental keyserver\n"
"\tPlease report any problems (or suggestions) to\n"
"\tTage Stabell-Kulø (tage@acm.org).\n\n");

	fclose(fd);
	close(sock);
	continue;

    }
}
