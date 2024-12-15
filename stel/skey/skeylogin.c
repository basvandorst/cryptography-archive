/*
 * $Author: vince $
 * $Header: /users/vince/src/skey/RCS/skeylogin.c,v 1.38 1996/04/26 11:43:45 vince Exp $
 * $Date: 1996/04/26 11:43:45 $
 * $Revision: 1.38 $
 * $Locker:  $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: skeylogin.c,v 1.38 1996/04/26 11:43:45 vince Exp $";
#endif	lint

/*
 *   Login code for S/KEY Authentication.  S/KEY is a trademark
 *   of Bellcore.
 *
 *   Mink is the former name of the S/KEY authentication system.
 *   Many references for mink  may still be found in this program.
 */

#include <sys/param.h>
#ifdef	QUOTA
#include <sys/quota.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "skey.h"

#ifdef SKEYD
#define TALKERROR "Can't talk with skey authentication server\n"
#define CONFERROR "Unable to use skey authentication server's configuration file\n"
#define TOUTERROR "Connection with skey authentication server timed out!\n"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include "des.h"
#include "md4.h"

static char		*skeyserver;
static int		skeyport;
des_key_schedule	schedule;

static int		talktimeoutflg;
#endif

char *skipspace();

#ifdef SKEYD
/* this violates standard skeylookup() syntax */
int skeylookup __ARGS((struct skey *mp,char *name, int staylocal));
#else
int skeylookup __ARGS((struct skey *mp,char *name));
#endif



/* Issue a skey challenge for user 'name'. If successful,
 * fill in the caller's skey structure and return 0. If unsuccessful
 * (e.g., if name is unknown) return -1.
 *
 * The file read/write pointer is left at the start of the
 * record.
 */
int
getskeyprompt(mp,name,prompt)
struct skey *mp;
char *name;
char *prompt;
{
	int rval;

	sevenbit(name);
#ifdef SKEYD
	rval = skeylookup(mp,name, 0);
#else
	rval = skeylookup(mp,name);
#endif
	strcpy(prompt,"s/key 55 latour1\n");
	switch(rval){
	case -1:	/* File error */
		return -1;
	case 0:		/* Lookup succeeded, return challenge */
		sprintf(prompt,"s/key %d %s\n",mp->n - 1,mp->seed);
		return 0;
	case 1:		/* User not found */
		sfclose(mp->keyfile);
		return -1;
	}
	return -1;	/* Can't happen */
}	
/* Return  a skey challenge string for user 'name'. If successful,
 * fill in the caller's skey structure and return 0. If unsuccessful
 * (e.g., if name is unknown) return -1.
 *
 * The file read/write pointer is left at the start of the
 * record.
 */
int
skeychallenge(mp,name, ss)
struct skey *mp;
char *name;
char *ss;
{
	int rval;

	bzero(mp, sizeof(struct skey));

#ifdef SKEYD
	rval = skeylookup(mp,name, 0);
#else
	rval = skeylookup(mp,name);
#endif
	switch(rval){
	case -1:	/* File error */
		return -1;
	case 0:		/* Lookup succeeded, issue challenge */
                sprintf(ss, "s/key %d %s",mp->n - 1,mp->seed);
		return 0;
	case 1:		/* User not found */
		sfclose(mp->keyfile);
		return -1;
	}
	return -1;	/* Can't happen */
}

/* Find an entry in the One-time Password database.
 * Return codes:
 * -1: error in opening database
 *  0: entry found, file R/W pointer positioned at beginning of record
 *  1: entry not found, file R/W pointer positioned at EOF
 */
int
skeylookup(mp,name
#ifdef SKEYD	/* please note: this violates standard skeylookup() syntax.
		the meaning of ``staylocal'' is: do not use key servers,
		even if /etc/skeyd.conf exists  */
		, staylocal
#endif
			)
