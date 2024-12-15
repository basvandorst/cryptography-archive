
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
 * http_dir.c: Handles the on-the-fly html index generation
 * 
 * Rob McCool
 * 3/23/93
 * 
 * Adapted to Shambhala by rst.
 */

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_request.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_main.h"
#include "util_script.h"

module dir_module;

/****************************************************************
 *
 * Handling configuration directives...
 */

#define FANCY_INDEXING 1	/* Indexing options */
#define ICONS_ARE_LINKS 2
#define SCAN_HTML_TITLES 4
#define SUPPRESS_LAST_MOD 8
#define SUPPRESS_SIZE 16
#define SUPPRESS_DESC 32

struct item {
    char *type;
    char *apply_to;
    char *apply_path;
    char *data;
};

typedef struct dir_config_struct {

    char *default_icon;
    char *index_names;
  
    array_header *icon_list, *alt_list, *desc_list, *ign_list;
    array_header *hdr_list, *rdme_list, *opts_list;
  
} dir_config_rec;

char c_by_encoding, c_by_type, c_by_path;

#define BY_ENCODING &c_by_encoding
#define BY_TYPE &c_by_type
#define BY_PATH &c_by_path

void push_item(array_header *arr, char *type, char *to, char *path, char *data)
{
    struct item *p = (struct item *)push_array(arr);

    if (!to) to = "";
    if (!path) path = "";
    
    p->type = type;
    p->data = data ? pstrdup(arr->pool, data): NULL;
    p->apply_path = pstrcat(arr->pool, path, "*", NULL);
    
    if((type == BY_PATH) && (!is_matchexp(to)))
        p->apply_to = pstrcat (arr->pool, "*", to, NULL);
    else if (to)
        p->apply_to = pstrdup (arr->pool, to);
    else
        p->apply_to = NULL;
}

char *add_alt(cmd_parms *cmd, void *d, char *alt, char *to)
{
    if (cmd->info == BY_PATH)
        if(!strcmp(to,"**DIRECTORY**"))
            to = "^^DIRECTORY^^";

    push_item(((dir_config_rec *)d)->alt_list, cmd->info, to, cmd->path, alt);
    return NULL;
}

char *add_icon(cmd_parms *cmd, void *d, char *icon, char *to)
{
    char *iconbak = pstrdup (cmd->pool, icon);

    if(icon[0] == '(') {
        char *alt = getword (cmd->pool, &iconbak, ',');
        iconbak[strlen(iconbak) - 1] = '\0'; /* Lose closing paren */
        add_alt(cmd, d, &alt[1], to);
    }
    if(cmd->info == BY_PATH) 
        if(!strcmp(to,"**DIRECTORY**"))
            to = "^^DIRECTORY^^";

    push_item(((dir_config_rec *)d)->icon_list, cmd->info, to, cmd->path,
	      iconbak);
    return NULL;
}

char *add_desc(cmd_parms *cmd, void *d, char *desc, char *to)
{
    push_item(((dir_config_rec *)d)->desc_list, cmd->info, to, cmd->path,desc);
    return NULL;
}

char *add_ignore(cmd_parms *cmd, void *d, char *ext) {
    push_item(((dir_config_rec *)d)->ign_list, 0, ext, cmd->path, NULL);
    return NULL;
}

char *add_header(cmd_parms *cmd, void *d, char *name) {
    push_item(((dir_config_rec *)d)->hdr_list, 0, NULL, cmd->path, name);
    return NULL;
}

char *add_readme(cmd_parms *cmd, void *d, char *name) {
    push_item(((dir_config_rec *)d)->rdme_list, 0, NULL, cmd->path, name);
    return NULL;
}


char *add_opts_int(cmd_parms *cmd, void *d, int opts) {
    push_item(((dir_config_rec *)d)->opts_list, (char*)opts, NULL,
	      cmd->path, NULL);
    return NULL;
}

char *fancy_indexing (cmd_parms *cmd, void *d, int arg)
{
    return add_opts_int (cmd, d, arg? FANCY_INDEXING : 0);
}

