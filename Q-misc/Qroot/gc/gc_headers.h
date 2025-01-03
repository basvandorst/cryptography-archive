/* 
 * Copyright 1988, 1989 Hans-J. Boehm, Alan J. Demers
 * Copyright (c) 1991, 1992 by Xerox Corporation.  All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to copy this garbage collector for any purpose,
 * provided the above notices are retained on all copies.
 */
# ifndef GC_HEADERS_H
# define GC_HEADERS_H
typedef struct hblkhdr hdr;

# if CPP_WORDSZ != 32 && CPP_WORDSZ < 36
	--> Get a real machine.
# endif

/*
 * The 2 level tree data structure that is used to find block headers.
 * If there are more than 32 bits in a pointer, the top level is a hash
 * table.
 */

# if CPP_WORDSZ > 32
#   define HASH_TL
# endif

/* Define appropriate out-degrees for each of the two tree levels	*/
# define LOG_BOTTOM_SZ 10
# ifndef HASH_TL
#   define LOG_TOP_SZ (WORDSZ - LOG_BOTTOM_SZ - LOG_HBLKSIZE)
# else
#   define LOG_TOP_SZ 11
# endif
# define TOP_SZ (1 << LOG_TOP_SZ)
# define BOTTOM_SZ (1 << LOG_BOTTOM_SZ)

typedef struct bi {
    hdr * index[BOTTOM_SZ];
	/*
 	 * The bottom level index contains one of three kinds of values:
	 * 0 means we're not responsible for this block.
	 * 1 < (long)X <= MAX_JUMP means the block starts at least
	 *        X * HBLKSIZE bytes before the current address.
	 * A valid pointer points to a hdr structure. (The above can't be
	 * valid pointers due to the GET_MEM return convention.)
	 */
    struct bi * asc_link;	/* All indices are linked in	*/
    				/* ascending order.		*/
    word key;			/* high order address bits.	*/
# ifdef HASH_TL
    struct bi * hash_link;	/* Hash chain link.		*/
# endif
} bottom_index;

/* extern bottom_index GC_all_nils; - really part of GC_arrays */

/* extern bottom_index * GC_top_index []; - really part of GC_arrays */
				/* Each entry points to a bottom_index.	*/
				/* On a 32 bit machine, it points to 	*/
				/* the index for a set of high order	*/
				/* bits equal to the index.  For longer	*/
				/* addresses, we hash the high order	*/
				/* bits to compute the index in 	*/
				/* GC_top_index, and each entry points	*/
				/* to a hash chain.			*/
				/* The last entry in each chain is	*/
				/* GC_all_nils.				*/


# define MAX_JUMP (HBLKSIZE - 1)

# ifndef HASH_TL
#   define BI(p) (GC_top_index \
		[(word)(p) >> (LOG_BOTTOM_SZ + LOG_HBLKSIZE)])
#   define HDR(p) (BI(p)->index \
		[((word)(p) >> LOG_HBLKSIZE) & (BOTTOM_SZ - 1)])
#   define GET_BI(p, bottom_indx) (bottom_indx) = BI(p)
#   define GET_HDR(p, hhdr) (hhdr) = HDR(p)
#   define SET_HDR(p, hhdr) HDR(p) = (hhdr)
#   define GET_HDR_ADDR(p, ha) (ha) = &(HDR(p))
# else /* hash */
/*  Hash function for tree top level */
#   define TL_HASH(hi) ((hi) & (TOP_SZ - 1))
/*  Set bottom_indx to point to the bottom index for address p */
#   define GET_BI(p, bottom_indx) \
	{ \
	    register word hi = \
	        (word)(p) >> (LOG_BOTTOM_SZ + LOG_HBLKSIZE); \
	    register bottom_index * _bi = GC_top_index[TL_HASH(hi)]; \
	    \
	    while (_bi -> key != hi && _bi != &GC_all_nils) \
	    	_bi = _bi -> hash_link; \
	    (bottom_indx) = _bi; \
	}
#   define GET_HDR_ADDR(p, ha) \
	{ \
	    register bottom_index * bi; \
	    \
	    GET_BI(p, bi);	\
	    (ha) = &(bi->index[((unsigned long)(p)>>LOG_HBLKSIZE) \
	        	  	& (BOTTOM_SZ - 1)]); \
	}
#   define GET_HDR(p, hhdr) { register hdr ** _ha; GET_HDR_ADDR(p, _ha); \
			      (hhdr) = *_ha; }
#   define SET_HDR(p, hhdr) { register hdr ** _ha; GET_HDR_ADDR(p, _ha); \
			      *_ha = (hhdr); }
#   define HDR(p) GC_find_header(p)
# endif
			    
/* Is the result a forwarding address to someplace closer to the	*/
/* beginning of the block or NIL?					*/
# define IS_FORWARDING_ADDR_OR_NIL(hhdr) ((unsigned long) (hhdr) <= MAX_JUMP)

/* Get an HBLKSIZE aligned address closer to the beginning of the block */
/* h.  Assumes hhdr == HDR(h) and IS_FORWARDING_ADDR(hhdr).		*/
# define FORWARDED_ADDR(h, hhdr) ((struct hblk *)(h) - (unsigned long)(hhdr))
# endif /*  GC_HEADERS_H */
