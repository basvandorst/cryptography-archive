/*
 * _hostent_com.c  -- Common routines between the /etc/hosts lookup functions
 *
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

#include "support.h"

/* Arbitrary limits */
#define MAX_ALIAS 4
#define MAX_ADDR 8

/* If this isn't big enough to store any address you're going to parse using
this function, you're gonna lose. */
#define MAX_ADDR_SZ  sizeof(struct in_addr6)

/* These are the buffers that become the returned static buffer */
static char h_addr_buf[sizeof(struct in_addr6) * MAX_ADDR];
static char *h_addr_list[MAX_ADDR];
static char *h_aliases[MAX_ALIAS];
struct hostent _hostent_buffer;

int _hostent_linelen = 128;
char *_hostent_linebuf;
FILE *_hostent_fh;
char *_hostent_file = "/etc/hosts";

/* Take a line from /etc/hosts, parse it, and put the results in the hostent
   buffer. Assumes that anything not a valid address is a name. Handles 
   comments. */
int _hostent_parse(char *line, int af, int addrsize)
{
  char *head, *tail;
  int c, naddr, nname;

  head = tail = line;
  c = naddr = nname = 0;

  _hostent_buffer.h_aliases = h_aliases;
  _hostent_buffer.h_addr_list = h_addr_list;
  _hostent_buffer.h_length = addrsize;
  _hostent_buffer.h_addrtype = af;

  while (1) {
    switch (*tail) {
      case '#':
      case '\0':
      case '\n':
	c++;
      case ' ':
      case '\t':
	*tail = 0;
	if (tail != head) {
          void *addr = (void *)h_addr_buf + (MAX_ADDR_SZ * naddr) + 
			(MAX_ADDR_SZ - addrsize);
	  memset(&(h_addr_buf[MAX_ADDR_SZ * naddr]), 0, MAX_ADDR_SZ);
	  if (ascii2addr(af, head, addr) == addrsize) {
	    h_addr_list[naddr++] = addr;
	  } else {
	    if (nname)
	      h_aliases[nname - 1] = head;
	    else
	      _hostent_buffer.h_name = head;
	    nname++;
	  }
	}
	if (c) {
	  memset(&(h_addr_buf[MAX_ADDR_SZ * naddr]), 0, MAX_ADDR_SZ);
	  h_addr_list[naddr] = NULL;
	  h_aliases[nname - 1] = NULL;
	  return (naddr && nname);
	}
	head = tail + 1;;
    }
    tail++;
  }
}
