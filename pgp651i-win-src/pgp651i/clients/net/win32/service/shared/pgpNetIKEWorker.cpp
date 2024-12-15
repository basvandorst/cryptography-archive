/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetIKEWorker.cpp,v 1.54.4.2 1999/06/16 04:52:08 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <assert.h>
#include <queue>

// PGP includes
#include "pgpRMWOLock.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpEndianConversion.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpIKE.h"
#include "pgpNetKernelXChng.h"
#include "pgpRWLockSignal.h"
#include "pgpNetQueueElement.h"
#include "pgpNetDSAarray.h"
#include "pgpNetCConfig.h"
#include "pgpNetIPC.h"

#include "pgpNetIKEWorker.h"

// external globals
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_IKEQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_UDPQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_KernelQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_GUIQueue;
extern CPGPrwLockSignal<CPGPnetDSAarray>					g_SAarray;
extern CPGPrwLockSignal<CPGPnetCConfig>	*					g_pConfig;

extern PGPUInt32		g_localIP;
extern PGPCond_t		g_localIPEvent;
extern PGPCond_t		g_ExitEvent;
extern PGPCond_t		g_ShutdownRequest;
extern PGPCond_t		g_SuspendEvent;
extern PGPContextRef	g_Context;

CPGPnetIKEWorker::CPGPnetIKEWorker()
: m_pIdData(0), m_pSharedKey(0), m_exitPending(FALSE)
{
	// Initialize IKE
	PGPError err = PGPNewIKEContext(g_Context, 
		CPGPnetIKEWorker::IKEMessageProc, 
		static_cast<void*>(this), 
		&m_ikeRef);

	if (IsPGPError(err)) {
		CPGPnetAppLog::instance()->logPGPEvent(err,
			0,
			__FILE__, 
			__LINE__);
		m_ikeRef = kInvalidPGPikeContextRef;
	}
}

CPGPnetIKEWorker::~CPGPnetIKEWorker()
{
	if (m_pIdData) {
		PGPFreeData(m_pIdData);
		m_pIdData = NULL;
	}
	delete [] m_pSharedKey;
	PGPFreeIKEContext(m_ikeRef);
}

