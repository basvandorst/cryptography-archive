
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
 * str.c: string utility things
 * 
 * 3/21/93 Rob McCool
 * 
 */


#include "httpd.h"
#include "http_conf_globals.h"	/* for user_id & group_id */

#ifdef NOTDEF
extern char** environ;

/* taken from bdflush-1.5 for Linux source code */
void inststr(char *dst[], int argc, char *src)
{
    if (strlen(src) <= strlen(dst[0]))
    {
        char *ptr;

        for (ptr = dst[0]; *ptr; *(ptr++) = '\0');

        strcpy(dst[0], src);
    } else
    {
        /* stolen from the source to perl 4.036 (assigning to $0) */
        char *ptr, *ptr2;
        int count;
        ptr = dst[0] + strlen(dst[0]);
        for (count = 1; count < argc; count++) {
            if (dst[count] == ptr + 1)
                ptr += strlen(++ptr);
        }
        if (environ[0] == ptr + 1) {
            for (count = 0; environ[count]; count++)
                if (environ[count] == ptr + 1)
                    ptr += strlen(++ptr);
        }
        count = 0;
        for (ptr2 = dst[0]; ptr2 <= ptr; ptr2++) {
            *ptr2 = '\0';
            count++;
        }
        strncpy(dst[0], src, count);
    }
}
#endif

char *get_time() {
    time_t t;
    char *time_string;

    t=time(NULL);
    time_string = ctime(&t);
    time_string[strlen(time_string) - 1] = '\0';
    return (time_string);
}

char *ht_time(pool *p, time_t t, char *fmt, int gmt) {
    char ts[MAX_STRING_LEN];
    struct tm *tms;

    tms = (gmt ? gmtime(&t) : localtime(&t));

    /* check return code? */
    strftime(ts,MAX_STRING_LEN,fmt,tms);
    return pstrdup (p, ts);
}

char *gm_timestr_822(pool *p, time_t sec) {
    return ht_time(p, sec, HTTP_TIME_FORMAT, 1);
}

/* What a pain in the ass. */
struct tm *get_gmtoff(long *tz) {
    time_t tt;
    struct tm *t;

    tt = time(NULL);
    t = localtime(&tt);
#if defined(HAS_GMTOFF)
    *tz = t->tm_gmtoff;
#else
    *tz = - timezone;
    if(t->tm_isdst)
        *tz += 3600;
#endif
    return t;
}


/* Match = 0, NoMatch = 1, Abort = -1 */
/* Based loosely on sections of wildmat.c by Rich Salz
 * Hmmm... shouldn't this really go component by component?
 */
int strcmp_match(char *str, char *exp) {
    int x,y;

    for(x=0,y=0;exp[y];++y,++x) {
        if((!str[x]) && (exp[y] != '*'))
            return -1;
        if(exp[y] == '*') {
            while(exp[++y] == '*');
            if(!exp[y])
                return 0;
            while(str[x]) {
                int ret;
                if((ret = strcmp_match(&str[x++],&exp[y])) != 1)
                    return ret;
            }
            return -1;
        } else 
            if((exp[y] != '?') && (str[x] != exp[y]))
                return 1;
    }
    return (str[x] != '\0');
}

int is_matchexp(char *str) {
    register int x;

    for(x=0;str[x];x++)
        if((str[x] == '*') || (str[x] == '?'))
            return 1;
    return 0;
}

/*
 * Parse .. so we don't compromise security
 */
void getparents(char *name)
{
    int l=0,w=0;
    const char *lookfor="..";

    while(name[l]!='\0') {
        if(name[l]!=lookfor[w]) (w>0 ? (l-=(w-1),w=0) : l++);
        else {
            if(lookfor[++w]=='\0') {
                if((name[l+1]=='\0') ||
		   ((name[l+1]=='/') &&
		    (((l > 3) && (name[l-2] == '/'))
		     || (l<=3)))) {
                    register int m=l+1,n;

                    l=l-3;
                    if(l>=0) {
                        while((l!=0) && (name[l]!='/')) --l;
                    }
                    else l=0;
                    n=l;
                    while((name[n]=name[m])) (++n,++m);
                    w=0;
                }
                else w=0;
            }
            else ++l;
        }
    }
}

void no2slash(char *name) {
    register int x,y;

    for(x=0; name[x]; x++)
        if(x && (name[x-1] == '/') && (name[x] == '/'))
            for(y=x+1;name[y-1];y++)
                name[y-1] = name[y];
}

