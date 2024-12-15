/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Definitions for CNTService

	$Id: pgpService.h,v 1.2.6.1 1999/06/14 03:22:39 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_NTService_h
#define _Included_NTService_h

#include <string>

#include "pgpTypes.h"

#include "pgpNetIKEserviceMsg.h" // Event message ids

#define SERVICE_CONTROL_USER 128

class CService
{
private:
	CService(const CService &);
	CService & operator=(const CService&);

    static HANDLE m_hEventSource;
	static std::string	m_szValueName;

protected:
    PGPCond_t	m_StopEvent;
	PGPBoolean	m_commandLine;

	void debugMsg(const char *fmt, ...);

public:
    CService(const char* szServiceName, 
		const char *szDisplayName);
    virtual ~CService();

    static PGPBoolean	ParseStandardArgs(int argc, char* argv[]);
    static PGPBoolean	IsInstalled();
    static PGPBoolean	Install();
    static PGPBoolean	Uninstall();
    static void		LogEvent(WORD wType,
						 DWORD dwID,
						 const char* pszS1 = NULL,
						 const char* pszS2 = NULL,
						 const char* pszS3 = NULL);
    PGPBoolean	StartService();
    void		SetStatus(DWORD dwState);
    PGPBoolean	Initialize();

	// virtual functions
    virtual void		Run();
	virtual PGPBoolean	OnInit();
    virtual void		OnStop();
    virtual void		OnInterrogate();
    virtual void		OnPause();
    virtual void		OnContinue();
    virtual void		OnShutdown();
    virtual PGPBoolean	OnUserControl(DWORD dwOpcode);
	virtual void		OnLogoff();
	virtual void		OnSuspend() = 0;
	virtual void		OnResume() = 0;
    
    // static member functions
    static void WINAPI	ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI	Handler(DWORD dwOpcode);
	static BOOL WINAPI	CtrlHandler(DWORD dwOpcode);

    // data members
	static std::string				m_szServiceName;
	static std::string				m_szDisplayName;
    static PGPInt32 				m_iMajorVersion;
    static PGPInt32 				m_iMinorVersion;
    SERVICE_STATUS_HANDLE 	m_hServiceStatus;
    SERVICE_STATUS 			m_Status;

	// access functions
	PGPBoolean 	commandLine()				{ return m_commandLine; };
	void 		commandLine(PGPBoolean b) 	{ m_commandLine = b; };
	DWORD		exitStatus()				{ return m_Status.dwWin32ExitCode; };
		   
	static CService *m_pThis;
};

#endif // _Included_NTService_h
