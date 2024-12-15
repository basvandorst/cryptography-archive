/*
 * Copyright (c) 1985,1989 Regents of the University of California.
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
char copyright[] =
"@(#) Copyright (c) 1985,1989 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	5.42 (Berkeley) 3/3/91";
#endif /* not lint */

/*
 *******************************************************************************
 *  
 *   main.c --
 *  
 *	Main routine and some action routines for the name server
 *	lookup program.
 *
 *	Andrew Cherenson
 *	U.C. Berkeley Computer Science Div.
 *	CS298-26, Fall 1985
 *  
 *******************************************************************************
 */

#include <sys/types.h>
#include <sys/param.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "res.h"

#ifdef __ultrix
#define nsaddr_list addr.ns_list
#endif

#include "pathnames.h"

/*
 *  Default Internet address of the current host.
 */

#if BSD < 43
#define LOCALHOST "127.0.0.1"
#endif


/*
 * Name of a top-level name server. Can be changed with 
 * the "set root" command.
 */

#ifndef ROOT_SERVER
#define		ROOT_SERVER "ns.nic.ddn.mil."
#endif
char		rootServerName[NAME_LEN] = ROOT_SERVER;


/*
 *  Import the state information from the resolver library.
 */

extern struct state _res;


/*
 *  Info about the most recently queried host.
 */

HostInfo	curHostInfo;
int		curHostValid = FALSE;


/*
 *  Info about the default name server.
 */

HostInfo	*defaultPtr = NULL;
char		defaultServer[NAME_LEN];
struct in_addr	defaultAddr;


/*
 *  Initial name server query type is Address.
 */

int		queryType = T_A;
int		queryClass = C_IN;
FILE* filePtr;

/*
 * Stuff for Interrupt (control-C) signal handler.
 */


/*
 *******************************************************************************
 *
 *  main --
 *
 *	Initializes the resolver library and determines the address
 *	of the initial name server. The yylex routine is used to
 *	read and perform commands.
 *
 *******************************************************************************
 */

