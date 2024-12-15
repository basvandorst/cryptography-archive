/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$ld$
____________________________________________________________________________*/
#include "CFree.h"

#include "pgpFileSpec.h"


class CFreePGPFileSpec : public CFree
{
public:			
				CFreePGPFileSpec( PFLFileSpecRef spec )
					: CFree( spec )
				{
				}

	virtual		~CFreePGPFileSpec( void )
				{
					if ( IsntNull( mMemory ) )
						PFLFreeFileSpec( (PFLFileSpecRef)mMemory );
				}
};


