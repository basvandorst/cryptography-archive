/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetIKEmanager.cpp,v 1.20.4.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#include <winsock2.h>

// PGP includes
#include "pgpMem.h"

// PGPnet includes
#include "pgpNetIKEWorker.h"
#include "pgpNetGUIWorker.h"
#include "pgpNetKernelWorker.h"
#include "pgpNetMainWorker.h"
#include "pgpNetReadWorker.h"
#include "pgpNetWriteWorker.h"
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"

#include "pgpNetIKEmanager.h"

CPGPnetIKEmanager::CPGPnetIKEmanager(const char *displayName, HINSTANCE hInstance)
	: m_numWorkers(6), 
	m_numKernelWorkers(1), 
	m_socket(INVALID_SOCKET)
{
	// assume 6 threads, one of each type
	initializeWorkers(displayName, hInstance);
}

CPGPnetIKEmanager::CPGPnetIKEmanager(PGPInt32 nKernelWorkers, 
									 const char *displayName,
									 HINSTANCE hInstance)
	: m_numWorkers(nKernelWorkers+5), m_socket(INVALID_SOCKET)
{
	m_numKernelWorkers = nKernelWorkers;

	initializeWorkers(displayName, hInstance);
}

void
CPGPnetIKEmanager::initializeWorkers(const char *displayName, HINSTANCE hInstance)
{
	m_workers = new CPGPnetWorker*[m_numWorkers];

	if (!m_workers) {
		reportOutOfMemory(-1);
		return;
	}

	PGPInt32 i;

	if (initializeWSA() != 0) {
		m_numWorkers = 0;
		CPGPnetAppLog::instance()->logServiceEvent(
			kPGPnetSrvcError_NoNetwork,
			0,
			0,
			__FILE__,
			__LINE__);
		return;
	}

	// create the single Main worker
	m_workers[0] = new CPGPnetMainWorker(displayName, hInstance);
	if (!m_workers[0]) {
		reportOutOfMemory(0);
		return;
	}

	// create the correct number of KernelWorkers
	for (i = 1; i < (1+m_numKernelWorkers); i++) {
		m_workers[i] = new CPGPnetKernelWorker;
		if (!m_workers[i]) {
			reportOutOfMemory(i);
			return;
		}
	}
	
	// create the single GUI worker
	m_workers[i++] = new CPGPnetGUIWorker;
	if (!m_workers[i-1]) {
		reportOutOfMemory(i);
		return;
	}
	
	// network readers and writers and IKE message Worker

	m_workers[i++] = new CPGPnetIKEWorker;
	if (!m_workers[i-1]) {
		reportOutOfMemory(i);
		return;
	}
	m_workers[i++] = new CPGPnetReadWorker(m_socket);
	if (!m_workers[i-1]) {
		reportOutOfMemory(i);
		return;
	}
	m_workers[i++] = new CPGPnetWriteWorker(m_socket);
	if (!m_workers[i-1]) {
		reportOutOfMemory(i);
		return;
	}
}


CPGPnetIKEmanager::~CPGPnetIKEmanager()
{
	if (m_workers) {
		for (PGPInt32 i = 0; i < m_numWorkers; i++)
			delete m_workers[i];

		delete [] m_workers;
	}

	closesocket(m_socket);
}

void
CPGPnetIKEmanager::GoResume()
{
	PGPUInt32 i = 1;

	// resume the KernelWorkers
	for (i = 1; i < (1+m_numKernelWorkers); i++) {
		if (m_workers[i])
			m_workers[i]->Go();
	}

	i++; // skip GUI worker
	
	// resume IKE Worker
	m_workers[i]->Go();
}

void
CPGPnetIKEmanager::Go()
{
	if (m_workers) {
		for (PGPInt32 i	= 0; i < m_numWorkers; i++) {
			if (m_workers[i])
				m_workers[i]->Go();
		}
	}
}


void
CPGPnetIKEmanager::WaitForWorkerFinish()
{
	HANDLE *lpHandles;
	DWORD ret;
	PGPInt32 i = 0;

	if (m_numWorkers == 0)
		return;

	lpHandles = new HANDLE[m_numWorkers];
	if (!lpHandles) {
		reportOutOfMemory(m_numWorkers);
		return;
	}

	for (i = 0; i < m_numWorkers; i++)
		lpHandles[i] = m_workers[i]->threadID();
	
	ret = WaitForMultipleObjects(i, lpHandles, TRUE, INFINITE);
	if (ret == WAIT_TIMEOUT) 
		CPGPnetDebugLog::instance()->dbgOut(
			"worker thread: wait timeout");
	else if (ret >= WAIT_ABANDONED_0 || ret >= WAIT_OBJECT_0) {
		CPGPnetDebugLog::instance()->dbgOut(
			"All worker threads finished");
	}

	delete [] lpHandles;
	return;
}

