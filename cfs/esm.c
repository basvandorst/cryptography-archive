/*
 * ESM - Encrypted Session Manager
 * v1.0.2
 * matt blaze
 * December 1995
 */

/* SunOS 5 port by Greg Onufer, based in part on the pty_termios
 * package written by Don Libes, NIST, 2/6/90
 */

/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1995 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
/*
 * Some of this file was stolen from the BSD "script" program, which
 * is covered under the following notice:
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
char copyright1[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
char copyright2[] =
"@(#) Copyright (c) 1995 AT&T\nAll rights reserved.\n";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#ifndef SUN
#include <sys/ioctl.h>
#endif
#include <sys/time.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <unistd.h>
#include <stdio.h>
#ifdef SOLARIS2X
#include <fcntl.h>
#endif
#ifdef PTMX
#include <stropts.h>
#endif
#include "global.h"
#include "rsaref.h"
#include "esm.h"


char	*shell;
int	master;
int	slave;
int	subchild;

int escape=036; /* ^^ */
int ciphstate=0;
int ciphbyte=0;

int keyed=0;

struct	termios tt;
struct	winsize win;
int	lb;
int	l;
#ifdef PTMX
char	*slave_name;
#else
char	line[] = "/dev/ptyXX";
#endif
int	aflg;

#define REMOTE 0
#define LOCAL 1
#define CALC 2

int mode=LOCAL;
int paranoid=0;

#define SL_START 0
#define SL_GOT1 1
#define SL_GOT2 2
#define SL_GOT3 3
#define SL_GOT4 4
#define SL_KEYING 5
#define SL_CRYPT 6
int sloutstate=SL_START;
char *cmd=NULL;

FILE *fpmaster;

#define bwrite(fp,buf,len) (fwrite(buf,len,1,fp))

main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	int ch;
	void finish();
	char *getenv();
	fd_set fds;

	while ((ch = getopt(argc, argv, "e:splrci")) != EOF)
		switch((char)ch) {
		    case 'e':
			cmd=optarg;
			break;
		    case 'i':
		    case 's':
		    case 'p':
			paranoid=1;
		    case 'r':
			mode=REMOTE;
			break;
		    case 'l':
			mode=LOCAL;
			break;
		    case 'c':
			mode=CALC;
			break;
		    case '?':
		    default:
			fprintf(stderr,
			   "usage: esm [-rlc] [-e program\n");
			exit(1);
		}
	argc -= optind;
	argv += optind;

	if ((shell=getenv("SHELL")) == NULL)
		shell = "/bin/sh";
	
	getmaster();

	(void) signal(SIGCHLD, finish);
	subchild = fork();
	if (subchild < 0) {
		perror("fork");
		fail();
	}
	if (subchild==0)
		doshell(mode);
	else {
		/* main loop */
		printf("ESM v1.0.2 - encrypted session manager\n");
		printf("    by Matt Blaze, AT&T Bell Labs, December 1995\n");
		randinit();
		cipherinit();
		switch (mode) {
		    case REMOTE:
			if (paranoid)
			     printf("remote server ready\n");
			else
			     printf("remote server ready; ctl-^ to escape\n");
			break;
		    case LOCAL:
			printf("local layer ready (run 'esm -s' on remote)\n");
			break;
		    default:  /* not yet */
			printf("esm ready\n");
		}
		rawtty();
		fpmaster=fdopen(master,"w");
		if (fpmaster == NULL)
			done();
		if (paranoid)
			startsession();
		FD_ZERO(&fds);
		FD_SET(0,&fds);
		FD_SET(master,&fds);
		while (select (FD_SETSIZE,&fds,NULL,NULL,NULL)>0) {
			if (FD_ISSET(0,&fds))
				doinput();
			if (FD_ISSET(master,&fds))
				dooutput();
			FD_ZERO(&fds);
			FD_SET(0,&fds);
			FD_SET(master,&fds);
		}
		done();
	}
}

#define TRANS 0
#define CMDWAIT 1
#define CIPHER 2
#define KEYWAIT 3

#define IV0 0
#define IV1 1
#define IV2 2
#define IV3 3
#define C0  4
#define C1  5

int state=TRANS;
int cstate=IV0;

doinput()
{
	int cc;
	int i;
	static unsigned char ibuf[512];
	
	if ((cc = read(0, ibuf, 512)) > 0) {
		switch (mode) {
		    case REMOTE:
			for (i=0; i<cc; i++)
				domasterin(ibuf[i]);
			break;
		    case LOCAL:
			for (i=0; i<cc; i++)
				doslavein(ibuf[i]);
			break;
		    default:
			for (i=0; i<cc; i++)
				bwrite(fpmaster,&ibuf[i],1);
			break;
		}
		fflush(fpmaster);
		fflush(stdout);
	} else
		done();
}

