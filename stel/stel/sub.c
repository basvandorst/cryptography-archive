/*
 * $Author: vince $
 * $Header: /users/vince/src/stel-dist/stel/RCS/sub.c,v 1.101 1996/04/26 16:29:23 vince Exp vince $
 * $Date: 1996/04/26 16:29:23 $
 * $Revision: 1.101 $
 * $Locker: vince $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: sub.c,v 1.101 1996/04/26 16:29:23 vince Exp vince $";
#endif	lint

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <syslog.h>
#include <varargs.h>

#include "regex.h"

#ifdef HAS_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifndef SOLARIS
#if (defined(BSD) && (BSD >= 199306))
#define SIGCLD  SIGCHLD
#endif
#endif

/* md5 stuff */
#include "global.h"
#include "md5.h"

#include "gmp.h"

#include "defs.h"

/* idea stuff */
#include "idea.h"
IDEAkey			s_ideakey;	/* server */
IDEAkey			c_ideakey;	/* client */

/* des stuff */
/* #include "des.h" */
des_key_schedule	s_schedule1;	/* server */
des_key_schedule	s_schedule2;	/* server */
des_key_schedule	s_schedule3;	/* server */
des_key_schedule	c_schedule1;	/* client */
des_key_schedule	c_schedule2;	/* client */
des_key_schedule	c_schedule3;	/* client */

des_cblock		s_IV;		/* server, IDEA and DES only */
des_cblock		c_IV;		/* client, IDEA and DES only */

/* rc4 stuff */
#include "rc4.h"
RC4Context		s_rc4;		/* server */
RC4Context		c_rc4;		/* client */

/* rc5 stuff */
unsigned char		s_rc5[16];	/* server */
unsigned char		c_rc5[16];	/* client */

/* initialisation variables */

/* global */
char		sessionkeyhash[8];

static char	*buildregexp();
static		oneofthese();
static		doaddrmatch();
static void	CFBcrypt();

establish(portnum)
int	portnum;
{
	struct sockaddr_in	sa;
	int			s, on = 1;
	struct servent		*serv;

	if (!portnum) {
		if ((serv = getservbyname(STELCLIENT, "tcp")) == NULL)
			portnum = PORTNUM;
		else
			portnum = serv->s_port;
	}
	
	sa.sin_family 		= AF_INET;
	sa.sin_addr.s_addr	= INADDR_ANY;
	sa.sin_port		= htons((u_short)portnum);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		syslog(LOGPRI, "socket(): %m");
		return(-1);
	}

	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
					(char *)&on, sizeof (on)) == -1) {
		syslog(LOGPRI, "setsockopt (SO_KEEPALIVE): %m");
		return(-1);
	}

#ifdef DEBUG
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
					(char *)&on, sizeof (on)) == -1) {
		syslog(LOGPRI, "setsockopt (SO_REUSEADDR): %m");
		return(-1);
	}
#endif
	
	if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		perror("bind()");
		(void)fprintf(stderr, "cannot bind to port %d; may be another server is running?\n",
							portnum);
		syslog(LOGPRI, "bind(%d): %m", portnum);
		return(-1);
	}
	
	if (listen(s, 10) == -1) {
		syslog(LOGPRI, "listen(): %m");
		return(-1);
	}
	return(s);
}

getconnection(s, hostname)
int	s;
char	*hostname;
{
	struct sockaddr_in	sa;
	struct hostent		*he;
	int			t, salen;

	salen = sizeof(sa);
	if ((t = accept(s, (struct sockaddr *)&sa, &salen)) < 0) {
		return(-1);
	}
	
	if ((he = gethostbyaddr((char *)&sa.sin_addr,
				sizeof(struct in_addr), AF_INET)) == NULL)
		(void)strcpy(hostname, (char *)inet_ntoa(sa.sin_addr));
	else
		(void)strncpy(hostname, he->h_name, MAXHNAMELEN);
	return(t);
}

char *
peername(fd)
int	fd;
{
	struct sockaddr_in	addr;
	struct hostent		*he;
	static char		name[MAXHNAMELEN];
	int			addrlen = sizeof(addr);

	if (getpeername(fd, (struct sockaddr *)&addr, &addrlen) == -1)
		return(NULL);

	if ((he = gethostbyaddr((char *)&addr.sin_addr,
				sizeof(struct in_addr), AF_INET)) == NULL)
		(void)strcpy(name, (char *) inet_ntoa(addr.sin_addr));
	else
		(void)strncpy(name, he->h_name, sizeof(name));
	return(name);
}

callsocket(hostname, portnum)
char	*hostname;
int	portnum;
{
	struct sockaddr_in	sa;
	struct hostent		*hp;
	int			s, on = 1;
	extern int		errno;

	bzero(&sa, sizeof(sa));
	if ((hp = gethostbyname(hostname)) == NULL) {
		if ((sa.sin_addr.s_addr = inet_addr(hostname)) == -1) {
			(void)fprintf(stderr, "%s: unknown host\n", hostname);
			return(-1);
		}
	}
	else {
		(void)memcpy((char *) &sa.sin_addr, hp->h_addr, hp->h_length);
		strcpy(hostname, hp->h_name);
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons((u_short)portnum);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return(-1);
	}
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
							sizeof(on)) == -1) {
		perror("setsockopt(SO_KEEPALIVE)");
		return(-1);
	}
	
#if defined(SOCKS)
	if (Rconnect(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
#else
	if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
#endif
                perror("connect()");
                return(-1);
        }
	
	return(s);
}

void
fireman()
{
#if defined(hpux) || defined(SOLARIS) || defined(IRIX) || defined(AIX) || defined(BSDI)
	int	wstatus;
#else
	union wait      wstatus;
#endif

#ifdef SOLARIS24
	(void)wait(NULL);
#else
        while (wait3(&wstatus, WNOHANG, NULL) > 0)
                ;
#endif
	(void)signal(SIGCLD, fireman);
}