struct skey *mp;
char *name;
#ifdef SKEYD
int	staylocal;
#endif
{
	int found;
	int len;
	long recstart;
	char *cp;
	struct stat statbuf;

#ifdef SKEYD
	static char		answer[BUFLEN];

	/* do not use skeyd even if it is available */
	if (staylocal)
		goto readlocal;

	if (stat(SKEYDCONF, &statbuf) == -1)
		goto readlocal;

	/* file should never be readable by anyone */
	if (statbuf.st_mode & S_IROTH || statbuf.st_mode & S_IRGRP) {
		statbuf.st_mode &= ~(S_IROTH | S_IRGRP);
		(void)chmod(SKEYDCONF, statbuf.st_mode);
	}

	if (readskeydconf())
		return (-1);

	if (skeytalk(GIVEMEINFO, name, answer)) {
		fprintf(stderr, TALKERROR);
		return (-1);
	}

	if (answer[0] == '\0')
		return (1);

	if ((mp->logname = strtok(answer," \t")) == NULL)
		return (-1);
	if((cp = strtok(NULL," \t")) == NULL)
		return (-1);

	mp->n = atoi(cp);

	if((mp->seed = strtok(NULL," \t")) == NULL)
		return (-1);
	if((mp->val = strtok(NULL," \t")) == NULL)
		return (-1);
	return(0);

readlocal: /* no networking set, use local file */

#endif /* SKEYD */

	/* See if the KEYFILE exists, and create it if not */
	if(stat(KEYFILE,&statbuf) == -1 && errno == ENOENT){
		mp->keyfile = fopen(KEYFILE,"w+");
	} else {
		/* Otherwise open normally for update */
		mp->keyfile = fopen(KEYFILE,"r+");
	}
	if(mp->keyfile == NULL)
		return -1;

	/* Look up user name in database */
	len = strlen(name);
	if( len > 8 ) len = 8;		/*  Added 8/2/91  -  nmh */
	found = 0;
	while(!feof(mp->keyfile)) {
		recstart = ftell(mp->keyfile);
		mp->recstart = recstart;
		if(fgets(mp->buf,sizeof(mp->buf),mp->keyfile) != mp->buf){
			break;
		}
		rip(mp->buf);
		if(mp->buf[0] == '#')
			continue;	/* Comment */
		if((mp->logname = strtok(mp->buf," \t")) == NULL)
			continue;
		if((cp = strtok(NULL," \t")) == NULL)
			continue;
		mp->n = atoi(cp);
		if((mp->seed = strtok(NULL," \t")) == NULL)
			continue;
		if((mp->val = strtok(NULL," \t")) == NULL)
			continue;
		if(strlen(mp->logname) == len
		 && strncmp(mp->logname,name,len) == 0){
			found = 1;
			break;
		}
	}
	if(found){
		fseek(mp->keyfile,recstart,0);
		return 0;
	} else
		return 1;
}

/* Verify response to a s/key challenge.
 *
 * Return codes:
 * -1: Error of some sort; database unchanged
 *  0:  Verify successful, database updated
 *  1:  Verify failed, database unchanged
 *
 * The database file is always closed by this call.
 */
int
skeyverify(mp,response)
struct skey *mp;
char *response;
{
	char key[8];
	char fkey[8];
	char filekey[8];
	time_t now;
	struct tm *tm;
	char tbuf[27],buf[60];
	char *cp;
#ifdef SKEYD
	struct skeymessage	talk;
	char	line[BUFLEN], answer[BUFLEN];
	int	useskeyd = 0;
	struct stat	statbuf;

	if (stat(SKEYDCONF, &statbuf) == 0)
			useskeyd++;

	/* access is considered harmful */
	/*
	if (!access(SKEYDCONF, R_OK))
		useskeyd++;
	*/
#endif

	time(&now);
	tm = localtime(&now);
	strftime(tbuf, sizeof(tbuf), " %b %d,%Y %T", tm);

	if(response == NULL){
		sfclose(mp->keyfile);
		return -1;
	}
	rip(response);

	/* Convert response to binary */
	if(etob(key,response) != 1 && atob8(key,response) != 0) {
		/* Neither english words or ascii hex */
		sfclose(mp->keyfile);
		return -1;
	}

	/* Compute fkey = f(key) */
	memcpy(fkey,key,sizeof(key));
	fff(fkey);
	/* in order to make the window of update as short as possible
           we must do the comparison here and if OK write it back
           other wise the same password can be used twice to get in
  	   to the system
	*/

#ifndef SOLARIS24
	setpriority(PRIO_PROCESS, 0, -4);
#endif

	/* reread the file record NOW*/
#ifdef SKEYD
	if (useskeyd) {
		if (skeytalk(GIVEMEINFO, mp->logname, answer)) {
			fprintf(stderr, TALKERROR);
			return (-1);
		}
		strncpy(mp->buf, answer, sizeof(mp->buf));
	}
	else
#endif
	{
		fseek(mp->keyfile,mp->recstart,0);
		if(fgets(mp->buf,sizeof(mp->buf),mp->keyfile) != mp->buf){
#ifndef SOLARIS24
			setpriority(PRIO_PROCESS, 0, 0);
#endif
			sfclose(mp->keyfile);
			return -1;
		}
	}

	rip(mp->buf);
	mp->logname = strtok(mp->buf," \t");
	cp = strtok(NULL," \t") ;
	mp->seed = strtok(NULL," \t");
	mp->val = strtok(NULL," \t");
	/* And convert file value to hex for comparison */
	(void)atob8(filekey,mp->val);

	/* Do actual comparison */
	if(memcmp(filekey,fkey,8) != 0){
		/* Wrong response */
#ifndef SOLARIS24
		setpriority(PRIO_PROCESS, 0, 0);
#endif
		sfclose(mp->keyfile);
		return 1;
	}

	btoa8(mp->val,key);
	mp->n--;

#ifdef SKEYD
	if (useskeyd) {
		sprintf(line, "%s %04d %-16s %s %-21s\n",
			mp->logname, mp->n,mp->seed, mp->val, tbuf);
		if (skeytalk(UPDATEKEYS, line, answer)) {
			fprintf(stderr, TALKERROR);
			return (-1);
		}
	}
	else
#endif
	{
		/* Update key in database by overwriting entire record. Note
		 * that we must write exactly the same number of bytes as in
		 * the original record (note fixed width field for N)
		 */
		fseek(mp->keyfile,mp->recstart,0);
		fprintf(mp->keyfile,"%s %04d %-16s %s %-21s\n",mp->logname,
						mp->n,mp->seed, mp->val, tbuf);

		sfclose(mp->keyfile);
	}

#ifndef SOLARIS24
	setpriority(PRIO_PROCESS, 0, 0);
#endif
	return 0;
}


