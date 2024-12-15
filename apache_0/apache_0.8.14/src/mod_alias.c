
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
 * http_alias.c: Stuff for dealing with directory aliases
 * 
 * Original by Rob McCool, rewritten in succession by David Robinson
 * and rst.
 * 
 */

#include "httpd.h"
#include "http_config.h"

typedef struct {
    char *real;
    char *fake;
    char *forced_type;
} alias_entry;

typedef struct {
    array_header *aliases;
    array_header *redirects;
} alias_server_conf;

module alias_module;

void *create_alias_config (pool *p, server_rec *s)
{
    alias_server_conf *a =
      (alias_server_conf *)pcalloc (p, sizeof(alias_server_conf));

    a->aliases = make_array (p, 20, sizeof(alias_entry));
    a->redirects = make_array (p, 20, sizeof(alias_entry));
    return a;
}

void *merge_alias_config (pool *p, void *basev, void *overridesv)
{
    alias_server_conf *a =
	(alias_server_conf *)pcalloc (p, sizeof(alias_server_conf));
    alias_server_conf *base = (alias_server_conf *)basev,
	*overrides = (alias_server_conf *)overridesv;

    a->aliases = append_arrays (p, overrides->aliases, base->aliases);
    a->redirects = append_arrays (p, overrides->redirects, base->redirects);
    return a;
}

/*
 * Strip trailing slashes off args to Alias --- range check ensures that
 * "////" gets canonicalized to "/" and not the empty string.
 */

void strip_slashes (char *s)
{
    char *cp = s + strlen(s) - 1;
    while ((cp > s) && (*cp == '/')) *cp-- = '\0';
}

char *add_alias(cmd_parms *cmd, void *dummy, char *f, char *r)
{
    server_rec *s = cmd->server;
    alias_server_conf *conf =
        (alias_server_conf *)get_module_config(s->module_config,&alias_module);
    alias_entry *new = push_array (conf->aliases);

    /* XX r can NOT be relative to DocumentRoot here... compat bug. */
    
    strip_slashes (f);  strip_slashes(r);
    new->fake = f; new->real = r; new->forced_type = cmd->info;
    return NULL;
}

char *add_redirect(cmd_parms *cmd, void *dummy, char *f, char *url)
{
    server_rec *s = cmd->server;
    alias_server_conf *conf =
        (alias_server_conf *)get_module_config(s->module_config,&alias_module);
    alias_entry *new = push_array (conf->redirects);

    if (!is_url (url)) return "Redirect to non-URL";
    
    new->fake = f; new->real = url;
    return NULL;
}

command_rec alias_cmds[] = {
{ "Alias", add_alias, NULL, RSRC_CONF, TAKE2, 
    "a fakename and a realname"},
{ "ScriptAlias", add_alias, CGI_MAGIC_TYPE, RSRC_CONF, TAKE2, 
    "a fakename and a realname"},
{ "Redirect", add_redirect, NULL, RSRC_CONF, TAKE2, 
    "a document to be redirected, then the destination URL" },
{ NULL }
};

char *try_alias_list (request_rec *r, array_header *aliases)
{
    alias_entry *entries = (alias_entry *)aliases->elts;
    int i;
    
    for (i = 0; i < aliases->nelts; ++i) {
        alias_entry *p = &entries[i];
        int l = strlen(p->fake);

        if(!strncmp(r->uri, p->fake, l)
	   && (p->fake[l-1] == '/' || l == strlen(r->uri) || r->uri[l] == '/'))
	{
	    if (p->forced_type)
		table_set (r->notes, "alias-forced-type", p->forced_type);
			   
            return pstrcat(r->pool, p->real, r->uri + l, NULL);
        }
    }

    return NULL;
}

int translate_alias_redir(request_rec *r)
{
    void *sconf = r->server->module_config;
    alias_server_conf *conf =
        (alias_server_conf *)get_module_config(sconf, &alias_module);
    char *ret;

    if (r->uri[0] != '/' && r->uri[0] != '\0') 
        return BAD_REQUEST;

    if ((ret = try_alias_list (r, conf->redirects)) != NULL) {
        table_set (r->headers_out, "Location", ret);
        return REDIRECT;
    }
    
    if ((ret = try_alias_list (r, conf->aliases)) != NULL) {
        r->filename = ret;
        return OK;
    }
    
    return DECLINED;
}

int type_forced_alias (request_rec *r)
{
    char *t = table_get (r->notes, "alias-forced-type");
    if (!t) return DECLINED;
    r->content_type = t;
    return OK;
}

module alias_module = {
   STANDARD_MODULE_STUFF,
   NULL,			/* initializer */
   NULL,			/* dir config creater */
   NULL,			/* dir merger --- default is to override */
   create_alias_config,		/* server config */
   merge_alias_config,		/* merge server configs */
   alias_cmds,			/* command table */
   NULL,			/* handlers */
   translate_alias_redir,	/* filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   NULL,			/* check access */
   type_forced_alias,		/* type_checker */
   NULL,			/* fixups */
   NULL				/* logger */
};
