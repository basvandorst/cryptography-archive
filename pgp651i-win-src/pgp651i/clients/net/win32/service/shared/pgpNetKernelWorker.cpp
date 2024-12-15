/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetKernelWorker.cpp,v 1.61.4.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <assert.h>
#include <winioctl.h>
#include <ntddndis.h>
#include <queue>

// PGP includes
#include "pgpRMWOLock.h"
#include "pgpErrors.h"
#include "pgpMem.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpIKE.h"
#include "pgpRWLockSignal.h"
#include "pgpNetCConfig.h"
#include "pgpEndianConversion.h"
#include "pgpNetQueueElement.h"
#include "pgpNetDSAarray.h"
#include "pgpNetIPC.h"
#include "pgpNetKernelXChng.h"

#include "pgpNetKernelWorker.h"

#define DBGOUT(s) CPGPnetDebugLog::DbgOutFL(__FILE__, __LINE__); \
			CPGPnetDebugLog::DbgOut((s));

// external globals
extern PGPCond_t							g_ShutdownRequest;
extern PGPCond_t							g_localIPEvent;
extern PGPCond_t							g_SuspendEvent;
extern PGPUInt32							g_localIP;
extern PGPContextRef						g_Context;
extern CPGPrwLockSignal< CPGPnetCConfig > *	g_pConfig;
extern DWORD								g_platformID;
extern PGPBoolean							g_isWAN;

extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_IKEQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_GUIQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_KernelQueue;
extern CPGPrwLockSignal<CPGPnetDSAarray>					g_SAarray;


CPGPnetKernelWorker::CPGPnetKernelWorker()
: m_hMac(INVALID_HANDLE_VALUE), m_MacName("")
{
}

void CPGPnetKernelWorker::debugMsg(int lineno, const char *fmt, ...)
{
	if (CPGPnetDebugLog::isDebug()) {
		char temp[256];
		sprintf(temp, "KernelWorker(%d)\t:\t", lineno);
		va_list args;
		va_start(args, fmt);
		CPGPnetDebugLog::instance()->vdbgOut(fmt, args, temp);
		va_end(args);
	}
}

unsigned int
CPGPnetKernelWorker::Run()
{
	PGPUInt32		ret = 0;
	const PGPInt16	numHandles = 4;
	HANDLE			lpHandles[numHandles];
	std::string		szMacNTName = "\\\\.\\";
	PGPBoolean		running = TRUE;
	PGPEVENT_CONTEXT pgpEvent;

	lpHandles[0] = g_ShutdownRequest;
	lpHandles[1] = g_KernelQueue.event();
	lpHandles[3] = g_SuspendEvent;

	debugMsg(__LINE__, ">> Run");
	// look for name of driver in registry
	ret = getMacName();
	szMacNTName += m_MacName;

	debugMsg(__LINE__, "MacName: %s", szMacNTName.c_str());

	m_hMac = CreateFile(szMacNTName.c_str(),
                  GENERIC_READ,
                  0, // FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL,
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL,
                  NULL
                  );

    if (m_hMac == INVALID_HANDLE_VALUE) {
		debugMsg(__LINE__, "Damn, unable to open driver");

		CPGPnetAppLog::instance()->logSysEvent(GetLastError(), 
			0,
			__FILE__, 
			__LINE__);
		CPGPnetAppLog::instance()->logServiceEvent(kPGPnetSrvcError_NoDriver,
			0,
			0,
			__FILE__,
			__LINE__);
		g_pConfig->startWriting();
		g_pConfig->data().isDriverUp(FALSE);
		g_pConfig->stopWriting();
		goto end;
    }

	if (!createSharedMemory()) {
		debugMsg(__LINE__, "Unable to create shared mem");
		CPGPnetAppLog::instance()->logServiceEvent(kPGPnetSrvcError_DrvSharedMemory,
			0,
			0,
			__FILE__,
			__LINE__);
		g_pConfig->startWriting();
		g_pConfig->data().isDriverUp(FALSE);
		g_pConfig->stopWriting();
		goto end;
	}

	// create SA event thingy
	if (!createKernelEvent(&pgpEvent)) {
		debugMsg(__LINE__, "Unable to create kernel event");
		CPGPnetAppLog::instance()->logServiceEvent(kPGPnetSrvcError_DrvEvent,
			0,
			0,
			__FILE__,
			__LINE__);
		g_pConfig->startWriting();
		g_pConfig->data().isDriverUp(FALSE);
		g_pConfig->stopWriting();
		goto end;
	} else {
		lpHandles[2] = pgpEvent.Win32EventHandle;
	}

	if (!createMsgQueue()) {
		debugMsg(__LINE__, "Unable to create message queue");
		CPGPnetAppLog::instance()->logServiceEvent(kPGPnetSrvcError_DrvSharedMemory,
			0,
			0,
			__FILE__,
			__LINE__);
		g_pConfig->startWriting();
		g_pConfig->data().isDriverUp(FALSE);
		g_pConfig->stopWriting();
		goto end;
	}
	
	debugMsg(__LINE__, "Driver looks good");
	g_pConfig->startWriting();
	g_pConfig->data().isDriverUp(TRUE);
	g_pConfig->stopWriting();

	debugMsg(__LINE__, "Starting wait loop");
	while (running) {
		ret = WaitForMultipleObjects(numHandles,
									 lpHandles, 
									 FALSE, 
									 5000);
		if (ret == WAIT_TIMEOUT) {
			// XXX collect some statistics and send to app
			// if app is around
			// also send to tray app if present
		} else if (ret >= WAIT_ABANDONED_0) {
			debugMsg(__LINE__, "something abandoned");
			running = FALSE;
			goto end;
		} else if (ret >= WAIT_OBJECT_0) {
			switch (ret - WAIT_OBJECT_0) {
			case 0:
				debugMsg(__LINE__, "ShutdownRequest signaled");
				running = FALSE;
				goto end;
				break;
			case 1:
				debugMsg(__LINE__, "Queue signaled");
				processQueue();
				break;
			case 2:
				debugMsg(__LINE__, "Event from kernel");
				processKernelEvent();
				break;
			case 3:
			{
				debugMsg(__LINE__, "Got Suspend Event");

				if (g_SAarray.data().numSAs() > 0)
					break;

				if (m_hMac != INVALID_HANDLE_VALUE) {
					if (m_pSharedMem) {
						deleteMsgQueue();
						deleteSharedMemory();
						deleteKernelEvent(&pgpEvent);
					}
					CloseHandle(m_hMac);
				}

				return kPGPError_NoErr;
			}
			break;
			default:
				debugMsg(__LINE__, "Something signaled");
				break;
			}
		}
	}

end:
	debugMsg(__LINE__, "Shutting down");
	if (m_hMac != INVALID_HANDLE_VALUE) {
		if (m_pSharedMem) {
			DWORD dwRet;
			(void) sendKernelRequest(OID_PGP_SHUTDOWN, 0, 0, &dwRet);
			deleteMsgQueue();
			deleteSharedMemory();
			deleteKernelEvent(&pgpEvent);
		}
		CloseHandle(m_hMac);
	}

	debugMsg(__LINE__, "<< Run");
	return kPGPError_NoErr;
}

