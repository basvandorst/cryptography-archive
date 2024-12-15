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
#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntlcom.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

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

/* a node in the hash table */
#define VALLEN		12	/* probably should be a multiple of 4 */
typedef struct node_s {
	uchar		val[VALLEN];
	struct node_s	*next;	/* next node in hash table with same hash */
	struct node_s	*prev;	/* node previously inserted in table */
	bigint		ctr;	/* counter for how far down the loop we are */
	int		nleg;	/* index into legs[] array below */
} node_t;

#define ISEMPTY(np)	( (np)->ctr == -1 )
#define MAKEEMPTY(np)	(np)->ctr = -1
#define HTBLLEN		(1<<18)
#define HASH(val)	( (char_to_int[(val)[3]] << 12) \
			| (char_to_int[(val)[4]] << 6) \
			| (char_to_int[(val)[5]]) )

#define ISDIST(val)	(((val)[0] == 'A') && ((val)[1] == 'A') \
			&& ((val)[2] == 'A'))

/* a whole hash table */
typedef struct {
	node_t	tbl[HTBLLEN];
	funcptr	realcol;	/* call me upon finding a real collision */
} htbl_t;

#define VALEQU(x,y)	( memcmp((x), (y), VALLEN) == 0 )

htbl_t	*distpts;

/* one leg_t structure per (processor,starting seed) pair */ 
typedef struct {
	uchar	seed[VALLEN];	/* starting point of this leg */
	char	host[MAXHOSTNAMELEN]; /* who reported this leg */
	int	retired;	/* old legs don't die, they just fade away */
	node_t	*prev;		/* last dist. pt. seen on this leg */
} leg_t;

#define NLEGS	(1<<12)		/* *more* than enough */
leg_t	legs[NLEGS];
int	nlegs = 0;

int	port = -1;		/* the server port */

int	reading_debug_files = 0; /* restoring old state */

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

/* initialize the hash table. */
void	h_init(funcptr f)
{
	int	i;

	distpts->realcol = f;
	for (i=0; i<HTBLLEN; i++) {
		MAKEEMPTY( (distpts->tbl)+i );
	}
}

/*
 * add a value to the hash table 'distpts' with counter 'n'.
 * if there's another element in the hash table that has the
 * same value for its block, call 'distpts->realcol'.
 * Return 1 if we hit a real collision, 0 otherwise.
 */
int	h_add(uchar val[VALLEN], bigint counter, int nleg)
{
	node_t	*np;
	node_t	*newnp, *nq, *ncol;

	np = &(distpts->tbl[ HASH(val) ]);
	if (ISEMPTY(np)) {
		/* copy right over an empty slot -- no chance of collision */
		BIGINTCOPY(np->ctr, counter);
		np->nleg = nleg;
		np->next = (node_t *) 0;
		memcpy(np->val, val, VALLEN);
		np->prev = legs[nleg].prev;

		legs[nleg].prev = np;
		return(0);
	}

	/* there's something else that has the same hash as this */
	newnp = (node_t *) malloc(sizeof(node_t));
	newnp->next = np->next;
	newnp->nleg = nleg;
	BIGINTCOPY(newnp->ctr, counter);
	memcpy(newnp->val, val, VALLEN);
	newnp->prev = legs[nleg].prev;

	ncol = (node_t *) 0;
	for (nq=np; nq; nq=nq->next)
		if (VALEQU(newnp->val, nq->val) && newnp != nq) {
			ncol = nq;
			break;
		}

	np->next = newnp;
	legs[nleg].prev = newnp;

	if (distpts->realcol && ncol) {
		/* it was a real collision, not just one of the hash */
		(*(distpts->realcol))(ncol, newnp);
		return(1);
	}
	return(0);
}

/* print a hash table entry */
void	printnode(node_t *np, int show_next)
{
	uchar	next[VALLEN];
	int	i;

	printf("value = %11.11s", np->val);
	if (!BIGINTNEG(np->ctr)) {
		printf("  ( ");
		BIGINTPRINT(np->ctr);
		printf(" )");
	}
	if (np->nleg >= 0)
		printf(" [leg %d] ", np->nleg);

	if (!show_next) {
		printf("\n");
		fflush(stdout);
		return;
	}

	f(np->val, next);

	printf("\tnext = %11.11s\n", next);
	fflush(stdout);
}

