Date: Sun, 21 Nov 93 01:09:40 GMT
Message-Id: <9311210109.AA08378@pizzabox.demon.co.uk>
From: "gtoal@gtoal.com" <gtoal@pizzabox.demon.co.uk>
Reply-To: Graham Toal <gtoal@gtoal.com>
To: uploads@demon.co.uk
Subject: Addon to pgpd
Status: RO

And here's a quick client to access it.  (This is so small I might as
well mail it here rather than upload it...)

--- pgpclient.h
#ifndef _H_PGPCLIENT /* Avoid double-inclusion as a matter of course */
#define _H_PGPCLIENT 000001 /* version 0.0.1 */
/*
 * Pgp server client program, based on the webster program by:
 *
 * David A. Curry
 * Purdue University
 * davy@ee.purdue.edu
 * April, 1986
 */

#include <stdio.h>

/* You can site-customise this header file by altering this to whichever
   host you want keys from - by default, it causes an error at runtime.
   Personally, I'd leave it as is and use an environment variable, to
   avoid accidents when someone else gets a binary copy of your prog...
 */

/* DEFAULT: #define PGPHOST "undefined" */
#define PGPHOST		"undefined"	/* host where the server lives	*/

#define PGPPORT		13013		/* port where the server lives	*/
#define PGPNAME		"pgp"		/* name of the service		*/
#define MINDELAY	60		/* no more often than one a min	*/

#define N_CLIENTERR 1000                /* Server return code must be <= 999 */

/* hostname may be NULL, portno may be 0                              */
/* returns FILE* to be used in pgp commands, or NULL                  */
FILE *open_pgp_server(char *hostname, int portno);

/* This version of command outputs its results to a file.             */
/* Other versions may follow after some thought...                    */
/* returns server rc, or >= N_CLIENTERR (rc > 999)                    */
/* (subtract N_CLIENTERR to get unix errno                            */
int pgp_fcommand(FILE *PGPSock, char *command, FILE *output);

/* Always remember to close the connection or you'll piss people off! */
/* returns TRUE or FALSE                                              */
int close_pgp_server(FILE *PGPSock);

#endif /* _H_PGPCLIENT */
--- pgpclient.c
#define DBMAIN
/*
 * pgp client program, based on the webster program by:
 *
 * David A. Curry
 * Purdue University
 * davy@ee.purdue.edu
 * April, 1986
 *
 * This derivative by gtoal@gtoal.com.  No copyright claimed.
 */

/* Since this is hacked from another source, a few of these headers may
   now be unnecessary */ 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include "pgpclient.h"

#define TRUE (0==0)
#define FALSE (0!=0)

static int debug = FALSE;

static void zzexit(int i, int lineno)
{
  if (!debug) exit(i);
  fprintf(stderr, "Exit %d at line %d\n", i, lineno);
  exit(i);
}
#define exit(i) zzexit(i, __LINE__)

static void debugerror(char *s)
{
  if (debug) fprintf(stderr, "%s\n", s);
}

/*
 * getline - read one line from the server and put it in s.
 */
static int getline(FILE *PGPSock, char *s)
{
  int c;

  /*
   * Read in chars. 
   */
  while ((c = getc(PGPSock)) != EOF) {
    c &= 255;
    if (c == '\r') continue;
    if (c == '\n') {
      *s = '\0';
      return(TRUE);
    }
    *s++ = c;
  }

  *s = '\0';
  return(FALSE);
}


/*
 * open_pgp_server - connects to the key server.  Server and host
 * may be given explicitly, or hostname may be NULL and/or portno may be 0.
 */