/* Convert 8-byte hex-ascii string to binary array
 * Returns 0 on success, -1 on error
 */
atob8(out,in)
register char *out,*in;
{
	register int i;
	register int val;

	if(in == NULL || out == NULL)
		return -1;

	for(i=0;i<8;i++){
		if((in = skipspace(in)) == NULL)
			return -1;
		if((val = htoi(*in++)) == -1)
			return -1;
		*out = val << 4;

		if((in = skipspace(in)) == NULL)
			return -1;
		if((val = htoi(*in++)) == -1)
			return -1;
		*out++ |= val;
	}
	return 0;
}

char *
skipspace(cp)
register char *cp;
{
	while(*cp == ' ' || *cp == '\t')
		cp++;

	if(*cp == '\0')
		return NULL;
	else
		return cp;
}

/* Convert 8-byte binary array to hex-ascii string */
int
btoa8(out,in)
register char *out,*in;
{
	register int i;

	if(in == NULL || out == NULL)
		return -1;

	for(i=0;i<8;i++){
		sprintf(out,"%02x",*in++ & 0xff);
		out += 2;
	}
	return 0;
}


/* Convert hex digit to binary integer */
int
htoi(c)
register char c;
{
	if('0' <= c && c <= '9')
		return c - '0';
	if('a' <= c && c <= 'f')
		return 10 + c - 'a';
	if('A' <= c && c <= 'F')
		return 10 + c - 'A';
	return -1;
}

#ifdef SKEYD

#define SKEYPWD		"skeypwd"
#define SKEYSERVER	"skeyserver"
#define SKEYPORT	"skeyport"