char *add_opts(cmd_parms *cmd, void *d, char *optstr) {
    char *w;
    int opts = 0;

    while(optstr[0]) {
        w = getword_conf(cmd->pool, &optstr);
        if(!strcasecmp(w,"FancyIndexing"))
            opts |= FANCY_INDEXING;
        else if(!strcasecmp(w,"IconsAreLinks"))
            opts |= ICONS_ARE_LINKS;
        else if(!strcasecmp(w,"ScanHTMLTitles"))
            opts |= SCAN_HTML_TITLES;
        else if(!strcasecmp(w,"SuppressLastModified"))
            opts |= SUPPRESS_LAST_MOD;
        else if(!strcasecmp(w,"SuppressSize"))
            opts |= SUPPRESS_SIZE;
        else if(!strcasecmp(w,"SuppressDescription"))
            opts |= SUPPRESS_DESC;
        else if(!strcasecmp(w,"None"))
            opts = 0;
	else
	    return "Invalid directory indexing option";
    }
    return add_opts_int(cmd, d, opts);
}

#define DIR_CMD_PERMS OR_INDEXES

command_rec dir_cmds[] = {
{ "AddIcon", add_icon, BY_PATH, DIR_CMD_PERMS, ITERATE2,
    "an icon URL followed by one or more filenames" },
{ "AddIconByType", add_icon, BY_TYPE, DIR_CMD_PERMS, ITERATE2,
    "an icon URL followed by one or more MIME types" },
{ "AddIconByEncoding", add_icon, BY_ENCODING, DIR_CMD_PERMS, ITERATE2,
    "an icon URL followed by one or more content encodings" },
{ "AddAlt", add_alt, BY_PATH, DIR_CMD_PERMS, ITERATE2,
    "alternate descriptive text followed by one or more filenames" },
{ "AddAltByType", add_alt, BY_TYPE, DIR_CMD_PERMS, ITERATE2,
    "alternate descriptive text followed by one or more MIME types" },
{ "AddAltByEncoding", add_alt, BY_ENCODING, DIR_CMD_PERMS, ITERATE2,
    "alternate descriptive text followed by one or more content encodings" },
{ "IndexOptions", add_opts, NULL, DIR_CMD_PERMS, RAW_ARGS,
    "one or more index options" },
{ "IndexIgnore", add_ignore, NULL, DIR_CMD_PERMS, ITERATE,
    "one or more file extensions" },
{ "AddDescription", add_desc, BY_PATH, DIR_CMD_PERMS, ITERATE2,
    "Descriptive text followed by one or more filenames" },
{ "HeaderName", add_header, NULL, DIR_CMD_PERMS, TAKE1, "a filename" },
{ "ReadmeName", add_readme, NULL, DIR_CMD_PERMS, TAKE1, "a filename" },
{ "FancyIndexing", fancy_indexing, NULL, DIR_CMD_PERMS, FLAG, NULL },
{ "DefaultIcon", set_string_slot,
    (void*)XtOffsetOf(dir_config_rec, default_icon),
    DIR_CMD_PERMS, TAKE1, "an icon URL"},
{ "DirectoryIndex", set_string_slot,
    (void*)XtOffsetOf(dir_config_rec, index_names),
    DIR_CMD_PERMS, RAW_ARGS, NULL },
{ NULL }
};

void *create_dir_config (pool *p, char *dummy)
{
    dir_config_rec *new =
        (dir_config_rec *) pcalloc (p, sizeof(dir_config_rec));

    new->index_names = NULL;
    new->icon_list = make_array (p, 4, sizeof (struct item));
    new->alt_list = make_array (p, 4, sizeof (struct item));
    new->desc_list = make_array (p, 4, sizeof (struct item));
    new->ign_list = make_array (p, 4, sizeof (struct item));
    new->hdr_list = make_array (p, 4, sizeof (struct item));
    new->rdme_list = make_array (p, 4, sizeof (struct item));
    new->opts_list = make_array (p, 4, sizeof (struct item));
    
    return (void *)new;
}

void *merge_dir_configs (pool *p, void *basev, void *addv)
{
    dir_config_rec *new=(dir_config_rec*)pcalloc (p, sizeof(dir_config_rec));
    dir_config_rec *base = (dir_config_rec *)basev;
    dir_config_rec *add = (dir_config_rec *)addv;

    new->default_icon = add->default_icon?add->default_icon:base->default_icon;
    new->index_names = add->index_names? add->index_names: base->index_names;

    new->alt_list = append_arrays (p, add->alt_list, base->alt_list);
    new->ign_list = append_arrays (p, add->ign_list, base->ign_list);
    new->hdr_list = append_arrays (p, add->hdr_list, base->hdr_list);
    new->desc_list = append_arrays (p, add->desc_list, base->desc_list);
    new->icon_list = append_arrays (p, add->icon_list, base->icon_list);
    new->rdme_list = append_arrays (p, add->rdme_list, base->rdme_list);
    new->opts_list = append_arrays (p, add->opts_list, base->opts_list);
    
    return new;
}