#ifdef IRIX
int	getpty(line)
char	*line;
{
	int		slave;
	char		*_getpty();
	char		*mline;

	mline = _getpty(&slave, O_RDWR, 0600, 1);
	if (mline == NULL) {
		perror("_getpty()");
		return(-1);
	}
	strcpy(line, mline);
	return(slave);
}
#else /* !IRIX */
#ifdef SOLARIS
#include <sys/stropts.h>
int	getpty(line)
char	*line;
{
	int	p;
	char	*buf;
	char	*ptsname();

	if ((p = open("/dev/ptmx", O_RDWR)) < 0) {
		puts("All network ports in use");
		return(-1);
	}
	if (grantpt(p) < 0 || unlockpt(p) < 0) {
		puts("Cannot initialize pty slave");
		return(-1);
	}

	if ((buf = ptsname(p)) == 0) {
		puts("Cannot find  pty slave");
		return(-1);
	}
	strcpy(line, buf);

	return(p);
}

#else /* !SOLARIS */
int	getpty(line)
char	*line;
{
        register int p;
        register char c, *p1, *p2;
        register int i;
	char tmp[16];

#ifdef hpux
        (void) sprintf(line, "/dev/ptym/ptyXX");
        p1 = &line[13];
        p2 = &line[14];
#else
        (void) sprintf(line, "/dev/ptyXX");
        p1 = &line[8];
        p2 = &line[9];
#endif
        for (c = 'p'; c <= 'z'; c++) {
                struct stat stb;

                *p1 = c;
                *p2 = '0';
                if (stat(line, &stb) < 0)
                        continue;
                for (i = 0; i < 16; i++) {
                        *p2 = "0123456789abcdef"[i];
                        p = open(line, O_RDWR);
			if (p > 0) {
#ifdef hpux
				sprintf(tmp, "/dev/pty/tty%c%c", *p1, *p2);
#else
				sprintf(tmp, "/dev/tty%c%c", *p1, *p2);
#endif
				if (!access(tmp, R_OK | W_OK)) {
					strcpy(line, tmp);
					return(p);
				}
                        }
                }
        }
        return(-1);
}
#endif /* SOLARIS */
#endif /* !IRIX */

/* 512 bits modulus */
char	*smallmodulostring = "\
D7EE241E121D4993D8FF64313C1B57F0388243704B98F9FABC7EFC1F\
A36803B88337E7220693839293974DD183990A5AA6FD5FCE0CC3DDED\
2CE73933B270CFA7";

/* 1024 bits modulus */
char *defaultmodulostring = "\
96F5D737535D8BC982698A80AB91DAE28E84E2982071998880C736B5\
695AF14D015401A942186B865496ECBECAFE964A5E70B7031F5756C0\
60AD53528687F4FBFF059150D529638C11FA6FAB6A58785DBD62D73D\
1001014BE3EA53E97D43944F1EF83885196E5BCDD4098744B7673B54\
BA2EB1FDB29ED2C3BA3AD8644FAFF05F";

/* 2048 bits modulo */
char *largemodulostring = "\
E269EF4E 56C7CC6B 34897619 7DD83DE3 C9BE3C34 ACE6A557 36A662E5\
F8210923 3BBF3357 5C5D4924 4AE73744 EEFDA659 884A7C63 C3BAD016\
426335E0 A4FA7B2C 323C4B0E AD4D745A 6A5DB92C 99A5D040 F6F4FBC8\
13132332 F6233C68 11AD3112 5148C55A F1B99D5A DE3531ED F68217AB\
D4B1D105 AF58A55F A2C4A1D4 A5E391DF 68C8FF1C 78C6ED6F 24D7B848\
5437E97F D71B6798 6A94DF03 9BD557D7 9D038724 382E86BF 2197E021\
516DC16E DAE86723 85FB1A84 00B9822E 4A6ADEF2 3EBC75C6 02D05FF6\
CB854914 0E7B91C3 A1011399 78A1EC83 B59EB508 F51F6098 6F4CEFF3\
BA13D59C 1136D8C8 288AB1BC A56CBD92 AAAF1CED 07D21764 FF4603FB\
4D2F6E8B";