PGPBoolean 
CPGPnetKernelWorker::sendKernelRequest(PGPUInt32 blockID, 
									   void *pBuffer, 
									   PGPUInt32 BufferSize, 
									   ULONG *pReturnedCount)
{
	ULONG		dwReturnedCount;
	PGPUInt32	dwBuffer;
	ULONG *		pMyReturnedCount;
	void *		pMyBuffer;
	PGPBoolean	result = FALSE;

	debugMsg(__LINE__, ">> sendKernelRequest(%x)", blockID);
	if (pReturnedCount) {
		pMyReturnedCount = pReturnedCount;
	} else {
		pMyReturnedCount = &dwReturnedCount;
	}

	*pMyReturnedCount = 0;
    
	if (pBuffer && BufferSize) {
		pMyBuffer = pBuffer;
	} else {
		pMyBuffer = &dwBuffer;
		BufferSize = sizeof(dwBuffer);
	}
    
	debugMsg(__LINE__, "Buffer size: %d", BufferSize);

	if (m_hMac != INVALID_HANDLE_VALUE) {

		debugMsg(__LINE__, "Sending DeviceIoControl()");

		result = DeviceIoControl(m_hMac,
			IOCTL_NDIS_QUERY_GLOBAL_STATS,
			&blockID,
			sizeof(blockID),
			pMyBuffer,
			BufferSize,
			pMyReturnedCount,
			NULL);
	
		debugMsg(__LINE__, "DeviceIoControl returned, result: %d, ReturnedCount: %d",
			result,
			*pMyReturnedCount);

		if (result) {
			if (pReturnedCount == NULL && dwReturnedCount > 0) {
				CPGPnetAppLog::instance()->logServiceEvent(
					kPGPnetSrvcError_DrvCommunication,
					0,
					0,
					__FILE__,
					__LINE__);
				debugMsg(__LINE__, "Unexpected data size=%d != 0\n",
					dwReturnedCount);
				result = FALSE;
			}
		} else {
			debugMsg(__LINE__, "Something wrong\n");
			CPGPnetAppLog::instance()->logSysEvent(GetLastError(),
				0,
				__FILE__,
				__LINE__);
			CPGPnetAppLog::instance()->logServiceEvent(
				kPGPnetSrvcError_DrvCommunication,
				0,
				0,
				__FILE__,
				__LINE__);
		}
	}
	debugMsg(__LINE__, "<< sendKernelRequest(%d)", result);
	return (result);
}