/****************************************************************
 *
 * Looking things up in config entries...
 */

/* Structure used to hold entries when we're actually building an index */

struct ent {
    char *name;
    char *icon;
    char *alt;
    char *desc;
    size_t size;
    time_t lm;
    struct ent *next;
};

char *find_item(request_rec *r, array_header *list, int path_only) {
    char *content_type = r->content_type;
    char *content_encoding = r->content_encoding;
    char *path = r->filename;

    struct item *items = (struct item *)list->elts;
    int i;

    for (i = 0; i < list->nelts; ++i) {
        struct item *p = &items[i];
      
        /* Special cased for ^^DIRECTORY^^ and ^^BLANKICON^^ */
        if((path[0] == '^') || (!strcmp_match(path,p->apply_path))) {
            if(!*(p->apply_to))
                return p->data;
            else if(p->type == BY_PATH || path[0] == '^') {
                if(!strcmp_match(path,p->apply_to))
                    return p->data;
            } else if(!path_only) {
                if(!content_encoding) {
                    if(p->type == BY_TYPE) {
                        if(!strcmp_match(content_type,p->apply_to))
                            return p->data;
                    }
                } else {
                    if(p->type == BY_ENCODING) {
                        if(!strcmp_match(content_encoding,p->apply_to))
                            return p->data;
                    }
                }
            }
        }
    }
    return NULL;
}

#define find_icon(d,p,t) find_item(p,d->icon_list,t)
#define find_alt(d,p,t) find_item(p,d->alt_list,t)
#define find_desc(d,p) find_item(p,d->desc_list,0)
#define find_header(d,p) find_item(p,d->hdr_list,0)
#define find_readme(d,p) find_item(p,d->rdme_list,0)

char *find_default_icon (dir_config_rec *d, char *bogus_name)
{
    request_rec r;

    /* Bleah.  I tried to clean up find_item, and it lead to this bit
     * of ugliness.   Note that the fields initialized are precisely
     * those that find_item looks at...
     */
    
    r.filename = bogus_name;
    r.content_type = r.content_encoding = NULL;

    return find_item (&r, d->icon_list, 1);
}

int ignore_entry(dir_config_rec *d, char *path) {
    array_header *list = d->ign_list;
    struct item *items = (struct item *)list->elts;
    char *tt;
    int i;

    if((tt=strrchr(path,'/')) == NULL)
      tt=path;
    else {
      tt++;
    }

    for (i = 0; i < list->nelts; ++i) {
        struct item *p = &items[i];
	char *ap;

	if((ap=strrchr(p->apply_to,'/')) == NULL)
	  ap=p->apply_to;
	else
	  ap++;

        if(!strcmp_match(path,p->apply_path) && !strcmp_match(tt,ap))
	   return 1;
    }
    return 0;
}

int find_opts(dir_config_rec *d, request_rec *r) {
    char *path = r->filename;
    array_header *list = d->opts_list;
    struct item *items = (struct item *)list->elts;
    int i;

    for (i = 0; i < list->nelts; ++i) {
        struct item *p = &items[i];
	
        if(!strcmp_match(path,p->apply_path))
            return (int)p->type;
    }
    return 0;
}

/*****************************************************************
 *
 * Actually generating output
 */


int insert_readme(char *name, char *readme_fname, int rule, request_rec *r) {
    char *fn;
    FILE *f;
    struct stat finfo;
    int plaintext=0;

    fn = make_full_path(r->pool, name, readme_fname);
    fn = pstrcat(r->pool, fn, ".html", NULL);
    if(stat(fn,&finfo) == -1) {
        /* A brief fake multiviews search for README.html */
        fn[strlen(fn)-5] = '\0';
        if(stat(fn,&finfo) == -1)
            return 0;
        plaintext=1;
        if(rule) rprintf(r,"<HR>%c",LF);
        rprintf(r,"<PRE>%c",LF);
    }
    else if(rule) rprintf(r,"<HR>%c",LF);
    if(!(f = pfopen(r->pool,fn,"r")))
        return 0;
    send_fd(f, r);
    pfclose(r->pool, f);
    if(plaintext)
        rprintf(r,"</PRE>%c",LF);
    return 1;
}


