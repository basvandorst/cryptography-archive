/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpSingletonDestroyer.h,v 1.3 1998/10/01 00:50:33 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpSingletonDestroyer_h
#define _Included_pgpSingletonDestroyer_h

template <class DOOMED>
class CPGPdestroyer {
private:
	// private and undefined copy and assignment
	CPGPdestroyer(const CPGPdestroyer<DOOMED>&);
	CPGPdestroyer<DOOMED>& operator=(const CPGPdestroyer<DOOMED>&);
	
	DOOMED* m_doomed;

public:
	CPGPdestroyer(DOOMED* d = 0)	{ m_doomed = d; };
	~CPGPdestroyer()				{ delete m_doomed; };
	
	void SetDoomed(DOOMED*d)		{ m_doomed = d; };
};

#endif // _Included_pgpSingletonDestroyer_h
