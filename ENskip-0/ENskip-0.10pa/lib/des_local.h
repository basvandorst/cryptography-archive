/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/* des_local.h */
/* Copyright (C) 1992 Eric Young - see COPYING for more details */

#define ITERATIONS 16
#define HALF_ITERATIONS 8

/* used in des_read and des_write */
#define MAXWRITE	(1024*16)
#define BSIZE		(MAXWRITE+4)

#define c2l(c,l)	(l =((u_long)(*((c)++)))    , \
			 l|=((u_long)(*((c)++)))<< 8, \
			 l|=((u_long)(*((c)++)))<<16, \
			 l|=((u_long)(*((c)++)))<<24)

/* NOTE - c is not incremented as per c2l */
#define c2ln(c,l1,l2,n)	{ \
			c+=n; \
			l1=l2=0; \
			switch (n) { \
			case 7: l2|=((u_long)(*(--(c))))<<16; \
			case 6: l2|=((u_long)(*(--(c))))<< 8; \
			case 5: l2|=((u_long)(*(--(c))));     \
			case 4: l1|=((u_long)(*(--(c))))<<24; \
			case 3: l1|=((u_long)(*(--(c))))<<16; \
			case 2: l1|=((u_long)(*(--(c))))<< 8; \
			case 1: l1|=((u_long)(*(--(c))));     \
				} \
			}

#define l2c(l,c)	(*((c)++)=(u_char)(((l)    )&0xff), \
			 *((c)++)=(u_char)(((l)>> 8)&0xff), \
			 *((c)++)=(u_char)(((l)>>16)&0xff), \
			 *((c)++)=(u_char)(((l)>>24)&0xff))

/* NOTE - c is not incremented as per l2c */
#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) { \
			case 7: *(--(c))=(u_char)(((l2)>>16)&0xff); \
			case 6: *(--(c))=(u_char)(((l2)>> 8)&0xff); \
			case 5: *(--(c))=(u_char)(((l2)    )&0xff); \
			case 4: *(--(c))=(u_char)(((l1)>>24)&0xff); \
			case 3: *(--(c))=(u_char)(((l1)>>16)&0xff); \
			case 2: *(--(c))=(u_char)(((l1)>> 8)&0xff); \
			case 1: *(--(c))=(u_char)(((l1)    )&0xff); \
				} \
			}
