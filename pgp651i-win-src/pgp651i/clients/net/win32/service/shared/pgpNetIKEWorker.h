/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetIKEWorker.h,v 1.17 1999/04/08 03:04:14 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetIKEWorker_h
#define _Included_pgpNetIKEWorker_h

#include "pgpNetWorker.h"
#include "pgpIKE.h"

class CPGPnetIKEWorker : public CPGPnetWorker {
private:
	PGPikeContextRef	m_ikeRef;
	PGPByte				*m_pSharedKey;	// for temp holding of data
	PGPByte				*m_pIdData;		// for temp holding of SA data
	PGPBoolean			m_exitPending;

	// private and undefined copy and assignment
	CPGPnetIKEWorker(const CPGPnetIKEWorker &);
	CPGPnetIKEWorker & operator=(const CPGPnetIKEWorker &);

	static PGPError IKEMessageProc(PGPikeContextRef ike,
		void * inUserData,
		PGPikeMessageType msg,
		void * data);
	
	void processQueue();
	void doPolicyCheck(PGPikeMTSASetup *tS);
	void doClientIDCheck(PGPikeMTClientIDCheck *tCC);
	void doPacket(PGPikeMTPacket *tP);
	void doLocalPGPCert(PGPikeMTCert *tLC);
	void doLocalX509Cert(PGPikeMTCert *tLC);
	void doRemoteCert(PGPikeMTRemoteCert *tRC);
	PGPError checkHostValidity(PGPikeMTRemoteCert *tRC,
		PGPUInt32 validity);
	PGPError addHostToConfig(PGPikeMTRemoteCert *tRC);

	PGPError syncKeySetValidity(
				PGPContextRef	context,
				PGPKeySetRef	keysetMain, 
				PGPKeySetRef	keyset);

protected:
	virtual unsigned int Run();
	
public:
	CPGPnetIKEWorker();
	virtual ~CPGPnetIKEWorker();

	// accesss
	const PGPikeContextRef ikeRef() { return m_ikeRef; };
};

#endif // _Included_pgpNetIKEWorker_h