char *make_dirstr(pool *p, char *s, int n) {
    register int x,f;
    char *res;

    for(x=0,f=0;s[x];x++) {
        if(s[x] == '/')
            if((++f) == n) {
		res = palloc(p, x + 2);
		strncpy (res, s, x);
		res[x] = '/';
		res[x+1] = '\0';
                return res;
            }
    }

    if (s[strlen(s) - 1] == '/')
        return pstrdup (p, s);
    else
        return pstrcat (p, s, "/", NULL);
}

int count_dirs(char *path) {
    register int x,n;

    for(x=0,n=0;path[x];x++)
        if(path[x] == '/') n++;
    return n;
}


void chdir_file(char *file) {
    int i;

    if((i = rind(file,'/')) == -1)
        return;
    file[i] = '\0';
    chdir(file);
    file[i] = '/';
}

char *getword(pool* atrans, char **line, char stop) {
    int pos = ind(*line, stop);
    char *res;

    if (pos == -1) {
        res = pstrdup (atrans, *line);
	*line += strlen (*line);
	return res;
    }
  
    res = palloc(atrans, pos + 1);
    strncpy (res, *line, pos);
    res[pos] = '\0';
    
    while ((*line)[pos] == stop) ++pos;
    
    *line += pos;
    
    return res;
}

/* Get a word, (new) config-file style --- quoted strings and backslashes
 * all honored
 */

char *substring_conf (pool *p, char *start, int len)
{
    char *result = palloc (p, len + 2);
    char *resp = result;
    int i;

    for (i = 0; i < len; ++i) {
        if (start[i] == '\\') 
	    *resp++ = start[++i];
	else
	    *resp++ = start[i];
    }

    *resp++ = '\0';
    return result;
}

char *getword_conf(pool* p, char **line) {
    char *str = *line, *strend, *res;
    char quote;

    while (*str && isspace (*str))
        ++str;

    if (!*str) {
        *line = str;
        return "";
    }

    if ((quote = *str) == '"' || quote == '\'') {
        strend = str + 1;
	while (*strend && *strend != quote) {
	    if (*strend == '\\' && strend[1]) strend += 2;
	    else ++strend;
	}
	res = substring_conf (p, str + 1, strend - str - 1);

	if (*strend == quote) ++strend;
    } else {
        strend = str;
	while (*strend && !isspace (*strend))
	    if (*strend == '\\' && strend[1]) strend += 2;
	    else ++strend;

	res = substring_conf (p, str, strend - str);
    }

    while (*strend && isspace(*strend)) ++ strend;
    *line = strend;
    return res;
}

void cfg_getword(char *word, char *line) {
    int x=0,y;
    
    for(x=0;line[x] && isspace(line[x]);x++);
    y=0;
    while(1) {
        if(!(word[y] = line[x]))
            break;
        if(isspace(line[x]))
            if((!x) || (line[x-1] != '\\'))
                break;
        if(line[x] != '\\') ++y;
        ++x;
    }
    word[y] = '\0';
    while(line[x] && isspace(line[x])) ++x;
    for(y=0;(line[y] = line[x]);++x,++y);
}

int
cfg_getline(char *s, int n, FILE *f) {
    register int i=0, c;

    s[0] = '\0';
    /* skip leading whitespace */
    do {
        c = getc(f);
    } while (c == '\t' || c == ' ');

    while(1) {
        if((c == '\t') || (c == ' ')) {
            s[i++] = ' ';
            while((c == '\t') || (c == ' ')) 
                c = getc(f);
        }
        if(c == CR) {
            c = getc(f);
        }
        if(c == EOF || c == 0x4 || c == LF || i == (n-1)) {
            /* blast trailing whitespace */
            while(i && (s[i-1] == ' ')) --i;
            s[i] = '\0';
            return (feof(f) ? 1 : 0);
        }
        s[i] = c;
        ++i;
        c = getc(f);
    }
}

char *escape_shell_cmd(pool *p, char *s) {
    register int x,y,l;
    char *cmd;

    l=strlen(s);
    cmd = palloc (p, 2 * l + 1); /* Be safe */
    strcpy (cmd, s);
    
    for(x=0;cmd[x];x++) {
        if(ind("&;`'\"|*?~<>^()[]{}$\\",cmd[x]) != -1){
            for(y=l+1;y>x;y--)
                cmd[y] = cmd[y-1];
            l++; /* length has been increased */
            cmd[x] = '\\';
            x++; /* skip the character */
        }
    }

    return cmd;
}

