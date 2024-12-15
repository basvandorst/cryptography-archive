/* Configuration file for the Freedom Remailer, 1995 jfleming@indiana.edu */

#ifndef REMAILER_CONFIG_H
#define REMAILER_CONFIG_H

#include <stdio.h>

#define YES 1
#define NO  0

/* Features - define to NO to disable */
#define USE_STATS	YES
#define ALLOW_POST	YES
#define USE_PGP		YES
#define DEBUG		NO

/* Paths and Filenames */

#define REMAILER_DIR "/home/test/freedom"
#define QUEUE_DIR    "/home/test/freedom/queue"
#define PGP_DIR      "/home/test/.pgp"
#define MAIL_SPOOL   "/usr/spool/mail/freedom"

#define REMAILER_BIN "remailer"
#define SENDMAIL     "/usr/lib/sendmail"
#define PGP          "/usr/local/bin/pgp"
#define INEWS        "/usr/local/bin/inews"

#define PASS_PHRASE  "foo-bar-mama"

#define HELP_FILE    "freedom-help"
#define STATS_FILE   "freedom-stats"
#define COUNT_FILE   "freedom-count"
#define SOURCE_BLOCK "blocked.source"
#define DEST_BLOCK   "blocked.destination"
#define SUBJ_BLOCK   "blocked.subject"
#define PGP_KEY_FILE "freedom-key"

#define REMAILER_NAME "Freedom Remailer"
#define REMAILER_ADDR "freedom@wherever.org"
#define COMPLAINTS_TO "remailer-admin@wherever.org"
#define ORGANIZATION  "Anonymous Posting Service"

/*************************************************************************/
/***************DON'T MODIFY ANYTHING BEYOND THIS POINT*******************/
/*************************************************************************/

#define STATS_MESSAGE 1
#define STATS_PGP     2
#define STATS_LATENT  3

#define NON_ANON      0
#define ANON_MESSAGE  1
#define STATS_REQ     2
#define HELP_REQ      3
#define PGP_MESSAGE   4
#define ANON_POST     5
#define CUTMARKS      6
#define KEY_REQ       7
#define LATENT_TIME   8
#define ENCRYPT_KEY   9
#define SUBJECT_BLOCKED	10

int scan_message(char *, char *, char *);
void remove_headers(char *, char *);
void stringlower(char *);
void updatestats(int);
void mailstats(char *);
void pgpdecrypt(char *, char *);
void mailhelp(char *);
void mailkey(char *);
void chop(char *);
int blocked(char *, char *);
void cut(char *, char *, char *, char *, int);
void queuelatent(char *, char *, char *, char *);
int getcount(void);
void makecount(void);
void pluscount(void);
int parsetime(char *, int);
void runlatentqueue(void);
void encrypt_key(char *, char *, char *);

#endif
