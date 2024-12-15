
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



#include "httpd.h"
#include "http_config.h"

module common_log_module;

static int xfer_flags = ( O_WRONLY | O_APPEND | O_CREAT );
static mode_t xfer_mode = ( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

typedef struct {
    char *fname;
    int log_fd;
} common_log_state;

void *make_common_log_state (pool *p, server_rec *s)
{
    common_log_state *cls =
      (common_log_state *)palloc (p, sizeof (common_log_state));

    cls->fname = DEFAULT_XFERLOG;
    cls->log_fd = -1;

    return (void *)cls;
}

char *set_common_log (cmd_parms *parms, void *dummy, char *arg)
{
    common_log_state *cls = get_module_config (parms->server->module_config,
					       &common_log_module);
  
    cls->fname = arg;
    return NULL;
}

command_rec common_log_cmds[] = {
{ "TransferLog", set_common_log, NULL, RSRC_CONF, TAKE1,
    "the filename of the access log" },
{ NULL }
};

void common_log_child (void *cmd)
{
    /* Child process code for 'TransferLog "|..."';
     * may want a common framework for this, since I expect it will
     * be common for other foo-loggers to want this sort of thing...
     */
    
    cleanup_for_exec();
    signal (SIGHUP, SIG_IGN);
    execl (SHELL_PATH, SHELL_PATH, "-c", (char *)cmd, NULL);
    fprintf (stderr, "Exec of shell for logging failed!!!\n");
    exit (1);
}

void open_common_log (server_rec *s, pool *p)
{
    common_log_state *cls = get_module_config (s->module_config,
					       &common_log_module);
  
    char *fname = server_root_relative (p, cls->fname);
    
    if (cls->log_fd > 0) return; /* virtual log shared w/main server */
    
    if (*cls->fname == '|') {
	FILE *dummy;
	
	spawn_child(p, common_log_child, (void *)(cls->fname+1),
		    kill_after_timeout, &dummy, NULL);

	if (dummy == NULL) {
	    fprintf (stderr, "Couldn't fork child for TransferLog process\n");
	    exit (1);
	}

	cls->log_fd = fileno (dummy);
    }
    else if((cls->log_fd = popenf(p, fname, xfer_flags, xfer_mode)) < 0) {
        fprintf(stderr,"httpd: could not open transfer log file %s.\n", fname);
        perror("open");
        exit(1);
    }
}

void init_common_log (server_rec *s, pool *p)
{
    for (; s; s = s->next) open_common_log (s, p);
}

int common_log_transaction(request_rec *orig)
{
    common_log_state *cls = get_module_config (orig->server->module_config,
					       &common_log_module);
  
    char str[HUGE_STRING_LEN];
    long timz;
    struct tm *t;
    char tstr[MAX_STRING_LEN],sign;
    conn_rec *c = orig->connection;
    request_rec *r;

    /* Common log format records an unholy melange of the original request
     * and whatever it was that we actually served.  Just stay compatible
     * here; the whole point of the module scheme is to allow people to
     * create better alternatives, but screwing up is an option we wish
     * to preserve...
     */
    
    for (r = orig; r->next; r = r->next)
        continue;

    t = get_gmtoff(&timz);
    sign = (timz < 0 ? '-' : '+');
    if(timz < 0) 
        timz = -timz;

    strftime(tstr,MAX_STRING_LEN,"%d/%b/%Y:%H:%M:%S",t);

    sprintf(str,"%s %s %s [%s %c%02ld%02ld] \"%s\" ",
            c->remote_name,
            (c->remote_logname ? c->remote_logname : "-"),
	    (c->user ? c->user : "-"),
            tstr,
            sign,
            timz/3600,
            timz%3600,
            (orig->the_request ? orig->the_request : "NULL") );
    
    if (r->status != -1)
        sprintf(str,"%s%d ",str,r->status);
    else
        strcat(str,"- ");

    if(r->bytes_sent != -1)
        sprintf(str,"%s%d",str,r->bytes_sent);
    else
        strcat(str,"-");

#if defined(APACHE_SSL) && 0
/* I don't like this method of using sprintf(), but for ease of patch
maintenance... Ben */
    if(c->client.nVerifyError)
      sprintf(str,"%s %d %s",str,c->client.nVerifyError,
	      X509_verify_error_string(c->client.nVerifyError));
    else
      strcat(str," - -");

    if(c->client.szClientX509)
      sprintf(str,"%s \"%s\"",str,c->client.szClientX509);
    else
      strcat(str," -");
#endif

    strcat(str,"\n");
    write(cls->log_fd, str, strlen(str));

    return OK;
}

module common_log_module = {
   STANDARD_MODULE_STUFF,
   init_common_log,		/* initializer */
   NULL,			/* create per-dir config */
   NULL,			/* merge per-dir config */
   make_common_log_state,	/* server config */
   NULL,			/* merge server config */
   common_log_cmds,		/* command table */
   NULL,			/* handlers */
   NULL,			/* filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   NULL,			/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   common_log_transaction	/* logger */
};