/* advance along f for 'iters' iterations */
void	run_for(uchar inout[VALLEN], bigint iters)
{
	uchar	val1[VALLEN], val2[VALLEN], *in, *out, *t;
	bigint	left;

	BIGINTCOPY(left, iters);

	memcpy(val1, inout, VALLEN);
	in = val1;
	out = val2;

	while (BIGINTPOS(left)) {
		f(in, out);
		t = in;
		in = out;
		out = t;
		BIGINTDEC(left);
	}

	memcpy(inout, in, VALLEN);
}

/*
 * Promise: there exists some n with f^n(x) = f^n(y).
 * Find n, and find f^(n-1)(x), f^(n-1)(y),
 * and check if they're any use to us.
 */
void	run_both(uchar x[VALLEN], uchar y[VALLEN], node_t *np1, node_t *np2)
{
	uchar	xx[VALLEN], yy[VALLEN], *inx, *outx, *iny, *outy, *t;
	bigint	cyclelen, total_crypts;
	int	i;

	inx = x; outx = xx;
	iny = y; outy = yy;

	for (;;) {
		f(inx, outx);
		f(iny, outy);
		if (VALEQU(outx, outy))
			break;
		t = inx; inx = outx; outx = t;
		t = iny; iny = outy; outy = t;
	}

	/* Promise: f(inx) = f(iny), inx != iny. */
	if (memcmp(inx, iny, 10) == 0)
		printf("useless collision: ");
	else
		printf("Cool! ");
	printf("legs=%d,%d ", np1->nleg, np2->nleg);
	if (np1->nleg == np2->nleg) {
		printf("(Rho) ldrlen=");
		BIGINTPRINT(np1->ctr);
		printf(" cyclelen=");
		BIGINTABSDIFF(cyclelen, np2->ctr, np1->ctr);
		BIGINTPRINT(cyclelen);
	} else {
		printf("(Lambda) leg1len=");
		BIGINTPRINT(np1->ctr);
		printf(" leg2len=");
		BIGINTPRINT(np2->ctr);
	}
	printf(" total_crypts=");
	BIGINTZERO(total_crypts);
	for (i=0; i<nlegs; i++)
		if (legs[i].prev)
			BIGINTADD(total_crypts, legs[i].prev->ctr);
	BIGINTPRINT(total_crypts);
	printf(":\n");
	printval(inx, DontPrint, 1);
	printval(iny, DontPrint, 1);
}

/*
 * We found a collision in the distinguished points.
 * Now proceed to phase 2 to pinpoint the exact collision.
 * We fork so we don't make the server unreachable;
 * note that it's not possible to fork any earlier
 * than this.
 */
