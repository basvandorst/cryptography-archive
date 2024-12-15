/* main.c - YTalk V2.0 main program */

/*			   NOTICE
 *
 * Copyright (c) 1990 Britt Yenne.  All rights reserved.
 * 
 * This software is provided AS-IS.  The author gives no warranty,
 * real or assumed, and takes no responsibility whatsoever for any 
 * use or misuse of this software, or any damage created by its use
 * or misuse.
 * 
 * This software may be freely copied and distributed provided that
 * no part of this NOTICE is deleted or edited in any manner.
 * 
 */

/* Mail comments or questions to yenne@ccwf.cc.utexas.edu */

#include "ytalk.h"
#include <signal.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#ifdef POSIX
#include <termios.h>
#endif
#ifdef HPUX
#include <sgtty.h>
#include <sys/bsdtty.h>
#endif /* HPUX */
#include "mpilib.h"
#ifdef __NeXT__
#include "libc.h"
#else
#include "unistd.h"
#endif

#define PAUSE	16		/* seconds between housekeeping processing */

void yytalkabort();
void offcrypt(int);

extern char myhost[100];	/* caller's host name */
extern int errno;		/* system error number */
extern CTL_MSG msg;		/* for communication with talk daemon */

char *prog;			/* program name, ie: "ytalk" */

extern person p[MAXC];		/* those in the conversation */
extern int pnum;		/* number of current conversationalists */
extern int fdp[100];		/* for translating file nums to user nums */

char edit[4];		/* my four edit keys: RUB, WORD, KILL, & CLR */
y_parm yparms;		/* my YTalk parameters */
char errstr[100], estr[100];	/* temporary string storage */
inptype inp;		/* for intermodule communication */
long doalarm = 0;	/* set if there is housekeeping to do */
long last_pulse = 0;	/* last housecleaning time */
cpack crec;	/* for inter-YTalk communication */
int conn = 0, sc = -1;	/* number of connected sockets */
extern int autofd;	/* desc of auto-invite socket */

fd_set fdset;		/* main fdset for select() */
int crypto_req = 0;
int crypto_ack = 0;

byte ideakey[16];

extern char Usage[];
extern int optind;

main(argc, argv)
char **argv;
{
    int n, i;
    fd_set sel;
    char *c;
#ifdef POSIX
    struct termios tio;
#else
    struct sgttyb tty;
    struct ltchars ltc;
#endif
    struct timeval tv;

    prog = argv[0];
    InitLog(argc, argv);
    argc -= (optind - 1);
    argv += (optind - 1);
    
    if(argc < 2)
    {
	fprintf(stderr, Usage, prog);
	exit(2);
    }
    
    init_socks();
    init_term();
    init_autoport();

    signal(SIGTERM, yytalkabort);
    signal(SIGQUIT, yytalkabort);
    signal(SIGINT,  yytalkabort);

#ifdef POSIX
    tcgetattr(0, &tio);
    RUB  = tio.c_cc[VERASE];
    KILL = tio.c_cc[VKILL];
#ifdef sco
    WORD = '';
#else
    WORD = tio.c_cc[VWERASE];
#endif
#else
    ioctl(0, TIOCGETP, &tty);
    ioctl(0, TIOCGLTC, (struct sgttyb *)&ltc);
    RUB  = tty.sg_erase;
    KILL = tty.sg_kill;
    WORD = ltc.t_werasc;
#endif
    if(WORD == -1)
	WORD = '\027';
    CLR = '\024';
    yparms.clr = CLR;

    FD_ZERO(&fdset);
    FD_SET(0, &fdset);

    for(argc--, argv++; argc; argc--, argv++)
    {
	if (argc > 1 && !strncmp(argv[1], "tty", 3))
	{
	    newuser(argv[0], argv[1], 1);
	    argv++; argc--;
	}
	else
		newuser(*argv, "", 1);
    }
    
    do_auto();

    /* For housecleaning to occur every PAUSE seconds, we make our own
     * little timer system.  SIGALRM is nice, but it interrupts system
     * calls, which can be disastrous for certain areas of YTalk, so we
     * affect the timer manually.
     */

    last_pulse = time((time_t *)0);  /* don't use SIGALRM - make own timer */

    for(;;)
    {
	if(pnum == 0)	/* Exit YTalk if all conversations have terminated */
	    break;
	if(conn != sc)
	{
	    /* Count the number of currently active conversations, if
	     * we know it has changed.
	     */
	    for(n = 0, conn = 0; n < pnum; n++)
		if(p[n].flags & P_CONNECT)
		    conn++;
	    if(conn == 0)
	    {
		if(sc != -1)
		    w_clr(0);
		for(n = 0; n < pnum; n++)
		    if(!(p[n].flags & P_CONNECT))
		    {
			showstr(0, "Waiting for ");
			showstr(0, p[n].name);
			showstr(0, "@");
			showstr(0, p[n].host);
			showstr(0, " to respond\n");
		    }
		init_crypto();
		DHprepare();
	    }
	    /* Keep a saved value of 'conn', so we know when the
	     * number of active conversations changes.
	     */
	    sc = conn;
	}
	sel = fdset;
	tv.tv_sec = 2L;	/* max 2 sec wait in select(), for timer purposes */
	tv.tv_usec = 0L;
	if((n = select(FD_SETSIZE, (int *) &sel, 0, 0, &tv)) < 0)
	{
	    if(errno == EINTR)
		continue;
	    panic("Select failed!");
	    continue;
	}

	if(time((time_t *)0) - last_pulse > PAUSE)	/* Check timer */
	    sigalrm();

	if(!n)
	    continue;

	for(i = 0; n && i < FD_SETSIZE; i++)
	    if(FD_ISSET(i, &sel))
	    {
		take_input(i);
		n--;
	    }
    }
    yytalkabort(0);
}

