//////////////////////////////////////////////////////////////////////////////
// CDriverCommThread.h
//
// Declaration of class CDriverCommThread
//////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDriverCommThread_h	// [
#define Included_CDriverCommThread_h

#include "PGPdiskPfl.h"
#include "DualErr.h"
#include "Packets.h"


//////////////////////////
// Class CDriverCommThread
//////////////////////////

// A CDriverCommThread thread is used to communicate with the driver. One such
// thread is created upon application startup, whereupon it registers with the
// driver. It exports the function SendPacket, which functions in the main
// application thread can call to send packets to the driver.

class CDriverCommThread : public CWinThread
{
	DECLARE_DYNCREATE(CDriverCommThread)

public:
	DualErr	mInitErr;

				CDriverCommThread();
	virtual		~CDriverCommThread();

	PGPBoolean	IsDriverOpen();
	
	DualErr		SendPacket(PPacket pPacket);
	void		SetDriverPrefs();

	DualErr		StartThread();
	void		EndThread();

	//{{AFX_VIRTUAL(CDriverCommThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

protected:
	HANDLE		mDeregSem;				// wait on this for exit
	HANDLE		mStartSem;				// wait on this for start
	
	HANDLE		mDriverHandle;			// PGPdisk driver handle
	PGPBoolean	mPGPDriverHandShook;	// shook hands with drvr?
	PGPUInt32	mDriverVersion;			// driver version number

	DualErr				OpenPGPdiskDriver();
	DualErr				ClosePGPdiskDriver();

	DualErr				FreeDriverPacket(PPacket pDApacket);
	void				ProcessReportError(PDA_ReportError pRE);
	void				ProcessUpdateApp(PDA_UpdateApp pUA);
	static DWORD WINAPI	ReceivePacketsFunc(PPacket pDApacket);

	//{{AFX_MSG(CDriverCommThread)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDriverCommThread_h
