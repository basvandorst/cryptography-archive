/* $Id: dynarray.c,v 1.2 1995/06/14 05:34:36 elkins Exp elkins $
 *
 * $Log: dynarray.c,v $
 * Revision 1.2  1995/06/14 05:34:36  elkins
 * fixed bug where DestroyDynamicArray will seg fault if exactly *max+n
 * elements were allocated.
 * From: weo@recco.de (Wolfgang Ocker)
 *
 * Revision 1.1  1995/06/01  23:14:35  elkins
 * Initial revision
 *
 */

/** routines for handling of dynamic arrays **/

char **
DynamicArray (p, record_size, max, n)
char **p;
int record_size;
int *max;
int n;
{
	int newmax, j;
	char **c;

	newmax = *max + n;

	if (!p)
		c = (char**)safe_malloc(record_size * (n+1));
	else
		c = (char**)safe_realloc(p, record_size * (newmax+1));

	if (!c)
		return(0);

	for (j = *max ; j < newmax ; j++)
		c[j] = 0;

	*max = newmax;

	return(c);
}

void
DestroyDynamicArray (p)
char **p;
{
	char **b;

	if (!p)
		return;
	b = p;
	while (*b)
		free(*b++);
	free(p);
}
