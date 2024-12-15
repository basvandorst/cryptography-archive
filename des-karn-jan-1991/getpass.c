#include <stdio.h>
#include <signal.h>
#include <sgtty.h>

#define	TTY	"/dev/tty"	/* Change to "con" for MS-DOS */

static struct sgttyb Ttysav;
FILE *Tty;
int (*Sig)();

/* Issue prompt and read reply with echo turned off */
char *
getpass(prompt)
char *prompt;
{
	struct sgttyb ttyb;
	register char *cp;
	int c;
	int getint();
	static char pbuf[128];

	if((Tty = fopen(TTY,"r+")) == NULL){
		fprintf(stderr,"getpass: cannot read key from %s\n",TTY);
		exit(1);
	}
	setbuf(Tty,(char *)NULL);
	fputs(prompt,Tty);

	Sig = signal(SIGINT, getint);
	ioctl(fileno(Tty), TIOCGETP, &ttyb);
	ioctl(fileno(Tty), TIOCGETP, &Ttysav);
	ttyb.sg_flags &= ~ECHO;
	ioctl(fileno(Tty), TIOCSETP, &ttyb);
	cp = pbuf;
	for (;;) {
		c = getc(Tty);
		if(c == '\r' || c == '\n' || c == EOF)
			break;
		if (cp < &pbuf[127])
			*cp++ = c;
	}
	*cp = '\0';
	ioctl(fileno(Tty), TIOCSETP, &Ttysav);
	signal(SIGINT, Sig);
	fputc('\n',Tty);
	fclose(Tty);
	return(pbuf);
}
int
getint()
{
	ioctl(fileno(Tty), TIOCSETP, &Ttysav);
	signal(SIGINT, Sig);
	exit(1);
}