doslavein(ibuf)
     unsigned char ibuf;
{
	int c;
	static int count=0;
	
	switch (sloutstate) {
	    case SL_CRYPT:
		if (ibuf==escape) {
			bwrite(stdout,">>",2);
			fflush(stdout);
			if (slescape()) {
				printf("q\r\nEntering CLEARTEXT mode\r\n");
				bwrite(fpmaster,"PPPPPPPPPPPPPPPP",16);
				sloutstate=SL_START;
			}
			break;
		}			
		c=cfb8_encrypt(ibuf);
		if (!(++count % 8))
			bwrite(fpmaster,"!",1);
		sendhex(fpmaster,c);
		break;
	    default:
		bwrite(fpmaster,&ibuf,1);
		break;
	}
}

slescape()
{
	char buf;
	int c;
	int escaped=0;
	
	while (read(0,&buf,1)>0) {
		if (escaped) {
			escaped=0;
			bwrite(fpmaster,&buf,1);
			continue;
		}
		if (buf==escape) {
			c=cfb8_encrypt(buf);
			sendhex(fpmaster,c);
			return 0;
		}
		bwrite(stdout,&buf,1);
		fflush(stdout);
		if (buf=='\\') {
			escaped=1;
			continue;
		}
		if (buf=='\r')
			return 0;
		if (buf=='C') {
			return 1;
		}
		printf("\r\nType one of the following:\r\n");
		printf("  \\[char] to send char as cleartext\r\n");
		printf("  ctrl-^ to send escape character\r\n");
		printf("  'C' to return to CLEARTEXT session\r\n");
		printf("  <enter> to return to encrypted session\r\n");
	}
	return 1; /* should never happen */
}

			       

domasterin(ibuf)
     unsigned char ibuf;
{
	int c;
	char ch;
	static int bad=0;
	
	switch (state) {
	    case TRANS:
		if (ibuf != escape)
			bwrite(fpmaster, &ibuf, 1);
		else {
			state=CMDWAIT;
			bwrite(stdout,">>",2);
			cstate=IV0;
		}
		break;
	    case CMDWAIT:
		if (ibuf == escape) {
			bwrite(fpmaster, &ibuf, 1);
			state=TRANS;
		} else switch (ibuf) {
		    case '\r':
		    case '\n':
			bwrite(stdout,"\r\n",2);
			state=TRANS;
			break;
		    case 's':
		    case 'S':
			startsession(LONG);
			break;
		    case 'Q':
			done();
			break;
		    default:
			printf("Type 's' to start encrypted session\r\n");
			printf("     'Q' to terminate remote session\r\n");
			printf("     ctrl-^ to send escape character\r\n");
			printf("     <enter> to return to session\r\n");
			break;
		}
		break;
	    case CIPHER:
		if (strchr("0123456789abcdef!",ibuf)!=NULL) {
			bad=0;
			if ((c = cipherout(ibuf)) >= 0) {
				ch=c;
				bwrite(fpmaster,&ch,1);
			}
		} else if (bad++ > 16) {
			delkey();
			bwrite(stdout,"XXXXXXXXXXXXXXXX",16);
			if (paranoid)
				done();
			state=TRANS;
		} else
			ciphstate=0;
		break;
	    case KEYWAIT:
		if (strchr("0123456789abcdef:",ibuf)!=NULL) {
			masterkeyin(ibuf);
		} else {
			delkey();
			bwrite(stdout,"XXXXXXXXXXXXXXXX",16);
			if (paranoid)
				done();
			state=TRANS;
		}
		break;
	}
}

int pubstat=0;
int pubpos=0;
unsigned char pubbyte=0;
int pksize = -1;

startsession()
{
	static unsigned char buf[5] = {0177, '~', 0177, '~', 'L'};
	static unsigned char colon=':';
	int i;

	pklen=dhparams[LONG].primeLen;
	printf("Starting remote side of %d bit key exchange.\r\n",pklen*8);
	printf("  (type any character to abort)");
	fflush(stdout);
	bwrite(stdout,buf,5);
	if (createdh(LONG) < 0)
		return -1;
	for (i=0; i<pklen; i++)
		sendhex(stdout,ourpub[i]);
	bwrite(stdout,&colon,1);
	pksize=LONG;
	pubstat=0;
	pubpos=0;
	pubbyte=0;
	state=KEYWAIT;
	return 0;
}

