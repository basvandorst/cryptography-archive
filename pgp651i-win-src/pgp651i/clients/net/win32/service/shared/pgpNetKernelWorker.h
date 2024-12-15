/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetKernelWorker.h,v 1.22.4.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetKernelWorker_h
#define _Included_pgpNetKernelWorker_h

#include <string>

#include "pgpNetKernelXChng.h"
#include "pgpNetWorker.h"

class CPGPnetKernelWorker : public CPGPnetWorker {
private:
	HANDLE					m_hMac;
	std::string				m_MacName;
	PGPByte *				m_pSharedMem;
	PGPMESSAGE_CONTEXT *	m_pMsgQueue;

	PGPBoolean sendKernelRequest(PGPUInt32 blockID, 
							   void *pBuffer, 
							   PGPUInt32 BufferSize, 
							   ULONG *pReturnedCount);

	PGPBoolean	createKernelEvent(PPGPEVENT_CONTEXT pPgpEvent);
	void		deleteKernelEvent(PPGPEVENT_CONTEXT pPgpEvent);

	PGPBoolean	createSharedMemory();
	void		deleteSharedMemory();

	PGPBoolean	createMsgQueue();
	void		deleteMsgQueue();

	PGPUInt32	getLocalIP();
	void		debugMsg(int lineno, const char *fmt, ...);

	void		generateBogusSARequest();
	void		processQueue();
	void		processKernelEvent();
	void		doSARequest(PGPUInt32 ipAddress,
		PGPUInt32 ipAddrStart,
		PGPUInt32 ipMaskEnd);
	void		doSARequest(struct _PGPNetHostEntry *host,
		PGPUInt32 ipAddress,
		PGPUInt32 ipAddrStart,
		PGPUInt32 ipMaskEnd);
	void		doSADied(PGPipsecSPI spi);
	void		doSARekey(PGPipsecSPI spi);
	PGPInt32	getMacName();

	// private and undefined copy and assignment
	CPGPnetKernelWorker(const CPGPnetKernelWorker &);
	CPGPnetKernelWorker & operator=(const CPGPnetKernelWorker &);
	
public:
	CPGPnetKernelWorker();
		
protected:
	virtual unsigned int Run();
};

#endif // _Included_pgpNetKernelWorker_h 
