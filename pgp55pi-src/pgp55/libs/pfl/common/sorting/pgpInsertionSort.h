/*____________________________________________________________________________
	Copyright 1991 Lloyd Chambers
	Rights granted to PGP to use this source code freely and
	without restriction.

	This file should be #included, and the following symbols defined:
	
	InsertionSortName
	InsertionSortItem
	
	For example:
	#define InsertionSortName	MySortName
	#define InsertionSortItem	MyStructName
	#include "pgpInsertionSort.h"
	#undef InsertionSortName
	#undef InsertionSortItem
	
	$Id: pgpInsertionSort.h,v 1.2 1997/09/17 00:45:42 mhw Exp $
____________________________________________________________________________*/


/*____________________________________________________________________________
	This function is declared static on purpose. Place it in a C file and
	write a wrapper if you want to make it visible.
____________________________________________________________________________*/
	static void
InsertionSortName(
	InsertionSortItem		*start,
	size_t					numItems,
	int 					(*compareProc)( InsertionSortItem *item1,
								InsertionSortItem *item2, void *userValue ),
	void *					userValue )
{
	InsertionSortItem		*stop;
	InsertionSortItem		*cur;
	InsertionSortItem		*least;
	InsertionSortItem		data;
	InsertionSortItem		temp;
	
	if ( numItems <= 1 )
		return;
	stop	= start + (numItems - 1);
	
	/*
	** find least element and place it at start so
	** we can guarantee halt to insertion
	*/
	least = cur = start;
	do
	{
		++cur;
		if ( compareProc( cur,least, userValue ) < 0 )
			least = cur;
	} while ( cur < stop );
	/* swap */
	temp	= *least;
	*least	= *start;
	*start	= temp;
	
	while ( start < stop )
	{
		++start;	/*  next item to insert */
		cur = start;
		
		data = *start;
		
		while ( compareProc( &data, cur - 1, userValue ) < 0 )
		{
			*cur	= *(cur - 1);
			--cur;
		}
		*cur	= data;
	}
}



