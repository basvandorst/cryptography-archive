
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

module referer_log_module;

static int xfer_flags = ( O_WRONLY | O_APPEND | O_CREAT );
static mode_t xfer_mode = ( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

typedef struct {
    char *fname;
    int referer_fd;
    array_header *referer_ignore_list;
} referer_log_state;

void *make_referer_log_state (pool *p, server_rec *s)
{
    referer_log_state *cls =
      (referer_log_state *)palloc (p, sizeof (referer_log_state));

    cls->fname = "";
    cls->referer_fd = -1;
    cls->referer_ignore_list = make_array(p, 1, sizeof(char *));
    return (void *)cls;
}

char *set_referer_log (cmd_parms *parms, void *dummy, char *arg)
{
    referer_log_state *cls = get_module_config (parms->server->module_config,
					       &referer_log_module);
  
    cls->fname = arg;
    return NULL;
}

char *add_referer_ignore (cmd_parms *parms, void *dummy, char *arg)
{
  char **addme;
  referer_log_state *cls = get_module_config (parms->server->module_config,
					      &referer_log_module);

  addme = push_array(cls->referer_ignore_list);
  *addme = pstrdup(cls->referer_ignore_list->pool, arg);
  return NULL;
}

command_rec referer_log_cmds[] = {
{ "RefererLog", set_referer_log, NULL, RSRC_CONF, TAKE1,
    "the filename of the referer log" },
{ "RefererIgnore", add_referer_ignore, NULL, RSRC_CONF, ITERATE,
    "referer hostnames to ignore" },
{ NULL }
};

void referer_log_child (void *cmd)
{
    /* Child process code for 'RefererLog "|..."';
     * may want a common framework for this, since I expect it will
     * be common for other foo-loggers to want this sort of thing...
     */
    
    cleanup_for_exec();
    signal (SIGHUP, SIG_IGN);
    execl (SHELL_PATH, SHELL_PATH, "-c", (char *)cmd, NULL);
    fprintf (stderr, "Exec of shell for logging failed!!!\n");
    exit (1);
}

void open_referer_log (server_rec *s, pool *p)
{
    referer_log_state *cls = get_module_config (s->module_config,
					       &referer_log_module);
  
    char *fname = server_root_relative (p, cls->fname);
    
    if (cls->referer_fd > 0) return; /* virtual log shared w/main server */
    
    if (*cls->fname == '|') {
	FILE *dummy;
	
	spawn_child(p, referer_log_child, (void *)(cls->fname+1),
		    kill_after_timeout, &dummy, NULL);

	if (dummy == NULL) {
	    fprintf (stderr, "Couldn't fork child for RefererLog process\n");
	    exit (1);
	}

	cls->referer_fd = fileno (dummy);
    }
    else if(*cls->fname != '\0') {
      if((cls->referer_fd = popenf(p, fname, xfer_flags, xfer_mode)) < 0) {
        fprintf(stderr,"httpd: could not open referer log file %s.\n", fname);
        perror("open");
        exit(1);
      }
    }
}

void init_referer_log (server_rec *s, pool *p)
{
    for (; s; s = s->next) open_referer_log (s, p);
}

int referer_log_transaction(request_rec *orig)
{
    char **ptrptr, **ptrptr2;
    referer_log_state *cls = get_module_config (orig->server->module_config,
					       &referer_log_module);
  
    char str[HUGE_STRING_LEN];
    char *referer;
    request_rec *r;

    if(cls->referer_fd <0)
      return OK;

    for (r = orig; r->next; r = r->next)
        continue;
    if (*cls->fname == '\0')	/* Don't log referer */
	return DECLINED;
    
    referer = table_get(orig->headers_in, "Referer");
    if(referer != NULL)
      {


	  /* The following is an upsetting mess of pointers, I'm sorry
	     Anyone with the motiviation and/or the time should feel free
	     to make this cleaner... */

	  ptrptr2 = (char **) (cls->referer_ignore_list->elts + 
		     (cls->referer_ignore_list->nelts *
		      cls->referer_ignore_list->elt_size));
	  
	  /* Go through each element of the ignore list and compare it to the
	     referer_host.  If we get a match, return without logging */

	  for(ptrptr = (char **) cls->referer_ignore_list->elts;
	      ptrptr < ptrptr2;
	      ptrptr += cls->referer_ignore_list->elt_size) 
	    {
		if(strstr(referer, *ptrptr))
		  return OK;
	    }
	  
	  
	  sprintf(str, "%s -> %s\n", referer,
		  r->uri);
	  write(cls->referer_fd, str, strlen(str));
      }

    return OK;
}

module referer_log_module = {
   STANDARD_MODULE_STUFF,
   init_referer_log,		/* initializer */
   NULL,			/* create per-dir config */
   NULL,			/* merge per-dir config */
   make_referer_log_state,	/* server config */
   NULL,			/* merge server config */
   referer_log_cmds,		/* command table */
   NULL,			/* handlers */
   NULL,			/* filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   NULL,			/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   referer_log_transaction	/* logger */
};
