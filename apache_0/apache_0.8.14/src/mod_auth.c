
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
 * http_auth: authentication
 * 
 * Rob McCool
 * 
 * Adapted to Shambhala by rst.
 */

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"

typedef struct auth_config_struct {
    char *auth_pwfile;
    char *auth_grpfile;
} auth_config_rec;

void *create_auth_dir_config (pool *p, char *d)
{
    return pcalloc (p, sizeof(auth_config_rec));
}

command_rec auth_cmds[] = {
{ "AuthUserFile", set_string_slot,
    (void*)XtOffsetOf(auth_config_rec,auth_pwfile), OR_AUTHCFG, TAKE1, NULL },
{ "AuthGroupFile", set_string_slot,
    (void*)XtOffsetOf(auth_config_rec,auth_grpfile), OR_AUTHCFG, TAKE1, NULL },
{ NULL }
};

module auth_module;

char *get_pw(request_rec *r, char *user, char *auth_pwfile)
{
    FILE *f;
    char l[MAX_STRING_LEN];
    char *rpw, *w;

    if(!(f=pfopen(r->pool, auth_pwfile, "r"))) {
        log_reason ("Could not open password file", auth_pwfile, r);
	return NULL;
    }
    while(!(cfg_getline(l,MAX_STRING_LEN,f))) {
        if((l[0] == '#') || (!l[0])) continue;
	rpw = l;
        w = getword(r->pool, &rpw, ':');

        if(!strcmp(user,w)) {
	    pfclose(r->pool, f);
            return pstrdup (r->pool, rpw);
	}
    }
    pfclose(r->pool, f);
    return NULL;
}

table *init_group(pool *p, char *grpfile) {
    FILE *f;
    table *grps = make_table (p, 15);
    char l[MAX_STRING_LEN];
    char *name, *ll;

    if(!(f=pfopen(p, grpfile, "r")))
        return NULL;

    while(!(cfg_getline(l,MAX_STRING_LEN,f))) {
        if((l[0] == '#') || (!l[0])) continue;
	ll = l;
        name = getword(p, &ll, ':');
	table_set (grps, name, ll);
    }
    pfclose(p, f);
    return grps;
}

int in_group(pool *p, char *user, char *group, table *grps) {
    char *l = table_get (grps, group);
    char *w;

    if (!l) return 0;

    while(l[0]) {
        w = getword_conf (p, &l);
	if(!strcmp(w,user))
	    return 1;
    }
    
    return 0;
}

/* These functions return 0 if client is OK, and proper error status
 * if not... either AUTH_REQUIRED, if we made a check, and it failed, or
 * SERVER_ERROR, if things are so totally confused that we couldn't
 * figure out how to tell if the client is authorized or not.
 *
 * If they return DECLINED, and all other modules also decline, that's
 * treated by the server core as a configuration error, logged and
 * reported as such.
 */

/* Determine user ID, and check if it really is that user, for HTTP
 * basic authentication...
 */

int authenticate_basic_user (request_rec *r)
{
    auth_config_rec *sec =
      (auth_config_rec *)get_module_config (r->per_dir_config, &auth_module);
    conn_rec *c = r->connection;
    char *sent_pw, *real_pw;
    char errstr[MAX_STRING_LEN];
    int res;
    
    if ((res = get_basic_auth_pw (r, &sent_pw))) return res;
    
    if(!sec->auth_pwfile) 
        return DECLINED;
	
    if (!(real_pw = get_pw(r, c->user, sec->auth_pwfile))) {
        sprintf(errstr,"user %s not found",c->user);
	log_reason (errstr, r->uri, r);
	note_basic_auth_failure (r);
	return AUTH_REQUIRED;
    }
    /* anyone know where the prototype for crypt is? */
    if(strcmp(real_pw,(char *)crypt(sent_pw,real_pw))) {
        sprintf(errstr,"user %s: password mismatch",c->user);
	log_reason (errstr, r->uri, r);
	note_basic_auth_failure (r);
	return AUTH_REQUIRED;
    }
    return OK;
}
    
/* Checking ID */
    
int check_user_access (request_rec *r) {
    auth_config_rec *sec =
      (auth_config_rec *)get_module_config (r->per_dir_config, &auth_module);
    char *user = r->connection->user;
    int m = r->method_number;
    
    array_header *reqs_arr = requires (r);
    require_line *reqs = (require_line *)reqs_arr->elts;

    register int x;
    char *t, *w;
    table *grpstatus;
    
    if(sec->auth_grpfile)
        grpstatus = init_group(r->pool, sec->auth_grpfile);
    else
        grpstatus = NULL;

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
        else if(!strcmp(w,"group")) {
            if(!grpstatus) 
	        return DECLINED;	/* DBM group?  Something else? */
	    
            while(t[0]) {
                w = getword_conf(r->pool, &t);
                if(in_group(r->pool, user, w, grpstatus))
		    return OK;
            }
        }
    }
    
    note_basic_auth_failure (r);
    return AUTH_REQUIRED;
}

module auth_module = {
   STANDARD_MODULE_STUFF,
   NULL,			/* initializer */
   create_auth_dir_config,	/* dir config creater */
   NULL,			/* dir merger --- default is to override */
   NULL,			/* server config */
   NULL,			/* merge server config */
   auth_cmds,			/* command table */
   NULL,			/* handlers */
   NULL,			/* filename translation */
   authenticate_basic_user,	/* check_user_id */
   check_user_access,		/* check auth */
   NULL,			/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   NULL				/* logger */
};
