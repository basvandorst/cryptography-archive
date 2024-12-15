#ifdef	USE_SSL
/* 
 * The modifications to support SSLeay were done by Tim Hudson
 * tjh@mincom.oz.au
 *
 * You can do whatever you like with these patches except pretend that
 * you wrote them. 
 *
 * Email ssl-users-request@mincom.oz.au to get instructions on how to
 * join the mailing list that discusses SSLeay and also these patches.
 *
 */

/* ssl.c    - interface to Eric Young's SSLeay library (eay@mincom.oz.au)
 *
 * see LICENSE for details 
 *
 * 01-Jul-95 tjh    merged patches from Steven Schoch 
 * ................ <schoch@sheba.arc.nasa.gov> that add in the certsok
 * ................ option for using signed certificates rather than 
 * ................ explicit passwords for authentication (modified a little
 * ................ to add in an option that controls this feature)
 * 26-Apr-95 tjh    original coding
 *
 * tjh@mincom.oz.au
 * Tim Hudson
 * Mincom Pty Ltd
 * Australia
 * +61 7 303 3333
 *
 */

#include <sys/types.h>
#include <arpa/telnet.h>
#include <stdio.h>
#ifdef	__STDC__
#include <stdlib.h>
#endif
#ifdef	NO_STRING_H
#include <strings.h>
#else
#include <string.h>
#endif

#include "auth.h"
#include "misc.h"

#include "ssl.h"

extern int auth_debug_mode;
static auth_ssl_valid = 0;
static char *auth_ssl_name = 0;    /* this holds the oneline name */

extern int ssl_only_flag;
extern int ssl_debug_flag;
extern int ssl_active_flag;
extern int ssl_secure_flag;
extern int ssl_verify_flag;
extern int ssl_certsok_flag;       /* if this is set then we enable the
                                    * /etc/ssl.users stuff for allowing
				    * access - just to make sure we don't
				    * switch it on unless we really want it
                                    */
static int verify_callback();
extern SSL *ssl_con;

/* compile this set to 1 to negotiate SSL but not actually start it */
static int ssl_dummy_flag=0;

static unsigned char str_data[1024] = { IAC, SB, TELOPT_AUTHENTICATION, 0,
			  		AUTHTYPE_SSL, };

#define AUTH_SSL_START     1
#define AUTH_SSL_ACCEPT    2
#define AUTH_SSL_REJECT    3


/* this is called by both the ssl.c auth connect and the mainline
 * telnet connect if we are talking straight ssl with no telnet
 * protocol --tjh
 */
int
display_connect_details(ssl_con,verbose)
SSL *ssl_con;
int verbose;
{
    X509 *peer;

    fprintf(stderr,"[SSL Connected - Cipher %s]\n",SSL_get_cipher(ssl_con));
    peer=SSL_get_peer_certificate(ssl_con);
    if (peer != NULL) {
	char *str;

	str=X509_NAME_oneline(X509_get_subject_name(peer));
	fprintf(stderr,"[SSL subject=%s]\n",str);
	free(str);
	str=X509_NAME_oneline(X509_get_issuer_name(peer));
	fprintf(stderr,"[SSL issuer=%s]\n",str);
	free(str);
	X509_free(peer);
			
    }
    fflush(stderr);
}


	void
fprintd(fp, data, cnt)
	FILE *fp;
	unsigned char *data;
	int cnt;
{
	if (cnt > 16)
		cnt = 16;
	while (cnt-- > 0) {
		fprintf(fp," %02x", *data);
		++data;
	}
}

/* support routine to send out authentication message */
static int Data(ap, type, d, c)
Authenticator *ap;
int type;
void *d;
int c;
{
        unsigned char *p = str_data + 4;
	unsigned char *cd = (unsigned char *)d;

	if (c == -1)
		c = strlen((char *)cd);

        if (auth_debug_mode) {
                fprintf(stderr,"%s:%d: [%d] (%d)",
                        str_data[3] == TELQUAL_IS ? ">>>IS" : ">>>REPLY",
                        str_data[3],
                        type, c);
                fprintd(stderr,d, c);
                fprintf(stderr,"\r\n");
        }
	*p++ = ap->type;
	*p++ = ap->way;
	*p++ = type;
        while (c-- > 0) {
                if ((*p++ = *cd++) == IAC)
                        *p++ = IAC;
        }
        *p++ = IAC;
        *p++ = SE;
	if (str_data[3] == TELQUAL_IS)
		printsub('>', &str_data[2], p - (&str_data[2]));
        return(net_write(str_data, p - str_data));
}

