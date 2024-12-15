/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$ld$
____________________________________________________________________________*/
#include "CFree.h"

#include "pgpIO.h"


class CFreePGPIO : public CFree
{
public:			
				CFreePGPIO( PGPIORef io )
					: CFree( io )
				{
				}

	virtual		~CFreePGPIO( void )
				{
					if ( IsntNull( mMemory ) )
						PGPFreeIO( (PGPIORef)mMemory );
				}
};