PGPError
CPGPnetIKEWorker::IKEMessageProc(PGPikeContextRef ike,
		void * inUserData,
		PGPikeMessageType msg,
		void * data)
{
	CPGPnetIKEWorker *pMyself = static_cast<CPGPnetIKEWorker*>(inUserData);

	switch (msg) {
		case kPGPike_MT_SARequestFailed:
				CPGPnetDebugLog::instance()->dbgOut("Got SARequestFailed");

				// put onto the kernel queue
				g_KernelQueue.startWriting();

				g_KernelQueue.data().push(
					CPGPnetQueueElement(
						sizeof(PGPikeMTSAFailed),
						data, 
						OID_PGP_SAFAILED,
						0));

				g_KernelQueue.stopWriting();
				g_KernelQueue.setEvent();
				
				CPGPnetAppLog::instance()->logServiceEvent(
					kPGPnetSrvcError_SAFailed,
					static_cast<PGPikeMTSAFailed*>(data)->ipAddress,
					0,
					__FILE__,
					__LINE__);
			break;
		case kPGPike_MT_SAEstablished:
			{
				CPGPnetDebugLog::instance()->dbgOut(
					"Got SAEstablished");
				
				// we store the *pointer*
				CPGPnetQueueElement tElem(sizeof(PGPikeSA*),	
					&data,
					OID_PGP_NEWSA,
					static_cast<PGPikeSA*>(data)->ipAddress);

				// put onto kernel Queue
				g_KernelQueue.startWriting();
				g_KernelQueue.data().push(tElem);
				g_KernelQueue.stopWriting();
				g_KernelQueue.setEvent();

				// put onto GUI Queue
				g_GUIQueue.startWriting();
				g_GUIQueue.data().push(tElem);
				g_GUIQueue.stopWriting();
				g_GUIQueue.setEvent();
			}
			break;
		case kPGPike_MT_SADied:
			{
				CPGPnetDebugLog::instance()->dbgOut("Got SADied");

				// we store a copy of the spi, since IKE will 
				// free this SA after this function returns
				CPGPnetQueueElement tElem(sizeof(PGPipsecSPI),	
					static_cast<PGPikeSA*>(data)->transform[0].u.ipsec.inSPI,
					OID_PGP_SADIED,
					static_cast<PGPikeSA*>(data)->ipAddress);

				g_SAarray.startWriting();
				g_SAarray.data().removeSA(data, sizeof(PGPikeSA));
				g_SAarray.stopWriting();

				// put onto kernel queue
				g_KernelQueue.startWriting();
				g_KernelQueue.data().push(tElem);
				g_KernelQueue.stopWriting();
				g_KernelQueue.setEvent();
	
				// put onto GUI Queue
				g_GUIQueue.startWriting();
				g_GUIQueue.data().push(tElem);
				g_GUIQueue.stopWriting();
				g_GUIQueue.setEvent();
	
				break;
			}
		case kPGPike_MT_PolicyCheck:
			CPGPnetDebugLog::instance()->dbgOut("Got PolicyCheck");
			pMyself->doPolicyCheck(static_cast<PGPikeMTSASetup*>(data));
			break;
		case kPGPike_MT_LocalPGPCert:
			CPGPnetDebugLog::instance()->dbgOut("Got LocalPGPCert");
			pMyself->doLocalPGPCert(static_cast<PGPikeMTCert*>(data));
			break;
		case kPGPike_MT_LocalX509Cert:
			CPGPnetDebugLog::instance()->dbgOut("Got LocalX509Cert");
			pMyself->doLocalX509Cert(static_cast<PGPikeMTCert*>(data));
			break;
		case kPGPike_MT_RemoteCert:
			CPGPnetDebugLog::instance()->dbgOut("Got RemoteCert");
			pMyself->doRemoteCert(static_cast<PGPikeMTRemoteCert*>(data));
			break;
		case kPGPike_MT_Packet:
			CPGPnetDebugLog::instance()->dbgOut("Got MT_Packet");
			pMyself->doPacket(static_cast<PGPikeMTPacket*>(data));
			break;
		case kPGPike_MT_ClientIDCheck:
			CPGPnetDebugLog::instance()->dbgOut("Got MTAlert");
			pMyself->doClientIDCheck(static_cast<PGPikeMTClientIDCheck*>(data));
			break;
		case kPGPike_MT_SAUpdate:
			{
				CPGPnetDebugLog::instance()->dbgOut("Got SAUpdate");

				// we store a copy of the spi, since IKE will 
				// free this SA sometime soon
				CPGPnetQueueElement tElem(sizeof(PGPipsecSPI),	
					static_cast<PGPikeSA*>(data)->transform[0].u.ipsec.inSPI,
					OID_PGP_SAUPDATE,
					static_cast<PGPikeSA*>(data)->ipAddress);

				// put onto kernel queue
				g_KernelQueue.startWriting();
				g_KernelQueue.data().push(tElem);
				g_KernelQueue.stopWriting();
				g_KernelQueue.setEvent();
	
				break;
			}
		case kPGPike_MT_Alert:
			CPGPnetDebugLog::instance()->dbgOut("Got MTAlert");
			CPGPnetAppLog::instance()->logIKEAlert(
				static_cast<PGPikeMTAlert*>(data));
			break;
		case kPGPike_MT_DebugLog:
			CPGPnetDebugLog::instance()->dbgOut("%s", static_cast<char*>(data));
			break;
		default:
			break;
	}

	return kPGPError_NoErr;
}

unsigned int
CPGPnetIKEWorker::Run()
{
	PGPUInt32		ret = 0;
	const PGPInt16	numHandles = 4;
	HANDLE			lpHandles[numHandles];

	lpHandles[0] = g_ShutdownRequest;
	lpHandles[1] = g_ExitEvent;
	lpHandles[2] = g_IKEQueue.event();
	lpHandles[3] = g_SuspendEvent;

	PGPBoolean running = TRUE;
	while (running) {
		ret = WaitForMultipleObjects(numHandles,
									 lpHandles, 
									 FALSE, 
									 500);
		if (ret == WAIT_TIMEOUT) {
			PGPError err = PGPikeProcessMessage(m_ikeRef, 
				kPGPike_MT_Idle,
				NULL);

			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_Idle: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					0,
					__FILE__,
					__LINE__);
			}
		} else if (ret >= WAIT_ABANDONED_0) {
			CPGPnetDebugLog::instance()->dbgOut(
				"IKEWorker: something abandoned");
			running = FALSE;
			goto end;
		} else if (ret >= WAIT_OBJECT_0) {
			switch (ret - WAIT_OBJECT_0) {
			case 0:
			{
				if (m_exitPending)
					break;

				m_exitPending = TRUE;
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: ShutdownRequest signaled");

				PGPError err = PGPikeProcessMessage(m_ikeRef, 
					kPGPike_MT_SAKillAll, 
					0);

				if (IsPGPError(err)) {
					CPGPnetDebugLog::instance()->dbgOut(
						"IKEWorker: PGPError processing kPGPike_MT_SAKillAll: %d",
						err);

					CPGPnetAppLog::instance()->logPGPEvent(
						err,
						0,
						__FILE__,
						__LINE__);
				}

	
				if (g_SAarray.data().numSAs() == 0) {
					PGPCondSignal(&g_ExitEvent);
					return kPGPError_NoErr;
				}
				break;
			}
			case 1:
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: ExitEvent signaled");
				running = FALSE;
				goto end;
				break;
			case 2:
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: IKEQueue event signaled");
				processQueue();
				break;
			case 3:
			{
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: SuspendEvent signaled");
				
				PGPError err = PGPikeProcessMessage(m_ikeRef, 
					kPGPike_MT_SAKillAll, 
					0);

				if (IsPGPError(err)) {
					CPGPnetDebugLog::instance()->dbgOut(
						"IKEWorker: PGPError processing kPGPike_MT_SAKillAll: %d",
						err);

					CPGPnetAppLog::instance()->logPGPEvent(
						err,
						0,
						__FILE__,
						__LINE__);
				}
	
				if (g_SAarray.data().numSAs() == 0) {
					return kPGPError_NoErr;
				}
				break;
			}
			default:
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: something signaled");
				break;
			}
		}
	}