#define MAXBUF 64

/* take_input() determines the nature of input on a file descriptor, and
 * processes the input accordingly.
 */
take_input(fd)
{
    register int n;
    register s_char* c;
    int i, socklen;
    struct sockaddr_in temp;
    s_char buf[MAXBUF];

    refresh_pseudorandom();
    if(fd == 0)		/* Keyboard input */
    {
	if((i = n = read(fd, buf, 1)) <= 0)	/* take it 1 char at a time */
	    yytalkabort(5);
	if(*buf == 27)	/* ESC == give menu */
	{
	    if(showmenu() == 0)
		switch(inp.code)
		{
		    case 'a':	/* add a new user */
			newuser(inp.name, "", 1);
			break;
		    case 'd':	/* delete a user */
			killuser(inp.name);
			break;
		    case 'o':	/* output a user */
			outfile(inp.name, inp.data);
			break;
		    case 'x':
		    	startcrypt(inp.data, 1);
			break;
		    case 'p':
		    	startcrypt(inp.data, 0);
			break;
		    default:
			sprintf(errstr, "Unknown command: '%c'", inp.code);
			errno = 0;
			panic(errstr);
		}
	}
	else if(*buf == 12)	/* ctrl-L */
	    redraw();
	else
	{
	    if(*buf == 13)
		*buf = '\n';	/* some RAW curses progs won't do this */
	    xmit_char(*buf);
	}
    }
    else if(fd == autofd)	/* input on auto-invite socket */
    {
	socklen = sizeof(struct sockaddr_in);
	if((fd = accept(autofd, (struct sockaddr *) &temp, &socklen)) == -1)
	{
	    panic("Cannot accept auto-socket connection");
	    return;
	}
	errno = 0;
	n = read(fd, &crec, CSIZ);
	close(fd);
	if(n != CSIZ || crec.code != AUTO)
	{
	    panic("Bad input on auto-invite socket");
	    return;
	}
	sprintf(estr, "Talk with %s@%s?", crec.ci_name, crec.ci_host);
	putc('\07', stderr);
	if(yes_no(estr) == 'y')
	{
	    sprintf(estr, "%s@%s", crec.ci_name, crec.ci_host);
	    newuser(estr, "", 0);
	}
    }
    else	/* input on a user's socket */
    {
	i = fdp[fd];
	if(i < 0 || i >= pnum)
	{
	    errno = 0;
	    panic("Input from unknown socket!\n");
	    close(fd);
	    return;
	}

	/* If we have not already connected to or exchanged edit keys
	 * with this user, then do so.
	 */

	if(!(p[i].flags & P_CONTACT))
	{
	    if(!(p[i].flags & P_CONNECT))
	    {
		send_dgram(myhost, DELETE, i);
		socklen = sizeof(struct sockaddr_in);
		if((n = accept(p[i].sfd,(struct sockaddr *) &temp, &socklen)) == -1)
		{
		    panic("Cannot accept connection");
		    FD_CLR(p[i].sfd, &fdset);
		    close(p[i].sfd);
		    return;
		}
		close(p[i].sfd);
		FD_CLR(p[i].sfd, &fdset);
		fd = p[i].sfd = n;
		FD_SET(n, &fdset);
		fdp[n] = i;
		bcopy(edit, errstr, 3);
		errstr[0] = RUBDEF;
		write(n, errstr, 3);	/* send the edit keys */
		p[i].flags |= P_CONNECT;
	    }
	    errno = 0;
	    if((n = read(fd, p[i].edit, 3)) != 3)
	    {
		panic("Unknown contact");
		FD_CLR(p[i].sfd, &fdset);
		close(p[i].sfd);
		return;
	    }
	    if(p[i].edit[0] == RUBDEF)
	    {
		p[i].flags |= P_YTALK;
		errno = 0;
		if(write(fd, &yparms, YPARMLEN) != YPARMLEN)
		{
		    panic("Cannot write to YTALK contact");
		    FD_CLR(p[i].sfd, &fdset);
		    close(p[i].sfd);
		    return;
		}
		if((n = read(fd, &p[i].yparms, YPARMLEN)) != YPARMLEN)
		{
		    panic("Unknown YTALK contact");
		    FD_CLR(p[i].sfd, &fdset);
		    close(p[i].sfd);
		    return;
		}
		p[i].CLR = p[i].yparms.clr;
	    }
	    else
	    {
		p[i].CLR = 255;
		p[i].WCOLS = 80;
		p[i].WROWS = 24;
	    }
	    p[i].flags |= P_CONTACT;
	    errno = 0;
	    InitTalk(p[i].name);
	    if((p[i].win = add_window(p[i].name, p[i].host, p[i].flags)) < 0)
	    {
		panic("Cannot add new window");
		FD_CLR(p[i].sfd, &fdset);
		close(p[i].sfd);
		return;
	    }
	    putc('\07', stderr);
	    putc('\07', stderr);

	    /* If we are connected to a YTalk host, send information about
	     * all current contacts so that the YTalk host can import them.
	     */

	    if(p[i].flags & P_YTALK)
		for(n = 0; n < pnum; n++)
		{
		    if(n == i)
			continue;
		    if(!(p[n].flags & (P_YTALK | P_CONTACT)))
			continue;
		    crec.code = IMPORT;
		    strncpy(crec.ci_name, p[n].name, NAMELEN);
		    strncpy(crec.ci_host, p[n].host, HOSTLEN);
		    write(fd, &crec, CSIZ);
		}
	    if(conn == 0)
		w_clr(0);
	    conn++;
	    return;
	}

	/* Take and display input from the user socket */

	if((n = read(fd, buf, MAXBUF)) < 0)
	{
	    char buf[1024];
	    sprintf(buf, "Cannot read from %s", p[i].name);
	    panic(buf);
	    return;
	}
	
	if(n == 0)
	{
	    /* Lost connection */
	    deluser(i);
	    conn--;
	    return;
	}

	/* Included in the user socket input can be control packets,
	 * telling this YTalk host to perform various tasks.  We must
	 * filter out and remove these packets for processing.  All
	 * other normal characters get displayed on the screen.
	 */

	if (crypto_ack && (p[i].flags & P_CRYPTACK) && (n  & 1) == 1)
	{
	    read(fd, &buf[n], 1);
	    n++;
	}
	
	for(c = buf; n > 0; c++, n--)
	    if(*c < 0 && *c != RUBDEF)
	    {
		/* process control packet */
		crec.code = *c;
		c++;
		n--;
		if(n >= CSIZ-1)
		{
		    bcopy(c, ((char *)(&crec))+1, CSIZ-1);
		    n -= CSIZ-1;
		    c += CSIZ-1;
		}
		else
		{
		    if(n > 0)
			bcopy(c, ((char *)(&crec))+1, n);
		    read(fd, ((char *)(&crec))+n+1, CSIZ-n-1);
		    n = 0;
		}
		process_pack(i);
		n++;
		c--;
	    }
	    else
	    {
		if (crypto_ack && (p[i].flags & P_CRYPTACK))
		{
		    n--;
		    c++;
		    ideacfb((byteptr)c, 1, p[i].inkey, p[i].iniv, DECRYPT_IT);
		}
	
		/* process normal character */
		process_char(p[i].win, *c, p[i].edit);

		/* If this user is being output, then update the
		 * output file.
		 */

		if(p[i].ffd != -1)
		    if(write(p[i].ffd, c, 1) != 1)
		    {
			panic("Write to output file failed");
			close(p[i].ffd);
			p[i].ffd = -1;
		    }
	    }
    }
}

