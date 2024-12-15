/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Defines class CPGPnetAppLog. This is a thread-safe singleton
	class that is used to send debug or logging information to a file

	$Id: pgpNetAppLog.h,v 1.12 1999/04/27 23:16:50 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetAppLog_h
#define _Included_pgpNetAppLog_h

#include "pgpBase.h"
#include "pgpFileSpec.h"

PGP_BEGIN_C_DECLARATIONS
#include "pgpNetLog.h"
PGP_END_C_DECLARATIONS

#include "pgpSingletonDestroyer.h"
#include "pgpLockable.h"

class CPGPnetAppLog : public CPGPlockable
{
private:
	// data members
	PGPnetLogRef	m_logRef;
	PGPBoolean		m_bLogToApp;
	HWND			m_hSendingWnd;

	// static members
	static CPGPnetAppLog *		sm_instance;
	static CPGPlockable 		sm_lock;
	static CPGPdestroyer<CPGPnetAppLog> sm_destroyer;

	// private functions
	CPGPnetAppLog(const CPGPnetAppLog &);
	CPGPnetAppLog & operator=(const CPGPnetAppLog &);

	void sendEventToApp(PGPnetLogEvent 	*event);
	void privateLogEvent(PGPnetLogEvent *p_Event);

protected:
	CPGPnetAppLog();
	virtual ~CPGPnetAppLog() {};
	
	friend class CPGPdestroyer<CPGPnetAppLog>;
	
public:
	void logEvent(PGPnetLogEvent *p_Event);
	void logEvent(PGPnetLogEventType eventType,
		PGPUInt32 ipAddress,
		PGPByte *eventData,
		PGPUInt32 dataSize);
	void logErrorEvent(PGPnetLogEventType event,
		PGPInt32 errCode,
		PGPUInt32 ipAddress,
		char *file,
		PGPUInt32 line);

	void logPGPEvent(PGPError err, 
		PGPUInt32 ipAddress,
		char *file,
		PGPUInt32 line);
	void logSysEvent(PGPUInt32 dataCode,
		PGPUInt32 ipAddress,
		char *file,
		PGPUInt32 line);
	void logServiceEvent(PGPnetServiceError err, 
		PGPUInt32 ipAddress,
		PGPKeyID *keyID,
		char *file, 
		PGPUInt32 line);
	void logIKEAlert(PGPikeMTAlert *pAlert);
	void clearLog();
	
	void logToApp(PGPBoolean b) { lock(); m_bLogToApp = b; unlock(); };
	void logRef(PGPnetLogRef lR) { lock(); m_logRef = lR; unlock(); };
	void sendingWnd(HWND w) { lock(); m_hSendingWnd = w; unlock(); };

	static CPGPnetAppLog *instance();
};

#endif	// _Included_pgpNetAppLog_h