int auth_ssl_init(ap, server)
Authenticator *ap;
int server;
{
	/* ssl only option skips all of this muck ... */
	if (ssl_only_flag)
	    return 0;

	if (server)
		str_data[3] = TELQUAL_REPLY;
	else
		str_data[3] = TELQUAL_IS;
	return(1);
}

/* client received a go-ahead for ssl */
int auth_ssl_send(ap)
Authenticator *ap;
{
	fprintf(stderr,"Trying to negotiate SSL\r\n");

	if (!Data(ap, AUTH_SSL_START, NULL, 0 )) {
		if (auth_debug_mode)
			fprintf(stderr,"Not enough room for start data\r\n");
		return(0);
	}

	return(1);
}

/* server received an IS -- could (only) be SSL START */
void auth_ssl_is(ap, data, cnt)
Authenticator *ap;
unsigned char *data;
int cnt;
{
	int valid;

	if (cnt-- < 1)
		return;
	switch (*data++) {

	case AUTH_SSL_START:
		Data(ap, AUTH_SSL_ACCEPT, (void *)0, 0);
		netflush();

		auth_ssl_valid = 1;
		auth_finished(ap, AUTH_VALID);

		/* server starts the SSL stuff now ... */
		if (ssl_dummy_flag)
		    return;

		if (!ssl_only_flag) {
		    /* only want/need verify if doing certsok stuff */
		    if (ssl_certsok_flag) 
			SSL_set_verify(ssl_con,ssl_verify_flag,verify_callback);
		    if (!SSL_accept(ssl_con)) {

			/*
			syslog(LOG_WARNING, "ssl_accept error");
			*/

			fprintf(stderr,"ssl_accept error\n");
			fflush(stderr);
			sleep(5);
			SSL_free(ssl_con);

			auth_finished(ap, AUTH_REJECT);

			_exit(1);
		    } else {
			ssl_active_flag=1;

		    }
		}
		break;

	default:
		if (auth_debug_mode)
			fprintf(stderr,"Unknown SSL option %d\r\n", data[-1]);
		Data(ap, AUTH_SSL_REJECT, (void *)0, 0);
		if (auth_debug_mode)
			fprintf(stderr,"SSL negotiation failed\r\n");
		auth_ssl_valid = 0;
		auth_finished(ap, AUTH_REJECT);
		break;
	}
}

/* client received REPLY -- could be SSL ACCEPT or REJECT */
void auth_ssl_reply(ap, data, cnt)
Authenticator *ap;
unsigned char *data;
int cnt;
{
	int i;

	if (cnt-- < 1)
		return;
	switch (*data++) {

	case AUTH_SSL_ACCEPT:
		if (auth_debug_mode)
			fprintf(stderr,"SSL ACCEPT\r\n");
		fprintf(stderr,"[SSL starting]\r\n");

		auth_finished(ap, AUTH_VALID);

		if (ssl_dummy_flag) {
		    fprintf(stderr,"[SSL Dummy Connected]\r\n");
		    fflush(stderr);
		    return;
		}

		/* right ... now we drop into the SSL library */
		if (!ssl_only_flag) {
		    if (SSL_connect(ssl_con) <= 0) {
			perror("telnet: Unable to ssl_connect to remote host");

			SSL_load_error_strings();
			ERR_print_errors(stderr);

			/* don't know what I "should" be doing here ... */

			auth_finished(0,AUTH_REJECT);
			return;
		    } else {
			display_connect_details(ssl_con,1);
			ssl_active_flag=1;
		    }
		}

		/* this is handy/required? */
		/*
		netflush();
		*/

		break;

	case AUTH_SSL_REJECT:
		if (auth_debug_mode)
			fprintf(stderr,"SSL REJECT\r\n");
		fprintf(stderr,"[SSL negotiation failed]\r\n");
		fprintf(stderr,"Trying plaintext login:\r\n");
		auth_finished(0,AUTH_REJECT);
		break;

	default:
		if (auth_debug_mode)
			fprintf(stderr,"Unknown SSL option %d\r\n", data[-1]);
		return;
	}
}

