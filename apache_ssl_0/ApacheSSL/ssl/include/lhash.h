/* lib/lhash/lhash.h */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
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
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
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

/* Header for dynamic hash table routines
 * Author - Eric Young
 */

#ifndef HEADER_LHASH_H
#define HEADER_LHASH_H

typedef struct lhash_node_st
	{
	char *data;
	struct lhash_node_st *next;
#ifndef NO_HASH_COMP
	unsigned long hash;
#endif
	} LHASH_NODE;

typedef struct lhash_st
	{
	LHASH_NODE **b;
	int (*comp)();
	unsigned long (*hash)();
	unsigned int num_nodes;
	unsigned int num_alloc_nodes;
	unsigned int p;
	unsigned int pmax;
	unsigned long up_load; /* load times 256 */
	unsigned long down_load; /* load times 256 */
	unsigned long num_items;

	unsigned long num_expands;
	unsigned long num_expand_reallocs;
	unsigned long num_contracts;
	unsigned long num_contract_reallocs;
	unsigned long num_hash_calls;
	unsigned long num_comp_calls;
	unsigned long num_insert;
	unsigned long num_replace;
	unsigned long num_delete;
	unsigned long num_no_delete;
	unsigned long num_retreve;
	unsigned long num_retreve_miss;
	unsigned long num_hash_comps;
	} LHASH;

#define LH_LOAD_MULT	256

#ifndef NOPROTO
LHASH *lh_new(unsigned long (*h)(), int (*c)());
void lh_free(LHASH *lh);
char *lh_insert(LHASH *lh, char *data);
char *lh_delete(LHASH *lh, char *data);
char *lh_retrieve(LHASH *lh, char *data);
void lh_stats(LHASH *lh, FILE *out);
void lh_node_stats(LHASH *lh, FILE *out);
void lh_node_usage_stats(LHASH *lh, FILE *out);
void lh_doall(LHASH *lh, void (*func)(char *a));
void lh_doall_arg(LHASH *lh, void (*func)(char *a),char *arg);
unsigned long lh_strhash(char *c);
#else
LHASH *lh_new();
void lh_free();
char *lh_insert();
char *lh_delete();
char *lh_retrieve();
void lh_stats();
void lh_node_stats();
void lh_node_usage_stats();
void lh_doall();
void lh_doall_arg();
unsigned long lh_strhash();
#endif

#endif
