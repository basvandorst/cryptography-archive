/*
 * hostname2addr.c  --  Name to address translation.  For now, only consults
 *                      /etc/hosts, but DNS stuff will go here eventually.
 *
 * Copyright 1995 by Randall Atkinson, Bao Phan, and Dan McDonald
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#	@(#)COPYRIGHT	1.1a (NRL) 17 August 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

This software and documentation were developed at NRL by various
people.  Those developers have each copyrighted the portions that they
developed at NRL and have assigned All Rights for those portions to
NRL.  Outside the USA, NRL also has copyright on the software
developed at NRL. The affected files all contain specific copyright
notices and those notices must be retained in any derived work.

NRL LICENSE

NRL grants permission for redistribution and use in source and binary
forms, with or without modification, of the software and documentation
created at NRL provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:

	This product includes software developed at the Information
	Technology Division, US Naval Research Laboratory.

4. Neither the name of the NRL nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THE SOFTWARE PROVIDED BY NRL IS PROVIDED BY NRL AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL NRL OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation
are those of the authors and should not be interpreted as representing
official policies, either expressed or implied, of the US Naval
Research Laboratory (NRL).

----------------------------------------------------------------------*/

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet6/in6.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

extern struct hostent _hostent_buffer;
extern int _hostent_parse(char *line, int af, int addrsize);
extern int _hostent_linelen;
extern char *_hostent_linebuf;
extern char *_hostent_file;
extern FILE *_hostent_fh;

/* Using the /etc/hosts file, find the (first) hostent entry matching the
   hostname given (may be either primary or an alias) */
static struct hostent *_hostname2addr_hosts(name, af)
/* const */ char *name;
int af;
{
  int addrsize, foo;

  /* Only open the /etc/hosts file once and keep it open. Is this a 
     Good Thing? (performance vs. memory/resources) */
  if (_hostent_fh) 
    rewind(_hostent_fh);
  else {
    if ((_hostent_fh = fopen(_hostent_file, "r")) == NULL)
      return (struct hostent *)NULL;
    if ((_hostent_linebuf = malloc(_hostent_linelen)) == NULL)
      return (struct hostent *)(_hostent_fh = NULL);
  }

  switch (af) {
    case AF_INET:
      addrsize = sizeof(struct in_addr);
      break;
    case AF_INET6:
      addrsize = sizeof(struct in_addr6);
      break;
    default:
      return (struct hostent *)NULL;
  }

  /* Read the file, line by line, and feed the lines to the parser.
     If the hostname or an alias matches what we were given to look
     for, it's a winner. */
  do 
    if (fgets(_hostent_linebuf, _hostent_linelen, _hostent_fh) 
	== _hostent_linebuf)
      if (_hostent_parse(_hostent_linebuf, af, addrsize))
        if (!strcmp(_hostent_buffer.h_name, name)) 
          return &_hostent_buffer;
        else 
          for (foo = 0; _hostent_buffer.h_aliases[foo]; foo++)
            if (!strcmp(_hostent_buffer.h_aliases[foo], name)) 
              return &_hostent_buffer;
  while (!feof(_hostent_fh));

  return NULL;
}

/* Standard API entry point. We only really handle the Internet protocols,
   though I really don't see why it wouldn't work for the rest, since the
   parser uses ascii2addr to do the job. */
/* Eventually, we'll probably want to look at /etc/hosts first, then,
   if we don't find anything, try DNS... ? (or maybe a config file?) */
/* The addresses in the returned hostent structure need to point into
   a buffer such that we can transform an in_addr into an in_addr6.
   The current functions do this; look at them before implementing
   other resolvers. */
struct hostent *hostname2addr(name, af)
/* const */ char *name;
int af;
{
  struct hostent *rval = (struct hostent *)NULL;

  switch (af) {
    case AF_INET6:
      rval = _hostname2addr_hosts(name, AF_INET6);
      if (rval)
	break;
      /* do DNS here */

      /* If there's an IPv4 address available, return it as a IPv4-as-IPv6
	 mapped address */
      rval = hostname2addr(name, AF_INET); 
      if (rval) {
	int i;
	struct in_addr6 in6_v4map_prefix;
	IN6_ADDR_ASSIGN(in6_v4map_prefix, 0, 0, htonl(0xffff), 0);
	for (i = 0; rval->h_addr_list[i]; i++) {
	  rval->h_addr_list[i] -= (sizeof(struct in_addr6) - 
				  sizeof(struct in_addr));
	  memcpy(rval->h_addr_list[i], &in6_v4map_prefix, 
		 (sizeof(struct in_addr6) - sizeof(struct in_addr)));
	}
	rval->h_addrtype = AF_INET6;
	rval->h_length = sizeof(struct in_addr6);
      }
      break;
    case AF_INET:
      rval = _hostname2addr_hosts(name, AF_INET);
      if (rval)
	break;
      /* do DNS here */
  }

  return rval;
}
