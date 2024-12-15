/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Definition of the CPGPnetIKEservice class

	$Id: pgpNetIKEservice.h,v 1.5.6.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetIKEservice_h
#define _Included_pgpNetIKEservice_h

#include "pgpService.h"

// forward declarations
class CPGPnetIKEmanager;

class CPGPnetIKEservice : public CService
{
private:
	CPGPnetIKEmanager *	m_pIKEmanager;
	PGPBoolean			m_bSuspend;

	// private and undefined copy and assignment
	CPGPnetIKEservice(const CPGPnetIKEservice &);
	CPGPnetIKEservice & operator=(const CPGPnetIKEservice &);

protected:

public:
	CPGPnetIKEservice(const char*, 
		const char*,
		HINSTANCE hInst);
	virtual ~CPGPnetIKEservice();

	// virtual overrides
	virtual PGPBoolean	OnInit();
    virtual void		Run();
	virtual void		OnShutdown();
	virtual void		OnLogoff();
	virtual void		OnSuspend();
	virtual void		OnResume();
};

#endif // _Included_pgpNetIKEservice_h
