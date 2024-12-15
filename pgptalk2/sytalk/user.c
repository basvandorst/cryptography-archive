/* user.c - YTalk user database module - YTalk V2.0 */

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
#include <sys/file.h>
#include <sys/types.h>
#ifdef __mips
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <netdb.h>
#include <stdio.h>
#ifdef __linux__
#include <gnu/types.h>
#define FD_SET __FD_SET
#define FD_CLR __FD_CLR
#endif

extern char myhost[100];	/* caller's host name */
extern int errno;		/* system error number */
extern CTL_MSG msg;		/* for communication with talk daemon */

person p[MAXC];		/* those in the conversation */
int pnum = 0;		/* number of current conversationalists */
int fdp[100];		/* for translating file numbers to user numbers */

extern char errstr[100], estr[100];	/* temporary string storage */
extern long doalarm;	/* set if there is housekeeping to do */
extern fd_set fdset;	/* main fdset for select() */
extern char edit[4];	/* edit keys */

/* finduser locates a user in the user list, if he exists.
 */
finduser(str)
char *str;
{
    char *hisname, *hishost;
    int n, x;

    hishost = NULL;
    hisname = str;
    for(; *str; str++)
	if(*str == '@')
	{
	    *str = '\0';
	    hishost = str+1;
	    break;
	}
    for(n = 0; n < pnum; n++)
    {
	if(strncmp(p[n].name, hisname, NAMELEN) != 0)
	    continue;
	if(hishost != NULL)
	    if(strncmp(p[n].host, hishost, HOSTLEN) != 0)
		continue;
	break;
    }
    if(n >= pnum)
    {
	if(hishost == NULL)
	    sprintf(errstr, "%s: Not in session", hisname);
	else
	    sprintf(errstr, "%s@%s: Not in session", hisname, hishost);
	errno = 0;
	panic(errstr);
	return -1;
    }
    return n;
}

/* killuser destroys a user by name.
 */
killuser(str)
char *str;
{
    int n;
    if((n = finduser(str)) < 0)
	return;
    deluser(n);
}

extern int crypto_req;
extern int crypto_ack;

/* deluser destroys all loose ends about a current conversation and removes
 * the user from the user list.
 */
deluser(i)
{
    if(i < 0 || i >= pnum)
	return;
    FD_CLR(p[i].sfd, &fdset);
    close(p[i].sfd);
    fdp[p[i].sfd] = -1;
    if(p[i].win >= 0)
	del_window(p[i].win);
    if(p[i].ffd != -1)
	close(p[i].ffd);
    if (p[i].flags & P_CRYPT)
    {
	if (crypto_req <= 0 || crypto_req > pnum) abort();
	crypto_req--;
    }
    
    if (p[i].flags & P_CRYPTACK)
    {
	if (crypto_ack <= 0 || crypto_ack > pnum) abort();
	crypto_ack--;
    }
    
    pnum--;
    
    for(; i < pnum; i++)
    {
	p[i] = p[i+1];
	fdp[p[i].sfd] = i;
    }
}

/* outfile opens an output file and attaches it to a particular user so
 * that all input from that user is copied to the output file.
 */
outfile(str, file)
char *str, *file;
{
    int n;

    if((n = finduser(str)) < 0)
	return;
    if(p[n].ffd != -1)
    {
	sprintf(errstr, "%s already has an output file", str);
	panic(errstr);
	return;
    }
    if((p[n].ffd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
    {
	sprintf(errstr, "Cannot open %s", file);
	panic(errstr);
	p[n].ffd = -1;
	return;
    }
}

/* newuser adds a new user to the conversation, either by accepting that
 * user's invitation, or instigating an invitation.  If the user in
 * question is using YTalk, newuser() will attempt to connect to his
 * auto-invitation port and forego the annoying annoucement message.
 */
newuser(str, histty, ring)
char *str, *histty, ring;
{
    char *hisname, *hishost;
    struct hostent *host;
    int n, x;

    if(pnum+1 >= MAXC)
    {
	errno = 0;
	panic("Too many people in conversation!\n");
	return;
    }

    /* First break down the username into login name and login host,
     * assuming our host as a default.
     */

    hishost = myhost;
    hisname = str;
    for(; *str; str++)
    {
	if(*str == '@')
	{
	    *str = '\0';
	    hishost = str+1;
	}
	if(*str == '#')
	{
	    *str = '\0';
	    histty = str+1;
	}
    }

    if((host = (struct hostent *) lookup_host(hishost)) == 0)
    {
	sprintf(errstr, "Unknown host: %s\n", hishost);
	panic(errstr);
	return;
    }
    strncpy(msg.r_name, hisname, NAME_SIZE);
    strncpy(msg.r_tty, "", TTY_SIZE);
    strncpy(p[pnum].name, hisname, NAMELEN);
    strncpy(p[pnum].host, host->h_name, HOSTLEN);
    strncpy(p[pnum].tty, histty, TTY_SIZE);
    p[pnum].ffd = -1;
    p[pnum].flags = 0;
    p[pnum].win = -1;
    p[pnum].last_ring = time((time_t *)0);
    p[pnum].a_id = 0;
    p[pnum].id = 0;

    for(n = 0; n < pnum; n++)
	if(strncmp(p[n].name, hisname, NAMELEN) == 0 &&
	   strncmp(p[n].host, host->h_name, HOSTLEN) == 0)
	{
	    if(p[n].flags & P_CONTACT)
		sprintf(errstr, "%s is already in this session", hisname);
	    else
		sprintf(errstr, "%s already being rung", hisname);
	    errno = 0;
	    panic(errstr);
	    return;
	}

    /* Check for a normal invitation */

    while((n = send_dgram(p[pnum].host, LOOK_UP, pnum)) == 0)
    {
	/* We are expected... */
	p[pnum].flags = 0;
	if((x = connect_to(pnum)) < 0)
	{
	    if(x == -2)
	    {
		send_dgram(p[pnum].host, DELETE, pnum);
		continue;
	    }
	    return;
	}
	fdp[p[pnum].sfd] = pnum;
	FD_SET(p[pnum].sfd, &fdset);
	bcopy(edit, errstr, 3);
	errstr[0] = RUBDEF;
	write(p[pnum].sfd, errstr, 3);	/* send the edit keys */
	pnum++;
	return;
    }
    if(n == -1)
	return;

    /* Leave an invitation for him, and announce ourselves. */

    strncpy(msg.r_tty, histty, TTY_SIZE);
    doalarm = 1;	/* set the housecleaning flag */
    p[pnum].flags = 0;
    if(newsock(pnum) != 0)
	return;
    (void) send_dgram(myhost, LEAVE_INVITE, pnum);
    if(ring)
	if((announce(p[pnum].host, pnum)) != 0)
	{
	    send_dgram(myhost, DELETE, pnum);
	    close(p[pnum].sfd);
	    return;
	}
    fdp[p[pnum].sfd] = pnum;
    FD_SET(p[pnum].sfd, &fdset);
    pnum++;
}
