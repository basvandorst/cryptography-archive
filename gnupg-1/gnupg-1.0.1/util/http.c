/* http.c  -  HTTP protocol handler
 *	Copyright (C) 1999 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifndef HAVE_DOSISH_SYSTEM

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "util.h"
#include "iobuf.h"
#include "i18n.h"

#include "http.h"

#define MAX_LINELEN 20000  /* max. length of a HTTP line */
#define VALID_URI_CHARS "abcdefghijklmnopqrstuvwxyz"   \
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
			"01234567890@"                 \
			"!\"#$%&'()*+,-./:;<=>?[\\]^_{|}~"

#ifndef EAGAIN
  #define EAGAIN  EWOULDBLOCK
#endif

static int parse_uri( PARSED_URI *ret_uri, const char *uri );
static void release_parsed_uri( PARSED_URI uri );
static int do_parse_uri( PARSED_URI uri, int only_local_part );
static int remove_escapes( byte *string );
static int insert_escapes( byte *buffer, const byte *string,
					 const byte *special );
static URI_TUPLE parse_tuple( byte *string );
static int send_request( HTTP_HD hd );
static byte *build_rel_path( PARSED_URI uri );
static int parse_response( HTTP_HD hd );

static int connect_server( const char *server, ushort port );
static int write_server( int sock, const char *data, size_t length );


int
http_open( HTTP_HD hd, HTTP_REQ_TYPE reqtype, const char *url,
					      unsigned int flags )
{
    int rc;

    if( !(reqtype == HTTP_REQ_GET || reqtype == HTTP_REQ_POST) )
	return G10ERR_INV_ARG;

    /* initialize the handle */
    memset( hd, 0, sizeof *hd );
    hd->sock = -1;
    hd->initialized = 1;
    hd->req_type = reqtype;
    hd->flags = flags;

    rc = parse_uri( &hd->uri, url );
    if( !rc ) {
	rc = send_request( hd );
	if( !rc ) {
	    hd->fp_write = iobuf_fdopen( hd->sock , "w" );
	    if( hd->fp_write )
		return 0;
	    rc = G10ERR_GENERAL;
	}
    }

    if( !hd->fp_read && !hd->fp_write && hd->sock != -1 )
	close( hd->sock );
    iobuf_close( hd->fp_read );
    iobuf_close( hd->fp_write);
    release_parsed_uri( hd->uri );
    hd->initialized = 0;

    return rc;
}


void
http_start_data( HTTP_HD hd )
{
    if( !hd->in_data ) {
	iobuf_put( hd->fp_write, '\n' );
	hd->in_data = 1;
    }
}


int
http_wait_response( HTTP_HD hd, unsigned int *ret_status )
{
    int rc;

    http_start_data( hd ); /* make sure that we are in the data */
    iobuf_flush( hd->fp_write );

    hd->sock = dup( hd->sock );
    if( hd->sock == -1 )
	return G10ERR_GENERAL;
    iobuf_close( hd->fp_write );
    hd->fp_write = NULL;
    shutdown( hd->sock, 1 );
    hd->in_data = 0;

    hd->fp_read = iobuf_fdopen( hd->sock , "r" );
    if( !hd->fp_read )
	return G10ERR_GENERAL;

    rc = parse_response( hd );
    if( !rc && ret_status )
	*ret_status = hd->status_code;

    return rc;
}


int
http_open_document( HTTP_HD hd, const char *document, unsigned int flags )
{
    int rc;

    rc = http_open( hd, HTTP_REQ_GET, document, flags );
    if( rc )
	return rc;

    rc = http_wait_response( hd, NULL );
    if( rc )
	http_close( hd );

    return rc;
}




void
http_close( HTTP_HD hd )
{
    if( !hd || !hd->initialized )
	return;
    if( !hd->fp_read && !hd->fp_write && hd->sock != -1 )
	close( hd->sock );
    iobuf_close( hd->fp_read );
    iobuf_close( hd->fp_write );
    release_parsed_uri( hd->uri );
    m_free( hd->buffer );
    hd->initialized = 0;
}



