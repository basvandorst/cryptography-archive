
/* $Log:	mystrchr.c,v $
 * Revision 1.1  91/09/20  14:48:15  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:34  morain
 * Initial revision
 *  */

/*	@(#)strchr.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

#define	NULL	0

char *
strchr(sp, c)
register char *sp, c;
{
	do {
		if(*sp == c)
			return(sp);
	} while(*sp++);
	return(NULL);
}