/* xmit_char() processes a keyboard char and transmits it to all hosts.
 */
xmit_char(ch)
char ch;
{
    register int n;

    if(conn)
	if(process_char(0, ch, edit) != 0)
	    return -1;
    if(ch == RUB)
	ch = RUBDEF;
		
    if (crypto_req > crypto_ack)
    {
	putc('\07', stderr);
	return;
    }
    
    for(n = 0; n < pnum; n++)	/* output char to all users */
    {
	if(!(p[n].flags & P_CONNECT))
	    continue;

	if(ch == CLR && !(p[n].flags & P_YTALK))
	{
	    write(p[n].sfd, "\n\n", 2);
	}
	else
	{
	    if (crypto_ack && (p[n].flags & P_CRYPTACK))
	    {
		byte buf[2];
	    
		buf[1] = ch;
		ideacfb(&buf[1], 1, p[n].outkey, p[n].outiv, ENCRYPT_IT);
		buf[0] = ' ';
		
		write(p[n].sfd, buf, 2);
	    }
	    else
	    {
		write(p[n].sfd, &ch, 1);
	    }
	}
    }
    return 0;
}

/* process_pack will interpret a control packet and perform whatever
 * function is required.
 */
process_pack(i)
{
    int n;

    switch(crec.code)
    {
	case IMPORT:	/* Prompt user if he wishes to import someone */
	    /* Don't import ourself or anyone we are already
	     * talking with.
	     */
#ifdef NOMULTIPLE
	    if(strncmp(crec.ci_name, msg.l_name, NAMELEN) == 0)
		break;
#endif
	    for(n = 0; n < pnum; n++)
		if(strncmp(crec.ci_name, p[n].name, NAMELEN) == 0 &&
		   strncmp(crec.ci_host, p[n].host, HOSTLEN) == 0)
		    break;
	    if(n < pnum)
		break;
	    sprintf(estr, "Import %s@%s?", crec.ci_name, crec.ci_host);
	    if(yes_no(estr) == 'y')
	    {
		sprintf(estr, "%s@%s", crec.ci_name, crec.ci_host);
		newuser(estr, "", 0);
		crec.code = EXPORT;
		write(p[i].sfd, &crec, CSIZ);
	    }
	    break;
	case EXPORT:	/* Tell one of our clients to connect with another */
	    for(n = 0; n < pnum; n++)
	    {
		if(strncmp(crec.ci_name, p[n].name, NAMELEN) != 0)
		    continue;
		if(crec.ci_host[0] != '\0')
		    if(strncmp(crec.ci_host, p[n].host, HOSTLEN) != 0)
			continue;
		break;
	    }
	    if(n >= pnum)
		break;
#ifndef NOMULTIPLE
#endif
	    crec.code = ACCEPT;
	    strncpy(crec.ci_name, p[i].name, NAMELEN);
	    strncpy(crec.ci_host, p[i].host, HOSTLEN);
	    if(p[n].flags & P_YTALK)
		write(p[n].sfd, &crec, CSIZ);
	    break;
	case ACCEPT:	/* Connect with another user */
	    sprintf(estr, "%s@%s", crec.ci_name, crec.ci_host);
	    newuser(estr, "", 0);
	    break;
	case CRYPT:
	    gotcrypt(i, 1);
	    break;
	case CRYPTP:
	    gotcrypt(i, 0);
	    break;
	case CRYPTIV:
	    gotiv(i);
	    break;
	case CRYPTOFF:
	    offcrypt(0);
	    break;
    }
}

