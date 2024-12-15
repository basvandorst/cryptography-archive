/*
 *  Name:              nslook.c
 *  Author:            nickel@cs.tu-berlin.de
 *  Version:           1.3 
 *  State:             save
 *  Last modification: Mon Sep 16 13:47:29 1991 by nickel@cs.tu-berlin.de
 */

/*
 * nslook.c  - simple Domain Name Service lookup. This is realized
 * just as a front end to gethostbyname() and gethostbyaddr().
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

/* exit codes: */
#define USAGE_ERROR 15		/* distinguish this from possible */
				/* values in h_errno */
#define UNKNOWN_ERROR 16	/* because h_errno is zero sometimes */
				/* even when gethostbyname() returns NULL */

char *progname ;
extern struct hostent *gethostbyname(), *gethostbyaddr() ;
extern int errno ;

main(argc, argv)
int argc ;
char **argv ;
{
    extern int optind ;		/* from getopt */
    int opt ;			/* option character */
    int aflg = 2 ;		/* 0=name, 1=address, 2=guess */
    int lflg = 0 ;		/* long output format */
    int errflg = 0 ;		/* usage error encountered */
    char *cp, *strrchr() ;

    progname = argv[0] ;
    if (cp = strrchr(progname, '/')) progname = cp + 1 ;
    while ((opt = getopt(argc, argv, "vlan")) != -1) {
	switch (opt) {
	  case 'l':		/* long */
	  case 'v':		/* verbose */
	    lflg = 1 ;
	    break ;
	  case 'a':		/* argument is address */
	    aflg = 1 ;
	    break ;
	  case 'n':		/* argument is name */
	    aflg = 0 ;
	    break ;
	  case '?':		/* give usage message */
	    errflg++ ;
	}
    }
    if (errflg || optind != argc - 1) {
	usage() ;
	exit(USAGE_ERROR) ;	/* distinguish from the other errors */
    }

    if (aflg == 2) {
      int i;			/* dummy for sscanf() */

      /* does the argument look like an address? */
      aflg = (4 == sscanf (argv[optind], "%d.%d.%d.%d", &i, &i, &i, &i));
    }
    if (aflg) {
	from_address(argv[optind], lflg) ;
    } else {
	from_name(argv[optind], lflg) ;
    }

    return 0 ;
}


from_address(astring, lflg)
char *astring ;
int lflg ;
{
    long addr ;			/* address in host order */
    long norder ;		/* address in network order */
    struct hostent *host ;	/* structure returned by gethostbyaddr() */
    int octet[4] ;		/* the four bytes of the address */

    /* parse string into address */
    if ((addr = inet_addr(astring)) == -1) {
	usage() ;
	exit(USAGE_ERROR) ;
    }

    /* try to find hostentry */
    if ((host = gethostbyaddr(&addr, 4, AF_INET)) == NULL) {
	gethost_error(progname, astring, 1) ;
    }

    /* print hostentry in long form or hostname */
    if (lflg) {
	print_long(host) ;
    } else {
	puts(host->h_name) ;
    }
}

from_name(name, lflg)
char *name ;
int lflg ;
{
    struct hostent *host ;

    /* try to find host entry */
    if ((host = gethostbyname(name)) == NULL) {
	gethost_error(progname, name, 0) ;
    }

    /* print host entry in long form or address */
    if (lflg) {
	print_long(host) ;
    } else {
	paddress(*host->h_addr_list) ;
    }
}

/* print host entry in long form */
print_long(host)
struct hostent *host ;
{
    int i ;
    unsigned long horder ;

    printf("Official name: %s\n", host->h_name) ;
    while (*host->h_aliases) {
	printf("Alias:         %s\n", *host->h_aliases++) ;
    }
    while (*host->h_addr_list) {
	printf("Address:       ") ;
	paddress(*host->h_addr_list++) ;
    }
}

/* print an internet address in network order with terminating newline */
paddress(paddr)
unsigned long *paddr ;
{
    unsigned long horder ;

    horder = ntohl(*paddr) ;
    printf("%u.%u.%u.%u\n", horder >> 24, (horder >> 16) & 0xff, 
	    (horder >> 8) & 0xff, horder & 0xff) ;

}
    

usage()
{
    fprintf(stderr, "\
Usage: %s [-l] [-n] hostname\n\
       %s [-l] [-a] IP-address\n\
-a forces interpretation of the argument as an address; -n forces\n\
interpretation of it as a hostname.  Otherwise, it is converted to\n\
a hostname if it looks like an address, and to an address if not.\n\
-l and -v both mean \"verbose output.\"\n",
	    progname, progname) ;
}

/* examine error condition */
gethost_error(s, name, addr_p)
char *s, *name ;
int addr_p;
{
    extern int h_errno ;	/* error code from gethostbyxxxx() */

    if (s && *s) {
	fputs(s, stderr) ;
	fputs(": ", stderr) ;
    }

    switch (h_errno) {
      case HOST_NOT_FOUND:
	fputs((addr_p ? "unknown address " : "unknown host "), stderr) ;
	fputs(name, stderr) ;
	fputs("\n", stderr) ;
	break ;
      case TRY_AGAIN:
	fputs("temporary error -- try again later\n", stderr) ;
	break ;
      case NO_RECOVERY:
	fputs("unrecoverable error\n", stderr) ;
	break ;
#ifdef NO_DATA
      case NO_DATA:
#else
#ifdef NO_ADDRESS
      case NO_ADDRESS:
#else
if neither NO_ADDRESS nor NO_DATA is defined, you are in trouble.
#endif /* NO_ADDRESS */
#endif /* NO_DATA  */
	fputs("no IP address\n", stderr) ;
	break ;
      default:
	fprintf(stderr, "unknown error number %d\n", h_errno) ;
	exit(UNKNOWN_ERROR) ;
    }
    exit(h_errno) ;
}
