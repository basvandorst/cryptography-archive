
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
 * Security options etc.
 * 
 * Module derived from code originally written by Rob McCool
 * 
 */

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

typedef struct {
    char *from;
    int limited;
} allowdeny;

/* things in the 'order' array */
#define DENY_THEN_ALLOW 0
#define ALLOW_THEN_DENY 1
#define MUTUAL_FAILURE 2

typedef struct {
    int order[METHODS];
    array_header *allows;
    array_header *denys;
} access_dir_conf;

module access_module;

void *create_access_dir_config (pool *p, char *dummy)
{
    access_dir_conf *conf =
        (access_dir_conf *)pcalloc(p, sizeof(access_dir_conf));
    int i;
    
    for (i = 0; i < METHODS; ++i) conf->order[i] = DENY_THEN_ALLOW;
    conf->allows = make_array (p, 1, sizeof (allowdeny));
    conf->denys = make_array (p, 1, sizeof (allowdeny));
    
    return (void *)conf;
}

char *order (cmd_parms *cmd, void *dv, char *arg)
{
    access_dir_conf *d = (access_dir_conf *)dv;
    int i, order;
  
    if (!strcasecmp (arg, "allow,deny")) order = ALLOW_THEN_DENY;
    else if (!strcasecmp (arg, "deny,allow")) order = DENY_THEN_ALLOW;
    else if (!strcasecmp (arg, "mutual-failure")) order = MUTUAL_FAILURE;
    else return "unknown order";

    for (i = 0; i < METHODS; ++i) 
        if (cmd->limited & (1 << i))
	    d->order[i] = order;
    
    return NULL;
}

char *allow_cmd (cmd_parms *cmd, void *dv, char *from, char *where)
{
    access_dir_conf *d = (access_dir_conf *)dv;
    allowdeny *a;
  
    if (strcasecmp (from, "from"))
        return "allow and deny must be followed by 'from'";
    
    a = (allowdeny *)push_array (cmd->info ? d->allows : d->denys);
    a->from = pstrdup (cmd->pool, where);
    a->limited = cmd->limited;
    return NULL;
}

static char its_an_allow;

command_rec access_cmds[] = {
{ "order", order, NULL, OR_LIMIT, TAKE1,
    "'allow,deny', 'deny,allow', or 'mutual failure'" },
{ "allow", allow_cmd, &its_an_allow, OR_LIMIT, ITERATE2,
    "'from' followed by hostnames or IP-address wildcards" },
{ "deny", allow_cmd, NULL, OR_LIMIT, ITERATE2,
    "'from' followed by hostnames or IP-address wildcards" },
{NULL}
};

int in_domain(char *domain, char *what) {
    int dl=strlen(domain);
    int wl=strlen(what);

    if((wl-dl) >= 0)
        return(!strcmp(domain,&what[wl-dl]));
    else
        return 0;
}

int in_ip(char *domain, char *what) {
    return(!strncmp(domain,what,strlen(domain)));
}

int find_allowdeny (conn_rec *c, array_header *a, int method)
{
    allowdeny *ap = (allowdeny *)a->elts;
    int mmask = (1 << method);
    int i;

    for (i = 0; i < a->nelts; ++i) {
        if (!(mmask & ap[i].limited))
	    continue;
	if (!strcmp (ap[i].from, "all"))
	    return 1;
        if (c->remote_host && isalpha(c->remote_host[0]))
            if (in_domain(ap[i].from, c->remote_host))
                return 1;
        if (in_ip (ap[i].from, c->remote_ip))
            return 1;
    }

    return 0;
}

int check_dir_access (request_rec *r)
{
    int method = r->method_number;
    access_dir_conf *a =
        (access_dir_conf *)
	   get_module_config (r->per_dir_config, &access_module);
    int ret = OK;
						
    conn_rec *c = r->connection;
    
    if (a->order[method] == ALLOW_THEN_DENY) {
        ret = FORBIDDEN;
        if (find_allowdeny (c, a->allows, method))
            ret = OK;
        if (find_allowdeny (c, a->denys, method))
            ret = FORBIDDEN;
    } else if (a->order[method] == DENY_THEN_ALLOW) {
        if (find_allowdeny (c, a->denys, method))
            ret = FORBIDDEN;
        if (find_allowdeny (c, a->allows, method))
            ret = OK;
    }
    else {
        if (find_allowdeny(c, a->allows, method) 
	    && !find_allowdeny(c, a->denys, method))
	    ret = OK;
	else
	    ret = FORBIDDEN;
    }

    if (ret == FORBIDDEN)
	log_reason ("Client denied by server configuration", r->filename, r);

    return ret;
}



module access_module = {
   STANDARD_MODULE_STUFF,
   NULL,			/* initializer */
   create_access_dir_config,	/* dir config creater */
   NULL,			/* dir merger --- default is to override */
   NULL,			/* server config */
   NULL,			/* merge server config */
   access_cmds,
   NULL,			/* handlers */
   NULL,			/* filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   check_dir_access,		/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   NULL				/* logger */
};