end:
	return kPGPError_NoErr;
}

void
CPGPnetIKEWorker::processQueue()
{
	PGPError err = kPGPError_NoErr;

	while (!g_IKEQueue.data().empty()) {
		g_IKEQueue.startReading();
		// pull one thing off;
		CPGPnetQueueElement queueElem = g_IKEQueue.data().front();
		g_IKEQueue.stopReading();

		g_IKEQueue.startWriting();
		g_IKEQueue.data().pop();
		g_IKEQueue.stopWriting();

		switch (queueElem.type()) {
		case kPGPike_MT_Packet:
		{
			// MTPackets are stored as flat buffers
			// in the queue, so we need to unflatten it
			// into a PGPikeMTPacket structure.
			const PGPByte *buf = static_cast<const PGPByte*>(queueElem.data());
			PGPikeMTPacket packetHolder;
			
			packetHolder.ipAddress = queueElem.ipAddress();
			packetHolder.packetSize = queueElem.size();
			// create some memory for the packet info
			packetHolder.packet = new PGPByte[queueElem.size()];
			if (packetHolder.packet) {
				// copy from the flat buffer to the new pointer
				pgpCopyMemory(buf, 
					packetHolder.packet, 
					packetHolder.packetSize);

				struct in_addr foo;
				foo.S_un.S_addr = packetHolder.ipAddress;

				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: Processing MT_Packet (size: %d, from %s)",
					packetHolder.packetSize,
					inet_ntoa(foo));

				err = PGPikeProcessMessage(m_ikeRef, 
					kPGPike_MT_Packet, 
					&packetHolder);

				if (IsPGPError(err)) {
					CPGPnetDebugLog::instance()->dbgOut(
						"IKEWorker: PGPError processing MT_Packet: %d",
						err);

					CPGPnetAppLog::instance()->logPGPEvent(
						err,
						packetHolder.ipAddress,
						__FILE__,
						__LINE__);
				}
			} else {
				CPGPnetAppLog::instance()->logPGPEvent(
					kPGPError_OutOfMemory,
					packetHolder.ipAddress,
					__FILE__,
					__LINE__);
			}

			delete [] packetHolder.packet; // IKE makes a copy, so delete
			break;

		}
		case kPGPike_MT_SADied:
		{
			if (queueElem.size() != sizeof(PGPikeSA*)) {
				CPGPnetDebugLog::instance()->dbgOut("Bad SADIED size!");
				pgpAssert(TRUE);
				return;
			}
			// the queueElem contains a copy of the *pointer*
			PGPikeSA *pSA = 0;
			pgpCopyMemory(queueElem.data(), &pSA, sizeof(pSA));
			err = PGPikeProcessMessage(m_ikeRef, kPGPike_MT_SADied, pSA);
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_SADied: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					pSA->ipAddress,
					__FILE__,
					__LINE__);
			}
			break;
		}
		case kPGPike_MT_SARekey:
		{
			if (queueElem.size() != sizeof(PGPikeSA*)) {
				CPGPnetDebugLog::instance()->dbgOut("Bad SARekey size!");
				pgpAssert(TRUE);
				return;
			}
			// the queueElem contains a copy of the *pointer*
			PGPikeSA *pSA = 0;
			pgpCopyMemory(queueElem.data(), &pSA, sizeof(pSA));
			err = PGPikeProcessMessage(m_ikeRef, kPGPike_MT_SARekey, pSA);
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_SARekey: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					pSA->ipAddress,
					__FILE__,
					__LINE__);
			}
			break;
		}
		case kPGPike_MT_SARequest:
		{
			const PGPikeMTSASetup *pSAsetup = 
				static_cast<const PGPikeMTSASetup*>(queueElem.data());

			err = PGPikeProcessMessage(m_ikeRef,
				kPGPike_MT_SARequest,
				const_cast<PGPikeMTSASetup*>(pSAsetup));
			
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_SARequest: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					pSAsetup->ipAddress,
					__FILE__,
					__LINE__);
			}
			delete [] pSAsetup->sharedKey;
			if (pSAsetup->u.ipsec.idData) {
				PGPFreeData(pSAsetup->u.ipsec.idData);
			//	pSAsetup->u.ipsec.idData = NULL;
			}
			break;
		}
		case kPGPike_MT_Pref:
			// send Prefs from global config data
			g_pConfig->startReading();
			err = PGPikeProcessMessage(m_ikeRef,
				kPGPike_MT_Pref,
				const_cast<PGPikeMTPref *>
				(&(g_pConfig->data().config()->IkeExpirationPrefs)));
			
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_Pref: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					0,
					__FILE__,
					__LINE__);
			}

			err = PGPikeProcessMessage(m_ikeRef,
				kPGPike_MT_Pref,
				const_cast<PGPikeMTPref *>
				(&(g_pConfig->data().config()->IkeIkeProposalPrefs)));

			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_Pref: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					0,
					__FILE__,
					__LINE__);
			}
			
			err = PGPikeProcessMessage(m_ikeRef,
				kPGPike_MT_Pref,
				const_cast<PGPikeMTPref *>
				(&(g_pConfig->data().config()->IkeIpsecProposalPrefs)));
			
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_Pref: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					0,
					__FILE__,
					__LINE__);
			}
			
			err = PGPikeProcessMessage(m_ikeRef,
				kPGPike_MT_Pref,
				const_cast<PGPikeMTPref *>
				(&(g_pConfig->data().config()->IkeAlgorithmPrefs)));
			
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing MT_Pref: %d",
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					0,
					__FILE__,
					__LINE__);
			}
			g_pConfig->stopReading();

			break;
		default:
			// all other IKE messages do not require any processing
			err = PGPikeProcessMessage(m_ikeRef,
				static_cast<PGPikeMessageType>(queueElem.type()),
				const_cast<void*>(queueElem.data()));
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut(
					"IKEWorker: PGPError processing %d: %d",
					queueElem.type(),
					err);

				CPGPnetAppLog::instance()->logPGPEvent(
					err,
					0,
					__FILE__,
					__LINE__);
			}
		}
	}
	g_IKEQueue.resetEvent();
}

