/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetWriteWorker.cpp,v 1.13 1999/05/20 03:06:31 elowe Exp $
____________________________________________________________________________*/

#include <winsock2.h>
#include <assert.h>
#include <queue>

// PGP includes
#include "pgpRMWOLock.h"
#include "pgpErrors.h"
#include "pgpMem.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpNetQueueElement.h"
#include "pgpRWLockSignal.h"

#include "pgpNetWriteWorker.h"

// external globals
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_UDPQueue;
extern PGPCond_t		g_ExitEvent;
extern PGPContextRef	g_Context;

CPGPnetWriteWorker::CPGPnetWriteWorker(SOCKET s)
: m_socket(s)
{
//	XXX m_writeEvent = WSACreateEvent();
//	XXX WSAEventSelect(s, m_writeEvent, FD_WRITE);
}

CPGPnetWriteWorker::~CPGPnetWriteWorker()
{
//	XXX WSACloseEvent(m_writeEvent);
}

unsigned int
CPGPnetWriteWorker::Run()
{
	PGPUInt32		ret = 0;
	const PGPInt16	numHandles = 2;
	HANDLE			lpHandles[numHandles];

	lpHandles[0] = g_ExitEvent;
	lpHandles[1] = g_UDPQueue.event();
	
	PGPBoolean running = TRUE;
	while (running) {
		ret = WaitForMultipleObjects(numHandles,
									 lpHandles, 
									 FALSE, 
									 INFINITE);
		if (ret >= WAIT_ABANDONED_0) {
			CPGPnetDebugLog::instance()->dbgOut(
				"WriteWorker: something abandoned");
			running = FALSE;
			goto end;
			break;
		} else if (ret >= WAIT_OBJECT_0) {
			switch (ret - WAIT_OBJECT_0) {
			case 0:
				CPGPnetDebugLog::instance()->dbgOut(
					"WriteWorker: ExitEvent signaled");
				running = FALSE;
				goto end;
				break;
			case 1:
				CPGPnetDebugLog::instance()->dbgOut(
					"WriteWorker: UDPQueue signaled");

				processQueue();
				break;
			default:
				CPGPnetDebugLog::instance()->dbgOut(
					"WriteWorker: something signaled");
				break;
			}
		}
	}

end:
	return kPGPError_NoErr;
}

void
CPGPnetWriteWorker::processQueue()
{
	struct sockaddr_in	toAddr;
	PGPInt32			toLen = sizeof(toAddr);

	while (!g_UDPQueue.data().empty()) {
		g_UDPQueue.startReading();
		// pull one thing off;
		CPGPnetQueueElement queueElem = g_UDPQueue.data().front();
		g_UDPQueue.stopReading();

		g_UDPQueue.startWriting();
		g_UDPQueue.data().pop(); 
		g_UDPQueue.stopWriting();

		if (queueElem.type() != kPGPike_MT_Packet)
			continue;

		toAddr.sin_family = AF_INET;
		toAddr.sin_port = htons(kPGPike_CommonPort);
		toAddr.sin_addr.s_addr = queueElem.ipAddress();

		CPGPnetDebugLog::instance()->dbgOut(
			"WriteWorker: Sending UDP Packet (size: %d) to %s",
			queueElem.size(),
			inet_ntoa(toAddr.sin_addr));

		PGPInt32 ret = sendto(m_socket, 
			(char*)queueElem.data(), 
			queueElem.size(), 
			0, 
			(struct sockaddr*)&toAddr, 
			toLen);
		if (ret == SOCKET_ERROR) {
			CPGPnetAppLog::instance()->logSysEvent(WSAGetLastError(),
				queueElem.ipAddress(),
				__FILE__,
				__LINE__);
		} else if (ret < queueElem.size()) {

			CPGPnetDebugLog::instance()->dbgOut(
				"WriteWorker: only managed to send %d of %d",
				ret,
				queueElem.size());

			// XXX what do to here? should this ever happen?
		} else {
			CPGPnetDebugLog::instance()->dbgOut("WriteWorker: sendto returned %d", ret);
		}
	}
	g_UDPQueue.resetEvent();
}