void
CPGPnetIKEmanager::WaitForSuspendFinish()
{
	HANDLE *lpHandles;
	DWORD ret;
	PGPInt32 i = 1;

	if (m_numWorkers == 0)
		return;

	lpHandles = new HANDLE[m_numKernelWorkers + 1];
	if (!lpHandles) {
		reportOutOfMemory(m_numWorkers);
		return;
	}

	// Add Kernel Workers
	for (i = 1; i < m_numKernelWorkers + 1; i++)
		lpHandles[i-1] = m_workers[i]->threadID();

	// Add IKE Worker
	lpHandles[i-1] = m_workers[i+1]->threadID();
	
	ret = WaitForMultipleObjects(i, lpHandles, TRUE, INFINITE);
	if (ret == WAIT_TIMEOUT) 
		CPGPnetDebugLog::instance()->dbgOut(
			"worker thread: wait timeout");
	else if (ret >= WAIT_ABANDONED_0 || ret >= WAIT_OBJECT_0) {
		CPGPnetDebugLog::instance()->dbgOut(
			"All Suspend threads finished");
	}

	delete [] lpHandles;
	return;
}

PGPInt32
CPGPnetIKEmanager::initializeWSA()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	const WORD v1 = MAKEWORD(1,0);
	const WORD v11 = MAKEWORD(1,1);
	const WORD v2 = MAKEWORD(2,0);
	int err, tries = 0;
	 
	wVersionRequested = v1;
 
retryStartup:
	err = WSAStartup( wVersionRequested, &wsaData );
	switch (err) {
	case 0:
		break;
	case WSAEINPROGRESS:
		Sleep(10);
		if(++tries > 60) {
			CPGPnetAppLog::instance()->logSysEvent(err, 0, __FILE__, __LINE__);
			return -1;
		}
		goto retryStartup;
		break;
	case WSAVERNOTSUPPORTED:
		switch (wVersionRequested) {
		case v1:
			wVersionRequested = v11;
			break;
		case v11:
			wVersionRequested = v2;
			break;
		case v2:
			CPGPnetAppLog::instance()->logSysEvent(err, 0, __FILE__, __LINE__);
			return -1;
			break;
		}
		goto retryStartup;
		break;
	default:
		CPGPnetAppLog::instance()->logSysEvent(err, 0, __FILE__, __LINE__);
		return -1;
	    break;
	}

	tries = 0;

	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_socket == INVALID_SOCKET) {
		CPGPnetAppLog::instance()->logSysEvent(WSAGetLastError(), 
			0,
			__FILE__,
			__LINE__);
		return -1;
	}

	unsigned long makeItSo;
	makeItSo = 1;
	(void) ioctlsocket(m_socket, FIONBIO, &makeItSo);

	struct sockaddr_in sin;
  
	pgpClearMemory(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(kPGPike_CommonPort);
    sin.sin_addr.s_addr = INADDR_ANY;

retryBind:
	if (bind(m_socket, (struct sockaddr *) &sin, sizeof sin) != 0) {
		err = WSAGetLastError();
		if (err == WSAEADDRINUSE) {
			Sleep(10);
			if(++tries <= 60)
				goto retryBind;
		} else if (err == WSAEINPROGRESS) {
			Sleep(10);
			if(++tries <= 60)
				goto retryBind;
		}
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		CPGPnetAppLog::instance()->logSysEvent(err, 0, __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

void
CPGPnetIKEmanager::reportOutOfMemory(PGPInt32 nMadeIt)
{
	CPGPnetAppLog::instance()->logPGPEvent(
		kPGPError_OutOfMemory,
		0,
		__FILE__,
		__LINE__);
	
	if (m_workers) {
		for (PGPInt32 i = 0; i < nMadeIt; i++)
			delete m_workers[i];

		delete [] m_workers;
	}
	m_numWorkers = 0;
	m_numKernelWorkers = 0;
}