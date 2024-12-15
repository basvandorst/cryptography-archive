/*

for starters:

one function, py_news_add_server, "news_add_server()"
will do NNTP, take one arg (hostname), and will support one operation,
close().

*/

#include "allobjects.h"
#include "modsupport.h"

#include "newsapi.h"

typedef struct {
	OB_HEAD
        news_server_type * server;
} py_news_server_object;

staticforward typeobject py_news_server_type;

static object *news_module_error;

#define is_py_news_server_object(v)		((v)->ob_type == &py_news_server_type)


static void
py_news_server_dealloc(py_news_server_object *py_server)

{
  if (py_server->server) {
    news_close_server(py_server->server);
    py_server->server = NULL;
  }
  DEL(py_server);
}

static object *
py_news_server_close (py_news_server_object 	*self,
		      object			*args)
{
  if (self->server) {
    news_close_server(self->server);
    self->server = NULL;
  }
  return (None);
}

static object *
py_news_group (py_news_server_object 	*self,
	       object			*args)
{
  int retval;
  news_group_type ng;
  char * group;

  if (!getargs(args, "s", &group))
    return (NULL);

  retval = news_group(self->server, group, &ng);

  if (!retval)
    return (mkvalue("(lll)",ng.low,ng.high,ng.estnum));
  else {
    char news_error_num[10];
    sprintf(news_error_num, "%d", retval);
    err_setstr(news_module_error, news_error_num);
    return (NULL);
  }
}

static object *
py_news_overviews (py_news_server_object	*self,
		   object			*args)
{
  int retval;
  news_overview_list_type	*ov_list;
  char * group;
  unsigned long start, end;
  
  if (!getargs (args, "(sll)", &group, &start, &end))
    return (NULL);

  retval = news_get_overviews (self->server,
			       group,
			       start,
			       end,
			       &ov_list);

  if (!retval) {
    int i;
    object *list = newlistobject(ov_list->length);
    if (list) {
      for (i = 0; i < ov_list->length; i++) {
	object * ov_obj;
	news_overview_type ov = (ov_list->overviews)[i];
	ov_obj = mkvalue("(sszzzllll)",
		     ov.subject,
		     ov.from,
		     ov.message_id,
		     ov.references,
		     ov.xref,
		     (unsigned long) ov.date,
		     ov.number,
		     ov.lines,
		     ov.bytes);
	setlistitem(list, i, ov_obj);
	if (err_occurred()) {
	  DECREF(list);
	  list = NULL;
	}
      }
    }
    news_free_overviews (ov_list);
    return list;
  }
  else {
    char news_error_num[10];
    sprintf(news_error_num, "%d", retval);
    err_setstr(news_module_error, news_error_num);
    return (NULL);
  }
}


static object *
py_news_list (py_news_server_object	*self,
	      object			*args)
{
  int retval;
  news_group_list_type	*group_list;
  char * group;
  
  if (!getargs (args, ""))
    return (NULL);

  retval = news_list_groups (self->server,&group_list);

  if (!retval) {
    int i;
    object *list = newlistobject(group_list->length);
    if (list) {
      for (i = 0; i < group_list->length; i++) {
	object * ng_obj;
	news_group_type ng = (group_list->groups)[i];
	ng_obj = mkvalue("(slllc)",
			 ng.group_name,
			 ng.low,
			 ng.high,
			 ng.estnum,
			 ng.flags);
	setlistitem(list, i, ng_obj);
	if (err_occurred()) {
	  DECREF(list);
	  list = NULL;
	}
      }
    }
    news_free_group_list(group_list);
    return list;
  }
  else {
    char news_error_num[10];
    sprintf(news_error_num, "%d", retval);
    err_setstr(news_module_error, news_error_num);
    return (NULL);
  }    
}


static object *
py_news_article (py_news_server_object	*self,
		 object			*args)
{
  int retval;
  news_article_type *art;
  char * group;
  long	artnum;

  if (!getargs (args, "(sl)", &group, &artnum))
    return(NULL);

  retval = news_article (self->server,
			 group,
			 (unsigned long) artnum,
			 &art);
  
  if (!retval) {
    object * result = mkvalue("(sll)", art->data, art->lines, art->bytes);
    news_free_article(art);
    return (result);
  }
  else {
    char news_error_num[10];
    sprintf(news_error_num, "%d", retval);
    err_setstr(news_module_error, news_error_num);
    return (NULL);
  }

}

static struct methodlist news_server_methods[] = {
	{"close",		(method)py_news_server_close},
	{"group",		(method)py_news_group},
	{"article",		(method)py_news_article},
	{"list",		(method)py_news_list},
	{"overviews",		(method)py_news_overviews},
	{NULL,			NULL}		/* sentinel */
};

static object *
py_news_server_getattr(self, name)
	py_news_server_object *self;
	char *name;
{
	return findmethod(news_server_methods, (object *)self, name);
}

static typeobject py_news_server_type = {
	OB_HEAD_INIT(&Typetype)
	0,					/*ob_size*/
	"news_server",				/*tp_name*/
	sizeof(py_news_server_object), 		/*tp_size*/
	0,					/*tp_itemsize*/
	/* methods */
	(destructor) py_news_server_dealloc, 	/*tp_dealloc*/
	0,					/*tp_print*/
	(getattrfunc) py_news_server_getattr, 	/*tp_getattr*/
	0,					/*tp_setattr*/
	0,					/*tp_compare*/
	0,					/*tp_repr*/
        0,					/*tp_as_number*/
};


/* create a new news server (nntp) */

static object *
py_news_nntp_server(object * self, object * args)
{
  py_news_server_object *py_server;
  int len;
  int retval;
  nntp_connect_type ct;

  /* last two args are optional here */
  if (!getargs(args, "(szz)", &ct.hostname, &ct.username, &ct.password))
    return NULL;
  
  py_server = NEWOBJ(py_news_server_object, &py_news_server_type);
  
  if (py_server == NULL)
    return NULL;
    
  retval = news_add_server (NNTP_SERVER_TYPE,
			    &ct,
			    &py_server->server);
  
  if (!retval)
    return ((object *) py_server);
  else {
    char news_error_num[10];
    sprintf(news_error_num, "%d", retval);
    err_setstr(news_module_error, news_error_num);
    return (NULL);
  }
}

/* create a new news server (file) */

static object *
py_news_file_server(object * self, object * args)
{
  py_news_server_object *py_server;
  int len;
  int retval;
  file_connect_type ct;

  /* last two args are optional here */
  if (!getargs(args, "(sszss)",
	       &ct.spool_root, &ct.lib_root, &ct.overview_root,
	       &ct.inews_program, &ct.dir_sep))
    return NULL;
  
  py_server = NEWOBJ(py_news_server_object, &py_news_server_type);
  
  if (py_server == NULL)
    return NULL;
    
  retval = news_add_server (FILE_SERVER_TYPE,
			    &ct,
			    &py_server->server);
  
  if (!retval)
    return ((object *) py_server);
  else {
    char news_error_num[10];
    sprintf(news_error_num, "%d", retval);
    err_setstr(news_module_error, news_error_num);
    return (NULL);
  }
}

/* List of functions exported by this module */

static struct methodlist news_server_functions[] = {
	{"nntp",		(method)py_news_nntp_server},
	{"file",		(method)py_news_file_server}, 
	{NULL,			NULL}		 /* Sentinel */
};


/* Initialize this module. */

void
initnews(void)
{
  object *dict, *module;
  module = initmodule("news", news_server_functions);
  dict = getmoduledict(module);
  news_module_error = newstringobject("newsapi error");
  dictinsert(dict, "error", news_module_error);
}
