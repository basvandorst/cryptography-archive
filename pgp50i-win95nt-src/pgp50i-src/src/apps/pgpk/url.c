/*
 * url.c - URL parsing code
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Original version copyright 1996 Brett A. Thomas (quark@baz.com).
 * Nonexclusive but otherwise unresistricted license granted to PGP, Inc.
 *
 * Modifications are copyright 1997 PGP, Inc.
 *
 * $Id: url.c,v 1.1.2.3 1997/06/07 09:49:15 mhw Exp $
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "pgpErr.h"

#include "url.h"

struct protocol
{
    char prefix[16];
    char protocol_name[16];
    int port;
};

struct protocol known_protocols[] =
{
    {"keys", "hkp", 11371},
    {"www", "http", 80},
    {"ftp", "ftp", 21},
    {"finger", "finger", 79},
    {"\0", "\0"}
};

static int find_port(const char *url, int *port);
static int find_protocol(const char *url, char *protocol);
static int find_server(const char *url, char *server);
static int find_file(const char *url, char *file);

/* XXX Fix Me!  This should really take char **s and do its own allocation.*/

int parse_url(char *protocol,
	      char *server,
	      char *file,
	      int *port,
	      const char *url)
{
    int rc = PGPERR_BADPARAM;
    char tmp_url[1024];
    int x = 0;

    if(protocol && server && file && port && url && *url) {
	*port = 0;
	*protocol = *server = *file = '\0';
	strncpy(tmp_url, url, sizeof(tmp_url) - 1);
	
	find_port(url, port);
	if(find_protocol(url, protocol))
	    if(find_server(url, server))
		if(find_file(url, file)) {
		    for(x = 0;
			known_protocols[x].protocol_name[0] && !*port;
			++x) {
			if(strcasecmp(protocol,
				      known_protocols[x].protocol_name)
			   == 0) {
			    *port = known_protocols[x].port;
			    rc = PGPERR_OK;
			}
		    }
		}
    }

    return(rc);
}
	
static int find_port(const char *url, int *port)
{
    int rc = 0;
    char *p, *p2;
    char digit_string[255];

    /*We can have as many as two colons in a URL, and as few as zero.  The
     *permutations are:
     *
     *http://server/file
     *http://server:port/file
     *www.server:port/file
     *
     *In all cases where we have a port, it
     *is only digits.  So, if we have  : and the first thing after it isn't
     *a digit, it's not the port.
     */

    p = (char *) url;

    while((p = strchr(p, ':')) && *p && !isdigit(*(p + 1)))
	++p;

    if(p && *p && isdigit(*(p + 1))) {
	++p;
	p2 = digit_string;
	while(*p &&
	      (p - url) < (unsigned short) sizeof(digit_string) - 1 &&
	      isdigit(*p)) {
	    *p2 = *p;
	    ++p2;
	    ++p;
	}
	*p2 = '\0';
	*port = atoi(digit_string);
	rc = 1;
    }

    return(rc);
}

static int find_protocol(const char *url, char *protocol)
{
    int rc = 0, x;
    char tmp_url[1024], *p;

    /*The protocol is semi-optional.  The possible permutations are:
     *
     *protocol://server/
     *www.server  (protocol == http)
     *ftp.server  (protocol == ftp)
     *keys.server (protocol == hkp)
     *
     *First, we look for the colon case.  If we don't succeed there, we
     *assume it's one of the second two.
     */

    strncpy(tmp_url, url, sizeof(tmp_url));

    if((p = strstr(tmp_url, "://"))) {
	*p = '\0';
	strcpy(protocol, tmp_url);
	rc = 1;
    }
    else
    {
	/*Ok, it's not protocol://server.  Either it's malformed, or it's
	 *www.something or ftp.something or keys.something
	 */

	if((p = strchr(tmp_url, '.'))) {
	    *p = '\0';
	    for(x = 0; known_protocols[x].prefix[0]; ++x)
		if(strcasecmp(tmp_url, known_protocols[x].prefix) == 0) {
		    strcpy(protocol, known_protocols[x].protocol_name);
		    rc = 1;
		}
	}
    }

    return(rc);
}

static int find_server(const char *url, char *server)
{
    int rc = 1;
    char *p;

    *server = '\0';

    /*The server is the only non-optional part of the URL.  Basic
     *permutations are:
     *
     *http://www.server.com[/file]
     *www.server.com[/file]   (Note the / is PART of file)
     *
     *The first case we look for is the initial.  If we don't find that,
     *we go with the second.
     *
     *Note that this actually can't fail as long as there is anything in
     *in the url at all.
     */

    if(*url) {
	if((p = strstr(url, "://"))) {
	    /*It's the first case.*/
	    p += 3;
	    strcpy(server, p);
	}
	else
	    strcpy(server, url);

	if((p = strchr(server, '/')))
	    *p = '\0';

	if((p = strchr(server, ':')))
	    *p = '\0';
    }
    else
	rc = 0;

    return(rc);
}

static int find_file(const char *url, char *file)
{
    int rc = 1;
    char *p;

    /*The most bare minimum file that we can return is "/", but that
     *actually can indicate no file was specified, since it's optional.
     *What we're looking for here is the first slash _after_
     *the protocol://, which is optional, anyway.  So, the first
     *thing we do is get past that (if it's there) and then find the next
     *slash (if _it_ is there).  Lastly, we need to strip off the
     *trailing port (if IT is there).  Basically, this one can't fail.
     */

    if((p = strstr(url, "://"))) {
	p += 3;
    }
    else
	p = (char *) url;

    if((p = strchr(p, '/'))) {
	/*Must be the beginning of the file*/
	strcpy(file, p);
	/*Now, let's see if there's an end to the file.*/
	if((p = strchr(file, ':')))
	    *p = '\0';
    }
    else {
	/*Hmmm.  No slashes implies no file...*/
	strcpy(file, "/");
    }

  return(rc);
}



