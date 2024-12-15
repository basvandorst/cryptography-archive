#if	defined(__TURBOC__) || defined(__STDC__) || defined(LATTICE)
#define	ANSIPROTO	1
#endif

#ifndef	__ARGS
#ifdef	ANSIPROTO
#define	__ARGS(x)	x
#else
#define	__ARGS(x)	()
#endif
#endif

#define	KEYFILE		"/etc/skeykeys"
#ifdef SKEYD
#define PTMPKEYFILE	"/etc/skeyptmp"
#endif

/* Server-side data structure for reading keys file during login */
struct skey {
	FILE *keyfile;
	char buf[256];
	char *logname;
	int n;
	char *seed;
	char *val;
	long	recstart; /*needed so reread of buffer is efficient*/


};

/* Client-side structure for scanning data stream for challenge */
struct mc {
	char buf[256];
	int skip;
	int cnt;
};

#ifdef SKEYD
#define SKEYDCONF "/etc/skeydconf"

#define GIVEMEINFO	0x01	/* orders to skey daemon */
#define UPDATEKEYS	0x02	/* critical */
#define SKEYDOK		0x04	/* ok */
#define SKEYDFAILED	0x08	/* something is wrong */
#ifdef SKEYINITINSECURE
#define FORCEKEYUPDATE	0x0f	/* _force_ updates */
#endif

#define BUFLEN		256
#define SEPARATOR	" \t\r\n"
#define SKEYTALKTIMEOUT	60
#define SKEYDTIMEOUT	30

struct skeymessage {
	unsigned char	iv[8];
	int		flag;
	char		buf[256];
	unsigned char	checksum[16];
	/* char		padding[8]; /* unused */
};
#endif

void f __ARGS((char *x));
int keycrunch __ARGS((char *result,char *seed,char *passwd));
char *btoe __ARGS((char *engout,char *c));
char *put8 __ARGS((char *out,char *s));
int etob __ARGS((char *out,char *e));
void rip __ARGS((char *buf));
int skeychallenge __ARGS((struct skey *mp,char *name,char *promt));
#ifdef SKEYD
int skeylookup __ARGS((struct skey *mp,char *name, int staylocal));
#else
int skeylookup __ARGS((struct skey *mp,char *name));
#endif
int skeyverify __ARGS((struct skey *mp,char *response));

#ifdef SOLARIS24
#define bzero(x, y)	memset(x, 0, y)
#endif
