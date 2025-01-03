/* logger.c  -	log functions
 *	Copyright (C) 1998 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "i18n.h"

static char pidstring[15];
static char *pgm_name;
static int errorcount;
static FILE *logfp;

/****************
 * Set the logfile to use (not yet implemneted) or, if logfile is NULL,
 * the Fd where logoutputs should go.
 */
void
log_set_logfile( const char *name, int fd )
{
    if( name )
	BUG();

    if( logfp && logfp != stderr && logfp != stdout )
	fclose( logfp );
    if( fd == 1 )
	logfp = stdout;
    else if( fd == 2 )
	logfp = stderr;
    else
	logfp = fdopen( fd, "a" );
    if( !logfp ) {
	logfp = stderr;
	log_fatal("can't open fd %d for logging: %s\n", fd, strerror(errno));
    }
}

FILE *
log_stream()
{
    if( !logfp )
	logfp = stderr;
    return logfp;
}


void
log_set_name( const char *name )
{
    m_free(pgm_name);
    if( name )
	pgm_name = m_strdup(name);
    else
	pgm_name = NULL;
}

const char *
log_get_name(void)
{
    return pgm_name? pgm_name : "";
}


void
log_set_pid( int pid )
{
    if( pid )
	sprintf(pidstring,"[%u]", (unsigned)pid );
    else
	*pidstring = 0;
}

int
log_get_errorcount( int clear)
{
    int n = errorcount;
    if( clear )
	errorcount = 0;
    return n;
}


void
g10_log_print_prefix(const char *text)
{
    if( !logfp )
	logfp = stderr;
    if( pgm_name )
	fprintf(logfp, "%s%s: %s", pgm_name, pidstring, text );
    else
	fprintf(logfp, "?%s: %s", pidstring, text );
}

static void
print_prefix_f(const char *text, const char *fname)
{
    if( !logfp )
	logfp = stderr;
    if( pgm_name )
	fprintf(logfp, "%s%s:%s: %s", pgm_name, pidstring, fname, text );
    else
	fprintf(logfp, "?%s:%s: %s", pidstring, fname, text );
}

void
g10_log_info( const char *fmt, ... )
{
    va_list arg_ptr ;

    g10_log_print_prefix("");
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
}

void
g10_log_info_f( const char *fname, const char *fmt, ... )
{
    va_list arg_ptr ;

    print_prefix_f("", fname);
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
}

void
g10_log_error( const char *fmt, ... )
{
    va_list arg_ptr ;

    g10_log_print_prefix("");
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
    errorcount++;
}

void
g10_log_error_f( const char *fname, const char *fmt, ... )
{
    va_list arg_ptr ;

    print_prefix_f("", fname);
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
    errorcount++;
}

void
g10_log_fatal( const char *fmt, ... )
{
    va_list arg_ptr ;

    g10_log_print_prefix("fatal: ");
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
    secmem_dump_stats();
    exit(2);
}

void
g10_log_fatal_f( const char *fname, const char *fmt, ... )
{
    va_list arg_ptr ;

    print_prefix_f("fatal: ", fname);
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
    secmem_dump_stats();
    exit(2);
}

void
g10_log_bug( const char *fmt, ... )
{
    va_list arg_ptr ;

    putc('\n', stderr );
    g10_log_print_prefix("Ohhhh jeeee: ");
    va_start( arg_ptr, fmt ) ;
    vfprintf(stderr,fmt,arg_ptr) ;
    va_end(arg_ptr);
    fflush(stderr);
    secmem_dump_stats();
    abort();
}

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5 )
void
g10_log_bug0( const char *file, int line, const char *func )
{
    log_bug(_("... this is a bug (%s:%d:%s)\n"), file, line, func );
}
#else
void
g10_log_bug0( const char *file, int line )
{
    log_bug(_("you found a bug ... (%s:%d)\n"), file, line);
}
#endif

void
g10_log_debug( const char *fmt, ... )
{
    va_list arg_ptr ;

    g10_log_print_prefix("DBG: ");
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
}

void
g10_log_debug_f( const char *fname, const char *fmt, ... )
{
    va_list arg_ptr ;

    print_prefix_f("DBG: ", fname);
    va_start( arg_ptr, fmt ) ;
    vfprintf(logfp,fmt,arg_ptr) ;
    va_end(arg_ptr);
}



void
g10_log_hexdump( const char *text, const char *buf, size_t len )
{
    int i;

    g10_log_print_prefix(text);
    for(i=0; i < len; i++ )
	fprintf(logfp, " %02X", ((const byte*)buf)[i] );
    fputc('\n', logfp);
}



