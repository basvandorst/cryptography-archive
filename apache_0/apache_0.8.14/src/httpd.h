 
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
 * httpd.h: header for simple (ha! not anymore) http daemon
 */

/* Headers in which EVERYONE has an interest... */

#include "conf.h"
#include "alloc.h"

/* ----------------------------- config dir ------------------------------ */

/* Define this to be the default server home dir. Anything later in this
 * file with a relative pathname will have this added.
 */
#define HTTPD_ROOT "/usr/local/etc/httpd"

/* Root of server */
#define DOCUMENT_LOCATION "/usr/local/etc/httpd/htdocs"

/* Max. number of dynamically loaded modules */
#define DYNAMIC_MODULE_LIMIT 64

/* Max. number of security defines */
#define MAX_SECURITY 50

/* Default administrator's address */
#define DEFAULT_ADMIN "[no address given]"

/* 
 * --------- You shouldn't have to edit anything below this line ----------
 *
 * Any modifications to any defaults not defined above should be done in the 
 * respective config. file. 
 *
 */


/* -------------- Port number for server running standalone --------------- */

#define DEFAULT_PORT 80

/* --------- Default user name and group name running standalone ---------- */
/* --- These may be specified as numbers by placing a # before a number --- */

#define DEFAULT_USER "#-1"
#define DEFAULT_GROUP "#-1"

/* The name of the log files */
#define DEFAULT_XFERLOG "logs/access_log"
#define DEFAULT_ERRORLOG "logs/error_log"
#define DEFAULT_PIDLOG "logs/httpd.pid"

/* Define this to be what your HTML directory content files are called */
#define DEFAULT_INDEX "index.html"

/* Define this to 1 if you want fancy indexing, 0 otherwise */
#define DEFAULT_INDEXING 0

/* Define this to be what type you'd like returned for files with unknown */
/* suffixes */
#define DEFAULT_TYPE "text/html"

/* Define this to be what your per-directory security files are called */
#define DEFAULT_ACCESS_FNAME ".htaccess"

/* The name of the server config file */
#define SERVER_CONFIG_FILE "conf/httpd.conf"

/* The name of the document config file */
#define RESOURCE_CONFIG_FILE "conf/srm.conf"

/* The name of the MIME types file */
#define TYPES_CONFIG_FILE "conf/mime.types"

/* The name of the access file */
#define ACCESS_CONFIG_FILE "conf/access.conf"

/* Whether we should enable rfc931 identity checking */
#define DEFAULT_RFC931 0
/* The default directory in user's home dir */
#define DEFAULT_USER_DIR "public_html"

/* The default path for CGI scripts if none is currently set */
#define DEFAULT_PATH "/bin:/usr/bin:/usr/ucb:/usr/bsd:/usr/local/bin"

/* The path to the Bourne shell, for parsed docs */
#define SHELL_PATH "/bin/sh"

/* The default string lengths */
#define MAX_STRING_LEN HUGE_STRING_LEN
#define HUGE_STRING_LEN 8192

/* The timeout for waiting for messages */
#define DEFAULT_TIMEOUT 1200

/* The size of the server's internal read-write buffers */
#define IOBUFSIZE 8192

/* The number of header lines we will accept from a client */
#define MAX_HEADERS 200

/* RFC 1123 format for date - this is what HTTP/1.0 wants */
#define HTTP_TIME_FORMAT "%a, %d %b %Y %T GMT"

/* Number of servers to spawn off by default --- also, if fewer than
 * this free when the caretaker checks, it will spawn more.
 */
#define DEFAULT_START_DAEMON 5

/* Maximum number of *free* server processes --- more than this, and
 * they will die off.
 */

#define DEFAULT_MAX_FREE_DAEMON 10

/* Minimum --- fewer than this, and more will be created */

#define DEFAULT_MIN_FREE_DAEMON 5

/* Limit on the total --- clients will be locked out if more servers than
 * this are needed.  It is intended solely to keep the server from crashing
 * when things get out of hand.
 */

#define DEFAULT_SERVER_LIMIT 150