sltranspub(len)
{
	static unsigned char colon=':';
	int i;
	
	for (i=0; i<len; i++)
		sendhex(fpmaster,ourpub[i]);
	bwrite(fpmaster,&colon,1);
}

masterkeyin(c)
     char c;
{
	int bits;

	if (c==':') {
		if (pubpos!=(pklen))
			goto abort;
		if (mcalckeys(pksize)<0)
			goto abort;
		ciphstate=0;
		state=CIPHER;
		/* state = CHECK */
		/* add code to do check */
		return;
	} else if (pubpos<MAXPUBKEY) {
		bits=atoh(c);
		if (bits<0)
			goto abort;
		if (pubstat) {
			pubbyte |= bits;
			otherpub[pubpos]=pubbyte;
			pubpos++;
		} else {
			pubbyte = bits<<4;
		}
		pubstat = 1-pubstat;
		return;
	}
    abort:
	bwrite(stdout,"XXXXXXXXXXXXXXXX",16);
	if (paranoid)
		done();
	state=TRANS;
}

int mcalckeys(len)
     int len;
{
	int i;
	static char kh[32];
	
	if (dhagree(len,1)<0) /* sets up session keys */
		return -1;
	for (i=0; i<8; i++) {
		ivin[i]=0;
		ivout[i]=0xff;
	}
	/* TODO
	sprintf(kh,"KEYHASH=%02x%02x%02x%02x",
		check[0],check[1],check[2],check[3]);
	putenv(kh);	*/
	return 0;
}

int slcalckeys(len)
     int len;
{
	int i;
	
	if (dhagree(len,0)<0) /* sets up session keys */
		return -1;
	for (i=0; i<8; i++) {
		ivout[i]=0;
		ivin[i]=0xff;
	}
	printf("\r\n(key hash is %02x%02x%02x%02x)\r\n",
		check[0],check[1],check[2],check[3]);
	return 0;
}

initslkey(param)
     int param;
{
	if ((param<0) || (param>2))
		return -1;
	pklen=dhparams[param].primeLen;
	pksize=param;
	pubstat=0;
	pubpos=0;
	pubbyte=0;
}

slkeyin(c)
     char c;
{
	int bits;

	if (c==':') {
		if (pubpos!=pklen)
			goto abort;
		printf("\r\nStarting local key exchange...");
		if (createdh(pksize)<0)
			goto abort;
		sltranspub(pklen);
		printf("calculating DH key...");
		fflush(stdout);
		if (slcalckeys(pksize)<0)
			goto abort;
		printf("Entering ENCRYPTED mode; type ctrl-^ to escape\r\n");
		ciphstate=0;
		sloutstate=SL_CRYPT;
		/* add code to send two ascci nulls for check */
		return;
	} else if (pubpos<MAXPUBKEY){
		bits=atoh(c);
		if (bits<0)
			goto abort;
		if (pubstat) {
			pubbyte |= bits;
			otherpub[pubpos]=pubbyte;
			pubpos++;
		} else {
			pubbyte = bits<<4;
		}
		pubstat = 1-pubstat;
		return;
	}
    abort:
	bwrite(stdout,"X",1);
	state=SL_START;

}



int cipherout(ch)
     char ch;
{
	int bits;
	
	if (ch=='!') {
		ciphstate=0;
		return -1;
	}
	bits=atoh(ch);
	if (ciphstate) {
		ciphbyte |= bits;
		ciphstate=0;
		return(cfb8_decrypt(ciphbyte));
	} else {
		ciphbyte = bits<<4;
		ciphstate=1;
		return -1;
	}
}

int ciphercalcin(ch)
     unsigned char ch;
{
	static unsigned char iv[8];
	static unsigned int cbuf;
	int c;
	int i;

	c = atoh(ch);
	switch (cstate) {
	    case IV0:
		for(i=0; i<8; i++)
			iv[i]=0;
		cbuf = (c&0xf)<<4;
		cstate=IV1;
		return '.';
	    case IV1:
		cbuf = cbuf | (c&0xf);
		iv[7]=cbuf;
		cstate=IV2;
		return '.';
	    case IV2:
		cbuf = (c&0xf)<<4;
		cstate=IV3;
		return '.';
	    case IV3:
		cbuf = cbuf | (c&0xf);
		iv[6]=cbuf;
		cstate=C0;
		return '.';
	    case C0:
		cbuf = (c&0xf)<<4;
		cstate=C1;
		return '.';
	    case C1:
		cbuf = cbuf | c&0xf;
		c=cfb8_decrypt(cbuf);
		cstate=C0;
		if (isprint(c)) {
			bwrite(fpmaster,&c,1);
			return '_';
		} else {
			return '?';
		}
	    default:
		return '?';
	}
}