dhexchange(s, side, firstack, rndstr)
int	s, side, firstack;
unsigned char	*rndstr;
{
	MP_INT		modulo, three, sharedsecret, mysecret,
			mypublic, hispublic;
	char		pwstr[4096], exchange[4096];
	unsigned char	randbuf[4096];
	int		i, modulobits, secretkeybits;
	MD5_CTX		mdContext;
	unsigned char	digest1[16], digest2[16];
	des_key_schedule	tmpsched;
	des_cblock	c_third, s_third;

	unsigned char	*internalrnd();

	mpz_init(&modulo);
	mpz_init(&three);
	mpz_init(&sharedsecret);
	mpz_init(&mysecret);
	mpz_init(&mypublic);
	mpz_init(&hispublic);

	mpz_set_ui(&three, 3L);
	if (firstack & FLG_LARGE_MODULUS) /* 2048 bits modulo */
		mpz_set_str(&modulo, largemodulostring, 16);
	else if (firstack & FLG_SMALL_MODULUS) /* 512 bits modulus */
		mpz_set_str(&modulo, smallmodulostring, 16);
	else /* 1024 bits modulus */
		mpz_set_str(&modulo, defaultmodulostring, 16);

	/* get randomness from system */
	strncpy(randbuf, internalrnd(), sizeof(randbuf));

	/* user-provided randomness */
	if (rndstr) {
		(void)strcat(randbuf, "\n");
		(void)strncat(randbuf, rndstr,
				sizeof(randbuf) - strlen(randbuf) - 2);
	}

	/* dh: generate a random number % modulo */
	mkrnd(randbuf, &mysecret, &modulo,
			(side == CLIENT_SIDE && firstack & FLG_DEBUG));

	modulobits = mpz_sizeinbase(&modulo, 2);
	secretkeybits = mpz_sizeinbase(&mysecret, 2);
	if (side == CLIENT_SIDE && firstack & FLG_BE_VERBOSE) {
		if (firstack & FLG_DEBUG)
			printf("\nGENERATING KEYS WITH DIFFIE-HELLMAN\n");
		printf("Using %d bits modulus, %d bits secret key\n",
						modulobits, secretkeybits);
		printf("exchanging keys with DH scheme (can be a lengthy process)...\n");
	}

	if (side == CLIENT_SIDE && firstack & FLG_BE_VERBOSE) {
		printf("x (client secret key) randomly generated\n");
		if (firstack & FLG_DEBUG) {
			printf("x (client secret key):\n");
			mpz_out_str(stdout, 10, &mysecret);
			printf("\n");
		}
	}

	/* dh: calculate exponential */
	mpz_powm(&mypublic, &three, &mysecret, &modulo);

	bzero(exchange, sizeof(exchange));
	mpz_get_str(exchange, 10, &mypublic);

	if (side == CLIENT_SIDE && firstack & FLG_BE_VERBOSE) {
		printf("3**x (client public key) exponentially generated\n");
		if (firstack & FLG_DEBUG) {
			printf("3**x (client public key):\n");
			mpz_out_str(stdout, 10, &mypublic);
			printf("\n");
		}
	}

	/* dh: exchange exponentials */
	if (write_data(s, exchange, sizeof(exchange)) != sizeof(exchange))
		return(-1);
	
	bzero(exchange, sizeof(exchange));
	if (read_data(s, exchange, sizeof(exchange)) != sizeof(exchange))
		return(-1);

	mpz_set_str(&hispublic, exchange, 10);

	if (side == CLIENT_SIDE && firstack & FLG_BE_VERBOSE) {
		printf("3**y (server public key) received from server\n");
		if (firstack & FLG_DEBUG) {
			printf("3**y (server public key):\n");
			mpz_out_str(stdout, 10, &hispublic);
			printf("\n");
		}
	}

	/* dh: calculate shared secret */
        mpz_powm(&sharedsecret, &hispublic, &mysecret, &modulo);

	if (side == CLIENT_SIDE && firstack & FLG_BE_VERBOSE) {
		printf("(3**y)**x (shared secret) exponentially generated\n");
		if (firstack & FLG_DEBUG) {
			printf("(3**y)**x (shared secret):\n");
			mpz_out_str(stdout, 10, &sharedsecret);
			printf("\n");
		}
	}
	
	/* convert shared secret to ascii decimal string, divide the
	decimal string into two halves.  the first half is the client
	one, the second one is the server one.  both halves are 
	digested by md5 in order to distill randomness and provide
	actual encryption keys.
	we use different keys for server and client in order to prevent
	false data injection, as it has been pointed out by S.Bellovin
	in Session Layer Encryption, V USENIX Symposium
	things are just natural for single DES (one 8 bytes key) and
	IDEA (one 16 bytes key), but triple des needs three 8 bytes
	keys.  we generate the 3des third key by means of the IDEA
	cipher */

	/* digest shared secret */
	bzero(pwstr, sizeof(pwstr));
	mpz_get_str(pwstr, 10, &sharedsecret);

	/* digest first half
	single des and idea client keys are based on digest1
	triple des client keys are based on digest1 and pwstr
	*/
	MD5Init(&mdContext);
	MD5Update(&mdContext, pwstr, strlen(pwstr) / 2);
	MD5Final(digest1, &mdContext);

	/* digest second half
	single des and idea server keys are based on digest2
	triple des server keys are based on digest2 and pwstr
	*/
	MD5Init(&mdContext);
	MD5Update(&mdContext, pwstr + strlen(pwstr) / 2, strlen(pwstr) / 2);
	MD5Final(digest2, &mdContext);

	/* calculate a special pwstr hash for use in the mutual authentication
	phase only.  since it is maintained in memory for use in the
	mutual auth phase, it could be eventually disclosed; this is why
	this is a one way hash, based upon a subset of the key.
	sessionkeyhash is calculated as follows: the first 8 bytes of
	digest1 are used as a des key to encrypt the first 8 bytes of
	digest2.
	*/
#ifdef DESCORE
	descore_key_sched(digest1, tmpsched);
#else
	des_set_key((des_cblock *)digest1, tmpsched);
#endif

	memcpy(sessionkeyhash, digest2, 8);

#ifdef DESCORE
	descore_ecb_encrypt(sessionkeyhash, sessionkeyhash, tmpsched, ENCRYPT);
#else
	des_ecb_encrypt((des_cblock *)sessionkeyhash,
			(des_cblock *)sessionkeyhash, tmpsched, ENCRYPT);
#endif
	bzero(tmpsched, sizeof(tmpsched));

	/* the itself key is random, so we do not need a changing IV */
	/* bzero(IV, sizeof(IV)); */

	/* set single des keys */
	if (firstack & FLG_USE_IDEA) {
		/* set client encryption key */
		init_this_key_idea(c_ideakey, digest1, FALSE);

		/* set server encryption key */
		init_this_key_idea(s_ideakey, digest2, FALSE);
	}
	/* set idea keys */
	else if (firstack & FLG_USE_SINGLE) {
		/* fold (xor) md5 digests 16 bytes into 8 bytes des keys */
		for (i = 0; i < 8; i++) {
			digest1[i] ^= digest1[i + 8];
			digest2[i] ^= digest2[i + 8];
		}

		/* set client encryption key */
#ifdef DESCORE
		descore_key_sched(digest1, c_schedule1);
#else
		des_set_key((des_cblock *)digest1, c_schedule1);
#endif

		/* set server encryption key */
#ifdef DESCORE
		descore_key_sched(digest2, s_schedule1);
#else
		des_set_key((des_cblock *)digest2, s_schedule1);
#endif

	}
	/* set rc4 keys */
	else if (firstack & FLG_USE_RC4) {
		rc4_init(&c_rc4, digest1, 16);
		rc4_init(&s_rc4, digest2, 16);
	}
	/* calculate and set triple des keys 
	triple DES is default encryption mode
	*/
	else {
		/* set client first encryption key */
#ifdef DESCORE
		descore_key_sched(digest1, c_schedule1);
#else
		des_set_key((des_cblock *)digest1, c_schedule1);
#endif

		/* set client second encryption key */
		/* fixed by Mark C. Henderson */
#ifdef DESCORE
		descore_key_sched((des_cblock *)digest1 + 1, c_schedule2);
#else
		des_set_key((des_cblock *)digest1 + 1, c_schedule2);
#endif

		/* set client third encryption key
		to calculate the third key we take advantage of the
		IDEA cipher, setting digest1 as encryption key 
		and using first digest2's half as plaintext.  the resulting
		ciphertexst is set into c_third
		*/
		init_this_key_idea(c_ideakey, digest1, FALSE);

#ifndef HIGHFIRST /* dunigan@thdsun.epm.ornl.gov */
		{
			word16  XX[4];
			memcpy(XX, digest2, 8);
			XX[0] = XX[0] >> 8 | XX[0] << 8;
			XX[1] = XX[1] >> 8 | XX[1] << 8;
			XX[2] = XX[2] >> 8 | XX[2] << 8;
			XX[3] = XX[3] >> 8 | XX[3] << 8;
			cipher_idea(XX, XX, c_ideakey);
			XX[0] = XX[0] >> 8 | XX[0] << 8;
			XX[1] = XX[1] >> 8 | XX[1] << 8;
			XX[2] = XX[2] >> 8 | XX[2] << 8;
			XX[3] = XX[3] >> 8 | XX[3] << 8;
			memcpy(c_third, XX, 8);
		}
#else
		cipher_idea(digest2, c_third, c_ideakey);
#endif HIGHFIRST

#ifdef DESCORE
		descore_key_sched(c_third, c_schedule3);
#else
		des_set_key((des_cblock *)c_third, c_schedule3);
#endif

		/* set server first encryption key */
#ifdef DESCORE
		descore_key_sched(digest2, s_schedule1);
#else
		des_set_key((des_cblock *)digest2, s_schedule1);
#endif

		/* set server second encryption key */
		/* fixed by Mark C. Henderson */
#ifdef DESCORE
		descore_key_sched(digest2 + 8, s_schedule2);
#else
		des_set_key((des_cblock *)digest2 + 1, s_schedule2);
#endif

		/* set server third encryption key
		to calculate the third key we take advantage of the
		IDEA cipher, setting digest2 as encryption key 
		and using first digest1's half as plaintext.  the resulting
		ciphertexst is set into c_third
		*/
		init_this_key_idea(s_ideakey, digest2, FALSE);

#ifndef HIGHFIRST /* dunigan@thdsun.epm.ornl.gov */
		{
			word16  XX[4];
			memcpy(XX, digest1, 8);
			XX[0] = XX[0] >> 8 | XX[0] << 8;
			XX[1] = XX[1] >> 8 | XX[1] << 8;
			XX[2] = XX[2] >> 8 | XX[2] << 8;
			XX[3] = XX[3] >> 8 | XX[3] << 8;
			cipher_idea(XX, XX, s_ideakey);
			XX[0] = XX[0] >> 8 | XX[0] << 8;
			XX[1] = XX[1] >> 8 | XX[1] << 8;
			XX[2] = XX[2] >> 8 | XX[2] << 8;
			XX[3] = XX[3] >> 8 | XX[3] << 8;
			memcpy(s_third, XX, 8);
		}
#else
		cipher_idea(digest1, s_third, s_ideakey);
#endif HIGHFIRST

#ifdef DESCORE
		descore_key_sched(s_third, s_schedule3);
#else
		des_set_key((des_cblock *)s_third, s_schedule3);
#endif
	}

	if (side == CLIENT_SIDE && firstack & FLG_BE_VERBOSE) {
		if (firstack & FLG_USE_IDEA) {
			hexdump("IDEA client key: ", digest1, 16);
			hexdump("IDEA server key: ", digest2, 16);
		}
		else if (firstack & FLG_USE_SINGLE) {
			hexdump("DES client key: ", digest1, 8);
			hexdump("DES server key: ", digest2, 8);
		}
		else if (firstack & FLG_USE_RC4) {
			hexdump("RC4 client key: ", digest1, 16);
			hexdump("RC4 server key: ", digest2, 16);
		}
		else { /* default is triple DES */
			hexdump("3DES client key #1: ", digest1, 8);
			hexdump("3DES client key #2: ", digest1 + 8, 8);
			hexdump("3DES client key #3: ", c_third, 8);
			hexdump("3DES server key #1: ", digest2, 8);
			hexdump("3DES server key #2: ", digest2 + 8, 8);
			hexdump("3DES server key #3: ", s_third, 8);
		}
	}

	/* delete traces */
	bzero(pwstr, sizeof(pwstr));
	bzero(digest1, 16);
	bzero(digest2, 16);
	mpz_clear(&modulo);
	mpz_clear(&three);
	mpz_clear(&sharedsecret);
	mpz_clear(&mysecret);
	mpz_clear(&mypublic);
	mpz_clear(&hispublic);
	return(0);
}