/****************
 * Parse an URI and put the result into the newly allocated ret_uri.
 * The caller must always use release_parsed_uri to releases the
 * resources (even on an error).
 */
static int
parse_uri( PARSED_URI *ret_uri, const char *uri )
{
   *ret_uri = m_alloc_clear( sizeof(**ret_uri) + strlen(uri) );
   strcpy( (*ret_uri)->buffer, uri );
   return do_parse_uri( *ret_uri, 0 );
}

static void
release_parsed_uri( PARSED_URI uri )
{
    if( uri )
    {
	URI_TUPLE r, r2;

	for( r = uri->query; r; r = r2 ) {
	    r2 = r->next;
	    m_free( r );
	}
	m_free( uri );
    }
}

static int
do_parse_uri( PARSED_URI uri, int only_local_part )
{
    URI_TUPLE *tail;
    char *p, *p2, *p3;
    int n;

    p = uri->buffer;
    n = strlen( uri->buffer );
    /* initialize all fields to an empty string or an empty list */
    uri->scheme = uri->host = uri->path = p + n;
    uri->port = 0;
    uri->params = uri->query = NULL;

    /* a quick validity check */
    if( strspn( p, VALID_URI_CHARS) != n )
	return G10ERR_BAD_URI; /* invalid characters found */

    if( !only_local_part ) {
	/* find the scheme */
	if( !(p2 = strchr( p, ':' ) ) || p2 == p )
	   return G10ERR_BAD_URI; /* No scheme */
	*p2++ = 0;
	strlwr( p );
	uri->scheme = p;
	if( !strcmp( uri->scheme, "http" ) )
	    ;
	else if( !strcmp( uri->scheme, "x-hkp" ) ) /* same as HTTP */
	    ;
	else
	    return G10ERR_INVALID_URI; /* Unsupported scheme */

	p = p2;

	/* find the hostname */
	if( *p != '/' )
	    return G10ERR_INVALID_URI; /* does not start with a slash */

	p++;
	if( *p == '/' ) {  /* there seems to be a hostname */
	    p++;
	    if( (p2 = strchr(p, '/')) )
		*p2++ = 0;
	    strlwr( p );
	    uri->host = p;
	    if( (p3=strchr( p, ':' )) ) {
		*p3++ = 0;
		uri->port = atoi( p3 );
	    }
	    else
	       uri->port = 80;
	    uri->host = p;
	    if( (n = remove_escapes( uri->host )) < 0 )
		return G10ERR_BAD_URI;
	    if( n != strlen( p ) )
		return G10ERR_BAD_URI; /* hostname with a Nul in it */
	    p = p2 ? p2 : NULL;
	}
    } /* end global URI part */

    /* parse the pathname part */
    if( !p || !*p ) /* we don't have a path */
	return 0; /* and this is okay */

    /* todo: here we have to check params */

    /* do we have a query part */
    if( (p2 = strchr( p, '?' )) )
	*p2++ = 0;

    uri->path = p;
    if( (n = remove_escapes( p )) < 0 )
	return G10ERR_BAD_URI;
    if( n != strlen( p ) )
	return G10ERR_BAD_URI; /* path with a Nul in it */
    p = p2 ? p2 : NULL;

    if( !p || !*p ) /* we don't have a query string */
	return 0;   /* okay */

    /* now parse the query string */
    tail = &uri->query;
    for(;;) {
	URI_TUPLE elem;

	if( (p2 = strchr( p, '&' )) )
	    *p2++ = 0;
	if( !(elem = parse_tuple( p )) )
	    return G10ERR_BAD_URI;
	*tail = elem;
	tail = &elem->next;

	if( !p2 )
	   break; /* ready */
	p = p2;
    }

    return 0;
}



/****************
 * Remove all %xx escapes; this is done inplace.
 * Returns: new length of the string.
 */