int auth_ssl_status(ap, name, level)
Authenticator *ap;
char *name;
int level;
{
	FILE *user_fp;
	char buf[2048];

	if (level < AUTH_USER)
		return(level);

	/*
	 * Look our name up in /etc/ssl.users.
	 * The format of this file is lines of this form:
	 *   user1,user2:/C=US/.....
	 * where user1 and user2 are usernames
	 */
	if (ssl_certsok_flag) {
	    user_fp = fopen("/etc/ssl.users", "r");
	    if (!auth_ssl_name || !user_fp) {
	        /* If we haven't received a certificate, then don't 
		 * return AUTH_VALID. 
		 */
		if (UserNameRequested)
			strcpy(name, UserNameRequested);
		/* be tidy ... */
		if (user_fp)
		    fclose(user_fp);
		return AUTH_USER;
	    }
	    while (fgets(buf, sizeof buf, user_fp)) {
		char *cp;
		char *n;

		/* allow for comments in the file ... always nice
		 * to be able to add a little novel in files and
		 * also disable easily --tjh
		 */
		if (buf[0]=='#')
		    continue;

		if (cp = strchr(buf, '\n'))
		    *cp = '\0';
		cp = strchr(buf, ':');
		if (!cp)
		    continue;
		*cp++ = '\0';
		if (strcasecmp(cp, auth_ssl_name) == 0) {
		    n = buf;
		    while (n) {
			cp = strchr(n, ',');
			if (cp)
			    *cp++ = '\0';
			if (!UserNameRequested || 
			            !strcmp(UserNameRequested, n)) {
			    strcpy(name, n);
			    fclose(user_fp);
			    return(AUTH_VALID);
			}
			n = cp;
		    }
		}
	    }
	    fclose(user_fp);
	    return(AUTH_USER);
	} else {
	    /* we force the user to provide a password (over the
	     * encrypted channel to do "normal" authentication
	     */
	    if (UserNameRequested && auth_ssl_valid) {
		    strcpy(name, UserNameRequested);
		    return(AUTH_VALID);
	    } else
		    return(AUTH_USER);
	}
}

#define	BUMP(buf, len)		while (*(buf)) {++(buf), --(len);}
#define	ADDC(buf, len, c)	if ((len) > 0) {*(buf)++ = (c); --(len);}

void auth_ssl_printsub(data, cnt, buf, buflen)
unsigned char *data, *buf;
int cnt, buflen;
{
	char lbuf[32];
	register int i;

	buf[buflen-1] = '\0';		/* make sure its NULL terminated */
	buflen -= 1;

	switch(data[3]) {

	case AUTH_SSL_START:
		strncpy((char *)buf, " START ", buflen);
		goto common;

	case AUTH_SSL_REJECT:		/* Rejected (reason might follow) */
		strncpy((char *)buf, " REJECT ", buflen);
		goto common;

	case AUTH_SSL_ACCEPT:		/* Accepted (name might follow) */
		strncpy((char *)buf, " ACCEPT ", buflen);

	common:
		BUMP(buf, buflen);
		if (cnt <= 4)
			break;
		ADDC(buf, buflen, '"');
		for (i = 4; i < cnt; i++)
			ADDC(buf, buflen, data[i]);
		ADDC(buf, buflen, '"');
		ADDC(buf, buflen, '\0');
		break;

	default:
		sprintf(lbuf, " %d (unknown)", data[3]);
		strncpy((char *)buf, lbuf, buflen);
	common2:
		BUMP(buf, buflen);
		for (i = 4; i < cnt; i++) {
			sprintf(lbuf, " %d", data[i]);
			strncpy((char *)buf, lbuf, buflen);
			BUMP(buf, buflen);
		}
		break;
	}
}

static int
verify_callback(ok, xs, xi, depth, error)
int ok;
char *xs, *xi;
int depth, error;
{
    /*
     * If the verification fails, then don't remember the name.  However,
     * if we don't require a certificate, then return success which will
     * still allow us to set up an encrypted session.
     *
     */
    if (!ok) {
	/* If we can't verify the issuer, then don't accept the name. */
	if (depth != 0 && auth_ssl_name) {
		free(auth_ssl_name);
		auth_ssl_name = 0;
	}
	return ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT ? 0 : 1;
    }
    if (depth == 0)
	auth_ssl_name =
	    (char *)X509_NAME_oneline(X509_get_subject_name(xs));
    return ok;
}

#endif /* USE_SSL */