void
CPGPnetIKEWorker::doPolicyCheck(PGPikeMTSASetup *tS)
{
	PGPBoolean found = FALSE;
	PGPInt32 i = 0;
	PGPNetHostEntry *host = 0;

	// fill in the values that I can right now
	tS->approved = FALSE;
	tS->doi = kPGPike_DOI_IPSEC;

	// put localIP request onto kernel queue
	g_KernelQueue.startWriting();
	g_KernelQueue.data().push(CPGPnetQueueElement(0,0,OID_PGP_LOCALIP,0));
	g_KernelQueue.stopWriting();
	g_KernelQueue.setEvent();	
	
	// wait for kernel queue to process request
	WaitForSingleObject(g_localIPEvent, INFINITE);
	tS->localIPAddress = g_localIP;

	g_pConfig->startReading();

	for (i = 0; i < g_pConfig->data().config()->uHostCount; i++) {
		host = &(g_pConfig->data().config()->pHostList[i]);
		if (tS->ipAddress == host->ipAddress) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		for (i = 0; i < g_pConfig->data().config()->uHostCount; i++) {
			host = &(g_pConfig->data().config()->pHostList[i]);
			if ((tS->ipAddress & host->ipMask) == (host->ipAddress & host->ipMask)) {
				found = TRUE;
				break;
			}
		}
	}

	if (found) {

		if (host->hostType == kPGPnetInsecureHost) {
			tS->approved = FALSE;
			goto end;
		}
					
		if (host->hostType == kPGPnetSecureHost) {
			tS->u.ipsec.packetMode = kPGPike_PM_Transport;
		}
		if (host->hostType == kPGPnetSecureGateway) {
			tS->u.ipsec.packetMode = kPGPike_PM_Tunnel;
		}
		
		if (g_pConfig->data().config()->bAllowUnconfigHost 
			&& g_pConfig->data().config()->bRequireSecure) {
				tS->u.ipsec.packetMode = kPGPike_PM_Transport;
		}

		tS->approved = TRUE;
		if (host->sharedSecret) {

			delete [] m_pSharedKey;

			PGPUInt32 secretSize = strlen(host->sharedSecret);
			m_pSharedKey = new PGPByte[secretSize+1]; // w/ null
			tS->sharedKey = m_pSharedKey;

			pgpCopyMemory(host->sharedSecret /*src*/,
				tS->sharedKey /*dest*/,
				secretSize+1); // w/ null

			tS->sharedKeySize = secretSize; // w/o null
		} else {
			tS->sharedKey = 0;
			tS->sharedKeySize = 0;
		}

		if (m_pIdData) {
			PGPFreeData(m_pIdData);
			m_pIdData = NULL;
		}

		tS->u.ipsec.idType = 
			(PGPipsecIdentity)host->identityType;

		if (tS->u.ipsec.idType == kPGPike_ID_IPV4_Addr) {
			PGPUInt32 dataSize = sizeof(PGPUInt32);

			m_pIdData = (PGPByte*) PGPNewData(
				PGPGetContextMemoryMgr(g_Context),
				dataSize, 
				kPGPMemoryMgrFlags_Clear); // XXX error check
			
			tS->u.ipsec.idData = m_pIdData;
			pgpCopyMemory(&(host->identityIPAddress) /*src*/,
				tS->u.ipsec.idData /*dest*/,
				dataSize);

			tS->u.ipsec.idDataSize = dataSize;
		} else if (tS->u.ipsec.idType == kPGPike_ID_DER_ASN1_DN) {
	
			PGPError err = PGPCreateDistinguishedName(g_Context,
				host->identity,
				&m_pIdData,
				&tS->u.ipsec.idDataSize);

			if (IsPGPError(err)) {
				tS->approved = FALSE;
				CPGPnetAppLog::instance()->logPGPEvent(err, 
					tS->ipAddress,
					__FILE__,
					__LINE__);
				goto end;
			}

			tS->u.ipsec.idData = m_pIdData;
		}  else {
			PGPUInt32 dataSize = strlen((char*)host->identity);

			m_pIdData = (PGPByte*) PGPNewData(
				PGPGetContextMemoryMgr(g_Context),
				dataSize + 1, 
				kPGPMemoryMgrFlags_Clear); // XXX error check

			tS->u.ipsec.idData = m_pIdData;
			pgpCopyMemory(host->identity /*src*/,
				tS->u.ipsec.idData /*dest*/,
				dataSize+1); // w/ null

			tS->u.ipsec.idDataSize = dataSize; // w/o null
		}
	} else {
		// didn't find host in host list
		// this is OK, as long as we allow unconfigured hosts
		// Also need a valid auth key
		if (g_pConfig->data().config()->bAllowUnconfigHost) {

			if (g_pConfig->data().pgpKeyRef() != kInvalidPGPKeyRef ||
				g_pConfig->data().x509KeyRef() != kInvalidPGPKeyRef) {

				tS->u.ipsec.packetMode = kPGPike_PM_Transport;
				tS->approved = TRUE;
				tS->sharedKey = 0;
				tS->sharedKeySize = 0;
			}
		}
	}

end:
	g_pConfig->stopReading();
}