void	distpt_col(node_t *np1, node_t *np2)
{
	int	i;
	node_t	*prev1, *prev2;
	uchar	val1[VALLEN], val2[VALLEN];
	bigint	len1, len2, lendiff, bigi, cyclelen;

	printf("Retiring leg %d (%s) because a collision was found.\n",
		np2->nleg, legs[np2->nleg].host);
	legs[np2->nleg].retired = 1;
	fflush(stdout);

#if 0
	switch(fork()) {
		case -1:	perror("fork"); /* if we can't fork, don't */
		case 0:		break; /* do the collision-finding work */
		default:	return; /* keep the server serving */
	}
#endif

	printf("real collision found between distinguished points:\n");
	printnode(np1, 0);
	printnode(np2, 0);

	printf("backtracking to two previous dist. points:\n");
	prev1 = np1->prev;
	prev2 = np2->prev;
	printnode(prev1, 0);
	printnode(prev2, 0);

	BIGINTABSDIFF(len1, np1->ctr, prev1->ctr);
	BIGINTABSDIFF(len2, np2->ctr, prev2->ctr);

	/*
	 * Here's the current state of affairs:
	 * 
	 * prev1->val		!=	prev2->val
	 * np1->val		==	np2->val
	 * f^len1(prev1->val)	== 	np1->val
	 * f^len2(prev2->val)	== 	np2->val
	 * 
	 * where f^2(x) = f(f(x)), f^3(x) = f(f(f(x))), etc.
	 * 
	 * Strategy is as follows:
	 * 
	 * Assume len1 > len2.  Iterate f() len1-len2
	 * time on prev1->val, and call that val1.
	 * Let val2 be prev2->val.
	 * Now continuously do:
	 *	if (f(val1) == f(val2))
	 *		pinpointed the collision;
	 *	val1 = f(val1);  val2 = f(val2);
	 */

	BIGINTCOPY(lendiff, len1);
	BIGINTSUB(lendiff, len2);
	memcpy(val1, prev1->val, VALLEN);
	memcpy(val2, prev2->val, VALLEN);
	if (BIGINTPOS(lendiff)) {
		printf("running just val1 ");
		printval(val1, lendiff, 0);
		run_for(val1, lendiff);
	} else if (BIGINTNEG(lendiff)) {
		BIGINTABSDIFF(lendiff, len1, len2);
		printf("running just val2 ");
		printval(val2, lendiff, 0);
		run_for(val2, lendiff);
	}

	printf("running both:\t");
	printf("val1 ");
	printval(val1, DontPrint, 0);
	printf("\t\tval2 ");
	printval(val2, DontPrint, 0);
	run_both(val1, val2, np1, np2);

#if 0
	printf("\tdone with this collision, child dying.\n");
	fflush(stdout);
	exit(0);
#endif
}

/* set up the server socket */
int	master_waits(void)
{
	struct sockaddr_in thissock;
	int	s, i;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("master_waits(): socket");
		exit(1);
	}

	memset((char *)&thissock, 0, sizeof(struct sockaddr_in));
	thissock.sin_family = AF_INET;
	thissock.sin_port = port;
	thissock.sin_addr.s_addr = htonl(INADDR_ANY);
 
	i = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&i, sizeof(int));

	if (bind(s, (struct sockaddr *)&thissock, sizeof(struct sockaddr_in))
						< 0) {
		perror("master_waits(): bind");
		exit(1);
	}
  
	if (listen(s, 5) < 0) {
		perror("master_waits(): listen");
		exit(1);
	}

	return(s);
}

/* deal with a new dist. pt.; return 1 if this host should be killed */
int	newpt(uchar val[VALLEN], bigint ctr, uchar seed[VALLEN], char *hostname)
{
	int	i;
	char	*err = 0;

	for (i=0; i<nlegs; i++)
		if (VALEQU(legs[i].seed, seed))
			break;
	if (i == nlegs && nlegs >= NLEGS) {
		fprintf(stderr, "too many seeds! up NLEGS and recompile.\n");
		exit(1);
	} else if (i == nlegs) {
		if (BIGINTPOS(ctr) || BIGINTNEG(ctr))
			err = "ctr != 0, but seeing a new seed";
		else {
			legs[nlegs].retired = 0;
			strncpy(legs[nlegs].host, hostname, MAXHOSTNAMELEN-1);
			memcpy(legs[nlegs++].seed, seed, VALLEN);
		}
	} else if (strcmp(legs[i].host, hostname) != 0) {
		err = "duplicate seeds";
	} else if (!ISDIST(val)) {
		err = "rcvd non-distinguished point";
	} else if (legs[i].retired) {
		err = "rcvd distinguished point for a retired leg";
	}

#ifdef	DEBUG
	fprintf(stderr, "DEBUG %s: %11.11s (%.0lf) seed %11.11s (%d)\n",
			legs[i].host, val, ctr, seed, i);
#endif

	if (err == (char *) 0)
		h_add(val, ctr, i);

	if (reading_debug_files)
		return(0);

	if (err)
		printf("Killing because %s.\n", err);
	if (legs[i].retired)
		printf("leg %d (%s) is retired.\n", i, legs[i].host);
	if (err || legs[i].retired) {
		printf("Killing %s, who reported seed ", hostname);
		printval(seed, DontPrint, 0);
		printf("\tand ");
		printval(val, ctr, 0);
		return(1);
	}
	return(0);
}
/* talk to a slave, get a dist. pt, add it to the hash table, etc. */
void	handle_slave(int t, struct sockaddr_in *sap)
{
	struct hostent *hp;
	uchar	thisval[VALLEN], seedval[VALLEN];
	char	*hostname;
	bigint	ctr;
	int	i;

	hp = gethostbyaddr((char *) &sap->sin_addr.s_addr,
				sizeof(struct sockaddr_in), AF_INET);
	if (hp == NULL) {
		printf("Ignoring (and trying to kill) unknown host %s.\n",
			inet_ntoa(sap->sin_addr.s_addr));
		fflush(stdout);
		write(t, "die!", 4);
		return;
	}
	hostname = hp->h_name;

	if (read(t, thisval, VALLEN) < VALLEN) {
		perror("read thisval");
		return;
	}
	if (read(t, seedval, VALLEN) < VALLEN) {
		perror("read seedval");
		return;
	}
	if (read(t, (char *) &ctr, sizeof(bigint)) < sizeof(bigint)) {
		perror("read ctr");
		return;
	}

	if (newpt(thisval, ctr, seedval, hostname))
		write(t, "die!", 4);
}

