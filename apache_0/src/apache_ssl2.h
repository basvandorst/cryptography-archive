
/* ====================================================================
 * Copyright (c) 1995 Ben Laurie.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * 4. The name "Apache-SSL Server" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Ben Laurie
 *    for use in the Apache-SSL HTTP server project."
 *
 * THIS SOFTWARE IS PROVIDED BY BEN LAURIE ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL BEN LAURIE OR
 * HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of patches to the Apache HTTP server interfacing it
 * to SSLeay.
 * For more information on Apache-SSL, contact Ben Laurie <ben@algroup.co.uk>.
 *
 * For more information on Apache see http://www.apache.org.
 *
 * For more information on SSLeay see http://www.psy.uq.oz.au/~ftp/Crypto/.
 *
 */
/*
Stuff for Apache to support (or not support, according to APACHE_SSL) SSLeay

14 Oct 1995
*/

void os_conn_close(conn_rec *conn);
void os_conn_abort(conn_rec *conn);
void os_conn_flush(conn_rec *conn);
struct in_addr os_conn_get_local_addr(os_conn_rec *os_conn);
int os_conn_vprintf(conn_rec *conn,const char *fmt,va_list args);
int os_conn_read(void *buf,int unit,int len,conn_rec *conn);
int os_conn_write(void *buf,int unit,int len,conn_rec *conn);
int os_conn_puts(char *buf,conn_rec *conn);
int os_conn_putc(int c,conn_rec *conn);
int os_conn_getpeername(conn_rec *conn,struct sockaddr *addr,int *len);
char *os_conn_gets(char *buf,int len,conn_rec *conn);
void os_one_time_init(void);

#ifdef APACHE_SSL

#include <ssl.h>

#define os_conn_close(conn) { int fd=SSL_get_fd(conn->client.ssl); SSL_free(conn->client.ssl); close(fd); }
#define os_conn_abort(conn) shutdown(SSL_get_fd(conn->client.ssl),2)
#define os_conn_get_local_addr(os_conn) get_local_addr(SSL_get_fd(os_conn->ssl))
#define os_conn_flush(conn) /* for now! */
#define os_conn_printf(conn) _os_conn_printf(conn
int _os_conn_printf(conn_rec *conn,const char *fmt,...);
#define os_conn_write(buf,unit,len,conn) SSL_write(conn->client.ssl,buf,(unit)*(len))
#define os_conn_getpeername(conn,addr,len) getpeername(SSL_get_fd(conn->client.ssl),addr,len)

void ApacheSSLSetupVerify(conn_rec *conn);

#else

#define os_conn_close(conn) fclose(conn->client.out),fclose(conn->client.in)
#define os_conn_abort(conn) shutdown(fileno(conn->client.out),2)
#define os_conn_get_local_addr(os_conn) get_local_addr(fileno(os_conn->out))
#define os_conn_flush(conn) fflush(conn->client.out)
#define os_conn_gets(buf,len,conn) fgets(buf,len,conn->client.in)
#define os_conn_printf(conn,args) fprintf(conn->client.out
#define os_conn_vprintf(conn,fmt,args) vfprintf(conn->client.out,fmt,args)
#define os_conn_read(buf,unit,len,conn) fread(buf,unit,len,conn->client.in)
#define os_conn_write(buf,unit,len,conn) fwrite(buf,unit,len,conn->client.out)
#define os_conn_puts(buf,conn) fputs(buf,conn->client.out)
#define os_conn_putc(c,conn) putc(c,conn->client.out)
#define os_conn_getpeername(conn,addr,len) getpeername(fileno(conn->client.out),addr,len)
#define os_one_time_init()

#endif