readskeydconf()
{
	FILE		*fp;
	char		line[BUFLEN];
	char		*p, *q, *sep = ": \t\n\r";
	extern int	errno;
	MD4_CTX		mdContext;
	des_cblock	key;
	int		i;
	int		keyset = 0, hstset = 0, prtset = 0;
	unsigned char	digest[16];

	if ((fp = fopen(SKEYDCONF, "r")) == NULL) {
		perror(SKEYDCONF);
		fprintf(stderr, CONFERROR);
		return (1);
	}
	errno = 0;
	while ((fgets(line, sizeof(line), fp)) != NULL) {
		/* skip blank lines, incomplete lines and remarks */
		if ((p = strtok(line, sep)) == NULL || *p == '#' ||
		    (q = strtok(NULL, sep)) == NULL)
			continue;
		if (!strncasecmp(p, SKEYPWD, strlen(SKEYPWD))) {
			bzero(&mdContext, sizeof(mdContext));
			MD4Init(&mdContext);
			MD4Update(&mdContext, q, strlen(q));
			MD4Final(digest, &mdContext);
			for (i = 0; i < 8; i++)
				digest[i] ^= digest[i + 8];
			memcpy(&key, digest, 8);
			des_set_key(&key, schedule);

#ifdef DEBUG
			printf("ENCRYPTION KEY: ");
			for (i = 0; i < 8; i++)
				printf("%02x", key[i]);
			printf("\n");
#endif

			keyset++;
		}
		else if (!strncasecmp(p, SKEYSERVER, strlen(SKEYSERVER))) {
			/*
			**  ULTRIX doesn't have strdup 
			**  studarus@psc.edu - Tue May  2 15:43:57 EDT 1995
			**  skeyserver = strdup(q);
			*/
			skeyserver = strdup(q);
			/*
			q = (char*)malloc(strlen(skeyserver) * sizeof(char)) ;
			strcpy(q,skeyserver) ;
			*/
			hstset++;
		}
		else if (!strncasecmp(p, SKEYPORT, strlen(SKEYPORT))) {
			skeyport = atoi(q);
			prtset++;
		}
	}

	if (errno) {
		perror("fgets()");
		return(1);
	}
	if (sfclose(fp) == -1) {
		perror("fclose()");
		return(1);
	}

	if (!keyset || !hstset || !prtset) {
		fprintf(stderr, "%s incomplete.\n", SKEYDCONF);
		fprintf(stderr, CONFERROR);
		skeyserver = NULL;
		skeyport = 0;
		return(1);
	}
	return(0);
}

skeytalk(flag, input, answer)
char	*input, *answer;
int	flag;
{
	struct skeymessage	talk;
	struct sockaddr_in	sin;
	int			s;
	static void		timeout();
	unsigned int		remaining = 0;
	void			(*oldsig) ();

	/* check that skeyd is really available. needed for skeyinit */
	if (skeyserver == NULL || skeyport == 0) {
		(void)fprintf(stderr, "skeyserver or skeyport not set!\n");
		return (1);
	}

	/* do not use name server for security */
	if ((sin.sin_addr.s_addr = inet_addr(skeyserver)) == -1) {
		(void)fprintf(stderr, "%s: unknown hostname\n", skeyserver);
		return (1);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons((ushort)skeyport);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return (1);
	}

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("connect()");
		return (1);
	}

	bzero(&talk, sizeof(talk));
	talk.flag = flag;
	strncpy(talk.buf, input, sizeof(talk.buf));

	talkencode(&talk);

	if ((oldsig = signal(SIGALRM, SIG_IGN)) != SIG_IGN)
		(void)signal(SIGALRM, timeout);
	remaining = alarm(SKEYTALKTIMEOUT);

	if (skey_write_data(s, &talk, sizeof(talk)) == -1) {
		if (errno == EINTR)
			(void)fprintf(stderr, TOUTERROR);
		else
			perror("write()");
		return (1);
	}

	if (skey_read_data(s, &talk, sizeof(talk)) == -1) {
		if (errno == EINTR)
			(void)fprintf(stderr, TOUTERROR);
		else
			perror("read()");
		return (1);
	}

	if (talktimeoutflg) {
		(void)fprintf(stderr, TOUTERROR);
		return (1);
	}

	(void)signal(SIGALRM, oldsig);
	(void)alarm(remaining);

	/* if talkdecode fails the password has been wrongly set
	   on the other side. talk.flag is not so important */
	if (talkdecode(&talk) || ntohl(talk.flag) != SKEYDOK) {
		(void)fprintf(stderr,
		"Received corrupted data from skey authentication server!\n");
		return(1);
	}

	(void)close(s);
	strcpy(answer, talk.buf);
	return(0);
}

/*
 * Modified read() & write() for sockets.
 * Returns: number of characters read if ok; -1 if fails.
 */
int		skey_read_data(s, buf, n)
int		s;
char		*buf;
int		n;
{
	int		bcount;
	int		br;

	bcount= 0;
	br= 0;
	while (bcount < n) {
		if ((br = read(s, buf, n - bcount)) > 0) {
			bcount += br;
			buf += br;
		}
		else if (br < 0)
			return(-1);
		else if (!br)
			break;
	}
	return(bcount);
}

int		skey_write_data(s, buf, n)
int		s;
char		*buf;
int		n;
{
	int		bcount;
	int		br;

	bcount= 0;
	br= 0;
	while (bcount < n) {
		if ((br = write(s, buf, n - bcount)) > 0) {
			bcount += br;
			buf += br;
		}
	else if (br < 0)
		return(-1);
	}
	return(bcount);
}