void
CPGPnetIKEWorker::doPacket(PGPikeMTPacket *tP)
{	
	g_pConfig->startReading();
	if (!g_pConfig->data().config()->bPGPnetEnabled) {
		g_pConfig->stopReading();
		return;
	}
	g_pConfig->stopReading();

	g_UDPQueue.startWriting();
	// push buffer onto queue, this copies the buffer
	g_UDPQueue.data().push(
		CPGPnetQueueElement(
			tP->packetSize, 
			tP->packet,
			kPGPike_MT_Packet,
			tP->ipAddress));

	g_UDPQueue.stopWriting();
	g_UDPQueue.setEvent();
}

void
CPGPnetIKEWorker::doLocalX509Cert(PGPikeMTCert *tLC)
{
#if 0
typedef struct PGPikeMTCert
{
	PGPUInt32				ipAddress;		/*  < */
	PGPKeySetRef			baseKeySet;		/* >  */
	PGPKeyRef				authKey;		/* >  ignored for X.509 */
	PGPSigRef				authCert;		/* >  used only for X.509 */
	PGPKeySetRef			authCertChain;	/* >  used only for X.509 */
	
	PGPBoolean				isPassKey;		/* >  */
	void *					pass;			/* >  null-term if passphrase, copied */
	PGPSize					passLength;		/* >  */
} PGPikeMTCert;

#endif

	g_pConfig->startReading();

	tLC->authKey		= g_pConfig->data().x509KeyRef();
	tLC->authCert		= g_pConfig->data().x509SigRef();
	tLC->authCertChain	= g_pConfig->data().keySetRef();
	tLC->baseKeySet		= g_pConfig->data().keySetRef();

	tLC->isPassKey = 0;
	if (g_pConfig->data().x509Passphrase()) {
		// XXX should copy passphrase ???
		tLC->pass = const_cast<char*>(g_pConfig->data().x509Passphrase());
		tLC->passLength = lstrlen(g_pConfig->data().x509Passphrase()) + 1;
	} else {
		tLC->pass = 0;
		tLC->passLength = 0;
	}

	g_pConfig->stopReading();
}

