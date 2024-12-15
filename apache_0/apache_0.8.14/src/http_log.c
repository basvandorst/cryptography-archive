
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
 * http_log.c: Dealing with the logs and errors
 * 
 * Rob McCool
 * 
 */


#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

void open_error_log(server_rec *s, pool *p)
{
    char *fname;
  
    fname = server_root_relative (p, s->error_fname);
    if(!(s->error_log = pfopen(p, fname, "a"))) {
        fprintf(stderr,"httpd: could not open error log file %s.\n", fname);
        perror("fopen");
        exit(1);
    }
}

void open_logs (server_rec *s_main, pool *p)
{
    server_rec *virt;
    
    open_error_log (s_main, p);

    for (virt = s_main->next; virt; virt = virt->next) {
	if (virt->error_fname) open_error_log (virt, p);
	else virt->error_log = s_main->error_log;
    }
}

void error_log2stderr(server_rec *s) {
    if(fileno(s->error_log) != STDERR_FILENO)
        dup2(fileno(s->error_log),STDERR_FILENO);
}

void log_pid(pool *p, char *pid_fname) {
    FILE *pid_file;

    if (!pid_fname) return;
    pid_fname = server_root_relative (p, pid_fname);
    if(!(pid_file = fopen(pid_fname,"w"))) {
        fprintf(stderr,"httpd: could not log pid to file %s\n", pid_fname);
        exit(1);
    }
    fprintf(pid_file,"%ld\n",(long)getpid());
    fclose(pid_file);
}

void log_error(char *err, server_rec *s) {
    fprintf(s->error_log, "[%s] %s\n",get_time(),err);
    fflush(s->error_log);
}

void log_reason(char *reason, char *file, request_rec *r) {
    fprintf (r->server->error_log,
	     "[%s] access to %s failed for %s, reason: %s\n",
	     get_time(), file, r->connection->remote_name, reason);
    fflush (r->server->error_log);
}