talkencode(talk)
struct skeymessage	*talk;
{
	MD4_CTX			mdContext;
	char			ivbuf[32];
	unsigned char		digest[16], ivbackup[8];

	/* set flag */
	talk->flag = htonl(talk->flag);

	/* build iv
	we just build up a suitably random string and then distill some
	randomness by means of md4.  this is not very strong, we know,
	but iv security is not really a concern here.
	*/
	bzero(ivbuf, sizeof(ivbuf));
	sprintf(ivbuf, "%ld %d %d", time(NULL), getpid(), getppid());

	MD4Init(&mdContext);
	MD4Update(&mdContext, (unsigned char *)ivbuf, strlen(ivbuf));
	MD4Final(digest, &mdContext);
	memcpy(talk->iv, digest, 8);
	memcpy(ivbackup, digest, 8);

	/* build checksum, skipping checksum itself.  the iv is included.
	since the iv gets modified in the des_cfb_encrypt step, we
	need to keep a iv backup copy to do things work */
	MD4Init(&mdContext);
	MD4Update(&mdContext, (unsigned char *)talk,
					sizeof(struct skeymessage) - 16);
	MD4Final(talk->checksum, &mdContext);

	/* encrypt buffer, skipping iv of course */
	des_cfb_encrypt((unsigned char *)talk + 8,
			(unsigned char *)talk + 8,
			8,
			sizeof(struct skeymessage) - 8,
			schedule,
			(des_cblock *)ivbackup,
			DES_ENCRYPT);

	return(0);
}

talkdecode(talk)
struct skeymessage	*talk;
{
	des_cblock		key;
	MD4_CTX			mdContext;
	unsigned char		digest[16], ivbackup[8];

	memcpy(ivbackup, (unsigned char *)talk->iv, 8);
	des_cfb_encrypt((unsigned char *)talk + 8,
			(unsigned char *)talk + 8,
			8,
			sizeof(struct skeymessage) - 8,
			schedule,
			(des_cblock *)talk->iv,
			DES_DECRYPT);

	memcpy((unsigned char *)talk->iv, ivbackup, 8);

	/* build checksum, skipping checksum itself */
	MD4Init(&mdContext);
	MD4Update(&mdContext, (unsigned char *)talk,
					sizeof(struct skeymessage) - 16);
	MD4Final(digest, &mdContext);

	talk->flag = htonl(talk->flag);

	return (memcmp(talk->checksum, digest, 16));
}

static void
timeout(s)
int	s;
{
	(void)signal(s, timeout);
	talktimeoutflg++;
}

#endif /* SKEYD */

/* safe close - check for pointer */
sfclose(fp)
FILE	*fp;
{
	int	x;

	if (fp == NULL)
		return(0);

	x = fclose(fp);
	fp = NULL;
	return (x);
}


#ifdef EXTRA
/* A set of small subroutines to make the lib work with obsolete code */
/*
 * skey_haskey ()
 *
 * Returns: 1 user doesnt exist, -1 fle error, 0 user exists.
 *
 */
skey_haskey (username)
  char *username;
{
  int i;
  struct skey skey;
 
#ifdef SKEYD
  return (skeylookup (&skey, username, 0));
#else
  return (skeylookup (&skey, username));
#endif
}
 
/*
 * skey_keyinfo ()
 *
 * Returns the current sequence number and
 * seed for the passed user.
 *
 */
char *skey_keyinfo (username)
  char *username;
{
  int i;
  static char str [50];
 
  struct skey skey;
 
  i = skeychallenge (&skey, username, str);
 
  if (i == -2)
     return 0;
 
  return str;
}
 
/*
 * skey_passcheck ()
 *
 * Check to see if answer is the correct one to the current
 * challenge.
 *
 * Returns: 0 success, -1 failure
 * XXX: calls skeyverify; therefore updates the record!! _H*
 */
skey_passcheck (username, passwd)
  char *username, *passwd;
{
  int i;
  struct skey skey;
 
#ifdef SKEYD
  i = skeylookup (&skey, username, 0);
#else
  i = skeylookup (&skey, username);
#endif
 
  if (i == -1 || i == 1)
      return -1;
 
  if (skeyverify (&skey, passwd) == 0)
      return skey.n;
 
  return -1;
}
#endif /* EXTRA */