void
CPGPnetIKEWorker::doLocalPGPCert(PGPikeMTCert *tLC)
{
	g_pConfig->startReading();

	tLC->authKey		= g_pConfig->data().pgpKeyRef();
	tLC->baseKeySet		= g_pConfig->data().keySetRef();

	tLC->isPassKey = 0;
	if (g_pConfig->data().pgpPassphrase()) {
		// XXX should copy passphrase ???
		tLC->pass = const_cast<char*>(g_pConfig->data().pgpPassphrase());
		tLC->passLength = lstrlen(g_pConfig->data().pgpPassphrase()) + 1;
	} else {
		tLC->pass = 0;
		tLC->passLength = 0;
	}

	g_pConfig->stopReading();
}

PGPError
CPGPnetIKEWorker::syncKeySetValidity (
		PGPContextRef	context,
		PGPKeySetRef	keysetMain,
		PGPKeySetRef	keyset)
{

	PGPKeySetRef	keysetCombined;
	PGPError		err;

	if (!PGPRefIsValid (keyset) ||
		!PGPRefIsValid (keysetMain)) 
	{
		return kPGPError_BadParams;
	}

	err = PGPNewKeySet (context, &keysetCombined);
	if (IsntPGPError (err)) 
	{
		err = PGPAddKeys (keyset, keysetCombined);
		if (IsntPGPError (err)) 
		{
			err = PGPAddKeys (keysetMain, keysetCombined);
			if (IsntPGPError (err)) 
			{
				err = PGPCheckKeyRingSigs (keyset, 
								keysetCombined, FALSE, NULL, 0);

				if (IsntPGPError (err)) 
				{
					err = PGPPropagateTrust (keysetCombined);
				}
			}
		}

		PGPFreeKeySet (keysetCombined);
	}

	return err;
}

void
CPGPnetIKEWorker::doRemoteCert(PGPikeMTRemoteCert *tRC)
{
	PGPKeySetRef	keyset = kInvalidPGPKeySetRef;
	PGPInt32		iValidity;
	PGPBoolean		bExpired = FALSE,
					bRevoked = FALSE;
	PGPError		err;

	// default is not approved
	tRC->approved = FALSE;

	// create keyset containing single remote key
	err = PGPNewSingletonKeySet(tRC->remoteKey, &keyset);
	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut("Unable to create remote key keyset");
		goto done;
	}

	// propogate the validity information from main keyset to singleton keyset
	g_pConfig->startReading();
	err = syncKeySetValidity(g_Context, g_pConfig->data().keySetRef(), keyset);
	g_pConfig->stopReading();

	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut("Unable to sync validity of remote key");
		goto done;
	}

	// if this is a X.509 key, then get the validity of
	// the userid holding the cert
	if (PGPSigRefIsValid (tRC->remoteCert))
	{
		err = PGPGetUserIDNumber (PGPGetSigUserID (tRC->remoteCert),
				kPGPUserIDPropValidity, &iValidity);
		if( IsntPGPError( err ) )
		{
			(void)PGPGetSigBoolean( tRC->remoteCert, kPGPSigPropIsExpired, &bExpired );
			(void)PGPGetSigBoolean( tRC->remoteCert, kPGPSigPropIsRevoked, &bRevoked );
		}
	}

	// otherwise this is a PGP key, just get the validity of
	// the only key in the keyset
	else
	{
		err = PGPGetKeyNumber (tRC->remoteKey, kPGPKeyPropValidity, &iValidity);
		if( IsntPGPError( err ) )
		{
			(void)PGPGetKeyBoolean( tRC->remoteKey, kPGPKeyPropIsExpired, &bExpired );
			(void)PGPGetKeyBoolean( tRC->remoteKey, kPGPKeyPropIsRevoked, &bRevoked );
		}
	}

	if (IsPGPError(err)) {		
		CPGPnetDebugLog::instance()->dbgOut("Unable to determine validity of remote key");
		tRC->approved = FALSE;
		goto done;
	}
	
	// Ok now we have the key validity. Check to see if the key matches the host key
	// the we stored the first time this host talked to us
	err = checkHostValidity(tRC, iValidity);

	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut("Unable to check Host validity");
		tRC->approved = FALSE;
	}
	
	if( tRC->approved && ( bExpired || bRevoked ) ) {
		tRC->approved = FALSE;
		if( bExpired ) {
			CPGPnetAppLog::instance()->logServiceEvent(
				(PGPSigRefIsValid(tRC->remoteCert)) ?
					kPGPnetSrvcError_ExpiredCert : kPGPnetSrvcError_ExpiredKey,
				tRC->ipAddress,
				0,
				__FILE__,
				__LINE__);
		}
		else {
			CPGPnetAppLog::instance()->logServiceEvent(
				(PGPSigRefIsValid(tRC->remoteCert)) ?
					kPGPnetSrvcError_RevokedCert : kPGPnetSrvcError_RevokedKey,
				tRC->ipAddress,
				0,
				__FILE__,
				__LINE__);
		}
	}