/* generate randomness by means of DES and MD5.

we have a really unpredictable strings, that is str,
which has been built by internalrnd().  we distill randomness from
str by means of the MD5 function.  we divide the digest into two
halves.  then, we repeatly copy the second half to randomstring
and DES/CBC encrypt it using the first digest as encryption key.
as a result, randomstring is now a suitably long supposed-to-be
random string which is converted into a multi-precision number
for use in the DH scheme.
yes, this is a weird method to get randomness from a unpredictable
string.  suggestions and criticism is welcome!
*/
mkrnd(str, rnd, modulo, logging)
unsigned char	*str;
MP_INT		*rnd, *modulo;
int		logging;
{
	MP_INT		value;

	MD5_CTX		mdContext;
	unsigned char	digest[16];

	IDEAkey		arandomkey;
	des_cblock	iv;

	/* the size of randomstring should be a multiple of 16, that is,
	the size of MDx digest; if sizeof(randomstring) == 512 then it
	is possible to generate up to 4096 bits numbers. in the
	present version, we only need 2048 bits longs numbers maximum.
	*/
	unsigned char	randomstring[512];

	/* digest user's input */
	MD5Init(&mdContext);
	MD5Update(&mdContext, str, strlen(str));
	MD5Final(digest, &mdContext);

	if (logging) {
		printf("\nGENERATING KEY RANDOMNESS\n");
		printf("random string: \"%s\"\n", str);
		hexdump("digest of random string: ", digest, 16);
	}

	/* build idea key */
	init_this_key_idea(arandomkey, digest, FALSE);
	/* set idea cfb */
	bzero(iv, 8);
	initcfb_idea((word16 *)iv, (byte *)arandomkey, FALSE);

	/* reset randomstring */
	bzero(randomstring, sizeof(randomstring));

	/* encrypt randomstring */
	ideacfb(randomstring, sizeof(randomstring));

	/* give a glance at the first random bytes... */
	if (logging) {
		hexdump("first 20 random bytes: ", randomstring, 20);
		hexdump("next 20 random bytes : ", randomstring + 20, 20);
		hexdump("next 20 random bytes : ", randomstring + 40, 20);
		printf("skipped all remaining random bytes (%d bytes in all)\n",
							sizeof(randomstring));
	}

#if 0
	/* build a verylong random number whose size is
	sizeof(randomstring) * 8 bits. this is accomplished in a very old
	fashioned way, but performances should not be an issue here
	*/
	mpz_set_ui(&ff, (unsigned long)256);
	mpz_set_ui(&value, (unsigned long)0);
	for (i = 0; i < sizeof(randomstring); i++) {
		int	tmp;

		/* y = 256**i */
		tmp = i;
		syslog(LOGPRI, "cycle: i = %d, tmp = %lu", i, tmp);
		mpz_pow_ui(&y, &ff, tmp);

		/* x = y * randomstring[i] */
		tmp = randomstring[i];
		mpz_mul_ui(&x, &y, (unsigned long)tmp);

		/* value = value + x */
		mpz_add(&value, &value, &x);

		/* value >= modulo ? break */
		if (mpz_cmp(&value, modulo) >= 0)
			break;
	}
#endif

	mpz_init(&value);
	str2mp(randomstring, sizeof(randomstring), &value);

	mpz_mod(rnd, &value, modulo);

	/* clear all traces */
	bzero(randomstring, sizeof(randomstring));
	bzero(str, strlen(str));
	mpz_clear(&value);
}