char *find_title(request_rec *r) {
    char titlebuf[MAX_STRING_LEN], *find = "<TITLE>";
    FILE *thefile = NULL;
    int x,y,n,p;

    if ((!strcmp(r->content_type,"text/html")) && (!r->content_encoding)) {
        if(!(thefile = pfopen(r->pool, r->filename,"r")))
            return NULL;
        n = fread(titlebuf,sizeof(char),MAX_STRING_LEN - 1,thefile);
        titlebuf[n] = '\0';
        for(x=0,p=0;titlebuf[x];x++) {
            if(titlebuf[x] == find[p]) {
                if(!find[++p]) {
                    if((p = ind(&titlebuf[++x],'<')) != -1)
                        titlebuf[x+p] = '\0';
                    /* Scan for line breaks for Tanmoy's secretary */
                    for(y=x;titlebuf[y];y++)
                        if((titlebuf[y] == CR) || (titlebuf[y] == LF))
                            titlebuf[y] = ' ';
		    pfclose (r->pool, thefile);
                    return pstrdup(r->pool, &titlebuf[x]);
                }
            } else p=0;
        }
	pfclose(r->pool, thefile);
    }
    return NULL;
}


#ifdef NOTDEF

/* The only call to this function anyplace is commented out in the base
 * code below.  It could be fixed along the same lines as the shell_escape
 * stuff in util.c, but for now, why bother?
 */

void escape_html(char *fn) {
    register int x,y;
    char copy[MAX_STRING_LEN];

    strcpy(copy,fn);
    for(x=0,y=0;copy[y];x++,y++) {
        if(copy[y] == '<') {
            strcpy(&fn[x],"&lt;");
            x+=3;
        }
        else if(copy[y] == '>') {
            strcpy(&fn[x],"&gt;");
            x+=3;
        }
        else if(copy[y] == '&') {
            strcpy(&fn[x],"&amp;");
            x+=4;
        }
        else
            fn[x] = copy[y];
    }
    fn[x] = '\0';
}
#endif

struct ent *make_dir_entry(char *name, int dir_opts,
			   dir_config_rec *d, request_rec *r)
{
    struct ent *p;

    if((name[0] == '.') && (!name[1]))
        return(NULL);

    if (ignore_entry(d, make_full_path (r->pool, r->filename, name)))
        return(NULL);

    p=(struct ent *)pcalloc(r->pool, sizeof(struct ent));
    p->name = pstrdup (r->pool, name);
    p->size = -1;
    p->icon = NULL;
    p->alt = NULL;
    p->desc = NULL;
    p->lm = -1;

    if(dir_opts & FANCY_INDEXING) {
        request_rec *rr = sub_req_lookup_file (name, r);
	
	if (rr->finfo.st_mode != 0) {
            p->lm = rr->finfo.st_mtime;
            if(S_ISDIR(rr->finfo.st_mode)) {
                if(!(p->icon = find_icon(d,rr,1)))
                    p->icon = find_default_icon(d,"^^DIRECTORY^^");
                if(!(p->alt = find_alt(d,rr,1)))
                    p->alt = "DIR";
                p->size = -1;
		p->name = pstrcat (r->pool, name, "/", NULL);
            }
            else {
                p->icon = find_icon(d, rr, 0);
                p->alt = find_alt(d, rr, 0);
                p->size = rr->finfo.st_size;
            }
        }
	
        p->desc = find_desc(d, rr);
	
        if((!p->desc) && (dir_opts & SCAN_HTML_TITLES))
            p->desc = pstrdup (r->pool, find_title(rr));

	destroy_sub_req (rr);
    }
    return(p);
}

char *terminate_description(dir_config_rec *d, char *desc, int dir_opts) {
    int maxsize = 23;
    register int x;
    
    if(dir_opts & SUPPRESS_LAST_MOD) maxsize += 17;
    if(dir_opts & SUPPRESS_SIZE) maxsize += 7;

    for(x=0;desc[x] && maxsize;x++) {
        if(desc[x] == '<') {
            while(desc[x] != '>') {
                if(!desc[x]) {
                    maxsize = 0;
                    break;
                }
                ++x;
            }
        }
        else --maxsize;
    }
    if(!maxsize) {
        desc[x-1] = '>';	/* Grump. */
	desc[x] = '\0';		/* Double Grump! */
    }
    return desc;
}