static int
remove_escapes( byte *string )
{
    int n = 0;
    byte *p, *s;

    for(p=s=string; *s ; s++ ) {
	if( *s == '%' ) {
	    if( s[1] && s[2] && isxdigit(s[1]) && isxdigit(s[2]) ) {
		s++;
		*p  = *s >= '0' && *s <= '9' ? *s - '0' :
		      *s >= 'A' && *s <= 'F' ? *s - 'A' + 10 : *s - 'a' + 10 ;
		*p <<= 4;
		s++;
		*p |= *s >= '0' && *s <= '9' ? *s - '0' :
		      *s >= 'A' && *s <= 'F' ? *s - 'A' + 10 : *s - 'a' + 10 ;
		p++;
		n++;
	    }
	    else {
		*p++ = *s++;
		if( *s )
		   *p++ = *s++;
		if( *s )
		   *p++ = *s++;
		if( *s )
		   *p = 0;
		return -1; /* bad URI */
	    }
	}
	else
	{
	    *p++ = *s;
	    n++;
	}
    }
    *p = 0; /* always keep a string terminator */
    return n;
}


static int
insert_escapes( byte *buffer, const byte *string, const byte *special )
{
    int n = 0;

    for( ; *string; string++ ) {
	if( strchr( VALID_URI_CHARS, *string )
	    && !strchr( special, *string ) )  {
	    if( buffer )
		*buffer++ = *string;
	    n++;
	}
	else {
	    if( buffer ) {
		sprintf( buffer, "%02X", *string );
		buffer += 3;
	    }
	    n += 3;
	}
    }
    return n;
}





static URI_TUPLE
parse_tuple( byte *string )
{
    byte *p = string;
    byte *p2;
    int n;
    URI_TUPLE tuple;

    if( (p2 = strchr( p, '=' )) )
	*p2++ = 0;
    if( (n = remove_escapes( p )) < 0 )
	return NULL; /* bad URI */
    if( n != strlen( p ) )
       return NULL; /* name with a Nul in it */
    tuple = m_alloc_clear( sizeof *tuple );
    tuple->name = p;
    if( !p2 )  {
	/* we have only the name, so we assume an empty value string */
	tuple->value = p + strlen(p);
	tuple->valuelen = 0;
    }
    else { /* name and value */
	if( (n = remove_escapes( p2 )) < 0 ) {
	    m_free( tuple );
	    return NULL; /* bad URI */
	}
	tuple->value = p2;
	tuple->valuelen = n;
    }
    return tuple;
}


/****************
 * Send a HTTP request to the server
 * Returns 0 if the request was successful
 */
static int
send_request( HTTP_HD hd )
{
    const byte *server;
    byte *request, *p;
    ushort port;
    int rc;
    const char *http_proxy = NULL;

    server = *hd->uri->host? hd->uri->host : "localhost";
    port   = hd->uri->port?  hd->uri->port : 80;

    if( (hd->flags & HTTP_FLAG_TRY_PROXY)
	&& (http_proxy = getenv( "http_proxy" )) ) {
	PARSED_URI uri;

	rc = parse_uri( &uri, http_proxy );
	if (rc) {
	    log_error("invalid $http_proxy: %s\n", g10_errstr(rc));
	    release_parsed_uri( uri );
	    return G10ERR_NETWORK;
	}
	hd->sock = connect_server( *uri->host? uri->host : "localhost",
				    uri->port? uri->port : 80 );
	release_parsed_uri( uri );
    }
    else
	hd->sock = connect_server( server, port );

    if( hd->sock == -1 )
	return G10ERR_NETWORK;

    p = build_rel_path( hd->uri );
    request = m_alloc( strlen(server) + strlen(p) + 50 );
    if( http_proxy ) {
	sprintf( request, "%s http://%s:%hu%s%s HTTP/1.0\r\n",
			  hd->req_type == HTTP_REQ_GET ? "GET" :
			  hd->req_type == HTTP_REQ_HEAD? "HEAD":
			  hd->req_type == HTTP_REQ_POST? "POST": "OOPS",
			  server, port,  *p == '/'? "":"/", p );
    }
    else {
	sprintf( request, "%s %s%s HTTP/1.0\r\n",
			  hd->req_type == HTTP_REQ_GET ? "GET" :
			  hd->req_type == HTTP_REQ_HEAD? "HEAD":
			  hd->req_type == HTTP_REQ_POST? "POST": "OOPS",
						  *p == '/'? "":"/", p );
    }
    m_free(p);

    rc = write_server( hd->sock, request, strlen(request) );
    m_free( request );

    return rc;
}




