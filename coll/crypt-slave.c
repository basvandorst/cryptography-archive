/*
 * collision search.  designed for parallel machines.
 * this is the master (i.e. server).  we're looking
 * for a collision in crypt(3).
 * based on ``Parallel Collision Search with Application
 * to Hash Functions and Discrete Logarithms'', P.C.
 * van Oorschot and M.J. Wiener, 2nd ACM conference on
 * computer and communications Security at Fairfax, Nov. 1994.
 * 
 * David Wagner dawagner@princeton.edu
 */

/*
 * Here's the technique.  We iterate the map f;
 * take a start vector and examine
 * 
 *	f(start), f(f(start)), f(f(f(start))), ...
 * 
 * when a loop is found, this will give us a collision in f,
 * which will give us two different keys that encrypt plaintext
 * to two equal ciphertexts.
 * 
 * Now, how to find out when that sequence loops?
 * I use a technique called distinguished points; set aside
 * 1 in 2^18 (say) of the range of f as distinguished.
 * Now calculate f^j(start) for each j; whenever f^j(start)
 * is a distinguished point, save it in a hash table.
 * This is phase 1.
 * 
 * We will soon find a loop, i.e. j and k such that
 * 
 *	f^j(start) = f^k(start)
 * 
 * where I'm using the notation f^2(x) = f(f(x)), ...
 * 
 * Now lets find the minimal j' <= j, k' <= k such that
 * 
 *	f^j'(start) = f^k'(start).
 * 
 * Do this by going back to the distinguished point just
 * before f^j(start) and the distinguished point just before
 * f^k(start) and run through the sequence one at a time,
 * saving every point, not just distinguished points.
 * That's phase 2.
 * 
 * We will soon find j', k' as desired.
 * From this it's easy to calculate the cycle length and
 * the header length and the collision and everything else. :-)
 * 
 * Here the function f(x) is
 * 
 *	f(x) = crypt(h(x), g(x))
 * 
 * where x is a string of length 11, g selects the first two
 * characters, and h selects the next 8 characters.
 * 
 * The master maintains the hash table of distributed points;
 * the slaves iterate f on different starting points and
 * report to the master every time they find a dist. point.
 */

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/fcntlcom.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <string.h>
#include <memory.h>
#include <math.h>
#include "des.h"

/* #define DEBUG */

typedef unsigned char uchar;

typedef void	(*funcptr)();

/* we need a counter that can count higher than 2^32 */
/* it just so happens that a double'll do it */
typedef double	bigint;

#define BIGINTZERO(x)	(x) = 0
#define BIGINTINC(x)	(x)++
#define BIGINTDEC(x)	(x)--
#define BIGINTCOPY(x,y)	(x) = (y)
#define BIGINTSUB(x,y)	(x) -= (y)
#define BIGINTADD(x,y)	(x) += (y)
#define BIGINTABSDIFF(x,y,z)	(x) = fabs( (y)-(z) )
#define BIGINTPOS(x)	( (x) > 0.0 )
#define BIGINTNEG(x)	( (x) < 0.0 )
#define BIGINTPRINT(x)	printf("%.0lf", (x))

int	char_to_int[128];	/* convert A-Za-z0-9 etc. to 0-63 */
char	int_to_char[] =	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789./";

#define VALLEN		12	/* probably should be a multiple of 4 */

/* avoid infinite loops if we get into a cycle without a dist. pt. */
/* 2^22 = 2^(dist_bits + 4) */
#define BIGINTDEADMAN(x) (x) = ((double) (1<<22))
#define ISDIST(val)	(((val)[0] == 'A') && ((val)[1] == 'A') \
			&& ((val)[2] == 'A'))

#define VALEQU(x,y)	( memcmp((x), (y), VALLEN) == 0 )

uchar	starting_seed[VALLEN];

struct sockaddr_in master_sock;

/* find the master's address */
void	find_master(char *addr, int port)
{
	struct hostent *hp;
	char	hostname[MAXHOSTNAMELEN];

	memset(&master_sock, 0, sizeof(struct sockaddr_in));
	master_sock.sin_family = AF_INET;
	master_sock.sin_addr.s_addr = inet_addr(addr);
	master_sock.sin_port = port;
	if (master_sock.sin_addr.s_addr == -1)
		hp = gethostbyname(addr);
	else
		hp = gethostbyaddr((char *)&master_sock.sin_addr.s_addr,
				sizeof(struct sockaddr_in), AF_INET);
	if (hp == NULL) {
		fprintf(stderr, "unknown host %s\n", addr);
		exit(1);
	}
	master_sock.sin_family = hp->h_addrtype;
	memcpy(&(master_sock.sin_addr), hp->h_addr, hp->h_length);
	strncpy(hostname, hp->h_name, MAXHOSTNAMELEN-1);
	if (master_sock.sin_family == AF_INET)
		printf("Master host: %s (%s)\n", hostname,
		inet_ntoa(*(struct in_addr *)&master_sock.sin_addr.s_addr));
	else
		printf("Master host: %s\n", hostname);
	fflush(stdout);
}