void
CPGPnetKernelWorker::processQueue()
{
	debugMsg(__LINE__, ">> processQueue");

	while (!g_KernelQueue.data().empty()) {
		g_KernelQueue.startReading();
		// pull one thing off;
		CPGPnetQueueElement queueElem = g_KernelQueue.data().front();
		g_KernelQueue.stopReading();

		g_KernelQueue.startWriting();
		g_KernelQueue.data().pop();
		g_KernelQueue.stopWriting();

		ULONG dwRet;
		switch (queueElem.type()) {
		case OID_PGP_NEWSA:
		{
			debugMsg(__LINE__, "Got OID_PGP_NEWSA");

			PGPikeSA *pSA = 0;
			pgpCopyMemory(queueElem.data(), &pSA, sizeof(PGPikeSA*));

			(void) sendKernelRequest(OID_PGP_NEWSA, 
				const_cast<void*>(pSA),
				sizeof(PGPikeSA),
				&dwRet);
			break;
		}
		case OID_PGP_SADIED:
			debugMsg(__LINE__, "Got OID_PGP_SADIED");

			if (queueElem.size() != sizeof(PGPipsecSPI)) {
				debugMsg(__LINE__, "SADIED wrong size data");
				pgpAssert(FALSE);
				return;
			}
			(void) sendKernelRequest(queueElem.type(), 
				const_cast<void*>(queueElem.data()),
				queueElem.size(),
				&dwRet);
			break;
		case OID_PGP_SAFAILED:
			debugMsg(__LINE__, "Got OID_PGP_SAFAILED");

			if (queueElem.size() != sizeof(PGPikeMTSAFailed)) {
				debugMsg(__LINE__, "SAFAILED wrong size data");
				pgpAssert(FALSE);
				return;
			}

			(void) sendKernelRequest(queueElem.type(), 
				const_cast<void*>(queueElem.data()),
				queueElem.size(),
				&dwRet);
			break;
		case OID_PGP_SAUPDATE:
			debugMsg(__LINE__, "Got OID_PGP_SAUPDATE");

			if (queueElem.size() != sizeof(PGPipsecSPI)) {
				debugMsg(__LINE__, "SAUPDATE wrong size data");
				pgpAssert(FALSE);
				return;
			}
			// fall-through
		case OID_PGP_ALLHOSTS:
		case OID_PGP_NEWHOST:
		case OID_PGP_NEWCONFIG:
			debugMsg(__LINE__, "Got One of OID_PGP_ALLHOSTS, OID_PGP_NEWHOST, OID_PGP_NEWCONFIG");

			(void) sendKernelRequest(queueElem.type(), 
				const_cast<void*>(queueElem.data()),
				queueElem.size(),
				&dwRet);
			break;
		case OID_PGP_LOCALIP:
			debugMsg(__LINE__, "Got OID_PGP_LOCALIP");

			(void) getLocalIP();
			PGPCondSignal(&g_localIPEvent);
			break;
		case PGPNET_ATTEMPTIKE:
		{
			debugMsg(__LINE__, "Got PGPNET_ATTEMPTIKE");

			// queueElem.ipAddress isn't the ipAddress of the host
			// but rather the index of the host in the host list
			// The ipAddress field was just a convienant place to put
			// the index.
			g_pConfig->startReading();
			PGPNetHostEntry * host =
				g_pConfig->data().findHostByIndex(queueElem.ipAddress());

			if (host) {
				if (host->childOf != -1) {
					PGPNetHostEntry * gw =
						g_pConfig->data().findHostByIndex(host->childOf);
					// Ask for tunnel SA
					doSARequest(
						gw,
						gw->ipAddress,
						host->ipAddress,
						host->ipMask);
				}
				if (host->hostType == kPGPnetSecureHost) {
					// Ask for transport SA
					doSARequest(host, host->ipAddress, 0, 0);
				}
				if (host->hostType == kPGPnetSecureGateway) {
					// Ask for tunnel SA with itself
					doSARequest(host, 
						host->ipAddress,
						host->ipAddress,
						host->ipMask);
				}
			}
			g_pConfig->stopReading();
			break;
		}
		default:
			break;
		}
	}
	g_KernelQueue.resetEvent();

	debugMsg(__LINE__, "<< processQueue");
}

#ifdef BOGUS_SAREQUEST
const char *        kTestSharedKey          = "test";
PGPUInt32 localIP;

void
CPGPnetKernelWorker::generateBogusSARequest()
{
	PGPikeMTSASetup tS;
	PGPBoolean found = FALSE;
	PGPInt32 i = 0;
	PGPNetHostEntry *host = 0;

	pgpClearMemory( &tS, sizeof( PGPikeMTSASetup ) );

	// fill in the values that I can right now
	tS.approved = TRUE;
	tS.doi = kPGPike_DOI_IPSEC;
	tS.u.ipsec.packetMode = kPGPike_PM_Transport;

	g_pConfig->startReading();

	host = &(g_pConfig->data().config()->pHostList[0]); // just grab the first one

	//struct hostent *hent;
    //hent = gethostbyname("www.nai.com");
	//pgpCopyMemory(hent->h_addr_list[0], &localIP, sizeof(localIP));

	localIP = PGPStorageToUInt32(host->ipAddress);

	tS.ipAddress = localIP;
	/*
	tS.sharedKey = (PGPByte*)host->sharedSecret;
	tS.sharedKeySize = kMaxNetHostSharedSecretLength + 1;
	tS.u.ipsec.idType = (PGPipsecIdentity)PGPStorageToUInt32(host->identityType);
	tS.u.ipsec.idData = host->identity;
	tS.u.ipsec.idDataSize = kMaxNetHostIdentityLength + 1;
	*/

	/* draft-ietf-ipsec-internet-key-00.txt */
	tS.sharedKey			= (PGPByte *) kTestSharedKey;
	tS.sharedKeySize		= strlen(kTestSharedKey);
	tS.u.ipsec.idData		= (PGPByte *)&localIP;
	tS.u.ipsec.idDataSize	= sizeof(localIP);
	tS.u.ipsec.idType		= kPGPike_ID_IPV4_Addr;

	g_pConfig->stopReading();

	g_IKEQueue.startWriting();

	g_IKEQueue.data().push(CPGPnetQueueElement(sizeof(tS),
		&tS, 
		kPGPike_MT_SARequest, 0));

	g_IKEQueue.stopWriting();

	g_IKEQueue.setEvent();
}
#endif

