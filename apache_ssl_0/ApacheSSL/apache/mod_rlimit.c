
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
 * mod_rlimit.c: developed by Community ConneXion
 * Set the max open files a bit larger based on the config option,
 *   per virtual server
 *
 * Sameer Parekh
 */

#include <sys/time.h>
#include <sys/resource.h>
#include "httpd.h"
#include "http_config.h"

/* The default is zero because the server already adds two per
   virtual server */

#define DEFAULT_NOFILEPLUS 0

module rlimit_module;

typedef struct
{
  int nofile;
} rlimit_conf;

/* Deal with a configuration option */
char *
set_rlimit_nofile (cmd_parms *parms, void *dummy, char *arg)
{
  rlimit_conf *cls = get_module_config (parms->server->module_config,
					&rlimit_module);

  cls->nofile = atoi(arg);
  return NULL;
}

/* Config file options */
command_rec rlimit_cmds[] = {
  { "MaxOpenFilesPlus", set_rlimit_nofile, NULL, RSRC_CONF, TAKE1,
      "how many additional files to possibly open for this server" },
  { NULL }
};

/* initialize config data structure */

void *
make_rlimit_conf (pool *p, server_rec *s)
{
  rlimit_conf *cls = (rlimit_conf *)palloc(p, sizeof(rlimit_conf));
  cls->nofile = DEFAULT_NOFILEPLUS;
  return (void *)cls;
}

/* Set all the rlimits */
void
init_rlimit(server_rec *s, pool *p)
{
  rlimit_conf *cls;
  struct rlimit limits;

  for(; s; s = s->next)
    {
      cls = get_module_config (s->module_config,
			       &rlimit_module);


      getrlimit ( RLIMIT_NOFILE, &limits );
      if ( limits.rlim_cur < limits.rlim_max ) {
	limits.rlim_cur += cls->nofile;
	if ( setrlimit ( RLIMIT_NOFILE, &limits ) < 0 )
	  fprintf (stderr, "Cannot exceed hard limit for open files");
      }
    }
}

module rlimit_module = {
   STANDARD_MODULE_STUFF,
   init_rlimit,    		/* initializer */
   NULL,			/* create per-dir config */
   NULL,			/* merge per-dir config */
   make_rlimit_conf, 	        /* server config */
   NULL,			/* merge server config */
   rlimit_cmds,		        /* command table */
   NULL,			/* handlers */
   NULL,			/* filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   NULL,		        /* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   NULL                  	/* logger */
};