done:
	if (tRC->approved) {
		if (PGPSigRefIsValid(tRC->remoteCert))
			CPGPnetAppLog::instance()->logServiceEvent(
				kPGPnetSrvcError_AuthenticatedCert,
				tRC->ipAddress,
				0,
				__FILE__,
				__LINE__);
		else {
			PGPKeyID keyID;
			err = PGPGetKeyIDFromKey(tRC->remoteKey, &keyID);
			if (IsPGPError(err)) {
				CPGPnetDebugLog::instance()->dbgOut("Unable to get Keyid from remote Key");
				CPGPnetAppLog::instance()->logPGPEvent(err, tRC->ipAddress, __FILE__, __LINE__);
			}
			CPGPnetAppLog::instance()->logServiceEvent(
				kPGPnetSrvcError_AuthenticatedKey,
				tRC->ipAddress,
				&keyID,
				__FILE__,
				__LINE__);
		}
	}

	if (PGPKeySetRefIsValid(keyset))
		PGPFreeKeySet(keyset);

	if (IsPGPError(err)) {
		CPGPnetAppLog::instance()->logPGPEvent(err,
			tRC->ipAddress,
			__FILE__,
			__LINE__);
	}
	return;
}

PGPError
CPGPnetIKEWorker::checkHostValidity(PGPikeMTRemoteCert *tRC, PGPUInt32 iValidity)
{
	PGPBoolean found = FALSE;
	PGPInt32 i = 0;
	PGPNetHostEntry *host = 0;
	PGPNetHostEntry hostCopy;
	PGPError err = kPGPError_NoErr;

	tRC->approved = FALSE;

	g_pConfig->startReading();

	// First need to find the host
	for (i = 0; i < g_pConfig->data().config()->uHostCount; i++) {
		host = &(g_pConfig->data().config()->pHostList[i]);
		if (tRC->ipAddress == host->ipAddress) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		for (i = 0; i < g_pConfig->data().config()->uHostCount; i++) {
			host = &(g_pConfig->data().config()->pHostList[i]);
			if ((tRC->ipAddress & host->ipMask) == (host->ipAddress & host->ipMask)) {
				found = TRUE;
				break;
			}
		}
	}

	// copy the host information to avoid thread issues with the global list
	if (found) {
		pgpCopyMemory(host, &hostCopy, sizeof(PGPNetHostEntry));
		host = 0;
	}
	
	g_pConfig->stopReading();

	if (found) {
		PGPKeyRef pKey = kInvalidPGPKeyRef;
		PGPSigRef pSig = kInvalidPGPSigRef;

		if (hostCopy.authKeyAlg == kPGPPublicKeyAlgorithm_Invalid) {
			/* we're just looking for any valid key */
			if ( ( iValidity >= kPGPValidity_Complete ) ||
					!g_pConfig->data().config()->bRequireValidKey )
				tRC->approved = TRUE;

			return kPGPError_NoErr;
		}

		PGPUInt32 alg;

		err = PGPGetKeyNumber(tRC->remoteKey,
			kPGPKeyPropAlgID,
			(PGPInt32 *)&alg);

		if (IsPGPError(err))
			return err;

		if (alg != hostCopy.authKeyAlg) {
			// Algorithm doesn't match
			return kPGPError_NoErr;
		}

		if (hostCopy.authCertIASNLength == 0) {
			// We have a PGP key. Compare the Key IDs
			PGPKeyID expKeyID;
			err = PGPImportKeyID(hostCopy.authKeyExpKeyID, &expKeyID); 
			if (IsPGPError(err))
				return err;

			PGPKeyID remKeyID;
			err = PGPGetKeyIDFromKey(tRC->remoteKey, &remKeyID);
			if (IsPGPError(err))
				return err;

			if (!PGPCompareKeyIDs(&expKeyID, &remKeyID)) {
				tRC->approved = TRUE;
			} else {
				// Key IDs don't match XXX what here
			}
			return kPGPError_NoErr;
		}

		if (hostCopy.authCertIASNLength > 0) {
			if (PGPSigRefIsValid(tRC->remoteCert)) {	
				PGPByte	iasn[256];
				PGPSize	iasnSize = sizeof(iasn);

				err = PGPGetSigPropertyBuffer(tRC->remoteCert, 
						kPGPSigPropX509IASN,
						iasnSize, iasn, &iasnSize);

				if (IsPGPError(err))
					return err;

				if (iasnSize == hostCopy.authCertIASNLength) {
					if (pgpMemoryEqual(hostCopy.authCertIASN,iasn, iasnSize)) {
						tRC->approved = TRUE;
					} else {
						// XXX what to do here, not approved other error
					}
				}
			}
		}
	} else {
		// We didn't find a matching host
		if (g_pConfig->data().config()->bRequireValidKey) {
			if (iValidity >= kPGPValidity_Complete)
				tRC->approved = TRUE;
		} else if( iValidity >= kPGPValidity_Complete )
			tRC->approved = TRUE;
		else {

			if (IsPGPError(err))
				return err;

			tRC->approved = TRUE;
		}
	}
	return kPGPError_NoErr;
}

