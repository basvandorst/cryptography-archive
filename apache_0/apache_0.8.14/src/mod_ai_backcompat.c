#include "httpd.h"
#include "http_config.h"
#include "http_core.h"

/* Why are you even thinking about using this? */

int ai_backcompat_kludge (request_rec *r)
{
    if (r->method_number != M_GET ||
	(r->args && r->args[0]) || (r->path_info && r->path_info[0]))
    {
        char *doit_filename = pstrcat (r->pool, r->filename, ".doit", NULL);
	struct stat finfo;

	if (stat (doit_filename, &finfo) >= 0) {
	    r->filename = doit_filename;
	    r->finfo = finfo;
	}
    }
    
    return DECLINED;
}

module ai_backcompat_module = {
   STANDARD_MODULE_STUFF,
   NULL,			/* initializer */
   NULL,			/* dir config creater */
   NULL,			/* dir merger --- default is to override */
   NULL,			/* server config */
   NULL,			/* merge server config */
   NULL,			/* command table */
   NULL,			/* handlers */
   NULL,			/* filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   NULL,			/* check access */
   ai_backcompat_kludge,	/* type_checker */
   NULL,			/* fixups */
   NULL				/* logger */
};