/****************
 * Build the relative path from the parsed URI.
 * Minimal implementation.
 */
static byte*
build_rel_path( PARSED_URI uri )
{
    URI_TUPLE r;
    byte *rel_path, *p;
    int n;

    /* count the needed space */
    n = insert_escapes( NULL, uri->path, "%;?&" );
    /* todo: build params */
    for( r=uri->query; r; r = r->next ) {
	n++; /* '?'/'&' */
	n += insert_escapes( NULL, r->name, "%;?&=" );
	n++; /* '='*/
	n += insert_escapes( NULL, r->value, "%;?&=" );
    }
    n++;

    /* now  allocate and copy */
    p = rel_path = m_alloc( n );
    n = insert_escapes( p, uri->path, "%;?&" );
    p += n;
    /* todo: add params */
    for( r=uri->query; r; r = r->next ) {
	*p++ = r == uri->query? '?':'&';
	n = insert_escapes( p, r->name, "%;?&=" );
	p += n;
	*p++ = '=';
	/* todo: use valuelen */
	n = insert_escapes( p, r->value, "%;?&=" );
	p += n;
    }
    *p = 0;
    return rel_path;
}



/***********************
 * Parse the response from a server.
 * Returns: errorcode and sets some fileds in the handle
 */
static int
parse_response( HTTP_HD hd )
{
    byte *line, *p, *p2;
    unsigned maxlen, len;

    /* Wait for the status line */
    do {
	maxlen = MAX_LINELEN;
	len = iobuf_read_line( hd->fp_read, &hd->buffer,
					    &hd->buffer_size, &maxlen );
	line = hd->buffer;
	if( !maxlen )
	    return -1; /* line has been truncated */
	if( !len )
	    return -1; /* eof */
    } while( !*line  );

    if( (p = strchr( line, '/')) )
	*p++ = 0;
    if( !p || strcmp( line, "HTTP" ) )
	return 0; /* assume http 0.9 */

    if( (p2 = strpbrk( p, " \t" ) ) ) {
	*p2++ = 0;
	p2 += strspn( p2, " \t" );
    }
    if( !p2 )
	return 0; /* assume http 0.9 */
    p = p2;
    /* todo: add HTTP version number check here */
    if( (p2 = strpbrk( p, " \t" ) ) )
	*p2++ = 0;
    if( !isdigit(p[0]) || !isdigit(p[1]) || !isdigit(p[2]) || p[3] ) {
	 /* malformed HTTP statuscode - assume HTTP 0.9 */
	hd->is_http_0_9 = 1;
	hd->status_code = 200;
	return 0;
    }
    hd->status_code = atoi( p );

    /* skip all the header lines and wait for the empty line */
    do {
	maxlen = MAX_LINELEN;
	len = iobuf_read_line( hd->fp_read, &hd->buffer,
			       &hd->buffer_size, &maxlen );
	line = hd->buffer;
	/* we ignore truncated lines */
	if( !len )
	    return -1; /* eof */
	/* time lineendings */
	if( (*line == '\r' && line[1] == '\n') || *line == '\n' )
	    *line = 0;
    } while( len && *line  );

    return 0;
}