HANDLE __stdcall OpenVxDHandle(
    HANDLE   hEvent                    
    )
{
	typedef DWORD (FAR WINAPI *FUNCPTR)(HANDLE);

    HINSTANCE   hModInst;
    FUNCPTR		lpOpenVxDHandle = NULL;
    HANDLE      hVxdEvent = hEvent;

    hModInst = LoadLibrary( "KERNEL32.DLL" );
    
    if (hModInst)
    {
        lpOpenVxDHandle = (FUNCPTR) GetProcAddress(hModInst, (LPSTR)"OpenVxDHandle");
        if (lpOpenVxDHandle)
        {
            hVxdEvent = (HANDLE) (*lpOpenVxDHandle) (hEvent);
        }
        else
        {
			CPGPnetDebugLog::instance()->dbgOut("KernelWorker\t:\tGetProcAddress FAILED");
        }
        FreeLibrary(hModInst);
    }
    return (hVxdEvent);
}

PGPBoolean
CPGPnetKernelWorker::createKernelEvent(PPGPEVENT_CONTEXT pPgpEvent)
{
    ULONG               dwBytesReturned = 0;
    BOOLEAN             result = FALSE;
    HANDLE              hEvent;

	debugMsg(__LINE__, ">> createKernelEvent");

	pgpClearMemory(pPgpEvent, sizeof(*pPgpEvent));

	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		static UCHAR        EventName[] = "PgpTstEvent";
		pgpCopyMemory(EventName /*src*/, 
			(PWCHAR)pPgpEvent->EventName /*dest*/,
			sizeof(EventName));
		
		hEvent = CreateEventA(NULL, FALSE, FALSE, (char*)EventName);
		if (hEvent)
		{
			pPgpEvent->Win32EventHandle = hEvent;
			pPgpEvent->DriverEventHandle = OpenVxDHandle(hEvent);
			if (!sendKernelRequest(OID_PGP_EVENT_CREATE, pPgpEvent, 
				sizeof(*pPgpEvent), &dwBytesReturned)
				|| dwBytesReturned != sizeof(*pPgpEvent))
			{
				debugMsg(__LINE__, "createKernelEvent FAILED - Unexpected size=%d or errorCode=%d",
					dwBytesReturned,
					GetLastError());
			}
			else if (pPgpEvent->DriverEventHandle == NULL)
			{
				debugMsg(__LINE__, "OID_PGP_EVENT_CREATE FAILED");
			}
			else
			{
		        result = TRUE;
			}
		}
		else
		{
			debugMsg(__LINE__,  "OID_PGP_EVENT_CREATE FAILED. Not able to create Win32 event.");
		}
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
	    static WCHAR        EventName[] = L"PgpTstEvent";
		pgpCopyMemory(EventName /*src*/, 
			(PWCHAR)pPgpEvent->EventName /*dest*/,
			sizeof(EventName));

		debugMsg(__LINE__, "Sending OID_PGP_EVENT_CREATE request");
		if (!sendKernelRequest(OID_PGP_EVENT_CREATE, pPgpEvent, 
			sizeof(*pPgpEvent), &dwBytesReturned)
			|| dwBytesReturned != sizeof(*pPgpEvent))
		{
			debugMsg(__LINE__, "createKernelEvent FAILED - Unexpected size=%d or errorCode=%d\n",
				dwBytesReturned,
				GetLastError());
		}
		else if (pPgpEvent->DriverEventHandle == NULL)
		{
	        debugMsg(__LINE__, "OID_PGP_EVENT_CREATE FAILED\n");
		}
		else
		{
	        hEvent = OpenEventW(SYNCHRONIZE, FALSE, EventName);
			if (hEvent)
			{
	            pPgpEvent->Win32EventHandle = hEvent;
				result = TRUE;
			}
			else
			{
	            debugMsg(__LINE__, "CreateEvent ErrorCode=%d - sending OID_PGP_EVENT_DESTROY", GetLastError());
            
				if (!sendKernelRequest(OID_PGP_EVENT_DESTROY, pPgpEvent, 
					sizeof(*pPgpEvent), &dwBytesReturned)
					|| dwBytesReturned != sizeof(*pPgpEvent))
				{
	                debugMsg(__LINE__, "Unexpected size=%d or errorCode=%d\n",
						dwBytesReturned, GetLastError());
				}
			}
		}
	}

	debugMsg(__LINE__, "<< createKernelEvent(%d)", result);
    return (result);
}

void
CPGPnetKernelWorker::deleteKernelEvent(PPGPEVENT_CONTEXT pPgpEvent)
{
    ULONG	dwBytesReturned = 0;

	debugMsg(__LINE__, ">> deleteKernelEvent");
	if (pPgpEvent) {
		if (g_platformID == VER_PLATFORM_WIN32_NT) {
			if (pPgpEvent->Win32EventHandle) {
				CloseHandle(pPgpEvent->Win32EventHandle);
				pPgpEvent->Win32EventHandle = NULL;
			}
		}
		if (pPgpEvent->DriverEventHandle) {
			sendKernelRequest(OID_PGP_EVENT_DESTROY, 
				pPgpEvent, 
				sizeof(*pPgpEvent), 
				&dwBytesReturned);
			pPgpEvent->DriverEventHandle = NULL;
		}
	}
	debugMsg(__LINE__, "<< deleteKernelEvent");
}