/* convert an ASCII string into a MP number */
str2mp(str, len, value)
char	*str;
int	len;
MP_INT	*value;
{
	MP_INT	ff, x, y;
	int	i;

	mpz_init(&ff);
	mpz_init(&x);
	mpz_init(&y);

	mpz_set_ui(&ff, (unsigned long)256);
	mpz_set_ui(value, (unsigned long)str[0]);

	i = 0;
	while (1) {
		if (i++ >= len - 1)
			break;

		mpz_pow_ui(&y, &ff, i);
		mpz_mul_ui(&x, &y, str[i]);
		mpz_add(value, value, &x);
	}

	mpz_clear(&ff);
	mpz_clear(&x);
	mpz_clear(&y);
}






#ifndef NO_IP_PROTO
killoptions(s, host)
int	s;
char	*host;
{
	unsigned char	buf[BUFSIZ / 3], *pbuf;
	char		dumpbuf[BUFSIZ], *pdump = dumpbuf;
	int		bufsiz = sizeof(buf);

	if (!getsockopt(s, IPPROTO_IP, IP_OPTIONS, (char *)buf, &bufsiz) &&
	    bufsiz != 0) { /* options are set */
		for (pbuf = buf; bufsiz > 0; pbuf++, bufsiz--, pdump += 3)
			sprintf(pdump, " %2.2x", *pbuf);
		syslog(LOGPRI,
			"connection from %s with IP_OPTIONS (ignored): %s",
							host, dumpbuf);
		if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, NULL, 0)) {
			syslog(LOG_ERR, "setsockopt(IP_OPTIONS): %m");
			return(1);
		}
	}
	return(0);
}
#endif


/* build a truly random string using some sources of randomness 
inside unix computer systems.  we use computer clock, PIDs,
system names, date of files, CWD and the truerand function which
is supposed to generate physically random numbers.  each string's
individual bit has considerably less than 1 bit of entropy;
however, the string is to be digested by md5, a strong hashing
algorithm.
*/
unsigned char *
internalrnd()
{
	static unsigned char	buf[4096];
	unsigned char		tmp[512];
	struct utsname		name;
	long			t;
	char			*ctime();
	struct stat 		sbuf;
	void			(*oldsig) ();

	bzero(buf, sizeof(buf));

	/* system names */
	if (uname(&name) >= 0) {
		(void)sprintf(tmp, "unames = (%s %s %s %s %s);\n",
			name.sysname, name.nodename, name.release,
			name.machine, name.version);
		(void)strcat(buf, tmp);
	}

	/* date */
	t = time(NULL);
	(void)sprintf(tmp, "time = %ld; ctime = %s ", t, ctime(&t));
	(void)strcat(buf, tmp);

	/* pids */
	(void)sprintf(tmp, "pid = %d; getpgrp = %d; getppid = %d;\n",
					getpid(), getpgrp(), getppid());
	(void)strcat(buf, tmp);

	/* dates of frequently changing files */
	if (stat("/etc/passwd", &sbuf) != -1) {
		(void)sprintf(tmp, "/etc/passwd dates = (%ld %ld %ld);\n",
			sbuf.st_atime, sbuf.st_mtime, sbuf.st_ctime);
		(void)strcat(buf, tmp);
	}
	if (stat("/dev/tty", &sbuf) != -1) {
		(void)sprintf(tmp, "/dev/tty dates = (%ld %ld %ld);\n",
			sbuf.st_atime, sbuf.st_mtime, sbuf.st_ctime);
		(void)strcat(buf, tmp);
	}
	if (stat("/dev/console", &sbuf) != -1) {
		(void)sprintf(tmp, "/dev/console dates = (%ld %ld %ld);\n",
			sbuf.st_atime, sbuf.st_mtime, sbuf.st_ctime);
		(void)strcat(buf, tmp);
	}
	if (stat("/tmp", &sbuf) != -1) {
		(void)sprintf(tmp, "/tmp dates = (%ld %ld %ld);\n",
			sbuf.st_atime, sbuf.st_mtime, sbuf.st_ctime);
		(void)strcat(buf, tmp);
	}

	if (stat("/usr/tmp", &sbuf) != -1) {
		(void)sprintf(tmp, "/usr/tmp dates = (%ld %ld %ld);\n",
			sbuf.st_atime, sbuf.st_mtime, sbuf.st_ctime);
		(void)strcat(buf, tmp);
	}

	/* cwd */
	if (getcwd(tmp, sizeof(tmp)) != NULL) {
		strcat(buf, "cwd: ");
		(void)strcat(buf, tmp);
		strcat(buf, "\n");
	}

	/* a few ``true random'' numbers */
	oldsig = signal(SIGALRM, SIG_DFL); /* truerand modifies sigalrm */
	(void)sprintf(tmp, "a few random numbers = (%d %d %d);\n",
		truerand(),
		truerand(),
		truerand());
	(void)signal(SIGALRM, oldsig);
	(void)strcat(buf, tmp);
	
	return(buf);
}

