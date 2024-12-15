
/*-
 * Copyright (c) 1995 The Apache Group. All rights reserved.
 * 
 *
 * Apache httpd license
 * ====================
 * 
 *
 * This is the license for the Apache Server. It covers all the
 * files which come in this distribution, and should never be removed.
 * 
 * The "Apache Group" has based this server, called "Apache", on
 * public domain code distributed under the name "NCSA httpd 1.3".
 * 
 * NCSA httpd 1.3 was placed in the public domain by the National Center 
 * for Supercomputing Applications at the University of Illinois 
 * at Urbana-Champaign.
 * 
 * As requested by NCSA we acknowledge,
 * 
 *  "Portions developed at the National Center for Supercomputing
 *   Applications at the University of Illinois at Urbana-Champaign."
 *
 * Copyright on the sections of code added by the "Apache Group" belong
 * to the "Apache Group" and/or the original authors. The "Apache Group" and
 * authors hereby grant permission for their code, along with the
 * public domain NCSA code, to be distributed under the "Apache" name.
 * 
 * Reuse of "Apache Group" code outside of the Apache distribution should
 * be acknowledged with the following quoted text, to be included with any new
 * work;
 * 
 * "Portions developed by the "Apache Group", taken with permission 
 *  from the Apache Server   http://www.apache.org/apache/   "
 *
 *
 * Permission is hereby granted to anyone to redistribute Apache under
 * the "Apache" name. We do not grant permission for the resale of Apache, but
 * we do grant permission for vendors to bundle Apache free with other software,
 * or to charge a reasonable price for redistribution, provided it is made
 * clear that Apache is free. Permission is also granted for vendors to 
 * sell support for Apache. We explicitly forbid the redistribution of 
 * Apache under any other name.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 */

/*
 * Prototypes for routines which either talk directly back to the user,
 * or control the ones that eventually do.
 */

/* Read a request and fill in the fields. */

request_rec *read_request (conn_rec *c);     

/* Send header for http response */

void send_http_header (request_rec *l);     
     
/* Send error back to client... last arg indicates error status in case
 * we get an error in the process of trying to deal with an ErrorDocument
 * to handle some other error.  In that case, we print the default report
 * for the first thing that went wrong, and more briefly report on the
 * problem with the ErrorDocument.
 */

void send_error_response (request_rec *r, int recursive_error);
     
/* Set last modified header line from the lastmod date of the associated file.
 * Also, set content length.
 *
 * May return an error status, typically USE_LOCAL_COPY (that when the
 * permit_cache argument is set to one).
 */

int set_content_length (request_rec *r, long length);
int set_last_modified (request_rec *r, time_t mtime);

void add_env_var (array_header *env, char *header_name, char *val);

/* Other ways to send stuff at the client.  All of these keep track
 * of bytes_sent automatically.  This indirection is intended to make
 * it a little more painless to slide things like HTTP-NG packetization
 * underneath the main body of the code later.  In the meantime, it lets
 * us centralize a bit of accounting (bytes_sent).
 *
 * These also return the number of bytes written by the call.
 * They should only be called with a timeout registered, for obvious reaasons.
 * (Ditto the send_header stuff).
 */

long send_fd(FILE *f, request_rec *r);
     
/* Hmmm... could macrofy these for now, and maybe forever, though the
 * definitions of the macros would get a whole lot hairier.
 */
     
long rprintf (request_rec *r, char *s, ...);     
int rputc (int c, request_rec *r);     
     
/*
 * Index used in custom_responses array for a specific error code
 * (only use outside protocol.c is in getting them configured).
 */

int index_of_response (int status);

/* Reading a block of data from the client connection (e.g., POST arg) */
     
long read_client_block (request_rec *r, char *buffer, int bufsiz);
     
/* Finally, this charming little number is here to encapsulate the
 * degree to which nph- scripts completely escape from any discipline
 * the protocol code might care to impose (this as opposed to other
 * scripts, which *partially* escape to the extent that they may try
 * to explicitly set the status line).
 */

void client_to_stdout (conn_rec *c); 


/* Support for the Basic authentication protocol.  Note that there's
 * nothing that prevents these from being in mod_auth.c, except that other
 * modules which wanted to provide their own variants on finding users and
 * passwords for Basic auth (a fairly common request) would then require
 * mod_auth to be loaded or they wouldn't work.
 *
 * get_basic_auth_pw returns 0 (OK) if it set the 'pw' argument (and assured
 * a correct value in r->connection->user); otherwise it returns an error
 * code, either SERVER_ERROR if things are really confused, AUTH_REQUIRED
 * if no authentication at all seemed to be in use, or DECLINED if there
 * was authentication but it wasn't Basic (in which case, the caller should
 * presumably decline as well).
 *
 * note_basic_auth_failure arranges for the right stuff to be scribbled on
 * the HTTP return so that the client knows how to authenticate itself the
 * next time.
 */

void note_basic_auth_failure(request_rec *r);
int get_basic_auth_pw (request_rec *r, char **pw);

/*
 * Setting up the protocol fields for subsidiary requests...
 * Also, a wrapup function to keep the internal accounting straight.
 */

void set_sub_req_protocol (request_rec *rnew, request_rec *r);
void finalize_sub_req_protocol (request_rec *sub_r);

/* This is also useful for putting sub_reqs and internal_redirects together */

void parse_uri (request_rec *r, char *uri);     
