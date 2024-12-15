
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
 * mod_userdir... implement the UserDir command.  Broken away fromthe
 * Alias stuff for a couple of good and not-so-good reasons:
 *
 * 1) It shows a real minimal working example of how to do something like this.
 * 2) I know people who are actually interested in changing this *particular*
 *    aspect of server functionality without changing the rest of it.  That's
 *    what this whole modular arrangement is supposed to be good at...
 */

#include "httpd.h"
#include "http_config.h"

module userdir_module;

/*
 * Sever config for this module is a little unconventional...
 * It's just one string anyway, so why pretend?
 */

void *create_userdir_config (pool *dummy, server_rec *s) { 
    return (void*)DEFAULT_USER_DIR; 
}

char *set_user_dir (cmd_parms *cmd, void *dummy, char *arg)
{
    void *server_conf = cmd->server->module_config;
    
    if (!strcasecmp (arg, "disabled")) arg = NULL;
    
    set_module_config (server_conf, &userdir_module, pstrdup (cmd->pool, arg));
    return NULL;
}

command_rec userdir_cmds[] = {
{ "UserDir", set_user_dir, NULL, RSRC_CONF, TAKE1,
    "the public subdirectory in users' home directories, or 'disabled'" },
{ NULL }
};

int translate_userdir (request_rec *r)
{
    void *server_conf = r->server->module_config;
    char *userdir = (char *)get_module_config(server_conf, &userdir_module);
    char *name = r->uri;
    
    if (userdir && (name[0] == '/') && (name[1] == '~'))
    {
        struct passwd *pw;
	char *w, *dname;

        dname = name + 2;
        w = getword(r->pool, &dname, '/');
        if(!(pw=getpwnam(w)))
            return NOT_FOUND;
	
	/* The 'dname' funny business involves backing it up to capture
	 * the '/' delimiting the "/~user" part from the rest of the URL,
	 * in case there was one (the case where there wasn't being just
	 * "GET /~user HTTP/1.0", for which we don't want to tack on a
	 * '/' onto the filename).
	 */
	
	if (dname[-1] == '/') --dname;
	r->filename = pstrcat (r->pool, pw->pw_dir, "/", userdir, dname, NULL);

	return OK;
    }

    return DECLINED;
}
    
module userdir_module = {
   STANDARD_MODULE_STUFF,
   NULL,			/* initializer */
   NULL,			/* dir config creater */
   NULL,			/* dir merger --- default is to override */
   create_userdir_config,	/* server config */
   NULL,			/* merge server config */
   userdir_cmds,		/* command table */
   NULL,			/* handlers */
   translate_userdir,		/*filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   NULL,			/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   NULL				/* logger */
};