/* set up the connection to the master */
int	call_master(void)
{
	int	fd;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("slave: socket");
		exit(1);
	}
	if (connect(fd, &master_sock, sizeof(master_sock)) < 0) {
		perror("slave: connect");
		exit(1);
	}
	return(fd);
}

/* brag to the master about the dist. pt. we found */
void	tell_master(uchar v[VALLEN], bigint ctr)
{
	int	s;
	char	diebuf[16];

	s = call_master();

	if (write(s, v, VALLEN) < VALLEN) {
		perror("write v");
		exit(1);
	}
	if (write(s, starting_seed, VALLEN) < VALLEN) {
		perror("write starting_seed");
		exit(1);
	}
	if (write(s, &ctr, sizeof(bigint)) < sizeof(bigint)) {
		perror("write ctr");
		exit(1);
	}
	if (read(s, diebuf, 16) > 0) {
		printf("The master asked for our head.\n");
		exit(0);
	}

	close(s);
}

/* the function we're iterating */
void	f(uchar in[VALLEN], uchar out[VALLEN])
{
	char	salt[3], pswd[9];

	memcpy(salt, in, 2);
	salt[2] = '\0';
	memcpy(pswd, in+2, 8);
	pswd[8] = '\0';
	memcpy(out, crypt(pswd, salt)+2, VALLEN);
	out[11] = '\0';
}

/* its negative, that's all that matters */
static bigint	DontPrint = -69.0;

/* show this value -- show f(val) if shownext is set */
/* Use ctr == DontPrint if you don't wanna print a counter value */
void	printval(uchar val[VALLEN], bigint ctr, int show_next)
{
	uchar	next[VALLEN];
	int	i;

	printf("value = %11.11s", val);
	if (!BIGINTNEG(ctr)) {
		printf("  ( ");
		BIGINTPRINT(ctr);
		printf(" )");
	}

	if (!show_next) {
		printf("\n");
		fflush(stdout);
		return;
	}

	f(val, next);

	printf("\tnext = %11.11s\n", next);
	fflush(stdout);
}

/*
 * run like mad!
 * 
 * iterate the encryption function starting with 'start',
 * sending distinguished points to the master.
 */
void	run(void)
{
	uchar	val1[VALLEN], val2[VALLEN], *in, *out, *t;
	bigint	counter, deadman_ctr;

	BIGINTDEADMAN(deadman_ctr);
	memcpy(val1, starting_seed, VALLEN);
	BIGINTZERO(counter);
	tell_master(val1, counter);

	in = val1;
	out = val2;

	for (;;) {
		f(in, out);
		BIGINTINC(counter);
		BIGINTDEC(deadman_ctr);
		if (ISDIST(out)) {
#ifdef DEBUG
			fprintf(stderr, "DEBUG: dist. pt. %11.11s (%.0lf)\n",
						out, counter);
#endif
			tell_master(out, counter);
			BIGINTDEADMAN(deadman_ctr);
		} else if (BIGINTNEG(deadman_ctr)) {
			/*
			 * We've gone through a loop a bunch of times without
			 * hitting a dist. pt., and the dead man's switch
			 * saved us.  Die now; we'll be reincarnated.
			 */
			printf("\tquitting this run... ");
			printval(out, counter, 0);
			fflush(stdout);
			exit(0);
		}

		t = in;
		in = out;
		out = t;
	}

	/*NOTREACHED*/
}

uchar		myrandom_in[8];

void	crunch(uchar inout[8], uchar ptextin[8], int entropy)
{
	int	i;
	des_key_schedule sched;

	for (i=0; i<4; i++)
		inout[i] ^= ((uchar *) &entropy)[i] << 1;
	des_set_key(inout, sched);
	des_ecb_encrypt(ptextin, inout, sched, 1);
}

/* this is not enough entropy, but screw it */
void	mysrandom(void)
{
	uchar		k[8], in[8];
	char		ourname[64];
	int		i;

	des_random_key(k);
	des_random_key(in);

	crunch(k, in, getgid());
	crunch(k, in, getuid());
	crunch(k, in, getpid());
	crunch(k, in, getppid());
	gethostname(ourname, 64);
	for (i=0; i<64; i++) {
		crunch(k, in, ourname[i]);
	}

	memcpy(myrandom_in, k, 8);
}

int	myrandom(void)
{
	static int	counter=0;
	uchar		k[8];
	int		r;

	des_random_key(k);
	crunch(k, myrandom_in, time((time_t *) 0));
	counter++;
	crunch(k, myrandom_in, counter);
	r = *((int *) k);
	return(r);
}

/* pick some random values and have at it! */
void	doit(void)
{
	int	i;
	uchar	key[VALLEN];

	mysrandom();

	for (i=0; i<128; i++)
		char_to_int[i] = 0;
	for (i=0; i<64; i++)
		char_to_int[int_to_char[i]] = i;

	for (i=0; i<VALLEN; i++)
		starting_seed[i] = int_to_char[ myrandom() & 63 ];
	starting_seed[11] = '\0';

	printf("Starting to run with initial seed ");
	printval(starting_seed, DontPrint, 0);
	fflush(stdout);

	run();
}

int	main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s master-host port\n", argv[0]);
		exit(1);
	}

	find_master(argv[1], atoi(argv[2]));

	doit();

	return(0);
}
