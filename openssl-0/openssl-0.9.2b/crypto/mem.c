/* crypto/mem.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "bio.h"
#include "lhash.h"
#include "cryptlib.h"

#ifdef CRYPTO_MDEBUG
static int mh_mode=CRYPTO_MEM_CHECK_ON;
#else
static int mh_mode=CRYPTO_MEM_CHECK_OFF;
#endif
static unsigned long order=0;

static LHASH *mh=NULL;

typedef struct mem_st
	{
	char *addr;
	int num;
	char *file;
	int line;
	unsigned long order;
	} MEM;

int CRYPTO_mem_ctrl(mode)
int mode;
	{
	int ret=mh_mode;

	CRYPTO_w_lock(CRYPTO_LOCK_MALLOC);
	switch (mode)
		{
	case CRYPTO_MEM_CHECK_ON:
		mh_mode|=CRYPTO_MEM_CHECK_ON;
		break;
	case CRYPTO_MEM_CHECK_OFF:
		mh_mode&= ~CRYPTO_MEM_CHECK_ON;
		break;
	case CRYPTO_MEM_CHECK_DISABLE:
		mh_mode&= ~CRYPTO_MEM_CHECK_ENABLE;
		break;
	case CRYPTO_MEM_CHECK_ENABLE:
		if (mh_mode&CRYPTO_MEM_CHECK_ON)
			mh_mode|=CRYPTO_MEM_CHECK_ENABLE;
		break;
	default:
		break;
		}
	CRYPTO_w_unlock(CRYPTO_LOCK_MALLOC);
	return(ret);
	}

static int mem_cmp(a,b)
MEM *a,*b;
	{
	return(a->addr - b->addr);
	}

static unsigned long mem_hash(a)
MEM *a;
	{
	unsigned long ret;

	ret=(unsigned long)a->addr;

	ret=ret*17851+(ret>>14)*7+(ret>>4)*251;
	return(ret);
	}

static char *(*malloc_locked_func)()=(char *(*)())malloc;
static void (*free_locked_func)()=(void (*)())free;
static char *(*malloc_func)()=	(char *(*)())malloc;
static char *(*realloc_func)()=	(char *(*)())realloc;
static void (*free_func)()=	(void (*)())free;

void CRYPTO_set_mem_functions(m,r,f)
char *(*m)();
char *(*r)();
void (*f)();
	{
	if ((m == NULL) || (r == NULL) || (f == NULL)) return;
	malloc_func=m;
	realloc_func=r;
	free_func=f;
	malloc_locked_func=m;
	free_locked_func=f;
	}

void CRYPTO_set_locked_mem_functions(m,f)
char *(*m)();
void (*f)();
	{
	if ((m == NULL) || (f == NULL)) return;
	malloc_locked_func=m;
	free_locked_func=f;
	}

void CRYPTO_get_mem_functions(m,r,f)
char *(**m)();
char *(**r)();
void (**f)();
	{
	if (m != NULL) *m=malloc_func;
	if (r != NULL) *r=realloc_func;
	if (f != NULL) *f=free_func;
	}

void CRYPTO_get_locked_mem_functions(m,f)
char *(**m)();
void (**f)();
	{
	if (m != NULL) *m=malloc_locked_func;
	if (f != NULL) *f=free_locked_func;
	}

char *CRYPTO_malloc_locked(num)
int num;
	{
	return(malloc_locked_func(num));
	}

void CRYPTO_free_locked(str)
char *str;
	{
	free_locked_func(str);
	}

char *CRYPTO_malloc(num)
int num;
	{
	return(malloc_func(num));
	}

char *CRYPTO_realloc(str,num)
char *str;
int num;
	{
	return(realloc_func(str,num));
	}

void CRYPTO_free(str)
char *str;
	{
	free_func(str);
	}

static unsigned long break_order_num=0;
char *CRYPTO_dbg_malloc(num,file,line)
int num;
char *file;
int line;
	{
	char *ret;
	MEM *m,*mm;

	if ((ret=malloc_func(num)) == NULL)
		return(NULL);

	if (mh_mode & CRYPTO_MEM_CHECK_ENABLE)
		{
		MemCheck_off();
		if ((m=(MEM *)Malloc(sizeof(MEM))) == NULL)
			{
			Free(ret);
			MemCheck_on();
			return(NULL);
			}
		CRYPTO_w_lock(CRYPTO_LOCK_MALLOC);
		if (mh == NULL)
			{
			if ((mh=lh_new(mem_hash,mem_cmp)) == NULL)
				{
				Free(ret);
				Free(m);
				ret=NULL;
				goto err;
				}
			}

		m->addr=ret;
		m->file=file;
		m->line=line;
		m->num=num;
		if (order == break_order_num)
			{
			/* BREAK HERE */
			m->order=order;
			}
		m->order=order++;
		if ((mm=(MEM *)lh_insert(mh,(char *)m)) != NULL)
			{
			/* Not good, but don't sweat it */
			Free(mm);
			}