void output_directories(struct ent **ar, int n,
			dir_config_rec *d, request_rec *r, int dir_opts)
{
    int x;
    char *name = r->uri;
    char *tp;
    pool *scratch = make_sub_pool (r->pool);
    
    if(name[0] == '\0') name = "/";

    if(dir_opts & FANCY_INDEXING) {
        rprintf (r, "<PRE>");
        if((tp = find_default_icon(d,"^^BLANKICON^^")))
            rprintf(r, "<IMG SRC=\"%s\" ALT=\"     \"> ", tp);
        rprintf (r, "Name                   ");
        if(!(dir_opts & SUPPRESS_LAST_MOD))
            rprintf(r, "Last modified     ");
        if(!(dir_opts & SUPPRESS_SIZE))
            rprintf(r, "Size  ");
        if(!(dir_opts & SUPPRESS_DESC))
            rprintf(r, "Description");
        rprintf(r,"%c<HR>%c",LF,LF);
    }
    else {
        rprintf (r, "<UL>");
    }
        
    for(x=0;x<n;x++) {
	char *anchor = NULL, *t = NULL, *t2 = NULL;
	
	clear_pool (scratch);
	
        if((!strcmp(ar[x]->name,"../")) || (!strcmp(ar[x]->name,".."))) {
            char *t = make_full_path (scratch, name, "../");
            getparents(t);
            if(t[0] == '\0') t = "/";
	    anchor = pstrcat (scratch, "<A HREF=\"", escape_uri(scratch, t),
			      "\">", NULL);
	    t2 = "Parent Directory</A>";
        }
        else {
	    t = ar[x]->name;
	    t2 = pstrdup(scratch, t);
            if(strlen(t2) > 21) {
                t2[21] = '\0';
		t2 = pstrcat (scratch, t2, "</A>..", NULL);
            } else 
            /* screws up formatting, but some need it. should fix. */
	    /* escape_html(t2); */
            t2 = pstrcat(scratch, t2, "</A>", NULL);
	    anchor = pstrcat (scratch, "<A HREF=\"", escape_uri(scratch, t),
			      "\">", NULL);
        }

        if(dir_opts & FANCY_INDEXING) {
            if(dir_opts & ICONS_ARE_LINKS)
                rprintf (r,"%s",anchor);
            if((ar[x]->icon) || d->default_icon) {
                rprintf(r, "<IMG SRC=\"%s\" ALT=\"[%s]\">",
			ar[x]->icon ? ar[x]->icon : d->default_icon,
			ar[x]->alt ? ar[x]->alt : "   ");
            }
            if(dir_opts & ICONS_ARE_LINKS) 
                rprintf (r, "</A>");

            rprintf(r," %s%-27.29s", anchor, t2);
            if(!(dir_opts & SUPPRESS_LAST_MOD)) {
                if(ar[x]->lm != -1) {
		    char time[MAX_STRING_LEN];
                    struct tm *ts = localtime(&ar[x]->lm);
                    strftime(time,MAX_STRING_LEN,"%d-%b-%y %H:%M  ",ts);
		    rprintf (r, "%s", time);
                }
                else {
                    rprintf(r, "                 ");
                }
            }
            if(!(dir_opts & SUPPRESS_SIZE)) {
                send_size(ar[x]->size,r);
                rprintf (r, "  ");
            }
            if(!(dir_opts & SUPPRESS_DESC)) {
                if(ar[x]->desc) {
                    rprintf(r,"%s",
			    terminate_description(d, ar[x]->desc, dir_opts));
                }
            }
        }
        else
            rprintf(r, "<LI> %s %s",anchor,t2);
        rputc(LF,r);
    }
    if(dir_opts & FANCY_INDEXING) {
        rprintf(r, "</PRE>");
    }
    else {
        rprintf(r, "</UL>");
    }
}