/* process_char interprets and displays input characters for each user.
 * If the input character is a special edit key, then perform whatever
 * editing function is required.
 */
process_char(w, ch, edit)
char ch, *edit;
{
    if (ch != 12)
	    ProcessChar(w, ch);
    
    if(ch == RUB)
	w_rub(w);
    else if(ch == KILL)
	w_kill(w);
    else if(ch == WORD)
	w_word(w);
    else if(ch == CLR)
	w_clr(w);
    else if(ch == 12)	/* ctrl-L */
	return 0;
    else
	return add_char(w, ch);
    return 0;
}

/* annouce() will announce our intention to talk to the user defined
 * in the msg structure.  First we check for an auto-invitation.  If
 * there isn't one, ring the user.
 */
announce(hishost, i)
char *hishost;
int i;	/* user number */
{
    int n, fd, ring = 1;
    char sav[NAME_SIZE];

    if(p[i].flags & P_CONNECT)	/* Don't ring a connected user */
    {
	sprintf(errstr, "Cannot ring %s: already connected", p[i].name);
	errno = 0;
	panic(errstr);
	return 1;
    }
    if(conn == 0)
    {
	showstr(0, "Ringing ");
	showstr(0, p[i].name);
	showstr(0, "@");
	showstr(0, p[i].host);
	showstr(0, "...");
    }
    strncpy(sav, msg.l_name, NAME_SIZE);
    strncpy(msg.l_name, "+AUTO", NAME_SIZE);
    msg.r_tty[0] = '\0';
    while((n = send_dgram(hishost, LOOK_UP, i)) == 0)
    {
	p[i].flags = 0;
	if((fd = connect_to(-1)) < 0)
	{
	    if(fd == -2)
	    {
		send_dgram(hishost, DELETE, i);
		continue;
	    }
	    strncpy(msg.l_name, sav, NAME_SIZE);
	    if(conn == 0)
		showstr(0, "\n");
	    return -1;
	}
	crec.code = AUTO;
	strncpy(crec.ci_name, sav, NAME_SIZE);
	crec.ci_name[NAMELEN-1] = '\0';
	strncpy(crec.ci_host, myhost, HOSTLEN);
	write(fd, &crec, CSIZ);
	close(fd);
	ring = 0;
	break;
    }

    strncpy(msg.l_name, sav, NAME_SIZE);
    strncpy(msg.r_tty, p[i].tty, TTY_SIZE);
    p[i].flags = 0;

    while(ring)
	if((n = send_dgram(hishost, ANNOUNCE, i)) == 0)
	    break;
	else
	{
	    if(n == 1 && p[i].tty[0] != '\0')
	    {
		/* If the user specified a TTY, but that user
		 * is not logged into that TTY, try looking him
		 * up on any TTY before giving up.
		 */
		p[i].tty[0] = '\0';
		msg.r_tty[0] = '\0';
		continue;
	    }
	    show_error(n, p[i].name, hishost);
	    if(conn == 0)
		showstr(0, "\n");
	    return -1;
	}
    if(conn == 0)
    {
	showstr(0, "\n");
    }
    return 0;
}