mauth(s, authbuf, side, firstack)
int	s, side, firstack;
char	*authbuf;
{
	MD5_CTX			mdContext;
	unsigned char		digest[16];
	des_key_schedule	authsched;
	unsigned char		cookie[8], *firsthalve, *secondhalve;
	unsigned char		othercookie[8], *otherfirst, *othersecond;
	int			i;
	int			logging;
	
	logging = side == CLIENT_SIDE && firstack & FLG_DEBUG;

	/* digest content of ~/.stelsecret */
	MD5Init(&mdContext);
	MD5Update(&mdContext, authbuf, strlen(authbuf));
	MD5Final(digest, &mdContext);

	for (i = 0; i < 8; i++)
		/* xor halves together */
		digest[i] ^= digest[i + 8];

	/* set a des key */
#ifdef DESCORE
	descore_key_sched(digest, authsched);
#else
	des_set_key((des_cblock *)digest, authsched);
#endif

	/* build K_E(sessionkeyhash) */
	memcpy(cookie, sessionkeyhash, 8);
#ifdef DESCORE
	descore_ecb_encrypt(cookie, cookie, authsched, ENCRYPT);
#else
	des_ecb_encrypt((des_cblock *)cookie, (des_cblock *)cookie,
							authsched, ENCRYPT);
#endif
	if (side == SERVER_SIDE) /* encrypt twice */
#ifdef DESCORE
		descore_ecb_encrypt(cookie, cookie, authsched, ENCRYPT);
#else
		des_ecb_encrypt((des_cblock *)cookie, (des_cblock *)cookie,
							authsched, ENCRYPT);
#endif

	if (logging) {
		printf("\nMUTUAL AUTHENTICATION\n");
		printf("AX authenticator is equal to E_K(sessionkeyhash), where:\n");
		hexdump("K is a stelsecret derived DES key, equal to: ",
								digest, 8);
		hexdump("sessionkeyhash is equal to: ", sessionkeyhash, 8);
		hexdump("AX authenticator: ", cookie, 8);
		printf(\
"AX and AY are divided in two halves; AX = AX1|AX2, AY = AY1|AY2\n");
	}

	/* split it into two halves */
	firsthalve = cookie;
	secondhalve = cookie + 4;

	otherfirst = othercookie;
	othersecond = othercookie + 4;

	/* Send halves in order. We have differently encrypted the two
	cookies (double encryption has been performed by the server
	while single encryption by the client), so a replay attack
	should be infeasible for the Man In The Middle */

	if (side == CLIENT_SIDE) {
		if (logging)
			hexdump("sending AX1: ", firsthalve, 4);
		if (write_data(s, firsthalve, 4) != 4)
			return(-1);

		if (read_data(s, otherfirst, 4) != 4)
			return(-1);
		if (logging)
			hexdump("received AY1: ", otherfirst, 4);

		if (logging)
			hexdump("sending AX2: ", secondhalve, 4);
		if (write_data(s, secondhalve, 4) != 4)
			return(-1);

		if (read_data(s, othersecond, 4) != 4)
			return(-1);
		if (logging)  {
			hexdump("received AY2: ", othersecond, 4);
			hexdump("AY: ", othercookie, 8);
		}
	}
	else {
		if (read_data(s, otherfirst, 4) != 4 ||
		    write_data(s, firsthalve, 4) != 4 ||
		    read_data(s, othersecond, 4) != 4 ||
		    write_data(s, secondhalve, 4) != 4)
			return(-1);
	}

	if (side == CLIENT_SIDE) /* decrypt it once */
#ifdef DESCORE
		descore_ecb_encrypt(othercookie, othercookie, authsched, DECRYPT);
#else
		des_ecb_encrypt((des_cblock *)othercookie,
			(des_cblock *)othercookie, authsched, DECRYPT);
#endif
	else if (side == SERVER_SIDE) /* encrypt once more */
#ifdef DESCORE
		descore_ecb_encrypt(othercookie, othercookie, authsched, ENCRYPT);
#else
		des_ecb_encrypt((des_cblock *)othercookie,
			(des_cblock *)othercookie, authsched, ENCRYPT);
#endif

	if (logging) {
		printf(\
"AY is double encrypted. We decrypt it before comparing AX with AY\n");
		hexdump("AY (decrypted): ", othercookie, 8);
		hexdump("AX: ", cookie, 8);
	}
	
	if (memcmp(cookie, othercookie, 8)) {
		if (logging)
			printf("AX != AY, authentication failed\n");
		return(1);
	}

	if (logging)
		printf("AX = AY, authentication succeeded\n");

	return(0);
}

hexdump(prompt, buf, len)
char		*prompt;
unsigned char	*buf;
int		len;
{
	int	i;

	if (prompt)
		printf("%s", prompt);
	for (i = 0; i < len; i++)
		printf("%02X", buf[i]);
	(void)fflush(stdout);
	printf("\r\n");
}

/*
 * Construct a control character sequence
 * for a special character.
 */
char *
control(c)
	register cc_t c;
{
	static char buf[5];
	register unsigned int uic = (unsigned int)c;

	if (uic == 0x7f)
		return ("^?");

	if (uic >= 0x80) {
		buf[0] = '\\';
		buf[1] = ((c>>6)&07) + '0';
		buf[2] = ((c>>3)&07) + '0';
		buf[3] = (c&07) + '0';
		buf[4] = 0;
	} else if (uic >= 0x20) {
		buf[0] = c;
		buf[1] = 0;
	} else {
		buf[0] = '^';
		buf[1] = '@'+c;
		buf[2] = 0;
	}
	return (buf);
}

/*
 * Modified read() & write() for sockets.
 * Returns: number of characters read if ok; -1 if fails.
 */
int		read_data(s, buf, n)
int		s;
char		*buf;
int		n;
{
	int		bcount;
	int		br;

	bcount= 0;
	br= 0;
	while (bcount < n) {
		if ((br = read(s, buf, n - bcount)) > 0) {
			bcount += br;
			buf += br;
		}
		else if (br < 0)
			return(-1);
		else if (!br)
			break;
	}
	return(bcount);
}

int		write_data(s, buf, n)
int		s;
char		*buf;
int		n;
{
	int		bcount;
	int		br;

	bcount= 0;
	br= 0;
	while (bcount < n) {
		if ((br = write(s, buf, n - bcount)) > 0) {
			bcount += br;
			buf += br;
		}
	else if (br < 0)
		return(-1);
	}
	return(bcount);
}


