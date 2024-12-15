/* Set block of memory to constant */
/***************************************************************
 *
 * Procedure memset
 *
 ***************************************************************/
#ifdef __STDC__

int memset(
	register char		 *blk,
	register char		  val,
	register unsigned	  size
)

#else

int memset(
	blk,
	val,
	size
)
register char		 *blk;
register char		  val;
register unsigned	  size;

#endif

{
	while(size-- != 0)
		*blk++ = val;
}

/* Copy block of memory */
/***************************************************************
 *
 * Procedure memcpy
 *
 ***************************************************************/
#ifdef __STDC__

int memcpy(
	register char		 *dest,
	register char		 *src,
	register unsigned	  size
)

#else

int memcpy(
	dest,
	src,
	size
)
register char		 *dest;
register char		 *src;
register unsigned	  size;

#endif

{
	while(size-- != 0)
		*dest++ = *src++;
}

/* Compare two blocks of memory */
/***************************************************************
 *
 * Procedure memcmp
 *
 ***************************************************************/
#ifdef __STDC__

int memcmp(
	register char		 *a,
	register char		 *b,
	register unsigned	  size
)

#else

int memcmp(
	a,
	b,
	size
)
register char		 *a;
register char		 *b;
register unsigned	  size;

#endif

{
	while(size-- != 0)
		if(*a++ != *b++)
			return 1;
	return 0;
}

