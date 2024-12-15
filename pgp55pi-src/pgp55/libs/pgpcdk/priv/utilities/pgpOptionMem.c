/*____________________________________________________________________________
	pgpOptionMem.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Memory allocation functions for use by optionlist package

	$Id: pgpOptionMem.c,v 1.3 1997/06/25 01:17:25 hal Exp $
____________________________________________________________________________*/

#include "pgpErrors.h"
#include "pgpMem.h"

#include "pgpContext.h"
#include "pgpOptionList.h"
#include "pgpEncodePriv.h"


        void *
PGPOptionListMemAlloc( PGPSize allocationSize )
{
        return( pgpAlloc( allocationSize ) );
}

		PGPError
PGPOptionListMemRealloc(
        void    **oldAllocation,
        PGPSize newAllocationSize )
{
		return( pgpRealloc( oldAllocation, newAllocationSize ) );
}

        void
PGPOptionListMemFree(void *allocation)
{
        pgpFree( allocation );
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
