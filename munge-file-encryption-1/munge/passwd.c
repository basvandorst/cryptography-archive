/*	passwd.c - Password reading/hashing routines
	(c) 1989 Philip Zimmermann.  All rights reserved.
	Implemented in Microsoft C.
	Routines for getting a pass phrase from the user's console.
*/

#include	<stdio.h>	/* for fprintf() */
#include	<ctype.h>	/* for isdigit(), toupper(), etc. */
#include	<string.h>	/* for strlen() */
#include	"md5.h"
#include 	"usuals.h"
#ifdef USE_TERMIOS
#include <termios.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
static jmp_buf save;
static void recsig();
static void pushsig();
static void popsig();
static void (*savsig[NSIG])();
#define MAXKEYLEN 1020	/* max byte length of pass phrase */
#define KEYBUFSIZ 1024
long tvbuf[TVBUFSIZ];
int tvc = 0;
static int read_pw();
extern int quiet;
extern int test_mode;
extern int suppress_kc;

void xxfgets(s,n,f)
char *s;
int n;
FILE *f;
{
	int i,j;
	struct timeval tv; struct timezone tz;
	j=0;
	do {
		i=getc(f);
		if (i != EOF)
			*s++=i;
		else
			*s++ = '\n';
		if (tvc < (TVBUFSIZ)-1)  {
			gettimeofday(&tv,&tz);
			i= tvbuf[tvc++] = tv.tv_usec;
		}
		j++;
		if (test_mode) {
			fprintf(stderr, ".", *s);
			fflush(stderr);
		}
	} while (*(s-1) != '\n' && j < n);
	*s = '\0';
}
		
void kcout(s)
unsigned char *s;
{
	unsigned char c=0;
	while(*s)
		c^=(*s++);
	fprintf(stderr, " kc=%02x", c);
}

static read_till_nl(in)
FILE *in;
	{
#define SIZE 4
	char buf[SIZE+1];

	do	{
		xxfgets(buf,SIZE,in);
		} while (index(buf,'\n') == NULL);
	}

int read_pw_string(buf,length,prompt,verify)
char *buf;
int length;
char *prompt;
int verify;
	{
	char buff[KEYBUFSIZ];
	int ret;

	ret=read_pw(buf,buff,(length>KEYBUFSIZ)?KEYBUFSIZ:length,prompt,verify);
	bzero(buff,KEYBUFSIZ);
	return(ret);
	}

static int read_pw(buf,buff,size,prompt,verify)
char *buf,*buff;
int size;
char *prompt;
int verify;
	{
#ifdef USE_TERMIOS
	struct termios tty_orig,tty_new;
#else
	struct sgttyb tty_orig,tty_new;
#endif
	int ok=0;
	char *p;
	int ps=0;
	FILE *tty;

	if ((tty=fopen("/dev/tty","r")) == NULL)
		tty=stdin;
#ifdef USE_TERMIOS
	tcgetattr(fileno(tty), &tty_orig);
#else
#ifdef TIOCGETP
	if (ioctl(fileno(tty),TIOCGETP,(char *)&tty_orig) == -1)
		return(-1);
#endif
#endif
	bcopy(&(tty_orig),&(tty_new),sizeof(tty_orig));
	if (setjmp(save))
		{
		ok=0;
		goto error;
		}
	pushsig();
	ps=1;
#ifdef USE_TERMIOS
	tty_new.c_lflag &= ~(ECHO | ICANON);
	tty_new.c_cc[VMIN] = 1;
	tty_new.c_cc[VTIME] = 0;
	tcdrain(fileno(tty));
	tcsetattr(fileno(tty), TCSAFLUSH, &tty_new);
#else
	tty_new.sg_flags &= ~(ECHO | ICANON);
#ifdef TIOCSETP
	if (ioctl(fileno(tty),TIOCSETP,(char *)&tty_new) == -1)
		return(-1);
#endif
#endif
	ps=2;

	while (!ok)
		{
		fputs(prompt,stderr);
		fflush(stderr);

		buf[0]='\0';
		xxfgets(buf,size,tty);
		if (feof(tty)) goto error;
		if ((p=(char *)index(buf,'\n')) != NULL)
			*p='\0';
		else	read_till_nl(tty);
		if (!suppress_kc)
			kcout(buf);
		if (verify)
			{
			fprintf(stderr,"\nVerifying password %s",prompt);
			fflush(stderr);
			buff[0]='\0';
			xxfgets(buff,size,tty);
			if (feof(tty)) goto error;
			if ((p=(char *)index(buff,'\n')) != NULL)
				*p='\0';
			else	read_till_nl(tty);
			if (!suppress_kc)
				kcout(buff);
				
			if (strcmp(buf,buff) != 0)
				{
				fprintf(stderr,"\nVerify failure - try again\n");
				fflush(stderr);
				continue;
				}
			}
		ok=1;
		}

error:
	fprintf(stderr,"\n");
	/* What can we do if there is an error? */
#ifdef USE_TERMIOS
	tcdrain(fileno(tty));
	tcsetattr(fileno(tty), TCSAFLUSH, &tty_orig);
#else
#ifdef TIOCSETP
	if (ps >= 2) ioctl(fileno(tty),TIOCSETP,(char *)&tty_orig);
#endif
#endif
	if (ps >= 1) popsig();
	if (stdin != tty) fclose(tty);
	return(ok);
	}
/*
**	hashpass - Hash pass phrase down to 128 bits (16 bytes).
**  keylen must be less than 1024.
**	Use the MD5 algorithm.
*/
void hashpass (char *keystring, int keylen, byte *hash)
{
	MD5_CTX mdContext;
	int		i;

	/* Calculate the hash */
	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *) keystring, keylen);
	MD5Final(&mdContext);
	/* Copy it to return variable */
	memcpy(hash, mdContext.digest, 16);
}	/* hashpass */


int getxxkey(char *keystring, char *hash, boolean noecho, char *prompt, int verify)
{	
	int len;
	if (!read_pw_string(keystring, MAXKEYLEN, prompt, verify))
		return 0;
	if ((len=strlen(keystring)) == 0)
		return 0;
	hashpass (keystring, len, (byte *) hash);
	return 1;
}	/* getxxkey */

static void pushsig()
	{
	int i;

	for (i=0; i<NSIG; i++)
		savsig[i]=signal(i,recsig);
	}

static void popsig()
	{
	int i;

	for (i=0; i<NSIG; i++)
		signal(i,savsig[i]);
	}

static void recsig()
	{
	longjmp(save,1);
	}