/*
 * CFB encrypt a single byte using DES, 3DES or IDEA.
 * mode: DES_ENCRYPT, DES_DECRYPT
 * cipher: DES_CIPHER, TRIPLEDES_CIPHER, IDEA_CIPHER
 * side: CLIENT_SIDE, SERVER_SIDE
 *
 * (uses global s_IV and c_IV initialisation variables) 
 */
typedef union { /* solaris needs this */
	char	octet[8];
} SHIFT;

static void
CFBcrypt(char *databyte, int mode, int cipher, int side)
{
	char	*shift1, *shift2, *first1, *first2;

	static SHIFT	c_shift1, c_shift2;
	static SHIFT	s_shift1, s_shift2;
	static char	c_first1, c_first2;
	static char	s_first1, s_first2;

	register char	*shift, add;
	register	i;

	des_key_schedule	*sched1, *sched2, *sched3;
	IDEAkey			*ideakey;
	des_cblock		*IV;

	if (databyte == NULL) { /* reset option, unused */
		bzero(&c_shift1, sizeof(SHIFT));
		bzero(&c_shift2, sizeof(SHIFT));
		bzero(&s_shift1, sizeof(SHIFT));
		bzero(&s_shift2, sizeof(SHIFT));
		c_first1 = c_first2 = s_first1 = s_first2 = 0;
	}

	if (mode < 0)
		return;

	/* rc4 is the fastests -- but probably insecure -- cipher */
	if (cipher == RC4_CIPHER) {
		*databyte ^= rc4_byte(side == CLIENT_SIDE ? &c_rc4 : &s_rc4);
		return;
	}
	
	/* we need to keep to different queues for both client
	   and server. This is done for security reasons and
	   syncronicity reasons */
	if (side == CLIENT_SIDE) {
		sched1 = &c_schedule1;
		sched2 = &c_schedule2;
		sched3 = &c_schedule3;
		ideakey = &c_ideakey;
		shift1 = (char *)&c_shift1;
		shift2 = (char *)&c_shift2;
		first1 = &c_first1;
		first2 = &c_first2;
		IV = (des_cblock *)&c_IV;
	}
	else { /* SERVER_SIDE */
		sched1 = &s_schedule1;
		sched2 = &s_schedule2;
		sched3 = &s_schedule3;
		ideakey = &s_ideakey;
		shift1 = (char *)&s_shift1;
		shift2 = (char *)&s_shift2;
		first1 = &s_first1;
		first2 = &s_first2;
		IV = (des_cblock *)&s_IV;
	}

	switch (mode) {
		case	DES_ENCRYPT:
			if (*first1 == 0) { /* First time we use this queue? */
				(void)memcpy(shift1, IV, 8);
				*first1 = 1;
			}
			shift = shift1;
			break;
		case	DES_DECRYPT:
			if (*first2 == 0) {
				(void)memcpy(shift2, IV, 8);
				*first2 = 1;
				*first2 = 2;
			}
			shift = shift2;
			break;
		default	:
			return;
	}

	if (mode == DES_DECRYPT)
		add = *databyte;

	if (cipher == IDEA_CIPHER) { /* idea */
		word16	XX[4];

		memcpy(XX, shift, 8);
		/* idea_ecb(shift, shift); */

#ifndef HIGHFIRST /* dunigan@thdsun.epm.ornl.gov */
		XX[0] = XX[0] >> 8 | XX[0] << 8;
		XX[1] = XX[1] >> 8 | XX[1] << 8;
		XX[2] = XX[2] >> 8 | XX[2] << 8;
		XX[3] = XX[3] >> 8 | XX[3] << 8;
                cipher_idea(XX, XX, *ideakey);
		XX[0] = XX[0] >> 8 | XX[0] << 8;
		XX[1] = XX[1] >> 8 | XX[1] << 8;
		XX[2] = XX[2] >> 8 | XX[2] << 8;
		XX[3] = XX[3] >> 8 | XX[3] << 8;
#else
		cipher_idea(XX, XX, *ideakey);
#endif HIGHFIRST

		memcpy(shift, XX, 8);
	}
	else if (cipher == TRIPLEDES_CIPHER) { /* 3 DES */
#ifdef DESCORE
		descore_ecb_encrypt(shift, shift, *sched1, DES_ENCRYPT);
		descore_ecb_encrypt(shift, shift, *sched2, DES_DECRYPT);
		descore_ecb_encrypt(shift, shift, *sched3, DES_ENCRYPT);
#else
		des_ecb_encrypt((des_cblock *)shift, (des_cblock *)shift,
							*sched1, DES_ENCRYPT);
		des_ecb_encrypt((des_cblock *)shift, (des_cblock *)shift,
							*sched2, DES_DECRYPT);
		des_ecb_encrypt((des_cblock *)shift, (des_cblock *)shift,
							*sched3, DES_ENCRYPT);
#endif
	}
	else if (cipher == DES_CIPHER) { /* single DES */
#ifdef DESCORE
		des_ecb_encrypt(shift, shift, *sched1, DES_ENCRYPT);
#else
		des_ecb_encrypt((des_cblock *)shift, (des_cblock *)shift,
							*sched1, DES_ENCRYPT);
#endif
	}
	else {
		exit(1);
	}

	*databyte ^= shift[0];	

	for (i = 1; i <= 7; i++)
		shift[i - 1] = shift[i];

	if (mode == DES_ENCRYPT)
		add = *databyte;

	shift[7] = add;
}

cryptbuf(buf, len, dec, cipher, side)
char	*buf;
int	len, dec, cipher, side;
{
	register	i;

	for (i = 0; i < len; i++)
		CFBcrypt(&buf[i], dec, cipher, side);
}

static
doaddrmatch(char *pat, char *str)
{
	regex_t		re;
	regmatch_t	pm;
	char		*pattern;

	/* fix: performances */
	if ((pattern = (char *)buildregexp(pat)) == NULL) {
		syslog(LOGPRI, "buildregexp() failed");
		exit(1);
	}

	if (regcomp(&re, pattern, REG_EXTENDED | REG_ICASE)) {
		syslog(LOGPRI, "regcomp() failed");
		exit(1);
	}

	/* regexec(&re, pbuf, 1, &pm, 0 */
	if (!regexec(&re, str, 1, &pm, 0))
		return(0);
	else
		return(1);
}

