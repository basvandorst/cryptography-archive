/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1995 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

typedef struct {
	char ek1[128];
	char ek2[128];
	char ek3[128];
} keystr;
extern keystr ks;
extern unsigned char check[8];
unsigned int cfb8_encrypt();
unsigned int cfb8_decrypt();
extern R_RANDOM_STRUCT rs;
extern R_DH_PARAMS dhparams[];
extern unsigned char ivin[], ivout[];
extern unsigned char otherpub[];
extern unsigned char ourpub[];
extern unsigned char ourpriv[];
extern int ourprivlen;
extern int pklen;

#define SHORT 0
#define MEDIUM 1
#define LONG 2

#define MAXPUBKEY 512
