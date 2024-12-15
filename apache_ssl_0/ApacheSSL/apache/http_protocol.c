
/* ====================================================================
 * Copyright (c) 1995 The Apache Group.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * IT'S CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */


/*
 * http_protocol.c --- routines which directly communicate with the
 * client.
 *
 * Code originally by Rob McCool; much redone by rst.
 */

#define CORE_PRIVATE
#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_main.h"
#include "http_log.h"		/* For errors detected in basic auth
				 * common support code...
				 */

#include <stdarg.h>

/* Handling of conditional gets (if-modified-since); Roy owes Rob beer. 
 * This would be considerably easier if strptime or timegm were portable...
 */

static char *months[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

int find_month(char *mon) {
    register int x;

    for(x=0;x<12;x++)
        if(!strcmp(months[x],mon))
            return x;
    return -1;
}

int later_than(struct tm *lms, char *ims) {
    char *ip;
    char mname[MAX_STRING_LEN];
    int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0, x;

    /* Whatever format we're looking at, it will start with weekday. */
    /* Skip to first space. */
    if(!(ip = strchr(ims,' ')))
        return 0;
    else
        while(isspace(*ip))
            ++ip;

    if(isalpha(*ip)) {
        /* ctime */
        sscanf(ip,"%s %d %d:%d:%d %d",mname,&day,&hour,&min,&sec,&year);
    }
    else if(ip[2] == '-') {
        /* RFC 850 (normal HTTP) */
        char t[MAX_STRING_LEN];
        sscanf(ip,"%s %d:%d:%d",t,&hour,&min,&sec);
        t[2] = '\0';
        day = atoi(t);
        t[6] = '\0';
        strcpy(mname,&t[3]);
        x = atoi(&t[7]);
        /* Prevent wraparound from ambiguity */
        if(x < 70)
            x += 100;
        year = 1900 + x;
    }
    else {
        /* RFC 822 */
        sscanf(ip,"%d %s %d %d:%d:%d",&day,mname,&year,&hour,&min,&sec);
    }
    month = find_month(mname);

    if((x = (1900+lms->tm_year) - year))
        return x < 0;
    if((x = lms->tm_mon - month))
        return x < 0;
    if((x = lms->tm_mday - day))
        return x < 0;
    if((x = lms->tm_hour - hour))
        return x < 0;
    if((x = lms->tm_min - min))
        return x < 0;
    if((x = lms->tm_sec - sec))
        return x < 0;

    return 1;
}


int set_content_length (request_rec *r, long clength)
{
    char ts[MAX_STRING_LEN];
    
    sprintf (ts, "%ld", (long)r->finfo.st_size);
    table_set (r->headers_out, "Content-length", pstrdup (r->pool, ts));
    return 0;
}

int set_last_modified(request_rec *r, time_t mtime)
{
    struct tm *tms;
    char ts[MAX_STRING_LEN];
    char *if_modified_since = table_get (r->headers_in, "If-modified-since");

    /* Cacheing proxies use the absence of a Last-modified header
     * to indicate that a document is dynamic and shouldn't be cached.
     * For the moment, we enforce that here, though it would probably
     * work just as well to generate an Expires: header in send_http_header.
     *
     * However, even in that case, if no_cache is set, we would *not*
     * want to send USE_LOCAL_COPY, since the client isn't *supposed*
     * to have it cached.
     */
    
    if (r->no_cache) return OK;
    
    tms = gmtime(&mtime);
    strftime(ts,MAX_STRING_LEN,HTTP_TIME_FORMAT,tms);
    table_set (r->headers_out, "Last-modified", pstrdup (r->pool, ts));

    /* Check for conditional GETs --- note that we only want this check
     * to succeed if the GET was successful; ErrorDocuments *always* get sent.
     */
    
    if (r->status == 200 &&
	if_modified_since && later_than(tms, if_modified_since))
      
        return USE_LOCAL_COPY;
    else
        return OK;
}

/*
 * Finally, real protocol stuff.
 */

static char *getline (char *s, int n, conn_rec *conn)
{
    char *retval = os_conn_gets (s, n, conn);
    char *cp;

    if (retval == NULL) return NULL;

    cp = s + strlen(s) - 1;

    while (cp >= s && (*cp == '\015' || *cp == '\012'))
        *cp-- = '\0';

    return s;
}

void parse_uri (request_rec *r, char *uri)
{
    /* If we ever want to do byte-ranges a la Netscape & Franks,
     * this is the place to parse them; with proper support in
     * rprintf and rputc, and the sub-request setup and finalizers
     * here, it'll all just work, even for vile cases like
     * inclusion of byte-ranges of the output of CGI scripts, with
     * the client requesting only a byte-range of *that*!
     *
     * But for now...
     */

    r->uri = getword (r->pool, &uri, '?');
    if (*uri) r->args= uri;
    else r->args = NULL;
}

int read_request_line (request_rec *r)
{
    char l[HUGE_STRING_LEN];
    char *ll = l, *uri;
    conn_rec *conn = r->connection;
    
    l[0] = '\0';
    if(!getline(l, HUGE_STRING_LEN, conn))
        return 0;
    if(!l[0]) 
        return 0;

    r->the_request = pstrdup (r->pool, l);
    r->method = getword(r->pool, &ll,' ');
    uri = getword(r->pool, &ll,' ');
    parse_uri (r, uri);
    
    r->assbackwards = (ll[0] == '\0');
    r->protocol = ll[0] ? pstrdup (r->pool, ll) : "HTTP/0.9";
    return 1;
}

void get_mime_headers(request_rec *r)
{
    char w[MAX_STRING_LEN];
    char *t;
    conn_rec *c = r->connection;

    while(getline(w, MAX_STRING_LEN-1, c)) {
        if(!w[0]) 
            return;
        if(!(t = strchr(w,':')))
            continue;
        *t++ = '\0';
        while(isspace(*t)) ++t;

	table_merge (r->headers_in, w, t);
    }
}

request_rec *read_request (conn_rec *conn)
{
    request_rec *r = (request_rec *)pcalloc (conn->pool, sizeof(request_rec));
  
    r->connection = conn;
    r->server = conn->server;
    r->pool = conn->pool;

    r->headers_in = make_table (r->pool, 50);
    r->subprocess_env = make_table (r->pool, 50);
    r->headers_out = make_table (r->pool, 5);
    r->err_headers_out = make_table (r->pool, 5);
    r->notes = make_table (r->pool, 5);

    r->request_config = create_request_config (r->pool);
    r->per_dir_config = r->server->lookup_defaults; /* For now. */

    r->bytes_sent = -1;
    
    r->status = 200;		/* Until further notice.
				 * Only changed by die(), or (bletch!)
				 * scan_script_header...
				 */

    /* Get the request... */
    
    hard_timeout ("read", r);
    if (!read_request_line (r)) return NULL;
    if (!r->assbackwards) get_mime_headers(r);
    kill_timeout (r);
    
    if(!strcmp(r->method, "HEAD")) {
        r->header_only=1;
	r->method_number = M_GET;
    }
    else if(!strcmp(r->method, "GET")) 
	r->method_number = M_GET;
    else if(!strcmp(r->method,"POST")) 
        r->method_number = M_POST;
    else if(!strcmp(r->method,"PUT")) 
        r->method_number = M_PUT;
    else if(!strcmp(r->method,"DELETE")) 
        r->method_number = M_DELETE;
    else 
        r->method_number = M_INVALID; /* Will eventually croak. */

    return r;
}

/*
 * A couple of other functions which initialize some of the fields of
 * a request structure, as appropriate for adjuncts of one kind or another
 * to a request in progress.  Best here, rather than elsewhere, since
 * *someone* has to set the protocol-specific fields...
 */

void set_sub_req_protocol (request_rec *rnew, request_rec *r)
{
    rnew->assbackwards = 1;	/* Don't send headers from this. */
    rnew->no_cache = 1;		/* Don't try to send USE_LOCAL_COPY for a
				 * fragment.
				 */
    rnew->method = "GET"; rnew->method_number = M_GET;
    rnew->protocol = "INCLUDED";

    rnew->status = 200;

    rnew->headers_in = r->headers_in;
    rnew->subprocess_env = copy_table (rnew->pool, r->subprocess_env);
    rnew->headers_out = make_table (rnew->pool, 5);
    rnew->err_headers_out = make_table (rnew->pool, 5);
    rnew->notes = make_table (rnew->pool, 5);
    
    rnew->main = r;
}

void finalize_sub_req_protocol (request_rec *sub)
{
    sub->main->bytes_sent += sub->bytes_sent;
} 

/* Support for the Basic authentication protocol.  
 */

void note_basic_auth_failure(request_rec *r)
{
    table_set (r->err_headers_out, "WWW-Authenticate",
	       pstrcat(r->pool, "Basic realm=\"", auth_name(r), "\"", NULL));
}


int get_basic_auth_pw (request_rec *r, char **pw)
{
    char *auth_line = table_get (r->headers_in, "Authorization");
    char *t;
    
    if(!(t = auth_type(r)) || strcasecmp(t, "Basic"))
        return DECLINED;

    if (!auth_name (r)) {
        log_reason ("need AuthName", r->uri, r);
	return SERVER_ERROR;
    }
    
    if(!auth_line) {
        note_basic_auth_failure (r);
	return AUTH_REQUIRED;
    }

    if (strcmp(getword (r->pool, &auth_line, ' '), "Basic")) {
        /* Client tried to authenticate using wrong auth scheme */
        log_reason ("client used wrong authentication scheme", r->uri, r);
        note_basic_auth_failure (r);
	return AUTH_REQUIRED;
    }

    t = uudecode (r->pool, auth_line);
    r->connection->user = getword (r->pool, &t, ':');
    r->connection->auth_type = "Basic";

    *pw = t;

    return OK;
}

#define RESPONSE_CODE_LIST " 200 302 304 400 401 403 404 500 503 501 "

/* New Apache routine to map error responses into array indicies 
 *  e.g.  400 -> 0,  500 -> 1,  502 -> 2 ...                     
 * the indicies have no significance
 */

char *status_lines[] = {
   "200 OK",
   "302 Found",
   "304 Not Modified",
   "400 Bad Request",
   "401 Unauthorized",
   "403 Forbidden",
   "404 Not found",
   "500 Server error",
   "503 Out of resources",
   "501 Not Implemented",
}; 

char *response_titles[] = {
   "200 OK",			/* Never actually sent, barring die(200,...) */
   "Document moved",		/* 302 Redirect */
   "304 Not Modified",		/* Never sent... 304 MUST be header only */
   "Bad Request",
   "Authorization Required",
   "Forbidden",
   "File Not found",
   "Server Error",
   "Out of resources",
   "Method not implemented"
};

int index_of_response(int err_no) { 
   char *cptr, err_string[10];
   static char *response_codes = RESPONSE_CODE_LIST;
   int index_number;
   
   sprintf(err_string,"%3d",err_no);
   
   cptr = response_codes;
   cptr++;
   index_number = 0;
   while (*cptr && strncmp(cptr, err_string, 3)) { 
      cptr += 4;
      index_number++;
   }
   if (!*cptr) return -1;
   return index_number;
}


void basic_http_header (request_rec *r)
{
    conn_rec *conn=r->connection;
    
    if (r->assbackwards) return;
    
    if (!r->status_line)
        r->status_line = status_lines[index_of_response(r->status)];
    if(!r->nostatus_line)
      rprintf(r,"%s %s\015\012", SERVER_PROTOCOL, r->status_line);
    rprintf(r,"Date: %s\015\012", gm_timestr_822 (r->pool, time(NULL)));
    rprintf(r,"Server: %s\015\012",SERVER_VERSION);
}

char *nuke_mime_parms (pool *p, char *content_type)
{
    /* How marvelous.  Arena doesn't *accept* "text/html; level=3"
     * as a MIME type, so we have to strip off the parms.
     */

#ifndef ARENA_BUG_WORKAROUND
    return content_type;
#else

    char *cp = strchr(content_type, ';');

    if (cp) {
        content_type = pstrdup (p, content_type);
	cp = strchr (content_type, ';');
	
        while (cp > content_type && isspace (cp[-1]))
	    --cp;
	*cp = '\0';
    }

    return content_type;
#endif
}

void send_http_header(request_rec *r)
{
    conn_rec *c = r->connection;

    array_header *hdrs_arr = table_elts (r->headers_out);
    table_entry *hdrs = (table_entry *)hdrs_arr->elts;

    array_header *err_hdrs_arr = table_elts (r->err_headers_out);
    table_entry *err_hdrs = (table_entry *)err_hdrs_arr->elts;
    int i;
    
    core_dir_config *dir_conf =
      (core_dir_config *)get_module_config(r->per_dir_config, &core_module);
    char *default_type = dir_conf->default_type;
  
    if (r->assbackwards) {
	r->bytes_sent = 0;
	return;
    }
    
    basic_http_header (r);
    
    if (r->content_type)
        rprintf(r, "Content-type: %s\015\012",
		 nuke_mime_parms (r->pool, r->content_type));
    else
        rprintf(r, "Content-type: %s\015\012", default_type);
    
    if (r->content_encoding)
        rprintf(r, "Content-encoding: %s\015\012", r->content_encoding);
    
    if (r->content_language)
        rprintf(r, "Content-language: %s\015\012", r->content_language);
    
    for (i = 0; i < hdrs_arr->nelts; ++i) {
        if (!hdrs[i].key) continue;
	rprintf(r, "%s: %s\015\012", hdrs[i].key, hdrs[i].val);
    }

    for (i = 0; i < err_hdrs_arr->nelts; ++i) {
        if (!err_hdrs[i].key) continue;
	rprintf(r, "%s: %s\015\012", err_hdrs[i].key, err_hdrs[i].val);
    }

    os_conn_puts("\015\012",c);

    r->bytes_sent = 0;		/* Whatever follows is real body stuff... */
}

long read_client_block (request_rec *r, char *buffer, int bufsiz)
{
    return os_conn_read (buffer, sizeof(char), bufsiz, r->connection);
}

long send_fd(FILE *f, request_rec *r)
{
    char buf[IOBUFSIZE];
    long total_bytes_sent;
    register int n,o,w;
    conn_rec *c = r->connection;
    
    total_bytes_sent = 0;
    while (!r->connection->aborted) {
        while ((n= fread(buf, sizeof(char), IOBUFSIZE, f)) < 1
	       && ferror(f) && errno == EINTR)
	    continue;
	
	if (n < 1) {
            break;
        }
        o=0;
        if (r->bytes_sent != -1) r->bytes_sent += n;
	total_bytes_sent += n;
	
        while(n && !r->connection->aborted) {
            w=os_conn_write(&buf[o],sizeof(char),n,c);
            n-=w;
            o+=w;
        }
    }
    os_conn_flush(c);
    
    return total_bytes_sent;
}

int rputc (int c, request_rec *r)
{
    if (r->connection->aborted) return EOF;
    os_conn_putc (c, r->connection);
    ++r->bytes_sent;
    return c;
}

long rprintf (request_rec *r, char *fmt, ...)
{
    va_list args;
    int retval;
    
    if (r->connection->aborted) return EOF;
    
    va_start (args, fmt);
    retval = os_conn_vprintf (r->connection, fmt, args);
    va_end (args);

    r->bytes_sent += retval;
    return retval;
}

void send_error_response (request_rec *r, int recursive_error)
{
    conn_rec *c = r->connection;
    char *custom_response;
    int status = r->status;
    int idx = index_of_response (status);
    char *location = table_get (r->headers_out, "Location");

    if (!r->assbackwards) {
	int i;
	table *err_hdrs_arr = r->err_headers_out;
	table_entry *err_hdrs = (table_entry *)err_hdrs_arr->elts;
  
        basic_http_header (r);
	
	/* For conditional get's which didn't send anything, *don't*
	 * send a bogus content-type, or any body --- but must still
	 * terminate header.
	 */
	
	if (status == USE_LOCAL_COPY) {
	    rprintf(r, "\015\012");
	    return;
	}
	
	if (status == REDIRECT)
	    rprintf(r, "Location: %s\015\012", location);
	
	for (i = 0; i < err_hdrs_arr->nelts; ++i) {
	    if (!err_hdrs[i].key) continue;
	    rprintf(r, "%s: %s\015\012",
		     err_hdrs[i].key, err_hdrs[i].val);
	}

	rprintf(r, "Content-type: text/html\015\012\015\012");
    }

    if (r->header_only) return;
    
    if ((custom_response = response_code_string (r, idx)))
        os_conn_puts (custom_response, c);
    else {
	char *title = response_titles[idx];
	
        rprintf(r,"<HEAD><TITLE>%s</TITLE></HEAD>%c",title,LF);
	rprintf(r,"<BODY><H1>%s</H1>%c",title,LF);
	
        switch (r->status) {
	case REDIRECT:
	  rprintf (r,"The document has moved <A HREF=\"%s\">here</A>.<P>\n",
		   escape_html(r->pool, location));
	    break;
	case AUTH_REQUIRED:
	    rprintf(r, "This server could not verify that you%c", LF);
	    rprintf(r, "are authorized to access the document you%c", LF);
	    rprintf(r, "requested.  Either you supplied the wrong%c", LF);
	    rprintf(r, "credentials (e.g., bad password), or your%c", LF);
	    rprintf(r, "browser doesn't understand how to supply%c", LF);
	    rprintf(r, "the credentials required.<P>%c", LF);
	    break;
	case BAD_REQUEST:
	    rprintf(r, "Your browser sent a query that%c", LF);
	    rprintf(r, "this server could not understand.<P>%c", LF);
	    break;
	case FORBIDDEN:
	    rprintf(r, "You don't have permission to access %s%c",
		     escape_html(r->pool, r->uri), LF);
	    rprintf(r, "on this server.<P>%c", LF);
	    break;
	case NOT_FOUND:
	    rprintf(r,
		     "The requested URL %s was not found on this server.<P>%c",
		     escape_html(r->pool, r->uri), LF);
	    break;
	case SERVER_ERROR:
	    rprintf(r,"The server encountered an internal error or%c",LF);
	    rprintf(r,"misconfiguration and was unable to complete%c",LF);
	    rprintf(r,"your request.<P>%c",LF);
	    rprintf(r,"Please contact the server administrator,%c",LF);
	    rprintf(r," %s ", escape_html(r->pool, r->server->server_admin));
	    rprintf(r,"and inform them of the time the error occurred,%c",LF);
	    rprintf(r,"and anything you might have done that may have%c",LF);
	    rprintf(r,"caused the error.<P>%c",LF);
	    break;
	case NOT_IMPLEMENTED:
	    rprintf(r,"%s to %s not supported.<P>%c",
		    escape_html(r->pool, r->method),
		    escape_html(r->pool, r->uri), LF);
	    break;
	}

        if (recursive_error) {
	    rprintf(r, "Additionally, an error of type %d was encountered%c",
		     recursive_error, LF);
	    rprintf(r, "while trying to use an ErrorDocument to%c", LF);
	    rprintf(r, "handle the request.%c", LF);
	}
	rprintf(r, "</BODY>%c", LF);
    }
        
}

/* Can't do this with SSL - the output would be encrypted! */
/* Finally, this... it's here to support nph- scripts
 * Now what ever are we going to do about them when HTTP-NG packetization
 * comes along?
 */

void client_to_stdout (conn_rec *c)
{
#ifdef APACHE_SSL
  log_error("client_to_stdout not supported with SSL",c->server);
  exit(1);
#else
  os_conn_flush (c);
  dup2 (fileno (c->client.out), STDOUT_FILENO);
#endif
}