static char *
buildregexp(char *str)
{
	char		*tmp1;
	static char	*tmp2;
	register	i, j;

	if ((tmp1 = (char *) malloc(strlen(str) * 2)) == NULL) {
		perror("malloc()");
		return(NULL);
	}

	/* . --> \. */
	for (i = 0, j = 0; i < strlen(str); i++) {
		if (str[i] == '.') {
			tmp1[j++] = '\\';
			tmp1[j++] = '.';
		}
		else
			tmp1[j++] = str[i];
	}
	tmp1[j] = '\0';

	if (tmp2 == NULL) {
		if ((tmp2 = (char *) malloc(strlen(tmp1) * 10)) == NULL) {
			perror("malloc()");
			return(NULL);
		}
	}
	else if ((tmp2 = (char *) realloc(tmp2, strlen(tmp1) * 10)) == NULL) {
		perror("realloc()");
		return(NULL);
	}
	/*
	 * * --> [a-z0-9]+
	 * ? --> [a-z0-9]
	 */
	j = 0;
	tmp2[j++] = '^';
	for (i = 0; i < strlen(tmp1); i++) {
		if (tmp1[i] == '*') {
			tmp2[j++] = '[';
			tmp2[j++] = '-';
			tmp2[j++] = 'a';
			tmp2[j++] = '-';
			tmp2[j++] = 'z';
			tmp2[j++] = '0';
			tmp2[j++] = '-';
			tmp2[j++] = '9';
			tmp2[j++] = ']';
			tmp2[j++] = '+';
		}
		else if (tmp1[i] == '?') {
			tmp2[j++] = '[';
			tmp2[j++] = '-';
			tmp2[j++] = 'a';
			tmp2[j++] = '-';
			tmp2[j++] = 'z';
			tmp2[j++] = '0';
			tmp2[j++] = '-';
			tmp2[j++] = '9';
			tmp2[j++] = ']';
		}
		else
			tmp2[j++] = tmp1[i];
	}
	tmp2[j++] = '$';
	tmp2[j] = '\0';

	free(tmp1);
	/*
	printf("regexp: %s --> %s\n", str, tmp2);
	*/
	return(tmp2);
}

/* host1,host2,host3,...,hostn:password */
char *
getstelsecret(char *pathname, char *hname)
{
	FILE		*fp;
	static char	authbuf[STEL_SECRET_MAX_LEN];
	char		line[STEL_SECRET_MAX_LEN];
	int		firsttoklen, linenum = 0;
	int		found = 0;
	char		*p;

	if ((fp = fopen(pathname, "r")) == NULL)
		return(NULL);

	while  (fgets(line, sizeof(line), fp) != NULL) {
		/* check address matching */
		p = (char *)strtok(line, ": \t");	

		linenum++;

		rip(p);
		if (p == NULL || *p == '#' || p == '\0')
			continue;

		/* save pointer to stel secret */
		firsttoklen = strlen(p);

		/* check that hname match any host in p */
		if (!oneofthese(hname, p))
			continue;

		/* extract stelsecret */
		p = (char *)strtok(line + firsttoklen + 1, "\n\t ");
		if (p == NULL) {
			syslog(LOGPRI, "%s syntax error on line %d\n",
							pathname, linenum);
			continue;
		}
	
		/* we found it */
		found++;
		(void)strncpy(authbuf, p, sizeof(authbuf));
		break;
	}

	(void)fclose(fp);

	if (!found)
		return(NULL);

	return(authbuf);
}

/* 0 = not found, 1 = found */
static
oneofthese(char *hname, char *hosts)
{
	char		*tmp;
	char		*p;
	int		i;
	int		found = 0;

	struct hostent	*hp;
	struct in_addr	x;
	int		j;

	if ((hp = gethostbyname(hname)) == NULL)
		return(0); /* Unknown host. should never happen */

	if ((tmp = (char *)strdup(hosts)) == NULL)
		return(0);

	i = 0;
	while ((p = (char *)strtok((i == 0 ? tmp : NULL), ", \t")) != NULL) {
		/*
		syslog(LOGPRI, "comparing %s with %s", hname, p);
		*/
		/* check official name */
		if (!doaddrmatch(p, (char *)hp->h_name)) {
			found++;
			break;
		}

		/* check aliases */
		for (j = 0; hp->h_aliases[j]; j++) {
			if (!doaddrmatch(p, hp->h_aliases[j])) {
				found++;
				break;
			}
		}

		for (j = 0; hp->h_addr_list[j]; j++) {
			memcpy((char *)&x, hp->h_addr_list[j], hp->h_length);
			if (!doaddrmatch(p, (char *)inet_ntoa(x))) {
				found++;
				break;
			}
		}
			
		if (found)
			break;
		i++;
	}

	(void)free(tmp);

	return(found);
}

char *
randomdigest()
{
	char		buf[128];
	MD5_CTX		mdContext;
	static char	digest[16];
	void		(*oldsig) ();

	oldsig = signal(SIGALRM, SIG_DFL); /* truerand modifies sigalrm */
	sprintf(buf, "%ld %d %d %d",
			time(NULL), truerand(), truerand(), getpid());
	(void)signal(SIGALRM, oldsig);
	MD5Init(&mdContext);
	MD5Update(&mdContext, buf, strlen(buf));
	MD5Final(digest, &mdContext);
	
	return(digest);
}

/* client printf */
#if defined(IRIX) || defined(SOLARIS)
cprintf(char *fmt, ...)
#else
cprintf(va_alist)
va_dcl
#endif
{
	va_list		args;
#if !defined(IRIX) && !defined(SOLARIS)
	char		*fmt;
#endif
	char		buf[128];

#if defined(IRIX) || defined(SOLARIS)
	va_start(args, fmt);
#else
	va_start(args);
	fmt = va_arg(args, char *);
#endif

	vsprintf(buf, fmt, args);
	printf("%s[%d]: %s", STELCLIENT, getpid(), buf);
	fflush(stdout);

	va_end(args);
}

#ifdef ASCIIBANNER
/* read and skip banner.  client only */
eatbanner(s)
int	s;
{
	char	buf[100], buf2[100];
	int	len;

	sprintf(buf, BANNER, VERSION);
	len = strlen(buf);
	if (read_data(s, buf, len) != len)
		return(1);
	return(0);
}

/* send banner.  used by server only */
sendbanner(s)
int	s;
{
	char	buf[100];

	sprintf(buf, BANNER, VERSION);
	if (write_data(s, buf, strlen(buf)) != strlen(buf))
		return(1);
	return(0);
}
#endif
