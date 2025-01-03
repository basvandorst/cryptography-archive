
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
 * sell support for for Apache. We explicitly forbid the redistribution of 
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
 * http_auth_unix: authentication
 * 
 * Rob McCool & Brian Behlendorf.
 * Adapted to Shambhala by rst. 
 * Modified by Sameer Parekh to just check UNIX passwd file
 *
 */

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include <ndbm.h>

typedef struct  {
    char *auth_passwdfile;
} unix_auth_config_rec;

void *create_unix_auth_dir_config (pool *p, char *d)
{
    return pcalloc (p, sizeof(unix_auth_config_rec));
}

command_rec unix_auth_cmds[] = {
{ "AuthUNIXPasswdFile", set_string_slot,
    (void*)XtOffsetOf(unix_auth_config_rec, auth_passwdfile),
    OR_AUTHCFG, TAKE1, NULL },
{ NULL }
};

module unix_auth_module;

int unix_authenticate_basic_user (request_rec *r)
{
    unix_auth_config_rec *sec =
      (unix_auth_config_rec *)get_module_config (r->per_dir_config,
						 &unix_auth_module);
    conn_rec *c = r->connection;
    char *sent_pw;
    char errstr[MAX_STRING_LEN];
    int res;
    
    if ((res = get_basic_auth_pw (r, &sent_pw))) return res;
    
    if(!sec->auth_passwdfile) 
        return DECLINED;
    /* 
    if (!(real_pw = get_pw(r, c->user, sec->auth_pwfile))) {
        sprintf(errstr,"user %s not found",c->user);
	log_reason (errstr, r->uri, r);
	note_basic_auth_failure (r);
	return AUTH_REQUIRED;
    }

    /* Run pwdauth */
    if(pwdauth(c->user, sent_pw))
      {
	sprintf(errstr,"user %s: login incorrect",c->user);
	log_reason (errstr, r->uri, r);
	note_basic_auth_failure (r);
	return AUTH_REQUIRED;
      }
    return OK;
}
    
/* Checking ID */
int unix_check_user_access (request_rec *r) {
    unix_auth_config_rec *sec =
      (unix_auth_config_rec *)get_module_config (r->per_dir_config,
						 &unix_auth_module);
    char *user = r->connection->user;
    int m = r->method_number;
    
    array_header *reqs_arr = requires (r);
    require_line *reqs = (require_line *)reqs_arr->elts;

    register int x;
    char *t, *w;
    
    for(x=0; x < reqs_arr->nelts; x++) {
      
	if (! (reqs[x].method_mask & (1 << m))) continue;
	
        t = reqs[x].requirement;
        w = getword(r->pool, &t, ' ');
        if(!strcmp(w,"valid-user"))
            return OK;
        if(!strcmp(w,"user")) {
            while(t[0]) {
                w = getword_conf (r->pool, &t);
                if(!strcmp(user,w))
                    return OK;
            }
        }
    }
    
    note_basic_auth_failure (r);
    return AUTH_REQUIRED;
}

module unix_auth_module = {
   STANDARD_MODULE_STUFF,
   NULL,			/* initializer */
   create_unix_auth_dir_config,	/* dir config creater */
   NULL,			/* dir merger --- default is to override */
   NULL,			/* server config */
   NULL,			/* merge server config */
   unix_auth_cmds,		/* command table */
   NULL,			/* handlers */
   NULL,			/* filename translation */
   unix_authenticate_basic_user,/* check_user_id */
   unix_check_user_access,	/* check auth */
   NULL,			/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   NULL				/* logger */
};