/* Update the automatic communications port.
 */
do_auto()
{
    int n;

    if(autofd == -1)
	return;
    if((n = send_auto(LEAVE_INVITE)) != 0)
    {
	show_error(n, "AUTO", myhost);
	FD_CLR(autofd, &fdset);
	close(autofd);
	autofd = -1;
    }
    FD_SET(autofd, &fdset);
}

/* panic() displays any errors in a user-friendly way, if possible.
 */
panic(str)
char *str;
{
    int e;

    e = errno;
    if(showerror(str, e) == 0)
	return;
    close_term();
    if(e)
    {
	errno = e;
	perror(str);
    }
    else
	fprintf(stderr, "%s\n", str);
    sleep(3);
    yytalkabort(5);
}

char *errs[] = {
	"No error (?)",
	"Not logged in",
	"Operation failed in daemon (?)",
	"Caller's machine unknown to daemon",
	"Not accepting messages",
	"Unknown request to daemon",
	"Version mismatch error",
	"Invalid TCP socket address",
	"Invalid UDP socket address"
};

show_error(n, name, host)
char *name, *host;
{
    if(n == -1)
	return;
    if(n >= 0 && n < 9)
	sprintf(errstr, "%s@%s: %s", name, host, errs[n]);
    else
	sprintf(errstr, "%s@%s: Unknown error in daemon", name, host);
    if(showerror(errstr, 0) == 0)
	return;
    close_term();
    fprintf(stderr, "%s\n", errstr);
    sleep(3);
    yytalkabort(1);
}