PGPBoolean
CPGPnetKernelWorker::createSharedMemory()
{
    ULONG       dwBytesReturned = 0;
    PVOID       tempPtr = NULL;
    HANDLE      ProcessHandle;

	debugMsg(__LINE__, ">> createSharedMemory");
    ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION,
                                FALSE,
                                GetCurrentProcessId());
 
    if (ProcessHandle == NULL)
    { 
		debugMsg(__LINE__, "OpenProcess failed, ErrorCode=%d",
			GetLastError());
    } 
    else
    {
        tempPtr = static_cast<void*>(ProcessHandle);
		debugMsg(__LINE__, "sending OID_PGP_SHARED_MEM_ALLOC");
        if (!sendKernelRequest(OID_PGP_SHARED_MEM_ALLOC, &tempPtr, 
                      sizeof(tempPtr), &dwBytesReturned)
            || dwBytesReturned != sizeof(tempPtr))
        {
			debugMsg(__LINE__, "Unexpected size=%d or errorCode=%d\n",
				dwBytesReturned, GetLastError());
            tempPtr = NULL;
        }
        CloseHandle(ProcessHandle);
    }
    
	m_pSharedMem = static_cast<PGPByte*>(tempPtr);

	debugMsg(__LINE__, "<< createSharedMemory(%d)", (m_pSharedMem != NULL));
    return (m_pSharedMem != NULL);
}

PGPBoolean
CPGPnetKernelWorker::createMsgQueue()
{
    ULONG       dwBytesReturned = 0;

	PGPMESSAGE_CONTEXT*	userMessageContext;
	
	userMessageContext = (PGPMESSAGE_CONTEXT*)(m_pSharedMem);
	
	debugMsg(__LINE__, ">> createMsgQueue");

	pgpClearMemory(userMessageContext, sizeof(PGPMESSAGE_CONTEXT));
	userMessageContext->header.head = 0;
	userMessageContext->header.tail = 0;
	userMessageContext->header.maxSlots = MaxSlots;
	userMessageContext->header.maxSlotSize = MaxSlotSize;

	debugMsg(__LINE__, "Sending OID_PGP_QUEUE_INITIALIZE");
	if (!sendKernelRequest(OID_PGP_QUEUE_INITIALIZE,
			userMessageContext,
			sizeof(PGPMESSAGE_CONTEXT),
			&dwBytesReturned))	{
		debugMsg(__LINE__, "Error creating message queue: %d",
			GetLastError());
		userMessageContext = NULL;
	}

	m_pMsgQueue = userMessageContext;

	debugMsg(__LINE__, "<< createMsgQueue(%d)", (m_pMsgQueue != 0));
	return (m_pMsgQueue != 0);
}

void
CPGPnetKernelWorker::deleteMsgQueue()
{
	ULONG       dwBytesReturned = 0;

	debugMsg(__LINE__, ">> deleteMsgQueue");
	sendKernelRequest(OID_PGP_QUEUE_RELEASE, NULL, 0, &dwBytesReturned);
	debugMsg(__LINE__, "<< deleteMsgQueue");
}

void
CPGPnetKernelWorker::deleteSharedMemory()
{
	ULONG       dwBytesReturned = 0;

	debugMsg(__LINE__, ">> deleteSharedMemory");
	if (!m_pSharedMem)
		return;

	HANDLE ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION,
		FALSE,
		GetCurrentProcessId());
 
	if (ProcessHandle != NULL) { 
		sendKernelRequest(OID_PGP_SHARED_MEM_FREE,
			&ProcessHandle,
			sizeof(ProcessHandle),
			NULL);
		CloseHandle(ProcessHandle);
	}
	debugMsg(__LINE__, "<< deleteSharedMemory");
}    

