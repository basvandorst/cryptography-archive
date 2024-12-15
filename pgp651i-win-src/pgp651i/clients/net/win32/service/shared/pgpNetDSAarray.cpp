/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetDSAarray.cpp,v 1.5 1998/11/25 19:09:53 elowe Exp $
____________________________________________________________________________*/

// PGP includes
#include "pgpMem.h"

#include "pgpNetDSAarray.h"

CPGPnetDSAarray::CPGPnetDSAarray(PGPUInt32 nElems)
: m_maxSAs(nElems), m_numSAs(0)
{
	m_pArray = (PGPikeSA**) calloc(nElems, sizeof(PGPikeSA*));

	if (m_pArray) {
		pgpClearMemory(m_pArray, (nElems * sizeof(PGPikeSA*)));
	} else {
		m_maxSAs = 0;
		m_numSAs = 0;
	}
}

CPGPnetDSAarray::~CPGPnetDSAarray()
{
	if (m_pArray)
		free(m_pArray);
}
	
PGPikeSA *
CPGPnetDSAarray::addSA(const PGPikeSA **data)
{
	if (m_numSAs == m_maxSAs) {
		PGPikeSA** pTmp = (PGPikeSA**) realloc(m_pArray, 
			(m_maxSAs+16) * sizeof(PGPikeSA*));

		if (!pTmp) {
			m_pArray = NULL;
			return NULL;
		}
		m_pArray = pTmp;
		m_maxSAs += 16;
	}

	// copy the pointer
	PGPikeSA** newpSA = &(m_pArray[m_numSAs]);
	pgpClearMemory(newpSA, sizeof(PGPikeSA*));
	pgpCopyMemory(data, newpSA, sizeof(PGPikeSA*));

	m_numSAs++;

	return *newpSA;
}

PGPikeSA *
CPGPnetDSAarray::removeSA(const void *data, PGPUInt32 dataSize)
{
	PGPikeSA* foundSA = 0;
	const PGPByte *pSPI = 0;

	switch (dataSize) {
	case sizeof(PGPikeSA):
		pSPI = static_cast<const PGPikeSA*>(data)->transform[0].u.ipsec.inSPI;
		break;
	case sizeof(PGPipsecSPI):
		pSPI = static_cast<const PGPByte*>(data);
		break;
	default:
		return 0;
		break;
	}
	
	for (PGPUInt32 i = 0; i<m_numSAs; i++) {
		if (pgpMemoryEqual(m_pArray[i]->transform[0].u.ipsec.inSPI,
					pSPI,
					sizeof(PGPipsecSPI))) {
			foundSA = m_pArray[i];
			pgpCopyMemory(
				&m_pArray[i+1], /* src */
				&m_pArray[i],	/* dest */
				(m_numSAs-i-1)*sizeof(PGPikeSA*));
			m_numSAs--;
			break;
		}
	}

	return foundSA;
}

PGPikeSA *
CPGPnetDSAarray::findSA(const void *data, PGPUInt32 dataSize)
{
	const PGPByte *pSPI = 0;

	switch (dataSize) {
	case sizeof(PGPikeSA):
		pSPI = static_cast<const PGPikeSA*>(data)->transform[0].u.ipsec.inSPI;
		break;
	case sizeof(PGPipsecSPI):
		pSPI = static_cast<const PGPByte*>(data);
		break;
	default:
		return 0;
		break;
	}
	
	for (PGPUInt32 i = 0; i<m_numSAs; i++) {
		if (pgpMemoryEqual(m_pArray[i]->transform[0].u.ipsec.inSPI,
					pSPI,
					sizeof(PGPipsecSPI))) {
			return m_pArray[i];
		}
	}

	return 0;
}

PGPikeSA*
CPGPnetDSAarray::flatBuffer(PGPikeSA **data)
{
	*data = new PGPikeSA[m_numSAs];
	
	for (PGPUInt32 i = 0; i < m_numSAs; i++) {
		pgpCopyMemory(m_pArray[i], &((*data)[i]), sizeof(PGPikeSA));
	}

	return *data;	// callers responsibility to free
}