#if 0
static int
start_server()
{
    struct sockaddr_in mya;
    struct sockaddr_in peer;
    int fd, client;
    fd_set rfds;
    int addrlen;
    int i;

    if( (fd=socket(AF_INET,SOCK_STREAM, 0)) == -1 ) {
	log_error("socket() failed: %s\n", strerror(errno));
	return -1;
    }
    i = 1;
    if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (byte*)&i, sizeof(i) ) )
	log_info("setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno) );

    mya.sin_family=AF_INET;
    memset(&mya.sin_addr, 0, sizeof(mya.sin_addr));
    mya.sin_port=htons(11371);

    if( bind( fd, (struct sockaddr *)&mya, sizeof(mya)) ) {
	log_error("bind to port 11371 failed: %s\n", strerror(errno) );
	close( fd );
	return -1;
    }

    if( listen( fd, 5 ) ) {
	log_error("listen failed: %s\n", strerror(errno) );
	close( fd );
	return -1;
    }

    for(;;) {
	FD_ZERO(&rfds);
	FD_SET( fd, &rfds );

	if( select( fd+1, &rfds, NULL, NULL, NULL) <= 0 )
	    continue; /* ignore any errors */

	if( !FD_ISSET( fd, &rfds ) )
	    continue;

	addrlen = sizeof peer;
	client = accept( fd, (struct sockaddr *)&peer, &addrlen);
	if( client == -1 )
	    continue; /* oops */

	log_info("connect from %s\n", inet_ntoa( peer.sin_addr ) );

	fflush(stdout);
	fflush(stderr);
	if( !fork() ) {
	    int c;
	    FILE *fp;

	    fp = fdopen( client , "r" );
	    while( (c=getc(fp)) != EOF )
		putchar(c);
	    fclose(fp);
	    exit(0);
	}
	close( client );
    }


    return 0;
}
#endif



static int
connect_server( const char *server, ushort port )
{
    struct sockaddr_in addr;
    struct hostent *host;
    int sd;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    host = gethostbyname((char*)server);
    if( !host )
	 return -1;

    addr.sin_addr = *(struct in_addr*)host->h_addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if( sd == -1 )
	return -1;

    if( connect( sd, (struct sockaddr *)&addr, sizeof addr) == -1 ) {
	close(sd);
	return -1;
    }

    return sd;
}


static int
write_server( int sock, const char *data, size_t length )
{
    int nleft, nwritten;

    nleft = length;
    while( nleft > 0 ) {
	nwritten = write( sock, data, nleft );
	if( nwritten == -1 ) {
	    if( errno == EINTR )
		continue;
	    if( errno == EAGAIN ) {
		struct timeval tv;

		tv.tv_sec =  0;
		tv.tv_usec = 50000;
		select(0, NULL, NULL, NULL, &tv);
		continue;
	    }
	    log_info("write failed: %s\n", strerror(errno));
	    return G10ERR_NETWORK;
	}
	nleft -=nwritten;
	data += nwritten;
    }

    return 0;
}

#endif /* HAVE_DOSISH_SYSTEM */

/**** Test code ****/
#ifdef TEST

int
main(int argc, char **argv)
{
    int rc;
    PARSED_URI uri;
    URI_TUPLE r;
    struct http_context hd;
    int c;

    log_set_name("http-test");
    if( argc == 1 ) {
	start_server();
	return 0;
    }

    if( argc != 2 ) {
	fprintf(stderr,"usage: http-test uri\n");
	return 1;
    }
    argc--; argv++;

    rc = parse_uri( &uri, *argv );
    if( rc ) {
	log_error("`%s': %s\n", *argv, g10_errstr(rc));
	release_parsed_uri( uri );
	return 1;
    }

    printf("Scheme: %s\n", uri->scheme );
    printf("Host  : %s\n", uri->host );
    printf("Port  : %u\n", uri->port );
    printf("Path  : %s\n", uri->path );
    for( r=uri->params; r; r = r->next ) {
	printf("Params: %s=%s", r->name, r->value );
	if( strlen( r->value ) != r->valuelen )
	    printf(" [real length=%d]", (int)r->valuelen );
	putchar('\n');
    }
    for( r=uri->query; r; r = r->next ) {
	printf("Query : %s=%s", r->name, r->value );
	if( strlen( r->value ) != r->valuelen )
	    printf(" [real length=%d]", (int)r->valuelen );
	putchar('\n');
    }
    release_parsed_uri( uri ); uri = NULL;

    rc = http_open_document( &hd, *argv, 0 );
    if( rc ) {
	log_error("can't get `%s': %s\n", *argv, g10_errstr(rc));
	return 1;
    }
    log_info("open_http_document succeeded; status=%u\n", hd.status_code );
    while( (c=iobuf_get( hd.fp_read)) != -1 )
	putchar(c);
    http_close( &hd );
    return 0;
}
#endif /*TEST*/