InitResolver()
{
    char	*wantedHost = NULL;
    Boolean	useLocalServer;
    int		result;
    int		i;
    struct hostent	*hp;
    extern int	h_errno;
    struct in_addr addr;

    /*
     *  Initialize the resolver library routines.
     */

    if (res_init() == -1) {
	fprintf(stderr,"*** Can't initialize resolver.\n");
	exit(1);
    }

    /*
     *  Allocate space for the default server's host info and
     *  find the server's address and name. If the resolver library
     *  already has some addresses for a potential name server,
     *  then use them. Otherwise, see if the current host has a server.
     *  Command line arguments may override the choice of initial server. 
     */

    defaultPtr = (HostInfo *) calloc(1, sizeof(HostInfo));

    useLocalServer = FALSE;
    
    if (_res.nscount == 0 || useLocalServer) {
	LocalServer(defaultPtr);
    } else {
	for (i = 0; i < _res.nscount; i++) {
#ifdef __ultrix
	    if (_res.ns_list[i].addr.sin_addr.s_addr == INADDR_ANY) {
#else
	    if (_res.nsaddr_list[i].sin_addr.s_addr == INADDR_ANY) {
#endif
	        LocalServer(defaultPtr);
		break;
	    } else {
#ifdef __ultrix
		result = GetHostInfoByAddr(&(_res.ns_list[i].addr.sin_addr), 
#else
		result = GetHostInfoByAddr(&(_res.nsaddr_list[i].sin_addr), 
#endif				    &(_res.nsaddr_list[i].sin_addr), 
				    defaultPtr);
		if (result != SUCCESS) {
		    fprintf(stderr,
		    "*** Can't find server name for address %s\n", 
#ifdef __ultrix
		       inet_ntoa(_res.ns_list[i].addr.sin_addr));
		} else {
		    defaultAddr = _res.ns_list[i].addr.sin_addr;
#else
		       inet_ntoa(_res.nsaddr_list[i].sin_addr));
		} else {
		    defaultAddr = _res.nsaddr_list[i].sin_addr;
#endif
		    break;
		}
	    }
	}

	/*
	 *  If we have exhausted the list, tell the user about the
	 *  command line argument to specify an address.
	 */

	if (i == _res.nscount) {
	    fprintf(stderr, "*** Default servers are not available\n");
	    exit(1);
	}

    }
    strcpy(defaultServer, defaultPtr->name);


#ifdef DEBUG
#ifdef DEBUG2
    _res.options |= RES_DEBUG2;
#endif
    _res.options |= RES_DEBUG;
    _res.retry    = 2;
#endif DEBUG

    /*
     * If we're in non-interactive mode, look up the wanted host and quit.
     * Otherwise, print the initial server's name and continue with
     * the initialization.
     */

}

LocalServer(defaultPtr)
    HostInfo *defaultPtr;
{
    char	hostName[NAME_LEN];
#if BSD < 43
    int		result;
#endif

    gethostname(hostName, sizeof(hostName));

#if BSD < 43
    defaultAddr.s_addr = inet_addr(LOCALHOST);
    result = GetHostInfoByName(&defaultAddr, C_IN, T_A, 
		hostName, defaultPtr, 1);
    if (result != SUCCESS) {
	fprintf(stderr,
	"*** Can't find initialize address for server %s\n",
			defaultServer);
	exit(1);
    }
#else
    defaultAddr.s_addr = htonl(INADDR_ANY);
    (void) GetHostInfoByName(&defaultAddr, C_IN, T_A, "0.0.0.0", defaultPtr, 1);
    free(defaultPtr->name);
    defaultPtr->name = calloc(1, sizeof(hostName)+1);
    strcpy(defaultPtr->name, hostName);
#endif
}


/*
 *******************************************************************************
 *
 * IsAddr --
 *
 *	Returns TRUE if the string looks like an Internet address.
 *	A string with a trailing dot is not an address, even if it looks
 *	like one.
 *
 *	XXX doesn't treat 255.255.255.255 as an address.
 *
 *******************************************************************************
 */

Boolean
IsAddr(host, addrPtr)
    char *host;
    unsigned long *addrPtr;	/* If return TRUE, contains IP address */
{
    register char *cp;
    unsigned long addr;

    if (isdigit(host[0])) {
	    /* Make sure it has only digits and dots. */
	    for (cp = host; *cp; ++cp) {
		if (!isdigit(*cp) && *cp != '.') 
		    return FALSE;
	    }
	    /* If it has a trailing dot, don't treat it as an address. */
	    if (*--cp != '.') { 
		if ((addr = inet_addr(host)) != (unsigned long) -1) {
		    *addrPtr = addr;
		    return TRUE;
#if 0
		} else {
		    /* XXX Check for 255.255.255.255 case */
#endif
		}
	    }
    }
    return FALSE;
}


/*
 *******************************************************************************
 *
 * DoLoookup --
 *
 *	Common subroutine for LookupHost and LookupHostWithServer.
 *
 *  Results:
 *	SUCCESS		- the lookup was successful.
 *	Misc. Errors	- an error message is printed if the lookup failed.
 *
 *******************************************************************************
 */

HostInfo*
DoLookup(host)
    char	*host;
{
    int result;
    struct in_addr *servAddrPtr;
    struct in_addr addr; 

    if (defaultPtr == NULL)
	    InitResolver();
    /*
     *  If the user gives us an address for an address query, 
     *  silently treat it as a PTR query. If the query type is already
     *  PTR, then convert the address into the in-addr.arpa format.
     *
     *  Use the address of the server if it exists, otherwise use the
     *	address of a server who knows about this domain.
     *  XXX For now, just use the first address in the list.
     */

    if (defaultPtr->addrList != NULL) {
	servAddrPtr = (struct in_addr *) defaultPtr->addrList[0];
    } else {
	servAddrPtr = (struct in_addr *) defaultPtr->servers[0]->addrList[0];
    }

    /* 
     * RFC1123 says we "SHOULD check the string syntactically for a 
     * dotted-decimal number before looking it up [...]" (p. 13).
     */
    if (IsAddr(host, &addr.s_addr)) {
	result = GetHostInfoByAddr(servAddrPtr, &addr, &curHostInfo);
    } else {
	result = GetHostInfoByName(servAddrPtr, queryClass, queryType, host, 
			&curHostInfo, 0);
    }

    switch (result)
    {
    case SUCCESS:
    case NONAUTH:
	return &curHostInfo;
	break;
	
    case NO_INFO:
	fprintf(stderr, "*** No %s records available for %s\n", 
		p_type(queryType), host);
	break;
	
    case TIME_OUT:
	fprintf(stderr, "*** Request to %s timed-out\n", defaultServer);
	break;
	
    default:
	fprintf(stderr, "*** %s can't find %s\n", defaultServer, host);
    }
    
    return 0;
}