#include <sys/wait.h>

void
finish()
{
	int status;
	register int pid;
	register int die = 0;

	while ((pid = waitpid((pid_t)-1, &status, WNOHANG)) > 0)
		if (pid == subchild)
			die = 1;

	if (die)
		done();
}

dooutput()
{
	register int cc;
	int i;
	unsigned char obuf[512];
	
	if ((cc = read(master, obuf, 512)) > 0) {
		switch (mode) {
		    case REMOTE:
			for (i=0; i<cc; i++)
				domasterout(obuf[i]);
			break;
		    case LOCAL:
			for (i=0; i<cc; i++)
				doslaveout(obuf[i]);
			break;
		    default:
			for (i=0; i<cc; i++)
				bwrite(stdout, &obuf[i], 1);
			break;
		}
		fflush(fpmaster);
		fflush(stdout);
	} else
		done();
}

domasterout(obuf)
     unsigned char obuf;
{
	int c;
	static int count=0;
	
	switch (state) {
	    case CIPHER:
		c=cfb8_encrypt(obuf);
		if (!(++count % 8))
			bwrite(stdout,"!",1);
		sendhex(stdout,c);
		break;
	    case TRANS:
		bwrite(stdout, &obuf, 1);
		break;
	    default:
		/* throw away since i/o screws up keying */
		break;
	}
}


doslaveout(obuf)
     unsigned char obuf;
{
	static int bad=0;
	int c;
	char ch;
	
	switch (sloutstate) {
	    case SL_START:
		bwrite(stdout, &obuf, 1);
		if (obuf==0177)
			sloutstate=SL_GOT1;
		break;
	    case SL_GOT1:
		if (obuf=='~')
			sloutstate=SL_GOT2;
		else {
			bwrite(stdout, &obuf, 1);
       			sloutstate=SL_START;
		}
		break;
	    case SL_GOT2:
		if (obuf==0177)
			sloutstate=SL_GOT3;
		else {
			bwrite(stdout, &obuf, 1);
			sloutstate=SL_START;
		}
		break;
	    case SL_GOT3:
		if (obuf=='~')
			sloutstate=SL_GOT4;
		else {
			bwrite(stdout, &obuf, 1);
			sloutstate=SL_START;
		}
		break;
	    case SL_GOT4: /* key size indicator */
		if (obuf=='S') {
			initslkey(SHORT);
			sloutstate=SL_KEYING;
		} else if (obuf=='M') {
			initslkey(MEDIUM);
			sloutstate=SL_KEYING;
		} else if (obuf=='L') {
			initslkey(LONG);
			sloutstate=SL_KEYING;
		} else {
			sloutstate=SL_START;
			bwrite(stdout, &obuf, 1);
		}
		if (createdh(pksize)<0)
			sloutstate=SL_START;
		break;
	    case SL_KEYING:
		if (strchr("0123456789abcdef:",obuf) != NULL) {
			slkeyin(obuf);
		} else {
			bwrite(stdout,"U",1);
			sloutstate=SL_START;
		}
		break;
	    case SL_CRYPT:
		if (strchr("0123456789abcdef!",obuf) != NULL) {
			bad=0;
			if ((c = cipherout(obuf))>=0) {
				ch=c;
				bwrite(stdout,&ch,1);
			}
		} else if (bad++ > 8) {
			fprintf(stderr,"\r\nEncrypted session terminated -");
			fprintf(stderr,"\r\npress enter for CLEARTEXT mode: ");
			waitenter();
			delkey();
			sloutstate=SL_START;
		} else
			ciphstate=0;
		break;
	    default:
		bwrite(stdout, &obuf, 1);
		break;
	}
}



doshell(mode)
     int mode;
{
	int t;

	/***
	t = open(_PATH_TTY, O_RDWR);
	if (t >= 0) {
		(void) ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	}
	***/
	/* setprompt(mode); */
        getslave();
	(void) close(master);
	(void) dup2(slave, 0);
	(void) dup2(slave, 1);
	(void) dup2(slave, 2);
	(void) close(slave);
	if (cmd==NULL)
		execl(shell, "sh", "-i", 0);
	else
		execl("/bin/sh", "sh", "-c", cmd, 0);
	perror(shell);
	fail();
}

