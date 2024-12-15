/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1994 by AT&T.
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

/* MacGuffin Constants */

#define SIZE (1<<16)
extern unsigned short stable[SIZE];
extern unsigned short lookupmasks[4][3];
extern unsigned short outputmasks[4];

/*
 * input and output lookup masks
 */

/* s1+s2 */
#define LOOK00	0x0036
#define LOOK01	0x06c0
#define LOOK02	0x6900
#define OUT0	0x000f

/* s3+s4 */
#define LOOK10	0x5048
#define LOOK11	0x2106
#define LOOK12	0x8411
#define OUT1	0x00f0

/* s5+s7 */
#define LOOK20	0x8601
#define LOOK21	0x4828
#define LOOK22	0x10c4
#define OUT2	0x3300

/* s6+s8 */
#define LOOK30	0x2980
#define LOOK31	0x9011
#define LOOK32	0x022a
#define OUT3	0xcc00

#define ROUNDS 32
#define KSIZE (ROUNDS*3)
typedef struct mcg_key {
	unsigned short val[KSIZE];
} mcg_key;