FILE *open_pgp_server(char *hostname, int portno)
{
  int s;
  FILE *PGPSock;
  struct sockaddr_in sin;
  struct hostent *hp;
  struct servent *sp;
  struct hostent *gethostbyname();
  struct servent *getservbyname();

  char *whichhost, *getenv();

  if (hostname != NULL) {\
    whichhost = hostname;
  } else {
    whichhost = getenv("PGPHOST");
    if (whichhost == NULL) whichhost = PGPHOST;
    if (strcmp(whichhost, "undefined") == 0) {
      fprintf(stderr,
        "pgpclient: %s\n%s\n%s\n",
        "You should either edit pgpclient.h and edit the PGPHOST line to"
        "#define PGPHOST \"some.host.name\"",
        "or put \"setenv PGPHOST some.host.name\" in your .login file"  
        );
      return(NULL);
    }
  }
  /*
   * Look up the host in the host file.
   */
  if ((hp = gethostbyname(whichhost)) == NULL) {
    fprintf(stderr, "pgpclient: %s: unknown host.\n", whichhost);
    return(NULL);
  }

  bzero(&sin, sizeof(struct sockaddr_in));

  /*
   * Build the server's address.
   */
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);

  if (portno != 0) {
    sin.sin_port = portno;
  } else {
    if ((sp = getservbyname(PGPNAME, "tcp")) == NULL) {
      sin.sin_port = htons(PGPPORT);
    } else {
      sin.sin_port = sp->s_port;
    }
  }
  /*
   * Get a TCP socket.
   */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    debugerror("pgpclient: socket");
    return(NULL);
  }

  /*
   * Try to connect.
   */
  if (connect(s, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0) {
    /*fprintf(stderr, "pgpclient: host %s: ", whichhost);*/
    debugerror("connect");
    close(s);
    return(NULL);
  }

  /*
   * Open the socket for stdio, connect to the server, drain the welcome message
   */
  PGPSock = fdopen(s, "r");
  if (pgp_fcommand(PGPSock, NULL, NULL) != 0) {
    fclose(PGPSock);
    return(NULL);
  }
  return(PGPSock);
}

static int getrc(char *buf)
{
  int rc = 0;
  rc += (buf[0]-'0')*100;
  rc += (buf[1]-'0')*10;
  rc += (buf[2]-'0');
  return(rc);
}

int pgp_fcommand(FILE *PGPSock, char *command, FILE *output)
{
  int rc;
  char buf[1024];
  /*
   * Send the command.
   */
  if (command != NULL) {  /* A special to drain the welcome message */
    if (strlen(command)+2 > sizeof(buf)-1) {
      fprintf(stderr, "pgpclient: command too long.\n");
      return(N_CLIENTERR);
    }
    sprintf(buf, "%s\r\n", command);
    if (send(fileno(PGPSock), buf, strlen(buf), 0) < 0) {
      debugerror("pgpclient: send");
      return(N_CLIENTERR);
    }
  }
  
  if (!getline(PGPSock, buf)) return(N_CLIENTERR);
  rc = getrc(buf); 
  /* I'll assume that 1?? means DATA/. follows */
  /* Except it doesn't...: */
  if (strncmp(buf, "101", 3) == 0) {
    /* Bad key, but need 'ok' following it.  What an abortion
       of a protocol. */
    if (!getline(PGPSock, buf)) return(N_CLIENTERR);
    return(rc); /* because of bad key */
  } else if (strncmp(buf, "1", 1) == 0) {
    for (;;) {
      if (!getline(PGPSock, buf)) return(N_CLIENTERR);
      if (strcmp(buf, ".") == 0) break;
      if (output != NULL) fprintf(output, "%s\n", buf);
    }
    if (!getline(PGPSock, buf)) return(N_CLIENTERR);
    /* 000 after data means OK, anything else probably an error */
    if (strncmp(buf, "000", 3) == 0) {
      return(0);
    } else {
      return(getrc(buf));
    }
  }
  /* We already have the first line of the response in buf */
  for (;;) {
    /* Don't output result codes to 'output', just data */
    /* 00? means end of reply? */
    if (strncmp(buf, "00", 2) == 0) {
      /* 000 is ok, all others are errors */
      if (strncmp(buf, "000", 3) == 0) {
        return(0);
      } else {
        return(getrc(buf));
      }
    }
    if (output != NULL) {
      /* A little paranoia about trailing spaces won't hurt */
/*    if (strlen(buf) >= 4) {  */
        /* Actually strlen is rather expensive in this context, really
           just want to look at first 4 bytes for a NUL */
      if ((buf[0] != '\0') && (buf[1] != '\0') &&
          (buf[2] != '\0') && (buf[3] != '\0')) {
        fprintf(output, "%s\n", buf+4);
      } else {
        fprintf(output, "\n");
      }
    }
    if (!getline(PGPSock, buf)) return(N_CLIENTERR);
  }
  return(rc);
}

