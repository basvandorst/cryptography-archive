/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetQueueElement.h,v 1.7 1998/11/12 01:22:24 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetQueueElement_h
#define _Included_pgpNetQueueElement_h

#include "pgpPubTypes.h"

class CPGPnetQueueElement {
private:
	PGPUInt32 	m_size;
	PGPByte *	m_data;
	PGPUInt32	m_type;
	PGPUInt32	m_ipAddress;
	
public:
	CPGPnetQueueElement();
	CPGPnetQueueElement(const CPGPnetQueueElement &);	// copy constructor
	CPGPnetQueueElement(PGPUInt32 size,
						void *data,
						PGPUInt32 type,
						PGPUInt32 ipAddress);
	CPGPnetQueueElement& operator=(const CPGPnetQueueElement &);

	~CPGPnetQueueElement();

	// access
	const PGPUInt32 type()		{ return m_type; }
	const void * data()			{ return m_data; }
	const PGPUInt32 size()		{ return m_size; }
	const PGPUInt32 ipAddress()	{ return m_ipAddress; }
		
protected:
};

#endif // _Included_pgpNetQueueElement_h