void
CPGPnetKernelWorker::processKernelEvent()
{
	PGPMESSAGE_CONTEXT	*	userMessageContext;
	PGPMESSAGE_CONTEXT	*	currentMsg;
	PGPnetMessageHeader *	userMessageHeader;

	debugMsg(__LINE__, ">> processKernelEvent");

	userMessageContext = m_pMsgQueue;
	userMessageHeader = &userMessageContext->header;

	debugMsg(__LINE__, "starting to process kernel events");
	while (userMessageHeader->tail != userMessageHeader->head)
	{
		userMessageHeader->tail++;

		if (userMessageHeader->tail > userMessageHeader->maxSlots)
			userMessageHeader->tail = 1;

		currentMsg = &userMessageContext[userMessageHeader->tail];

		switch (currentMsg->messageType)
		{
		case PGPnetMessageHeaderType:

			debugMsg(__LINE__, "should never get a HeaderType message!");
			break;

		case PGPnetMessageSARequest:
		{
			debugMsg(__LINE__, "Got SARequest");

			PGPnetSARequestMsg *	pSARequestMsg = 0;
			pSARequestMsg = (PGPnetSARequestMsg*)&currentMsg->message;
			if (pSARequestMsg)
				doSARequest(pSARequestMsg->ipAddress, pSARequestMsg->ipAddrStart, pSARequestMsg->ipMaskEnd);
			break;
		}

		case PGPnetMessageSARekey:
		{
			debugMsg(__LINE__, "Got SARekey");

			PGPnetSARekeyMsg *	pSARekeyMsg = 0;
			pSARekeyMsg = (PGPnetSARekeyMsg*)&currentMsg->message;
			if (pSARekeyMsg)
				doSARekey(pSARekeyMsg->spi);
			
			break;

		}
		case PGPnetMessageSADied:
		{
			debugMsg(__LINE__, "Got SADied");

			PGPnetSADiedMsg *	pSADiedMsg = 0;
			pSADiedMsg = (PGPnetSADiedMsg*)&currentMsg->message;
			if (pSADiedMsg)
				doSADied(pSADiedMsg->spi);
			
			break;
		}
		case PGPnetMessageError:
		{
			debugMsg(__LINE__, "Got Error from Kernel");

			PGPnetErrorMsg *	pErrorMsg = 0;
			pErrorMsg = (PGPnetErrorMsg*)&currentMsg->message;
			if (pErrorMsg) {
				CPGPnetAppLog::instance()->logPGPEvent(
					pErrorMsg->errorCode,
					pErrorMsg->ipAddress,
					__FILE__, 
					__LINE__);
			}
			
			break;
		}
		case PGPnetMessageRASdisconnect:
			debugMsg(__LINE__, "Received RAS event");

			// put onto IKE Queue
			g_IKEQueue.startWriting();
			g_IKEQueue.data().push(
				CPGPnetQueueElement(0, 0, kPGPike_MT_SAKillAll, 0));
			g_IKEQueue.stopWriting();
			g_IKEQueue.setEvent();

			break;
		case PGPnetMessageReserved:
		default:
			debugMsg(__LINE__, "Message Type not supported");
			break;
		}
	}
	debugMsg(__LINE__, "<< processKernelEvent");
}

void
CPGPnetKernelWorker::doSADied(PGPipsecSPI spi)
{		
	debugMsg(__LINE__, ">> doSADied(%d)", spi);
	g_SAarray.startReading();
	PGPikeSA* pSA = g_SAarray.data().findSA(spi, sizeof(PGPipsecSPI));
	g_SAarray.stopReading();
	if (!pSA)
		return;

	// put a copy of the *pointer* onto IKE queue
	g_IKEQueue.startWriting();
	g_IKEQueue.data().push(CPGPnetQueueElement(
		sizeof(PGPikeSA*),
		&pSA,
		kPGPike_MT_SADied,
		pSA->ipAddress));
	g_IKEQueue.stopWriting();
	g_IKEQueue.setEvent();
	debugMsg(__LINE__, "<< doSADied(%d)", spi);
}

void
CPGPnetKernelWorker::doSARekey(PGPipsecSPI spi)
{
	debugMsg(__LINE__, ">> doSARekey(%d)", spi);
	g_SAarray.startReading();
	PGPikeSA* pSA = g_SAarray.data().findSA(spi, sizeof(PGPipsecSPI));
	g_SAarray.stopReading();
	if (!pSA)
		return;

	// put a copy of the *pointer* onto IKE queue
	g_IKEQueue.startWriting();
	g_IKEQueue.data().push(CPGPnetQueueElement(
		sizeof(PGPikeSA*),
		&pSA,
		kPGPike_MT_SARekey,
		pSA->ipAddress));
	g_IKEQueue.stopWriting();
	g_IKEQueue.setEvent();

	debugMsg(__LINE__, "<< doSARekey(%d)", spi);
}

void
CPGPnetKernelWorker::doSARequest(PGPUInt32 ipAddress, 
								 PGPUInt32 ipAddrStart, 
								 PGPUInt32 ipMaskEnd)
{
	PGPNetHostEntry *host = 0;

	debugMsg(__LINE__, ">> doSARequest(%s,%s,%s)", inet_ntoa(*((struct in_addr *) &ipAddress)),
		inet_ntoa(*((struct in_addr *) &ipAddrStart)),
		inet_ntoa(*((struct in_addr *) &ipMaskEnd)));

	g_pConfig->startReading();

	host = g_pConfig->data().findHost(ipAddress);
	if (host) {
		doSARequest(host, ipAddress, ipAddrStart, ipMaskEnd);
		g_pConfig->stopReading();
	} else {
		g_pConfig->stopReading();
		PGPikeMTSASetup tS;

		pgpClearMemory(&tS, sizeof(PGPikeMTSASetup));

		tS.approved = TRUE;
		tS.doi = kPGPike_DOI_IPSEC;
		tS.localIPAddress = getLocalIP(); // XXX error check
		tS.ipAddress = ipAddress;
		// trying to talk to an unconfigured host, must be transport mode
		tS.u.ipsec.packetMode = kPGPike_PM_Transport;

		g_IKEQueue.startWriting();

		g_IKEQueue.data().push(
			CPGPnetQueueElement(sizeof(tS),	&tS, kPGPike_MT_SARequest, 0));

		g_IKEQueue.stopWriting();
		g_IKEQueue.setEvent();
	}

	debugMsg(__LINE__, "<< doSARequest(%s,%s,%s)", inet_ntoa(*((struct in_addr *) &ipAddress)),
		inet_ntoa(*((struct in_addr *) &ipAddrStart)),
		inet_ntoa(*((struct in_addr *) &ipMaskEnd)));}