/* showstr() displays a string on the terminal.
 */
showstr(w, str)
char *str;
{
    for(; *str; str++)
	add_char(w, *str);
}

/* sigalrm performs housecleaning, including resending the auto-invitation
 * to make sure the daemon doesn't time it out, and re-ringing any
 * unconnected parties.
 */
sigalrm()
{
    long t;
    int n, e, found = 0;

    if(autofd != -1)
	do_auto();

    t = time((time_t *)0);
    if(doalarm == 0)
    {
	last_pulse = t;
	return;
    }
    for(n = 0; n < pnum; n++)
	if(!(p[n].flags & P_CONNECT))
	{
	    if(!found)
		found = 1;
	    if(t - p[n].last_ring < RING_WAIT)
		continue;
	    p[n].last_ring = t;
	    strncpy(msg.r_name, p[n].name, NAME_SIZE);
	    strncpy(msg.r_tty, "", TTY_SIZE);
	    (void) send_dgram(myhost, LEAVE_INVITE, n);
	    sprintf(errstr, "Rering %s@%s?", p[n].name, p[n].host);
	    if(found == 1)
	    {
		putc('\07', stderr);
		found = 2;
	    }
	    if(yes_no(errstr) == 'y')
	    {
		if(announce(p[n].host, n) != 0)
		{
		    send_dgram(myhost, DELETE, n);
		    deluser(n);
		    n--;
		}
	    }
	    else
	    {
		sprintf(errstr, "DELETE %s@%s?", p[n].name, p[n].host);
		if(yes_no(errstr) == 'y')
		{
		    send_dgram(myhost, DELETE, n);
		    deluser(n);
		    n--;
		}
	    }
	}
    if(!found)
	doalarm = 0;
    last_pulse = time((time_t *)0);
}

/* Finally, yytalkabort() terminates YTalk on error or sucessful completion.
 */
void yytalkabort(n)
{
    int x;

    EndLog();
    close_term();
    for(x = 3; x < 32; x++)
    {
	shutdown(x, 2);
	close(x);
    }
    DHburn();
    exit(n);
}

/* offcrypt()

   Turn off encryption.  If initiate is true, ask all our peers to do
   the same
*/
void offcrypt(initiate)
int initiate;
{
    int i;
    
    crypto_req = 0;
    crypto_ack = 0;
    crec.code = CRYPTOFF;
    for (i = 0 ; i < pnum ; i++)
    {
	if (p[i].flags & P_CRYPT)
	{
	    p[i].flags &= ~(P_CRYPT | P_CRYPTWAIT | P_CRYPTACK);
	    change_title(p[i].win, p[i].name, p[i].host, p[i].flags);
	    if (initiate)
		    write(p[i].sfd, &crec, CSIZ);
	}
    }
    change_title(0, "YTalk 2.1", NULL, 0);
}