#ifdef NOTDEF
/* make this work more generally */
setprompt(mode)
     int mode;
{
	/* TO DO - make this work for csh */
        char *oldps1;
	static char newps1[128];
	char *getenv();

	if ((oldps1=getenv("PS1")) == NULL)
		oldps1="$ ";
	switch (mode) {
	    case LOCAL:
		sprintf(newps1,"PS1=ESM:%s",oldps1);
		break;
	    case REMOTE:
		sprintf(newps1,"PS1=SECURE:%s",oldps1);
		break;;
	    default:
		sprintf(newps1,"PS1=?esm:%s",oldps1);
		break;
	}
	putenv(newps1);
}
#endif

rawtty()
{

	struct termios sbuf;

        sbuf = tt;
        sbuf.c_iflag &= ~(INLCR|IGNCR|ICRNL|IXON);
        sbuf.c_oflag &= ~OPOST;
        sbuf.c_lflag &= ~(ICANON|ISIG|ECHO);
        sbuf.c_cc[VMIN] = 1;
        sbuf.c_cc[VTIME] = 0;
	(void) tcsetattr(0, TCSAFLUSH, &sbuf);
}

cookedtty()
{
	(void) tcsetattr(0, TCSAFLUSH, &tt);
}

fail()
{

	(void) kill(0, SIGTERM);
	done();
}

done()
{
	cookedtty();
	switch (mode) {
	    case REMOTE:
		if (state==CIPHER)
			printf("XXXXXXXXXXXXXXXX\n");
		printf("esm remote ");
		break;
	    case LOCAL:
		printf("esm local ");
		break;
	}
	printf("done\n");
	exit(0);
}


#ifdef PTMX

getmaster()
{
	if ((master = open("/dev/ptmx", O_RDWR)) == -1)
		return (-1);
	if ((slave_name = (char *)ptsname(master)) == NULL ||
		unlockpt(master) ||
		grantpt(master)) {
		close(master);
		fail();
	}
	(void) ioctl(master, TIOCFLUSH, NULL);
	(void) tcgetattr(0, &tt);
    	(void) ioctl(0, TIOCGWINSZ, (char *)&win);
}

getslave()
{
	(void) setsid();
	if ((slave = open(slave_name, O_RDWR)) < 0) {
		fail();
	}
        if (ioctl(slave, I_PUSH, "ptem")) {
		fail();
        }
        if (ioctl(slave, I_PUSH, "ldterm")) {
		fail();
        }
        if (ioctl(slave, I_PUSH, "ttcompat")) {
		fail();
        }
	(void) tcsetattr(slave, TCSAFLUSH, &tt);
	(void) ioctl(slave, TIOCSWINSZ, (char *)&win);
}

#else

getmaster()
{
	char *pty, *bank, *cp;
	struct stat stb;

	pty = &line[strlen("/dev/ptyp")];
	for (bank = "pqrstuvwxyz"; *bank; bank++) {
		line[strlen("/dev/pty")] = *bank;
		*pty = '0';
		if (stat(line, &stb) < 0)
			break;
		for (cp = "0123456789abcdef"; *cp; cp++) {
			*pty = *cp;
			master = open(line, O_RDWR);
			if (master >= 0) {
				char *tp = &line[strlen("/dev/")];
				int ok;

				/* verify slave side is usable */
				*tp = 't';
				ok = access(line, R_OK|W_OK) == 0;
				*tp = 'p';
				if (ok) {
					(void) tcgetattr(0, &tt);
				    	(void) ioctl(0, TIOCGWINSZ, 
						(char *)&win);
					return;
				}
				(void) close(master);
			}
		}
	}
	fprintf(stderr, "Out of pty's\n");
	fail();
}

getslave()
{

	line[strlen("/dev/")] = 't';
	slave = open(line, O_RDWR);
	if (slave < 0) {
		perror(line);
		fail();
	}
	(void) tcsetattr(slave, TCSAFLUSH, &tt);
	(void) ioctl(slave, TIOCSWINSZ, (char *)&win);
	(void) setsid();
	(void) ioctl(slave, TIOCSCTTY, 0);
}

#endif

waitenter()
{
	unsigned char b;
	do {
		read(0,&b,1);
	} while (b!='\r');
	printf("\r\n");
}

sendhex(fp,c)
     FILE *fp;
     unsigned int c;
{
	static char buf[16];

	sprintf(buf,"%02x",c);
	bwrite(fp,buf,2);
}
