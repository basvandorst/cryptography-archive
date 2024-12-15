/*
 * support.h  --  Definitions for the IPv6 support library
 *                This may belong in netdb.h.
 *
 * Copyright 1995 by Dan McDonald, Bao Phan, and Randall Atkinson,
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#       @(#)COPYRIGHT   1.1a (NRL) 17 August 1995

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


#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <netdb.h>

#define ADDR2ASCII_BUFSZ_INET     16
#define ADDR2ASCII_BUFSZ_INET6    46
#define ADDR2ASCII_BUFSZ_MAX      ADDR2ASCII_BUFSZ_INET6

struct conninfo {
  int ci_flags;              /* see below */
  int ci_af;                 /* address family of record */
  int ci_socktype;           /* socket() type */
  int ci_proto;              /* socket() protocol */
  int ci_namelen;            /* length of data pointed to by ci_name (bytes) */
  char *ci_canon;            /* "canonical name" of the host, iff requested */
  struct sockaddr *ci_name;  /* address info for bind()/connect() */
  struct conninfo *ci_next;  /* pointer to the next one */
};

/* ci_flags values */
#define CI_PASSIVE    1
#define CI_CANONICAL  2

int ascii2addr __P((int, /* const */ char *, void *));
char *addr2ascii __P((int, /* const */ void *, int, char *));
struct hostent *hostname2addr __P((/* const */ char *, int));
struct hostent *addr2hostname __P((/* const */ void *, int, int));

int getconninfo __P((char *, char *, struct conninfo *, struct conninfo **));
void freeconninfo __P((struct conninfo *));
int getinfobysockaddr __P((struct sockaddr *, int, int, char *, char *));

#endif /* _SUPPORT_H */