/* sendDHpublic()

   Send our DH public part.  If private is true, we use private key
   authentication.  If askpass is true, we ask the user for the private
   passphrase.  If askpass is false, the user was already asked for the
   passphrase.
*/
void sendDHpublic(askpass, private)
int askpass;
int private;
{
    int n;
    IDEAkey key;
    word16 iv[4];
    int explen;
    cpack crec1;
    
    int headlen;
    int bodylen;
    
    headlen = RAND_PREFIX_LENGTH + 2;
    bodylen = sizeof(crec.ci_crypt) - headlen;
    
    /* ENCRYPT our key half */
    fill0(crec1.ci_crypt, sizeof(crec1.ci_crypt));
    
    if (crypto_req == 0)
    {
	if (private)
	{
	    if (askpass)
		    getpassphrase();
	    hashpass(inp.data, strlen(inp.data), ideakey);
	}

	init_crypto();
	explen = create_getDHpublic(crec1.ci_crypt + headlen);
	change_title(0, "YTalk 2.1", NULL, P_CRYPT | P_CRYPTACK);
    }
    else
    {
	explen = getDHpublic(crec1.ci_crypt + headlen);
    }
    
    crec1.ci_sig[0] = 0;
    crec1.ci_sig[1] = 0;
	
    if (crypto_req < pnum && !private && getenv("PGPPATH") != NULL)
    {
	char cmd[1024];
	char fname[16];
	FILE* pgpf;
	int c;
	
	strcpy(fname, "/tmp/ytkpXXXXXX");
#if !(defined(sco) || defined(__ultrix))
	close(mkstemp(fname));
#else
	mktemp(fname);
#endif
	
	sprintf(cmd, "%s -f +verbose=0 +armor=off +clearsig=off +textmode=off -sb > %s",
		PGPPROG, fname);
	shell_mode();
	fflush(stdout);
	pgpf = popen(cmd, "w");
	fwrite(crec1.ci_crypt + headlen, 1, bodylen, pgpf);
	pclose(pgpf);
	curses_mode();
	pgpf = fopen(fname, "r");
	c = fread(crec1.ci_sig + 2, 1, sizeof(crec1.ci_sig) - 2, pgpf);
	if (c == sizeof(crec1.ci_sig) - 2 || c <= 0)
		panic("Invalid output from pgp");
	else
	{
	    crec1.ci_sig[0] = c & 255;
	    crec1.ci_sig[1] = c >> 8;
	}
	fclose(pgpf);
	
	strcpy(fname, "/tmp/ytkeXXXXXX");
#if !(defined(sco) || defined(__ultrix))
	close(mkstemp(fname));
#else
	mktemp(fname);
#endif
	
	pgpf = fopen(fname, "w");
	fwrite(crec1.ci_crypt + headlen, 1, bodylen, pgpf);
	fclose(pgpf);
	/*unlink(fname);*/
    }
    else if (crypto_req < pnum && !private)
    {
	panic("Public-key authentication, but PGPPATH unset");
    }
    
    if (explen > bodylen)
	    abort();

    if (private)
    {
	init_idea_stream(crec1.ci_crypt, ideakey, ENCRYPT_IT, key, iv );
	ideacfb(crec1.ci_crypt + headlen, bodylen, key, iv, ENCRYPT_IT);
	crec1.code = CRYPT;
    }
    else
	    crec1.code = CRYPTP;
    
    for (n = 0 ; n < pnum ; n++)
    {
	if ((p[n].flags & P_CRYPT) == 0)
	{
	    write(p[n].sfd, &crec1, CSIZ);
	    p[n].flags |= P_CRYPT;
	    crypto_req++;
	    change_title(p[n].win, p[n].name, p[n].host, p[n].flags);
	}
    }
}

/* startcrypt()

   Initiate encryption with peers.  The user was already asked for the
   passphrase (if private is true).  If we are already encrypting,
   initiate turning encryption off.
*/
startcrypt(char* key, int private)
{
    char buf[4096];
    
    byte* expptr;
    
    if (crypto_req == pnum)
    {
	offcrypt(1);
	return;
    }
    
    sendDHpublic(0, private);
    
    change_title(0, "YTalk 2.1", 0, P_CRYPT | P_CRYPTACK);
    putc('\07', stderr);
}

/* checksig()

   Check a PGP signature.
*/