int close_pgp_server(FILE *PGPSock)
{
  int rc;
  rc = pgp_fcommand(PGPSock, "QUIT", NULL);
  fclose(PGPSock);
  return(rc == 1);
}

#ifdef DBMAIN
int main(int argc, char **argv)
{
  FILE *PGPServ;
  int parentid;
  int rc;
  char *server = NULL, *portnostr = NULL;
  int portno = 0;

  /* First, a useful little test harness */

  while (argc > 1) {
    if (*argv[1] == '-') {
      int c = *(argv[1]+1);
      if (c == 'd') {
        debug = TRUE;
        argv += 1; argc -= 1;
      } else if (c == 'h') {
        fprintf(stdout, "pgptest:\n");
        fprintf(stdout, "  -h             Print this help info\n");
        fprintf(stdout, "  -s servername  Use given server\n");
        fprintf(stdout, "  -p portno      Call server on given port\n");
        fprintf(stdout, "  -d             Additional debug info on errors\n");
        exit(0);
      } else if (c == 'p') {
        if (argv[1][2] != '\0') {
          portnostr = argv[1]+2;
        } else {
          if (argc < 3) {
            fprintf(stderr, "pgptest: missing port number to -p\n");
            exit(1);
          }
          portnostr = argv[2]; argv += 1; argc -= 1;
        }
        if (sscanf(portnostr, "%d", &portno) != 1) {
          fprintf(stderr, "pgptest: argument to -p must be an int (not \"%s\")\n", portnostr);
          exit(1);
        }
        if (portnostr == NULL) {
          fprintf(stderr, "Internal error: portnostr = NULL\n"); exit(1);
        }
        if (debug) fprintf(stderr, "Using port %d\n", portno);
        argv += 1; argc -= 1;
      } else if (c == 's') {
        if (argv[1][2] != '\0') {
          server = argv[1]+2;
        } else {
          if (argc < 3) {
            fprintf(stderr, "pgptest: missing server name to -s\n");
            exit(1);
          }
          server = argv[2]; argv += 1; argc -= 1;
        }
        if (server == NULL) {
          fprintf(stderr, "Internal error: server = NULL\n"); exit(1);
        }
        if (debug && (server != NULL))
          fprintf(stderr, "Using server %s\n", server);
        argv += 1; argc -= 1;
      } else {
        fprintf(stderr, "syntax: pgptest [-d] [-p portno] [-s server]\n");
      }
    } else {
      fprintf(stderr, "pgptest: unexpected argument '%s' - try pgptest -h for help\n", argv[1]);
      exit(1);
    }
  }
  
  /* Test library calls here: */
  
  /* Exit on unknown host etc. */
  if ((PGPServ = open_pgp_server(server, portno)) == NULL) exit(1);
  
  if (pgp_fcommand(PGPServ, "HELP", stdout) != 0)
    fprintf(stderr, "HELP failed\n");
  if (pgp_fcommand(PGPServ, "GET branko", stdout) != 0)
    fprintf(stderr, "GET known key failed\n");
  if (pgp_fcommand(PGPServ, "GET fleeble", stdout) == 0)
    fprintf(stderr, "GET unknown key failed to report error\n");
  if (pgp_fcommand(PGPServ, "HELP", stdout) != 0)
    fprintf(stderr, "error recovery failed\n");
  
  if (!close_pgp_server(PGPServ)) fprintf(stderr, "QUIT failed\n");

  exit(0);
  return(1);
}
#endif

