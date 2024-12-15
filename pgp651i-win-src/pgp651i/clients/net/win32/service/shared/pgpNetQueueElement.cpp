/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetQueueElement.cpp,v 1.7 1998/11/18 23:42:28 elowe Exp $
____________________________________________________________________________*/

#include <assert.h>
#include <stdio.h>

// PGP includes
#include "pgpErrors.h"
#include "pgpMem.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"

#include "pgpNetQueueElement.h"

CPGPnetQueueElement::CPGPnetQueueElement(PGPUInt32 size, 
										 void *data, 
										 PGPUInt32 type,
										 PGPUInt32 ipAddress)
: m_size(size), m_type(type), m_ipAddress(ipAddress), m_data(0)
{
	if (size > 0) {
		m_data = new PGPByte[size];
		pgpCopyMemory(data, m_data, size);
	}
}

CPGPnetQueueElement::CPGPnetQueueElement()
: m_size(0), m_type(kPGPike_MT_Idle), m_data(0), m_ipAddress(0)
{
}

CPGPnetQueueElement::CPGPnetQueueElement(const CPGPnetQueueElement &rhs)
: m_size(rhs.m_size), m_type(rhs.m_type), m_ipAddress(rhs.m_ipAddress), m_data(0)
{
	if (m_size > 0) {
		m_data = new PGPByte[m_size];
		pgpCopyMemory(rhs.m_data, m_data, m_size);
	}
}

CPGPnetQueueElement&
CPGPnetQueueElement::operator=(const CPGPnetQueueElement &rhs)
{
	m_size = rhs.m_size;
	m_type = rhs.m_type;
	m_ipAddress = rhs.m_ipAddress;

	delete [] m_data;

	if (m_size > 0) {
		m_data = new PGPByte[m_size];
		pgpCopyMemory(rhs.m_data, m_data, m_size);
	} else
		m_data = 0;

	return *this;
}

CPGPnetQueueElement::~CPGPnetQueueElement()
{
	delete [] m_data;
	m_data = 0;
}