PGPError
CPGPnetIKEWorker::addHostToConfig(PGPikeMTRemoteCert *tRC)
{
	PGPError err = kPGPError_NoErr;

	g_pConfig->startWriting();

	err = PGPnetAddPromiscuousHostToConfiguration(g_Context,
		const_cast<PNCONFIG*>(g_pConfig->data().config()),
		tRC->ipAddress,
		g_localIP,
		tRC->remoteKey,
		tRC->remoteCert);

	g_pConfig->stopWriting();

	if (IsPGPError(err))
		return err;

	g_pConfig->startReading();
	err = g_pConfig->data().saveConfiguration();
	if (IsntPGPError(err))
		g_pConfig->data().propogateConfig();
	g_pConfig->stopReading();

	if (IsPGPError(err))
		return err;

	// Tell App
	g_GUIQueue.startWriting();
	
	g_GUIQueue.data().push(
		CPGPnetQueueElement(0, 0, PGPNET_PROMISCUOUSHOSTADDED, 0));

	g_GUIQueue.stopWriting();
	g_GUIQueue.setEvent();

///	SendMessage (hwndApp, PGPNET_M_APPMESSAGE,
//			PGPNET_PROMISCUOUSHOSTADDED, 0);

	return kPGPError_NoErr;
}

void
CPGPnetIKEWorker::doClientIDCheck(PGPikeMTClientIDCheck *	tCC)
{
	PGPBoolean	found = FALSE;
	PGPUInt16	i = 0;
	PGPNetHostEntry *host = 0;
	
	tCC->approved	= FALSE;
	
	g_pConfig->startReading();

	for( i = 0; i < g_pConfig->data().config()->uHostCount; i++ )
	{
		host = &(g_pConfig->data().config()->pHostList[i]);
		if( ( tCC->ipAddrStart == host->ipAddress ) &&
			( tCC->ipMaskEnd == host->ipMask ) &&
			!tCC->destIsRange )
		{
			found = TRUE;
			break;
		}
	}
	if( found )
	{
		if ( host->childOf != -1 )
		{
			if( g_pConfig->data().config()->pHostList[host->childOf].ipAddress ==
				tCC->ipAddress )
				tCC->approved = TRUE;
		}
		else if( ( tCC->ipAddress == tCC->ipAddrStart ) &&
					( tCC->ipMaskEnd == 0xFFFFFFFF ) )
		{
			tCC->approved = TRUE;
			
		}
	}
	g_pConfig->stopReading();
}