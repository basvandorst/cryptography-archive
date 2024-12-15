/* Hobbit's efforts at a unified s/key. */

#include <stdio.h>
#ifdef HASSTDLIB
#include <stdlib.h>
#else
#include <sys/types.h>
#endif
#include <string.h>

#ifdef	__MSDOS__
#include <dos.h>
#else	/* Assume unix; "TERMIOS" fix is implicit below */
#include <fcntl.h>
#include <signal.h>
/* #endif -- but wait, there's more */

#undef USE_ECHO			/* default to not echoing! */

#ifdef stty
# undef stty
#endif
#ifdef gtty
# undef gtty
#endif

#ifdef SYSV
#ifdef BSDI
# include <termios.h>
# define termio termios
#define TCSETA TIOCSETA
#define TCGETA TIOCGETA
#else
# include <termio.h>
#endif
# define TTYSTRUCT termio
# define stty(fd,buf) ioctl((fd),TCSETA,(buf))
# define gtty(fd,buf) ioctl((fd),TCGETA,(buf))
#else
# include <sgtty.h>
# define TTYSTRUCT sgttyb
# define stty(fd,buf) ioctl((fd),TIOCSETN,(buf))
# define gtty(fd,buf) ioctl((fd),TIOCGETP,(buf))
#endif

struct	TTYSTRUCT	newtty;
struct	TTYSTRUCT	oldtty;

#ifdef SIGVOID
#define SIGTYPE void
#else
#define SIGTYPE void
#endif

SIGTYPE trapped();	/* XXX */

#endif /* MSDOS */

#include "md4.h"

#include "skey.h"

/* Crunch a key:
 * concatenate the seed and the password, run through MD4 and
 * collapse to 64 bits. This is defined as the user's starting key.
 */
int
keycrunch(result,seed,passwd)
char *result;	/* 8-byte result */
char *seed;	/* Seed, any length */
char *passwd;	/* Password, any length */
{
	char *buf;
	MD4_CTX md;
	unsigned long results[4];
	unsigned int buflen;
	
	buflen = strlen(seed) + strlen(passwd);
	if((buf = (char *)malloc(buflen+1)) == NULL)
		return -1;
	strcpy(buf,seed);
	strcat(buf,passwd);

	/* Crunch the key through MD[45] */
	sevenbit(buf);
	MD4Init(&md);
	MD4Update(&md,(unsigned char *)buf,buflen);
	MD4Final((unsigned char *)results,&md);
	free(buf);

	/* Fold result from 128 to 64 bits */
	results[0] ^= results[2];
	results[1] ^= results[3];
	memcpy(result,(char *)results,8);
	return 0;
}

/* The one-way function f(). Takes 8 bytes and returns 8 bytes in place */
void
fff(x)
char *x;
{
	MD4_CTX md;
	unsigned long results[4];

	MD4Init(&md);
	MD4Update(&md,(unsigned char *)x,8);
	MD4Final((unsigned char *)results,&md);

	/* Fold 128 to 64 bits */
	results[0] ^= results[2];
	results[1] ^= results[3];

	/* Only works on byte-addressed little-endian machines!! */
	memcpy(x,(char *)results,8);
}

/* Strip trailing cr/lf from a line of text */
void
rip(buf)
char *buf;
{
	char *cp;

	if((cp = strchr(buf,'\r')) != NULL)
		*cp = '\0';

	if((cp = strchr(buf,'\n')) != NULL)
		*cp = '\0';
}

#ifdef	__MSDOS__
char *
readpass(buf,n)
char *buf;
int n;
{
	int i;
	char *cp;

	for(cp=buf,i = 0; i < n ; i++)
		if ((*cp++ = bdos(7,0,0)) == '\r')
			break;
	*cp = '\0';
	printf("\n");
	rip(buf);
	return buf;
}

#else /* MSDOS */
char *
readpass(buf,n)
char *buf;
int n;
{
#ifndef USE_ECHO
    set_term ();
    echo_off ();
#endif

	fgets(buf,n,stdin);

	/*
	after the secret key is taken from the keyboard, the line feed is
	written to standard error instead of standard output.  That means that
	anyone using the program from a terminal won't notice, but capturing
	standard output will get the key words without a newline in front of
	them. 
	*/
        fprintf(stderr, "\n");
        fflush(stderr);
#ifndef USE_ECHO
    unset_term ();
#endif

	rip(buf);
	sevenbit(buf);
	return buf;
}

/* XXX: perhaps merge Colin Campbell's TERMIOS stuff in here */
set_term ()
{
    gtty (fileno(stdin), &newtty);
    gtty (fileno(stdin), &oldtty);
 
    signal (SIGINT, trapped);
}

echo_off ()
{
#ifdef SYSV
    newtty.c_lflag &= ~(ICANON | ECHO | ECHONL);
#else
    newtty.sg_flags |= CBREAK;
    newtty.sg_flags &= ~ECHO;
#endif

#ifdef SYSV
    newtty.c_cc[VMIN] = 1;
    newtty.c_cc[VTIME] = 0;
    newtty.c_cc[VINTR] = 3;
#else
    ioctl(fileno(stdin), TIOCGETC, &chars);
    chars.t_intrc = 3;
    ioctl(fileno(stdin), TIOCSETC, &chars);
#endif

    stty (fileno (stdin), &newtty);
}

unset_term ()
{
    stty (fileno (stdin), &oldtty);
 
#ifndef SYSV
    ioctl(fileno(stdin), TIOCSETC, &chars);
#endif
}

void trapped()
 {
  signal (SIGINT, trapped);
  printf ("^C\n");
  unset_term ();
  exit (-1);
 }

#endif /* MSDOS */

/* removebackspaced over charaters from the string*/
backspace(buf)
char *buf;
{
	char bs = 0x8;
	char *cp = buf;
	char *out = buf;

	while(*cp){
		if( *cp == bs ) {
			if(out == buf){
				cp++;
				continue;
			}
			else {
			  cp++;
			  out--;
			}
		}
		else {
			*out++ = *cp++;
		}

	}
	*out = '\0';

}
sevenbit(s)
char *s;
{
	/* make sure there are only 7 bit code in the line*/
	while(*s){
		*s = 0x7f & ( *s);
		s++;
	}
}