void	init(void)
{
	int	i;

	for (i=0; i<128; i++)
		char_to_int[i] = 0;
	for (i=0; i<64; i++)
		char_to_int[int_to_char[i]] = i;

	distpts = (htbl_t *) malloc(sizeof(htbl_t));
	h_init(distpt_col);
}

/* we wait for word from one of our slaves */
void	doit(void)
{
	int	i, s, t;
	struct sockaddr_in thatsock;

	/* listen for net connections -- note that we don't fork */
	s = master_waits();
	printf("Master waiting on port %d...\n", port);
	fflush(stdout);
	for (;;) {
		i = sizeof(struct sockaddr_in);
		if ((t = accept(s, (struct sockaddr *)&thatsock, &i)) < 0) {
			perror("accept");
			exit(1);
		}
		/* sometimes takes a while, but slaves'll just haveta wait */
		handle_slave(t, &thatsock);
		close(t);
	}
}

void	read_debug_file(char *path)
{
	FILE	*in;
	char	*p, *q, line[1024], hostname[128];
	uchar	seed[VALLEN], val[VALLEN];
	bigint	ctr;

	in = fopen(path, "r");
	if (in == NULL) {
		fprintf(stderr, "Couldn't open %s for reading.\n", path);
		return;
	}

	reading_debug_files = 1;

	while (fgets(line, 1023, in)) {
		p = strchr(line, ':');
		q = strchr(line, ')');
		if (strncmp(line, "DEBUG ", 6) != 0 || p == NULL || q == NULL) {
			printf("Ignoring line %s", line);
			continue;
		}
		*p++ = '\0'; /* delete ':' */
		strncpy(hostname, line+6, 128);
		p++; /* skip space */
		strncpy(val, p, VALLEN);
		val[VALLEN-1] = '\0';
		p += VALLEN-1;
		p++; /* skip space */
		p++; /* skip '(' */
		*q = '\0'; /* delete ')' */
		sscanf(p, "%lf", &ctr);
		q++; /* skip ')' */
		q++; /* skip space */
		if (strncmp(q, "seed ", 5) != 0) {
			printf("Ignoring line %s", line);
			continue;
		}
		q += 5;	/* skip "seed " */
		strncpy(seed, q, VALLEN);
		seed[VALLEN-1] = '\0';

		newpt(val, ctr, seed, hostname);
	}

	reading_debug_files = 0;

	fclose(in);
}

int	main(int argc, char **argv)
{
	int	i;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s port [debug-files ...]\n", argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);

	init();

	for (i=2; i<argc; i++)
		read_debug_file(argv[i]);

	doit();

	return(0);
}
