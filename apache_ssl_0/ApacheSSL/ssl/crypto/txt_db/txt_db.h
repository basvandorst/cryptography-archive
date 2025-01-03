/* lib/txt_db/txt_db.h */
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

#include "stack.h"
#include "lhash.h"

#define DB_ERROR_OK	        	0
#define DB_ERROR_MALLOC         	1
#define DB_ERROR_INDEX_CLASH    	2
#define DB_ERROR_INDEX_OUT_OF_RANGE	3
#define DB_ERROR_NO_INDEX		4
#define DB_ERROR_INSERT_INDEX_CLASH    	5

typedef struct txt_db_st
        {
        int num_fields;
        STACK /* char ** */ *data;
        LHASH **index;
        int (**qual)();
        long error;
        long arg1;
        long arg2;
	char **arg_row;
        } TXT_DB;

#ifndef NOPROTO
TXT_DB *TXT_DB_load(FILE *in, int num);
long TXT_DB_write(FILE *out, TXT_DB *db);
int TXT_DB_create_index(TXT_DB *db,int field,int (*qual)(),
	 unsigned long (*hash)(),int (*cmp)());
void TXT_DB_free(TXT_DB *db);
char **TXT_DB_get_by_index(TXT_DB *db, int index, char **value);
int TXT_DB_insert(TXT_DB *db,char **value);

#else

TXT_DB *TXT_DB_load();
long TXT_DB_write();
int TXT_DB_create_index();
void TXT_DB_free();
char **TXT_DB_get_by_index();
int TXT_DB_insert();

#endif
