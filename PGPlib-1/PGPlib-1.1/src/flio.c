/*********************************************************************
 * Filename:      flio.c
 * Description:	  file-like i/o for stdio files and dynamic buffers
 * Author:        Thomas Roessler <roessler@guug.de>
 * Modified at:   Mon Jan 19 08:19:59 1998
 *
 * 	Copyright (c) Thomas Roessler
 * 	See the file COPYING for details
 *
 ********************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "pgplib.h"

#define CHUNKSIZE 1024

static int 
flio_enlarge_buffer(flio_t *p, u_long chunksize)
{
    u_char *scratch;

    if(p->magic != FLIO_BUFF)
	return -1;
    
    if((scratch = realloc(p->d.buff.b, p->d.buff.size + chunksize)) == NULL)
	return -1;

    p->d.buff.size += chunksize;
    p->d.buff.b = scratch;

    /* Make sure we clear the "new" area. Otherwise we might end up with 
     * all sort of junk in the buffer.
     * 15Jan1998 Per H. Myrvang <perm@stud.cs.uit.no> 
     */
    memset(p->d.buff.b + p->d.buff.size - chunksize, 0, chunksize);
    
    return 0;
}

void flio_close(flio_t *p)
{
    if(!p) return;
    
    if(p->magic == FLIO_FILE && p->d.f) {
	fclose(p->d.f);
	p->d.f = NULL;
    } else if(p->magic == FLIO_BUFF && p->d.buff.b) {
	free(p->d.buff.b);
    }

    free(p);
}

flio_t *flio_fopen(const char *fname, const char *mode)
{
    flio_t *p;
    FILE *f;

    if(!(f = fopen(fname, mode)))
	return NULL;

    if(!(p = malloc(sizeof(flio_t)))) {
	fclose(f);
	return NULL;
    }
    
    p->magic = FLIO_FILE;
    p->d.f = f;
    return p;
}

flio_t *flio_newbuff(void)
{
    flio_t *p;

    if(!(p = malloc(sizeof(flio_t))))
	return NULL;

    p->magic = FLIO_BUFF;
    p->d.buff.b = NULL;
    p->d.buff.p = 0;
    p->d.buff.size = 0;
    return p;
}


int flio_getpos(flio_t *p, flio_pos_t *pos)
{
    pos->magic = p->magic;
    if(p->magic == FLIO_BUFF) {
	pos->d.p = p->d.buff.p;
	return 0;
    } else if(p->magic == FLIO_FILE) {
	return fgetpos(p->d.f, &pos->d.f);
    } else {
	errno = EINVAL;
	return -1;
    }
}
    

int flio_setpos(flio_t *p, flio_pos_t *pos)
{
    if(pos->magic != p->magic)
	return -1;

    if(p->magic == FLIO_BUFF) {
	if(pos->d.p > p->d.buff.size)
	    return -1;
	p->d.buff.p = pos->d.p;
	return 0;
    } else if(p->magic == FLIO_FILE) {
	return fsetpos(p->d.f, &pos->d.f);
    } else
	return -1;

}


int 
flio_getc(flio_t *p)
{
    if(p->magic == FLIO_FILE)
	return fgetc(p->d.f);
    else 
	if(p->magic == FLIO_BUFF) 
	{
	    if(p->d.buff.p < p->d.buff.size)
		return (int) p->d.buff.b[p->d.buff.p++];
	    else
		return EOF;
	} 
	else 
	{
	    errno = EINVAL;
	    return EOF;
	}
}

/* XXX - use flio_getc()? */

char *
flio_gets(char *b, u_long s, flio_t *p)
{
    u_char *source; 
    char *dest;

    if(p->magic == FLIO_FILE)
	return fgets(b, s, p->d.f);
    else 
	if(p->magic == FLIO_BUFF) 
	{
	    dest = b;
	    source = p->d.buff.b;
	    if(!s-- || !p->d.buff.size || p->d.buff.p == p->d.buff.size)
		return NULL;
	
	    while(s-- && p->d.buff.p < p->d.buff.size) 
	    {
		if((*dest++ = source[p->d.buff.p++]) == '\n')
		    break;
	    }
	    
	    *dest = '\0';
	    return b;
	} 
	else
	    return NULL;
}

int flio_putc(int c, flio_t *p)
{
    unsigned char d = (unsigned char) c;

    if(p->magic == FLIO_FILE)
	return fputc(c, p->d.f);
    else if(p->magic == FLIO_BUFF) {
	if(p->d.buff.p == p->d.buff.size) {
	    if(flio_enlarge_buffer(p, CHUNKSIZE) == -1)
		return EOF;
	}
	
	p->d.buff.b[p->d.buff.p++] = d;
	return (int) d; /* fputc semantics */
    } else {
	errno = EINVAL;
	return EOF;
    }
}


int flio_puts(const char *s, flio_t *p)
{
    if(p->magic == FLIO_FILE) {
	return fputs(s, p->d.f);
    } else if(p->magic == FLIO_BUFF) {
	int rv;

	for(rv = 0; *s && (rv = flio_putc(*s, p)) != EOF; s++)
	    ;
	return rv;
    } else {
	errno = EINVAL;
	return EOF;
    }
}


int flio_write(void *ptr, u_long s, u_long n, flio_t *p)
{
    int i;
    u_char *src;

    if(p->magic == FLIO_FILE) {
	return fwrite(ptr, s, n, p->d.f);
    } else  if(p->magic == FLIO_BUFF) {
	for(i = 0, src = ptr; n; n--, i++) {
	    if(p->d.buff.p + s >= p->d.buff.size) {
		if(flio_enlarge_buffer(p, s) == -1)
		    return i;
	    }
	    memcpy(p->d.buff.b + p->d.buff.p, src, s);
	    p->d.buff.p += s;
	    src += s;
	}
	return i;
    } else {
	errno = EINVAL;
	return 0;
    }
}