checksig(siglen, i)
int i;
int siglen;
{
    int headlen;
    int bodylen;
    
    headlen = RAND_PREFIX_LENGTH + 2;
    bodylen = sizeof(crec.ci_crypt) - headlen;
    
    if (getenv("PGPPATH") != NULL)
    {
	char cmd[1024];
	char fname[32];
	char sname[32];
	char str1[60];
	char str2[60];
	char str3[60];
	
	FILE* pgpf;
	
	strcpy(fname, "/tmp/ytkxXXXXXX");
#if !(defined(sco) || defined(__ultrix))
	close(mkstemp(fname));
#else
	mktemp(fname);
#endif
	
	pgpf = fopen(fname, "w");
	fwrite(crec.ci_crypt + headlen, 1, bodylen, pgpf);
	fclose(pgpf);
	
	strcpy(sname, "/tmp/ytksXXXXXX");
#if !(defined(sco) || defined(__ultrix))
	close(mkstemp(sname));
#else
	mktemp(sname);
#endif
	
	pgpf = fopen(sname, "w");
	fwrite(crec.ci_sig + 2, 1, siglen, pgpf);
	fclose(pgpf);
	
	str1[0] = 0;
	str2[0] = 0;
	str3[0] = 0;
	
	sprintf(cmd,
		"%s +batch +verbose=0 %s %s | tee | egrep '(Signature|signature from)'",
		PGPPROG, sname, fname);
	fflush(stdout);
	pgpf = popen(cmd, "r");
	if (fgets(str1, 64, pgpf) != NULL)
		if (fgets(str2, 64, pgpf) != NULL)
			fgets(str3, 64, pgpf);
	
	pclose(pgpf);
	
	/*fprintf(stderr, "Press any key:");
	mycbreak();
	getchar();*/
	
	str1[strlen(str1) - 1] = 0;
	str2[strlen(str2) - 1] = 0;
	str2[strlen(str3) - 1] = 0;
	
	if (!str1[0])
		strcpy(str1, "Invalid pgp signature");

	showmsg(str1, str2, str3);
	/*unlink(fname);*/
    }
    else
	    panic("Can't check pgp signature 'cause PGPPATH is unset");
}

/* checksig()

   This is called when we get a DH public key packet.
*/
gotcrypt(i, private)
int i;
int private;
{
    char buf[4096];
    unit peerkey[MAX_UNIT_PRECISION];
    
    int n, j;
    
    IDEAkey key;
    word16 iv[4];
    
    byte* expptr;
    int explen;
    
    int headlen;
    int bodylen;
    int siglen;
    
    headlen = RAND_PREFIX_LENGTH + 2;
    bodylen = sizeof(crec.ci_crypt) - headlen;
    
    if (p[i].flags & P_CRYPTWAIT)
    {
	panic("Protocol error! Already got session key from peer");
	return;
    }
    
    sendDHpublic(1, private);
    
    /* DECRYPT their key half */
    if (private)
    {
	if (init_idea_stream(crec.ci_crypt, ideakey, DECRYPT_IT, key, iv )
	    < 0)
	{
	    panic("Warning! Keys don't match.");
	    mp_init(peerkey, 2);
	    /* Some sane value so we can continue -
	     This value will force the session keys to be different.
	     This is useful to demonstrate that someone with the
	     wrong key will get garbage. */
	}
	else
	{
	    ideacfb(crec.ci_crypt + headlen, bodylen, key, iv, DECRYPT_IT) ;
	    mpi2reg(peerkey, crec.ci_crypt + headlen);
	}
    }
    else
    {
	mpi2reg(peerkey, crec.ci_crypt + headlen);
    }

    siglen = (unsigned char)crec.ci_sig[0];
    siglen += (unsigned char)crec.ci_sig[1] << 8;
    
    if (siglen)
	    checksig(siglen, i);

    explen = computeDHagreed_key(peerkey, &expptr);
	
    fill0(p[i].key, sizeof(p[i].key));
    for (n = 0 ; n < 16 ; n ++)
    {
	for (j = 0 ; j + n < explen ; j += 16)
	{
	    p[i].key[n] ^= expptr[j + n];
	}
    }
    
    crec.code = CRYPTIV;
    init_idea_stream(crec.ci_crypt, p[i].key, ENCRYPT_IT,
		     p[i].outkey, p[i].outiv );
    
    write(p[i].sfd, &crec, CSIZ);
    p[i].flags |= P_CRYPTWAIT;
}   

gotiv(i)
int i;
{
    if ((p[i].flags & P_CRYPTWAIT) == 0)
    {
	panic("Protocol error!  Got IV before session key from peer");
	return;
    }
    
    if (init_idea_stream(crec.ci_crypt, p[i].key, DECRYPT_IT,
			 p[i].inkey, p[i].iniv ) < 0)
	    panic("Warning!  Session keys don't match.");
    p[i].flags |= P_CRYPTACK;
    crypto_ack++;
    
    change_title(p[i].win, p[i].name, p[i].host, p[i].flags);
    
    if (crypto_ack == pnum)
	    putc('\07', stderr);
}
