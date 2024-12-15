/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetDSAarray.h,v 1.3 1998/11/12 01:22:21 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetDSAarray_h
#define _Included_pgpNetDSAarray_h

// PGP includes
#include "pgpPubTypes.h"
#include "pgpIKE.h"

class CPGPnetDSAarray {
private:
	PGPUInt32	m_numSAs;
	PGPUInt32 	m_maxSAs;
	PGPikeSA	**m_pArray;
	
	// private and undefined assignment
	CPGPnetDSAarray& operator=(const CPGPnetDSAarray &);
	CPGPnetDSAarray(const CPGPnetDSAarray &);	// copy constructor
	
public:
	CPGPnetDSAarray(PGPUInt32 nElems = 0);
	~CPGPnetDSAarray();

	PGPikeSA *addSA(const PGPikeSA **data);
	PGPikeSA *removeSA(const void *data, PGPUInt32 dataSize);
	PGPikeSA *findSA(const void *data, PGPUInt32 dataSize);

	PGPikeSA *findElement();
	PGPikeSA *flatBuffer(PGPikeSA **data);

	// access
	const PGPUInt32 numSAs()		{ return m_numSAs; };
		
protected:
};

#endif // _Included_pgpNetDSAarray_h