err:
		CRYPTO_w_unlock(CRYPTO_LOCK_MALLOC);
		MemCheck_on();
		}
	return(ret);
	}

void CRYPTO_dbg_free(addr)
char *addr;
	{
	MEM m,*mp;

	if ((mh_mode & CRYPTO_MEM_CHECK_ENABLE) && (mh != NULL))
		{
		MemCheck_off();
		CRYPTO_w_lock(CRYPTO_LOCK_MALLOC);
		m.addr=addr;
		mp=(MEM *)lh_delete(mh,(char *)&m);
		if (mp != NULL)
			Free(mp);
		CRYPTO_w_unlock(CRYPTO_LOCK_MALLOC);
		MemCheck_on();
		}
	free_func(addr);
	}

char *CRYPTO_dbg_realloc(addr,num,file,line)
char *addr;
int num;
char *file;
int line;
	{
	char *ret;
	MEM m,*mp;

	ret=realloc_func(addr,num);
	if (ret == addr) return(ret);

	if (mh_mode & CRYPTO_MEM_CHECK_ENABLE)
		{
		MemCheck_off();
		if (ret == NULL) return(NULL);
		m.addr=addr;
		CRYPTO_w_lock(CRYPTO_LOCK_MALLOC);
		mp=(MEM *)lh_delete(mh,(char *)&m);
		if (mp != NULL)
			{
			mp->addr=ret;
			lh_insert(mh,(char *)mp);
			}
		CRYPTO_w_unlock(CRYPTO_LOCK_MALLOC);
		MemCheck_on();
		}
	return(ret);
	}

char *CRYPTO_remalloc(a,n)
char *a;
int n;
	{
	if (a != NULL) Free(a);
	a=(char *)Malloc(n);
	return(a);
	}

char *CRYPTO_dbg_remalloc(a,n,file,line)
char *a;
int n;
char *file;
int line;
	{
	if (a != NULL) CRYPTO_dbg_free(a);
	a=(char *)CRYPTO_dbg_malloc(n,file,line);
	return(a);
	}


typedef struct mem_leak_st
	{
	BIO *bio;
	int chunks;
	long bytes;
	} MEM_LEAK;

static void print_leak(m,l)
MEM *m;
MEM_LEAK *l;
	{
	char buf[128];

	if(m->addr == (char *)l->bio)
	    return;
	sprintf(buf,"%5ld file=%s, line=%d, number=%d, address=%08lX\n",
		m->order,m->file,m->line,m->num,(long)m->addr);
	BIO_puts(l->bio,buf);
	l->chunks++;
	l->bytes+=m->num;
	}

void CRYPTO_mem_leaks(b)
BIO *b;
	{
	MEM_LEAK ml;
	char buf[80];

	if (mh == NULL) return;
	ml.bio=b;
	ml.bytes=0;
	ml.chunks=0;
	CRYPTO_w_lock(CRYPTO_LOCK_MALLOC);
	lh_doall_arg(mh,(void (*)())print_leak,(char *)&ml);
	CRYPTO_w_unlock(CRYPTO_LOCK_MALLOC);
	if (ml.chunks != 0)
		{
		sprintf(buf,"%ld bytes leaked in %d chunks\n",
			ml.bytes,ml.chunks);
		BIO_puts(b,buf);
		}

#if 0
	lh_stats_bio(mh,b);
        lh_node_stats_bio(mh,b);
        lh_node_usage_stats_bio(mh,b);
#endif
	}

static void (*mem_cb)()=NULL;

static void cb_leak(m,cb)
MEM *m;
char *cb;
	{
	void (*mem_callback)()=(void (*)())cb;
	mem_callback(m->order,m->file,m->line,m->num,m->addr);
	}

void CRYPTO_mem_leaks_cb(cb)
void (*cb)();
	{
	if (mh == NULL) return;
	CRYPTO_w_lock(CRYPTO_LOCK_MALLOC);
	mem_cb=cb;
	lh_doall_arg(mh,(void (*)())cb_leak,(char *)mem_cb);
	mem_cb=NULL;
	CRYPTO_w_unlock(CRYPTO_LOCK_MALLOC);
	}

#ifndef NO_FP_API
void CRYPTO_mem_leaks_fp(fp)
FILE *fp;
	{
	BIO *b;

	if (mh == NULL) return;
	if ((b=BIO_new(BIO_s_file())) == NULL)
		return;
	BIO_set_fp(b,fp,BIO_NOCLOSE);
	CRYPTO_mem_leaks(b);
	BIO_free(b);
	}
#endif