/* Number of requests to try to handle in a single process.  If <= 0,
 * the children don't die off.  That's the default here, since I'm still
 * interested in finding and stanching leaks.
 */

#define DEFAULT_MAX_REQUESTS_PER_CHILD 0

/* ------------------------------ error types ------------------------------ */

#define SERVER_VERSION "Apache/0.8.14"
#define SERVER_PROTOCOL "HTTP/1.0"
#define SERVER_SUPPORT "http://www.apache.org/apache/"

#define DECLINED -1		/* Module declines to handle */
#define OK 0			/* Module has handled this stage. */

#define DOCUMENT_FOLLOWS 200
#define REDIRECT 302
#define USE_LOCAL_COPY 304
#define BAD_REQUEST 400
#define AUTH_REQUIRED 401
#define FORBIDDEN 403
#define NOT_FOUND 404
#define SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define SERVICE_UNAVAILABLE 503
#define RESPONSE_CODES 10

#define METHODS 5
#define M_GET 0
#define M_PUT 1
#define M_POST 2
#define M_DELETE 3
#define M_INVALID 4

#define CGI_MAGIC_TYPE "application/x-httpd-cgi"
#define INCLUDES_MAGIC_TYPE "text/x-server-parsed-html"
#define INCLUDES_MAGIC_TYPE3 "text/x-server-parsed-html3"
#define MAP_FILE_MAGIC_TYPE "application/x-type-map"
#define ASIS_MAGIC_TYPE "httpd/send-as-is"
#define DIR_MAGIC_TYPE "httpd/unix-directory"

/* Just in case your linefeed isn't the one the other end is expecting. */
#define LF 10
#define CR 13

/* Things which may vary per file-lookup WITHIN a request ---
 * e.g., state of MIME config.  Basically, the name of an object, info
 * about the object, and any other info we may ahve which may need to
 * change as we go poking around looking for it (e.g., overridden by
 * .htaccess files).
 *
 * Note how the default state of almost all these things is properly
 * zero, so that allocating it with pcalloc does the right thing without
 * a whole lot of hairy initialization... so long as we are willing to
 * make the (fairly) portable assumption that the bit pattern of a NULL
 * pointer is, in fact, zero.
 */

typedef struct conn_rec conn_rec;
typedef struct server_rec server_rec;
typedef struct request_rec request_rec;

struct request_rec {

  pool *pool;
  conn_rec *connection;
  server_rec *server;

  request_rec *next;		/* If we wind up getting redirected,
				 * pointer to the request we redirected to.
				 */
  request_rec *prev;		/* If this is an internal redirect,
				 * pointer to where we redirected *from*.
				 */
  
  request_rec *main;		/* If this is a sub_request (see request.h) 
				 * pointer back to the main request.
				 */

  /* Info about the request itself... we begin with stuff that only
   * protocol.c should ever touch...
   */
  
  char *the_request;		/* First line of request, so we can log it */
  int assbackwards;		/* HTTP/0.9, "simple" request */
  int header_only;		/* HEAD request, as opposed to GET */
  char *protocol;		/* Protocol, as given to us, or HTTP/0.9 */
  
  char *status_line;		/* Status line, if set by script */
  int status;			/* In any case */
  
  /* Request method, two ways; also, protocol, etc..  Outside of protocol.c,
   * look, but don't touch.
   */
  
  char *method;			/* GET, HEAD, POST, etc. */
  int method_number;		/* M_GET, M_POST, etc. */

  /* int header_bytes_sent; */
  int bytes_sent;		/* body --- not headers */
  
  /* MIME header environments, in and out.  Also, an array containing
   * environment variables to be passed to subprocesses, so people can
   * write modules to add to that environment.
   *
   * The difference between headers_out and err_headers_out is that the
   * latter are printed even on error, and persist across internal redirects
   * (so the headers printed for ErrorDocument handlers will have them).
   *
   * The 'notes' table is for notes from one module to another, with no
   * other set purpose in mind...
   */
  