void
CPGPnetKernelWorker::doSARequest(PGPNetHostEntry *host,
								 PGPUInt32 ipAddress,
								 PGPUInt32 ipAddrStart,
								 PGPUInt32 ipMaskEnd)
{
	if (!host)
		return;

	debugMsg(__LINE__, ">> doSARequest(%s,%s,%s)", inet_ntoa(*((struct in_addr *) &ipAddress)),
		inet_ntoa(*((struct in_addr *) &ipAddrStart)),
		inet_ntoa(*((struct in_addr *) &ipMaskEnd)));

	PGPikeMTSASetup tS;

	pgpClearMemory(&tS, sizeof(PGPikeMTSASetup));

	tS.approved = TRUE;
	tS.doi = kPGPike_DOI_IPSEC;
	tS.localIPAddress = getLocalIP(); // XXX error check
	tS.ipAddress = ipAddress;

	if (host->hostType == kPGPnetSecureGateway) {
		tS.u.ipsec.packetMode = kPGPike_PM_Tunnel;
		tS.u.ipsec.destIsRange = FALSE;
		tS.u.ipsec.ipAddrStart = ipAddrStart;
		tS.u.ipsec.ipMaskEnd = ipMaskEnd;
		debugMsg(__LINE__, "Asking for Tunnel SA: %s, %s, %s",
			inet_ntoa(*((struct in_addr *) &host->ipAddress)),
			inet_ntoa(*((struct in_addr *) &ipAddrStart)),
			inet_ntoa(*((struct in_addr *) &ipMaskEnd)));
	} else if (host->hostType == kPGPnetSecureHost) {
		tS.u.ipsec.packetMode = kPGPike_PM_Transport;
	}

	if (host->sharedSecret) {
		// this is freed in IKEworker::processQueue()
		PGPUInt32 secretSize = strlen(host->sharedSecret);
		tS.sharedKey = new PGPByte[secretSize+1]; // w/ null

		pgpCopyMemory(host->sharedSecret /*src*/,
			tS.sharedKey /*dest*/,
			secretSize+1); // w/ null

		tS.sharedKeySize = secretSize; // w/o null
	} else {
		tS.sharedKey = 0;
		tS.sharedKeySize = 0;
	}
	tS.u.ipsec.idType = (PGPipsecIdentity)host->identityType;

	if (tS.u.ipsec.idType == kPGPike_ID_IPV4_Addr) {

		tS.u.ipsec.idDataSize = sizeof(PGPUInt32);

		// this is freed in IKEworker::processQueue()
		tS.u.ipsec.idData = (PGPByte*) PGPNewData(
			PGPGetContextMemoryMgr(g_Context),
			tS.u.ipsec.idDataSize,
			kPGPMemoryMgrFlags_Clear);	// XXX error check

		pgpCopyMemory(&(host->identityIPAddress) /*src*/,
			tS.u.ipsec.idData /*dest*/,
			tS.u.ipsec.idDataSize);

	} else if (tS.u.ipsec.idType == kPGPike_ID_DER_ASN1_DN) {
		// this is freed in IKEworker::processQueue();
		PGPError err = PGPCreateDistinguishedName(g_Context,
			host->identity,
			&tS.u.ipsec.idData,
			&tS.u.ipsec.idDataSize);	// XXX error check
	} else {
		tS.u.ipsec.idDataSize = strlen((char*)host->identity);

		// this is freed in IKEworker::processQueue()
		tS.u.ipsec.idData = (PGPByte*) PGPNewData(
			PGPGetContextMemoryMgr(g_Context),
			tS.u.ipsec.idDataSize + 1,	// include null terminator
			kPGPMemoryMgrFlags_Clear);	// XXX error check

		pgpCopyMemory(host->identity /*src*/,
			tS.u.ipsec.idData /*dest*/,
			tS.u.ipsec.idDataSize + 1); // w/ null
	}

	g_IKEQueue.startWriting();

	g_IKEQueue.data().push(
		CPGPnetQueueElement(sizeof(tS),	&tS, kPGPike_MT_SARequest, 0));

	g_IKEQueue.stopWriting();
	g_IKEQueue.setEvent();

	debugMsg(__LINE__, "<< doSARequest(%s,%s,%s)", inet_ntoa(*((struct in_addr *) &ipAddress)),
		inet_ntoa(*((struct in_addr *) &ipAddrStart)),
		inet_ntoa(*((struct in_addr *) &ipMaskEnd)));
}

