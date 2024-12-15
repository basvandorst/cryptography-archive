#ifndef  _ytalk_h_
#  define _ytalk_h_
/* ytalk.h V2.0 */

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

/* --------------------------------------- */
/* IT IS NOT NECESSARY TO MODIFY THIS FILE */
/* --------------------------------------- */

#include "config.h"
#include "talkd.h"
#include "pgp.h"
#include <errno.h>

#define MAXC	10	/* Max number of talk windows at one time */

typedef struct {
    unsigned char w_rows, w_cols;	/* terminal rows and cols */
    char clr;				/* clear the screen key */
    char pad[61];
} y_parm;
#define YPARMLEN	sizeof(y_parm)
#define WROWS		yparms.w_rows
#define WCOLS		yparms.w_cols

#ifndef __ultrix
#ifdef POSIX
typedef signed char s_char;  /* some systems don't default "char" as signed */
#else
typedef char s_char;
#endif
#endif

typedef struct {
#define NAMELEN	12
    char name[NAMELEN];		/* user name */
#define HOSTLEN 64
    char host[HOSTLEN];		/* user host */
    char tty[TTY_SIZE];		/* his tty */
    IDEAkey	inkey;
    IDEAkey	outkey;
    byte	key[16];
    word16	outiv[4];
    word16	iniv[4];
    int sfd;			/* socket fd */
    int ffd;			/* file fd (if applicable) */
    int flags;			/* flags (see below) */
    int id, a_id;		/* talkd id's */
    int win;			/* window id */
    long last_ring;		/* time of last ring */
    struct sockaddr_in sock;	/* socket */
    s_char edit[4];		/* edit characters */
    y_parm yparms;		/* ytalk parameters */
} person;

#define RUB	edit[0]
#define KILL	edit[1]
#define WORD	edit[2]
#define CLR	edit[3]

/* FLAGS */
#define P_CONTACT	0x01	/* Contact has been made */
#define P_CONNECT	0x02	/* Socket is connected */
#define P_PRIVATE	0x04	/* Session is private */
#define P_YTALK		0x08	/* Is a YTalk program (vs. Berkeley) */
#define P_CRYPT		0x10	/* Is it encrypted */
#define P_CRYPTWAIT	0x20	/* wait for IV */
#define P_CRYPTACK	0x40	/* negotiation done */

/* DAEMON TYPES */
#define D_UNKNOWN	0	/* unknown as of yet */
#define D_BSD		1	/* BSD before 4.2 */
#define D_BSD42		2	/* BSD after 4.2 */
#define D_YTALK		4	/* BSD through YTalkd */

typedef struct {
    s_char code;
    char name[60];
    char data[256];
} inptype;

/* The following structure is for communicating between YTalk hosts.
 * If you change this structure, BE SURE the final structure can be
 * transferred and understood by different machine architectures.
 * Ie: check your padding and address boundaries for integers.
 */

typedef struct {
    s_char code;		/* see below */
    char filler;
    union
    {
	struct 
	{
	    char name[NAMELEN];		/* user name */
	    char host[HOSTLEN];	/* user host */
	} info;
	struct
	{
	    byte crypt[128];
	    byte sig[250];
	} cryptinfo;
    } data;
} cpack;

#define ci_name data.info.name
#define ci_host data.info.host
#define ci_crypt data.cryptinfo.crypt
#define ci_sig data.cryptinfo.sig

#define CSIZ	sizeof(cpack)
#define EXPORT	(s_char) -126	/* export a user */
#define IMPORT	(s_char) -125	/* import a user */
#define ACCEPT	(s_char) -124	/* accept a connection from a user */
#define AUTO	(s_char) -123	/* accept auto invitation */
#define CRYPT	(s_char) -122	/* start encryption (private key) */
#define CRYPTP	(s_char) -121	/* start encryption (public key) */
#define CRYPTIV	(s_char) -120	/* exchange encryption initial vector */
#define CRYPTOFF (s_char) -119	/* ask encryption to be turned off*/
#define CRYPTSIG (s_char) -118	/* signature for authentication */
#define RUBDEF	(s_char) -2	/* rub a character */

#define RCSIZ	508
#define YTPORT	15348
typedef struct {
    short type;
    short length;
    char buf[RCSIZ];
} rcpack;


#ifdef NOTOUCHOLAP
#define wtouch(x,y)	touchwin((y))
#else
#define wtouch(x,y)	touchoverlap((x),(y))
#endif

#ifdef NOCBREAK
#define cmode()		crmode()
#else
#define cmode()		cbreak()
#endif

/* External variables related to cryptography */
extern int crypto_req;
int crypto_ack;

extern byte ideakey[16];
extern cpack crec;
extern int pnum;
extern person p[MAXC];
extern inptype inp;

/* EOF */
#endif /* _ytalk_h_ */
