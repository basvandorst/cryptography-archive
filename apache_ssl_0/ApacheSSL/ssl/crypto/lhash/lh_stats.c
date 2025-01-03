/* lib/lhash/lh_stats.c */
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lhash.h"

void lh_stats(lh, out)
LHASH *lh;
FILE *out;
	{
	fprintf(out,"num_items             = %lu\n",lh->num_items);
	fprintf(out,"num_nodes             = %u\n",lh->num_nodes);
	fprintf(out,"num_alloc_nodes       = %u\n",lh->num_alloc_nodes);
	fprintf(out,"num_expands           = %lu\n",lh->num_expands);
	fprintf(out,"num_expand_reallocs   = %lu\n",lh->num_expand_reallocs);
	fprintf(out,"num_contracts         = %lu\n",lh->num_contracts);
	fprintf(out,"num_contract_reallocs = %lu\n",lh->num_contract_reallocs);
	fprintf(out,"num_hash_calls        = %lu\n",lh->num_hash_calls);
	fprintf(out,"num_comp_calls        = %lu\n",lh->num_comp_calls);
	fprintf(out,"num_insert            = %lu\n",lh->num_insert);
	fprintf(out,"num_replace           = %lu\n",lh->num_replace);
	fprintf(out,"num_delete            = %lu\n",lh->num_delete);
	fprintf(out,"num_no_delete         = %lu\n",lh->num_no_delete);
	fprintf(out,"num_retreve           = %lu\n",lh->num_retreve);
	fprintf(out,"num_retreve_miss      = %lu\n",lh->num_retreve_miss);
	fprintf(out,"num_hash_comps        = %lu\n",lh->num_hash_comps);
	fprintf(out,"num_alloc_nodes       = %u\n",lh->num_alloc_nodes);
#ifdef DEBUG
	fprintf(out,"p                     = %u\n",lh->p);
	fprintf(out,"pmax                  = %u\n",lh->pmax);
	fprintf(out,"up_load               = %lu\n",lh->up_load);
	fprintf(out,"down_load             = %lu\n",lh->down_load);
#endif
	}

void lh_node_stats(lh, out)
LHASH *lh;
FILE *out;
	{
	LHASH_NODE *n;
	unsigned int i,num;

	for (i=0; i<lh->num_nodes; i++)
		{
		for (n=lh->b[i],num=0; n != NULL; n=n->next)
			num++;
		fprintf(out,"node %6u -> %3u\n",i,num);
		}
	}

void lh_node_usage_stats(lh, out)
LHASH *lh;
FILE *out;
	{
	LHASH_NODE *n;
	unsigned long num;
	unsigned int i;
	unsigned long total=0,n_used=0;

	for (i=0; i<lh->num_nodes; i++)
		{
		for (n=lh->b[i],num=0; n != NULL; n=n->next)
			num++;
		if (num != 0)
			{
			n_used++;
			total+=num;
			}
		}
	fprintf(out,"%lu nodes used out of %u\n",n_used,lh->num_nodes);
	fprintf(out,"%lu items\n",total);
	if (n_used == 0) return;
	fprintf(out,"load %d.%02d  actual load %d.%02d\n",
		(int)(total/lh->num_nodes),
		(int)((total%lh->num_nodes)*100/lh->num_nodes),
		(int)(total/n_used),
		(int)((total%n_used)*100/n_used));
	}