PGPInt32
CPGPnetKernelWorker::getMacName()
{
	debugMsg(__LINE__, ">> getMacName");

	if (g_platformID == VER_PLATFORM_WIN32_WINDOWS) {
		m_MacName = "PgpMac";
		return ERROR_SUCCESS;
	} else if (g_platformID == VER_PLATFORM_WIN32_NT) {
		// look for name of driver in registry
		char *tmpString = 0;
		DWORD dwSize = 0;
		PGPInt32 ret = 0;
		std::string sKey;
		HKEY hKey;
		
		if (g_isWAN) {
			sKey = 
				"SYSTEM\\CurrentControlSet\\Services\\NdisWanPgpMacMp\\Linkage";
		} else {
			sKey = "SYSTEM\\CurrentControlSet\\Services\\PgpMacMp\\Linkage";
		}

		debugMsg(__LINE__, "Looking for name in (%s)", sKey.c_str());

		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,sKey.c_str(),0,KEY_READ,&hKey);
		if (ret == ERROR_SUCCESS) {
			ret = RegQueryValueEx(hKey, "Export", 0, 0, 0, &dwSize);
			tmpString = new char[dwSize];
			ret = RegQueryValueEx(hKey,
				"Export",
				0,
				0,
				(PGPByte*)tmpString,
				&dwSize);

			RegCloseKey(hKey);
			if (ret != ERROR_SUCCESS) {
				debugMsg(__LINE__, "Unable to query key: %d", GetLastError());
				m_MacName = "PgpMacMP2";
			} else {
				m_MacName = strrchr(tmpString, '\\');
				m_MacName.erase(m_MacName.begin());
			}
		} else {
			debugMsg(__LINE__, "Unable to open key: %d", GetLastError());
			m_MacName = "PgpMacMP2";
		}

		delete [] tmpString;
		debugMsg(__LINE__, "<< getMacName(%d)", ret);
		return ret;
	}
	
	debugMsg(__LINE__, "<< getMacName(0)");
	return 0;
}



PGPUInt32
CPGPnetKernelWorker::getLocalIP()
{
	debugMsg(__LINE__, "<< getLocalIP");
	if (g_isWAN && g_platformID == VER_PLATFORM_WIN32_NT) {
		char *tmpString = 0;
		DWORD dwSize = 0;
		HKEY hKey;
		PGPInt32 ret;
		std::string sKey = 
			"SOFTWARE\\Network Associates\\PGP\\PGPnet";
		std::string adapterNum, realMac;

		// Step One: Get AdapterNum
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,sKey.c_str(),0,KEY_READ,&hKey);
		if (ret != ERROR_SUCCESS)
			goto error;
		
		ret = RegQueryValueEx(hKey, "AdapterNum", 0, 0, 0, &dwSize);
		tmpString = new char[dwSize];
		ret = RegQueryValueEx(hKey,
			"AdapterNum",
			0,
			0,
			(PGPByte*)tmpString,
			&dwSize);

		RegCloseKey(hKey);
		if (ret != ERROR_SUCCESS) {
			delete [] tmpString;
			goto error;
		}
		adapterNum = tmpString;
			
		// Step Two: Get RealMac
		delete [] tmpString;
		sKey = "SYSTEM\\CurrentControlSet\\Services\\NdisWanPgpMacMp";
		sKey += adapterNum + "\\Parameters";

		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,sKey.c_str(),0,KEY_READ,&hKey);
		if (ret != ERROR_SUCCESS)
			goto error;

		ret = RegQueryValueEx(hKey, "RealMac", 0, 0, 0, &dwSize);
		tmpString = new char[dwSize];
		ret = RegQueryValueEx(hKey,
			"RealMac",
			0,
			0,
			(PGPByte*)tmpString,
			&dwSize);

		RegCloseKey(hKey);
		if (ret != ERROR_SUCCESS) {
			delete [] tmpString;
			goto error;
		}
		realMac = strrchr(tmpString, '\\');
		realMac.erase(realMac.begin());	
			
		// Step Three: Get DhcpIpAddress
		delete [] tmpString;
		sKey = "SYSTEM\\CurrentControlSet\\Services\\";
		sKey += realMac + "\\Parameters\\Tcpip";

		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,sKey.c_str(),0,KEY_READ,&hKey);
		if (ret != ERROR_SUCCESS)
			goto error;

		ret = RegQueryValueEx(hKey, "DhcpIPAddress", 0, 0, 0, &dwSize);
		tmpString = new char[dwSize];
		ret = RegQueryValueEx(hKey,
			"DhcpIPAddress",
			0,
			0,
			(PGPByte*)tmpString,
			&dwSize);

		RegCloseKey(hKey);
		if (ret != ERROR_SUCCESS) {
			delete [] tmpString;
			goto error;
		}
		
		// Step Four: Convert string to PGPUInt32
		g_localIP = inet_addr(tmpString);
		delete [] tmpString;
		return g_localIP;
	}
	
	// All other instances query driver
	DWORD dwBytesReturned;
	dwBytesReturned = 0;

	debugMsg(__LINE__, "Sending OID_PGP_LOCALIP");
	if (!sendKernelRequest(OID_PGP_LOCALIP, &g_localIP, 
		sizeof(g_localIP), &dwBytesReturned)
		|| dwBytesReturned != sizeof(g_localIP))
	{
		debugMsg(__LINE__, "OID_PGP_LOCALIP failed - Unexpected size=%d or errorCode=%d\n",
			dwBytesReturned,
			GetLastError());
	}

	debugMsg(__LINE__, "<< getLocalIP(%s)", inet_ntoa(*((struct in_addr *) &g_localIP)));
	return g_localIP;

error:
	debugMsg(__LINE__, "<< getLocalIP(%s)", inet_ntoa(*((struct in_addr *) &g_localIP)));

	g_localIP = 0;
	return g_localIP;
}