void plustospace(char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}

void spacetoplus(char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == ' ') str[x] = '+';
}

char x2c(char *what) {
    register char digit;

    digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void unescape_url(char *url) {
    register int x,y;

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}

char *construct_url(pool *p, char *uri, server_rec *s) {
    char portnum[10];		/* Long enough.  Really! */
  
    if (s->port == 80) {
        return pstrcat (p, "http://", s->server_hostname, uri, NULL);
    } else {
        sprintf (portnum, "%d", s->port);
	return pstrcat (p, "http://", s->server_hostname, ":", portnum, uri,
			NULL);
    }
}

#define c2x(what,where) sprintf(where,"%%%2x",what)

char *escape_uri(pool *p, char *uri) {
    register int x,y;
    char *copy = palloc (p, 3 * strlen (uri) + 1);
            
    for(x=0,y=0; uri[x]; x++,y++) {
        if (ind (":% ?+&",(copy[y] = uri[x])) != -1) {
            c2x(uri[x],&copy[y]);
            y+=2;
        }
    }
    copy[y] = '\0';
    return copy;
}

#ifdef NOTDEF

void escape_url(char *url) {
    register int x,y;
    register char digit;
    char *copy;

    copy = strdup(url);
            
    for(x=0,y=0;copy[x];x++,y++) {
        if(ind("% ?+&",url[y] = copy[x]) != -1) {
            c2x(copy[x],&url[y]);
            y+=2;
        }
    }
    url[y] = '\0';
    free(copy);
}

#endif

int is_directory(char *path) {
    struct stat finfo;

    if(stat(path,&finfo) == -1)
        return 0; /* in error condition, just return no */

    return(S_ISDIR(finfo.st_mode));
}

char *make_full_path(pool *a, char *src1,char *src2) {
    register int x;

    x = strlen(src1);
    if (x == 0) return pstrcat (a, "/", src2, NULL);

    if (src1[x - 1] != '/') return pstrcat (a, src1, "/", src2, NULL);
    else return pstrcat (a, src1, src2, NULL);
}

int is_url(char *u) {
    register int x;

    for(x=0;u[x] != ':';x++)
        if((!u[x]) || (!isalpha(u[x])))
            return 0;

    if((u[x+1] == '/') && (u[x+2] == '/'))
        return 1;
    else return 0;
}

int can_exec(struct stat *finfo) {
    if(user_id == finfo->st_uid)
        if(finfo->st_mode & S_IXUSR)
            return 1;
    if(group_id == finfo->st_gid)
        if(finfo->st_mode & S_IXGRP)
            return 1;
    return (finfo->st_mode & S_IXOTH);
}

#ifdef NEED_STRDUP
char *strdup (char *str)
{
  char *dup;

  if(!(dup = (char *)malloc (strlen (str) + 1)))
      return NULL;
  dup = strcpy (dup, str);

  return dup;
}
#endif

/* The following two routines were donated for SVR4 by Andreas Vogel */
#ifdef NEED_STRCASECMP
int strcasecmp (const char *a, const char *b)
{
    const char *p = a;
    const char *q = b;
    for (p = a, q = b; *p && *q; p++, q++)
    {
      int diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    if (*p) return 1;       /* p was longer than q */
    if (*q) return -1;      /* p was shorter than q */
    return 0;               /* Exact match */
}

#endif

#ifdef NEED_STRNCASECMP
int strncasecmp (const char *a, const char *b, int n)
{
    const char *p = a;
    const char *q = b;

    for (p = a, q = b; /*NOTHING*/; p++, q++)
    {
      int diff;
      if (p == a + n) return 0;     /*   Match up to n characters */
      if (!(*p && *q)) return *p - *q;
      diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    /*NOTREACHED*/
}
#endif



#ifdef NEED_INITGROUPS
int initgroups(const char *name, gid_t basegid)
{
  gid_t groups[NGROUPS_MAX];
  struct group *g;
  int index = 0;

  groups[index++] = basegid;

  while (index < NGROUPS_MAX && ((g = getgrent()) != NULL))
    if (g->gr_gid != basegid)
    {
      char **names;

      for (names = g->gr_mem; *names != NULL; ++names)
        if (!strcmp(*names, name))
          groups[index++] = g->gr_gid;
    }

  return setgroups(index, groups);
}
#endif

#ifdef NEED_WAITPID
/* From ikluft@amdahl.com */
/* this is not ideal but it works for SVR3 variants */
/* httpd does not use the options so this doesn't implement them */
int waitpid(pid_t pid, int *statusp, int options)
{
    int tmp_pid;
    if ( kill ( pid,0 ) == -1) {
        errno=ECHILD;
        return -1;
    }
    while ((( tmp_pid = wait(statusp)) != pid) && ( tmp_pid != -1 ));
    return tmp_pid;
}
#endif

int ind(const char *s, char c) {
    register int x;

    for(x=0;s[x];x++)
        if(s[x] == c) return x;

    return -1;
}

int rind(const char *s, char c) {
    register int x;

    for(x=strlen(s)-1;x != -1;x--)
        if(s[x] == c) return x;

    return -1;
}

void str_tolower(char *str) {
    while(*str) {
        *str = tolower(*str);
        ++str;
    }
}
        
uid_t uname2id(char *name) {
    struct passwd *ent;

    if(name[0] == '#') 
        return(atoi(&name[1]));

    if(!(ent = getpwnam(name))) {
        fprintf(stderr,"httpd: bad user name %s\n",name);
        exit(1);
    }
    else return(ent->pw_uid);
}

gid_t gname2id(char *name) {
    struct group *ent;

    if(name[0] == '#') 
        return(atoi(&name[1]));

    if(!(ent = getgrnam(name))) {
        fprintf(stderr,"httpd: bad group name %s\n",name);
        exit(1);
    }
    else return(ent->gr_gid);
}

int get_portnum(int sd) {
    struct sockaddr addr;
    int len;

    len = sizeof(struct sockaddr);
    if(getsockname(sd,&addr,&len) < 0)
        return -1;
    return ntohs(((struct sockaddr_in *)&addr)->sin_port);
}

struct in_addr get_local_addr(int sd) {
    struct sockaddr addr;
    int len;

    len = sizeof(struct sockaddr);
    if(getsockname(sd,&addr,&len) < 0) {
        fprintf (stderr, "Can't get local host address!\n");
	perror ("getsockname");
	exit(1);
    }
         
    return ((struct sockaddr_in *)&addr)->sin_addr;
}

unsigned long get_virthost_addr (char *w, int wild_allowed) {
    struct hostent *hep;
    unsigned long my_addr;
    
    if (wild_allowed && !strcmp(w, "*")) 
	return htonl(INADDR_ANY);
	
    my_addr = inet_addr(w);
    if (my_addr != ((unsigned long) 0xffffffff))
	return my_addr;

    hep = gethostbyname(w);
	    
    if ((!hep) || (hep->h_addrtype != AF_INET || !hep->h_addr_list[0])) {
	fprintf (stderr, "Cannot resolve host name %s --- exiting!\n", w);
	exit(1);
    }
	    
    if (hep->h_addr_list[1]) {
	fprintf(stderr, "Host %s has multiple addresses ---\n", w);
	fprintf(stderr, "you must choose one explicitly for use as\n");
	fprintf(stderr, "a virtual host.  Exiting!!!\n");
	exit(1);
    }
	    
    return ((struct in_addr *)(hep->h_addr))->s_addr;
}

#ifdef DEBUG_ACCESS
/* Host name and IP address used if getpeername() fails.
 * The most frequent cause of this is running the server in inetd-mode
 * with the tty as input and output, under the debugger.
 */
#define NO_SUCH_HOST_NAME "volterra.ai.mit.edu"
#define NO_SUCH_HOST_ADDR "128.52.39.173"
#else
#define NO_SUCH_HOST_NAME "UNKNOWN_HOST"
#define NO_SUCH_HOST_ADDR "UNKNOWN_IP"
#endif

void get_remote_host(conn_rec *conn)
{
    struct sockaddr addr;
    int len;
    struct in_addr *iaddr;
#if defined(MAXIMUM_DNS) || !defined(MINIMAL_DNS)
    struct hostent *hptr;
#endif

    len = sizeof(struct sockaddr);

    if ((getpeername(fileno(conn->client), &addr, &len)) < 0) {
	conn->remote_name = conn->remote_host = NO_SUCH_HOST_NAME;
	conn->remote_ip = NO_SUCH_HOST_ADDR;
        return;
    }

    iaddr = &(((struct sockaddr_in *)&addr)->sin_addr);
#ifndef MINIMAL_DNS
    hptr = gethostbyaddr((char *)iaddr, sizeof(struct in_addr), AF_INET);
    if(hptr) {
        conn->remote_host = pstrdup(conn->pool, (void *)hptr->h_name);
        conn->remote_name = conn->remote_host;
        str_tolower (conn->remote_host);
    }
    else 
#endif
        conn->remote_host = NULL;

#ifdef MAXIMUM_DNS
    /* Grrr. Check THAT name to make sure it's really the name of the addr. */
    /* Code from Harald Hanche-Olsen <hanche@imf.unit.no> */
    if (conn->remote_host) {
        char **haddr;

        hptr = gethostbyname(conn->remote_host);
        if (hptr) {
            for(haddr=hptr->h_addr_list;*haddr;haddr++) {
                if(((struct in_addr *)(*haddr))->s_addr == iaddr->s_addr)
                    break;
            }
        }
        if((!hptr) || (!(*haddr)))
	    conn->remote_host = conn->remote_name = NULL;
    }
#endif
    conn->remote_ip = pstrdup (conn->pool, inet_ntoa(*iaddr));
    if(!conn->remote_host){ 	
        conn->remote_name = conn->remote_ip;
    }
    if (!conn->remote_name) {
	conn->remote_name = "UNKNOWN_HOST";
    }
}

#ifdef NOTDEF    
    
char *get_remote_logname(FILE *fd) {
    int len;
    char *result;
#if defined(NEXT) || defined(BSD4_4) || defined(SOLARIS2) || defined(LINUX)
    struct sockaddr sa_server, sa_client;
#else
    struct sockaddr_in sa_server,sa_client;
#endif

    len = sizeof(sa_client);
    if(getpeername(fileno(stdout),&sa_client,&len) != -1) {
        len = sizeof(sa_server);
        if(getsockname(fileno(stdout),&sa_server,&len) == -1)
            result = "unknown";
        else
            result = rfc931((struct sockaddr_in *) & sa_client,
                                    (struct sockaddr_in *) & sa_server);
    }
    else result = "unknown";

    return result; /* robm=pinhead */
}
#endif    

static char *find_fqdn(pool *a, struct hostent *p) {
    int x;

    if(ind(p->h_name,'.') == -1) {
        for(x=0;p->h_aliases[x];++x) {
            if((ind(p->h_aliases[x],'.') != -1) && 
               (!strncmp(p->h_aliases[x],p->h_name,strlen(p->h_name))))
                return pstrdup(a, p->h_aliases[x]);
        }
        return NULL;
    } else return pstrdup(a, (void *)p->h_name);
}

char *get_local_host(pool *a)
{
    char str[128];
    int len = 128;
    char *server_hostname;

    struct hostent *p;
    gethostname(str, len);
    if((!(p=gethostbyname(str))) || (!(server_hostname = find_fqdn(a, p)))) {
        fprintf(stderr,"httpd: cannot determine local host name.\n");
	fprintf(stderr,"Use ServerName to set it manually.\n");
	exit(1);
    }

    return server_hostname;
}

/* aaaack but it's fast and const should make it shared text page. */
const int pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char *uudecode(pool *p, char *bufcoded) {
    int nbytesdecoded;
    register unsigned char *bufin;
    register char *bufplain;
    register unsigned char *bufout;
    register int nprbytes;
    
    /* Strip leading whitespace. */
    
    while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;
    
    /* Figure out how many characters are in the input buffer.
     * Allocate this many from the per-transaction pool for the result.
     */
    bufin = (unsigned char *)bufcoded;
    while(pr2six[*(bufin++)] <= 63);
    nprbytes = (char *)bufin - bufcoded - 1;
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    bufplain = palloc(p, nbytesdecoded + 1);
    bufout = (unsigned char *)bufplain;
    
    bufin = (unsigned char *)bufcoded;
    
    while (nprbytes > 0) {
        *(bufout++) = 
            (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) = 
            (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) = 
            (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }
    
    if(nprbytes & 03) {
        if(pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }
    bufplain[nbytesdecoded] = '\0';
    return bufplain;
}