int dsortf(struct ent **s1,struct ent **s2)
{
    return(strcmp((*s1)->name,(*s2)->name));
}

    
int index_directory(request_rec *r, dir_config_rec *dir_conf)
{
    char *title_name = pstrdup (r->pool, r->uri);
    char *title_endp;
    char *name = r->filename;
    
    DIR *d;
    struct DIR_TYPE *dstruct;
    int num_ent=0,x;
    struct ent *head,*p;
    struct ent **ar;
    char *tmp;
    int dir_opts = find_opts(dir_conf, r);

    if(!(d=opendir(name))) return FORBIDDEN;

    r->content_type = "text/html";
    
    soft_timeout ("send directory", r);
    send_http_header(r);

    if (r->header_only) {
	closedir (d);
	return 0;
    }

    /* Spew HTML preamble */
    
    title_endp = title_name + strlen(title_name) - 1;

    while (title_endp > title_name && *title_endp == '/')
	*title_endp-- = '\0';
    
    rprintf (r,"<HEAD><TITLE>Index of %s</TITLE></HEAD><BODY>%c",
	     title_name, LF);

    if((!(tmp = find_header(dir_conf,r))) || (!(insert_readme(name,tmp,0,r))))
        rprintf(r,"<H1>Index of %s</H1>%c",title_name,LF);

    /* 
     * Since we don't know how many dir. entries there are, put them into a 
     * linked list and then arrayificate them so qsort can use them. 
     */
    head=NULL;
    while((dstruct=readdir(d))) {
        if((p = make_dir_entry(dstruct->d_name, dir_opts, dir_conf, r))) {
            p->next=head;
            head=p;
            num_ent++;
        }
    }
    ar=(struct ent **) palloc(r->pool, num_ent*sizeof(struct ent *));
    p=head;
    x=0;
    while(p) {
        ar[x++]=p;
        p = p->next;
    }
    
    qsort((void *)ar,num_ent,sizeof(struct ent *),
#ifdef ULTRIX_BRAIN_DEATH
          (int (*))dsortf);
#else
          (int (*)(const void *,const void *))dsortf);
#endif
     output_directories(ar, num_ent, dir_conf, r, dir_opts);
     closedir(d);

     if (dir_opts & FANCY_INDEXING)
         if((tmp = find_readme(dir_conf, r)))
             insert_readme(name,tmp,1,r);
     else {
         rprintf(r, "</UL>");
     }

     rprintf(r, "</BODY>");
     return 0;
}

/* The formal handler... */

int handle_dir (request_rec *r)
{
    dir_config_rec *d =
      (dir_config_rec *)get_module_config (r->per_dir_config, &dir_module);
    char *names_ptr = d->index_names ? d->index_names : DEFAULT_INDEX;
    int allow_opts = allow_options (r);

    if (r->method_number != M_GET) return NOT_IMPLEMENTED;
    
    if (!r->path_info || *r->path_info != '/') {
        char* ifile = pstrcat (r->pool, r->uri, "/", NULL);
	table_set (r->headers_out, "Location",
		   construct_url(r->pool, ifile, r->server));
	return REDIRECT;
    }

    /* KLUDGE --- make the sub_req lookups happen in the right directory.
     * Fixing this in the sub_req_lookup functions themselves is difficult,
     * and would probably break virtual includes...
     */

    r->filename = pstrcat (r->pool, r->filename, "/", NULL);
    
    while (*names_ptr) {
          
	char *name_ptr = getword_conf (r->pool, &names_ptr);
	request_rec *rr = sub_req_lookup_uri (name_ptr, r);
           
	if (rr->status == 200 && rr->finfo.st_mode != 0) {
            char *new_uri = pstrdup (r->pool, rr->uri);
	    destroy_sub_req (rr);
	    internal_redirect (new_uri, r);
	    return OK;
	}

        destroy_sub_req (rr);
    }

    /* OK, nothing easy.  Trot out the heavy artillery... */

    if (allow_opts & OPT_INDEXES) 
        return index_directory (r, d);
    else
        return FORBIDDEN;
}


handler_rec dir_handlers[] = {
{ DIR_MAGIC_TYPE, handle_dir },
{ NULL }
};

module dir_module = {
   STANDARD_MODULE_STUFF,
   NULL,			/* initializer */
   create_dir_config,		/* dir config creater */
   merge_dir_configs,		/* dir merger --- default is to override */
   NULL,			/* server config */
   NULL,			/* merge server config */
   dir_cmds,			/* command table */
   dir_handlers,		/* handlers */
   NULL,			/* filename translation */
   NULL,			/* check_user_id */
   NULL,			/* check auth */
   NULL,			/* check access */
   NULL,			/* type_checker */
   NULL,			/* fixups */
   NULL				/* logger */
};