  table *headers_in;
  table *headers_out;
  table *err_headers_out;
  table *subprocess_env;
  table *notes;

  char *content_type;		/* Break these out --- we dispatch on 'em */
  char *content_encoding;
  char *content_language;
  
  int no_cache;
  
  /* What object is being requested (either directly, or via include
   * or content-negotiation mapping).
   */
  
  char *uri;
  char *filename;
  char *path_info;
  char *args;			/* QUERY_ARGS, if any */
  struct stat finfo;		/* ST_MODE set to zero if no such file */
  
  /* Various other config info which may change with .htaccess files
   * These are config vectors, with one void* pointer for each module
   * (the thing pointed to being the module's business).
   */
  
  void *per_dir_config;		/* Options set in config files, etc. */
  void *request_config;		/* Notes on *this* request */
  
};


/* Things which are per connection
 */

struct conn_rec {
  
  pool *pool;
  server_rec *server;
  
  /* Information about the connection itself */
  
  FILE *client;			/* Connetion to the guy */
  FILE *request_in;		/* Connection from the guy */
  int aborted;			/* Are we still talking? */
  
  /* Who is the client? */
  
  char *remote_ip;		/* Client's IP address */
  char *remote_host;		/* Client's DNS name, if known */
  char *remote_name;		/* Host ID --- same as remote_host, if known;
				 * otherwise same as remote_ip.
				 */
  char *remote_logname;		/* Only ever set if doing_rfc931 */
  
  char *user;			/* If an authentication check was made,
				 * this gets set to the user name.  We assume
				 * that there's only one user per connection(!)
				 */
  char *auth_type;		/* Ditto. */
};

/* Per-vhost config... */

struct server_rec {

  server_rec *next;
  
  /* Full locations of server config info */
  
  char *srm_confname;
  char *access_confname;
  
  /* Contact information */
  
  char *server_admin;
  char *server_hostname;
  
  /* Log files --- note that transfer log is now in the modules... */
  
  char *error_fname;
  FILE *error_log;
  
  /* Module-specific configuration for server, and defaults... */

  void *module_config;		/* Config vector containing pointers to
				 * modules' per-server config structures.
				 */
  void *lookup_defaults;	/* MIME type info, etc., before we start
				 * checking per-directory info.
				 */
  /* Transaction handling */

  short port;
  struct in_addr host_addr;	/* Specific address, if "virtual" server */
  int timeout;			/* Timeout, in seconds, before we give up */
  int do_rfc931;		/* See if client is advertising a username? */
  
};

/* Prototypes for utilities... util.c.
 */

/* Time */

struct tm *get_gmtoff(long *tz);
char *get_time();
char *ht_time (pool *p, time_t t, char *fmt, int gmt);     
char *gm_timestr_822(pool *p, time_t t);
     
/* String handling */     
     
char *getword(pool *p, char **line, char stop);
char *getword_conf (pool *p, char **line);      
     
int is_url(char *u);
void unescape_url(char *url);
void no2slash(char *name);
void getparents(char *name);
char *escape_uri (pool *p, char *s);
char *construct_url (pool *p, char *path, server_rec *s);     
char *escape_shell_cmd (pool *p, char *s);
     
int count_dirs(char *path);
char *make_dirstr(pool *a, char *s, int n);
char *make_full_path(pool *a, char *dir, char *f);
     
int is_matchexp(char *str);
int strcmp_match(char *str, char *exp);
char *uudecode (pool *, char *);

void str_tolower (char *);
int ind (const char *, char);	/* Sigh... */
int rind (const char *, char);     

int cfg_getline(char *s, int n, FILE *f);
     
/* Misc system hackery */
     
uid_t uname2id(char *name);
gid_t gname2id(char *name);
int is_directory(char *name);
int can_exec(struct stat *);     
void chdir_file(char *file);
     
char *get_local_host(pool *);
struct in_addr get_local_addr (int sd);
unsigned long get_virthost_addr (char *hostname, int wild_allowed);
void get_remote_host(conn_rec *conn);
int get_portnum(int sd);
     

