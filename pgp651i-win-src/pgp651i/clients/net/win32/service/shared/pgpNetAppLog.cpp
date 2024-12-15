/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Implements class CPGPnetAppLog, a thread-safe singleton
	class for logging and debugging output.

	$Id: pgpNetAppLog.cpp,v 1.18 1999/04/27 23:16:50 elowe Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <errno.h>
#include <windows.h>

#include "pgpErrors.h"
#include "pgpIPsecErrors.h"
#include "pgpUtilities.h"
#include "pgpMem.h"
#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS
#include "pgpNetPaths.h"
#include "pgpNetIPC.h"
PGP_END_C_DECLARATIONS

#include "pgpNetLogUtils.h"
#include "pgpNetAppLog.h"

extern PGPContextRef	g_Context;

//
// assignments of static members
//
CPGPnetAppLog *					CPGPnetAppLog::sm_instance = 0;
CPGPlockable 					CPGPnetAppLog::sm_lock;
CPGPdestroyer<CPGPnetAppLog>	CPGPnetAppLog::sm_destroyer;

CPGPnetAppLog::CPGPnetAppLog()
	: m_bLogToApp(FALSE), m_hSendingWnd(0), m_logRef(0)
{
}

void
CPGPnetAppLog::logEvent(PGPnetLogEventType eventType,
						PGPUInt32 ipAddress, 
						PGPByte *eventData,
						PGPUInt32 dataSize)
{
	lock();
	PGPnetLogEvent 	event;

	// PGPnetLogEventToFile will fill in the current time
	// if you specify a zero time.  Or you can call PGPGetTime
	// yourself.
	event.timeOfEvent = 0;

	// Fill in the event type.  For now this is a PGPUInt32 ...
	event.typeOfEvent = eventType;

	event.ipaddress   = ipAddress;

	// Fill in event-specific data
	if (eventData) {
		pgpCopyMemory(eventData, &event.info, min(dataSize, sizeof(event.info)));
	}

	privateLogEvent(&event);
	unlock();
}

void
CPGPnetAppLog::logErrorEvent(PGPnetLogEventType eventType,
							 PGPInt32 errCode,
							 PGPUInt32 ipAddress,
							 char *file,
							 PGPUInt32 line)
{
	lock();
	PGPnetLogEvent 	event;

	event.timeOfEvent = 0;	// gets filled in automatically
	event.typeOfEvent = eventType;
	event.ipaddress   = ipAddress;

	event.info.syserror.error = errCode;
	event.info.syserror.lineno = line;

	char *tstr = strrchr(file, '\\');
	if (tstr) {
		strncpy(event.info.syserror.filename, 
			tstr+1, 
			pgpMin(strlen(tstr), sizeof(event.info.syserror.filename)));
	}

	privateLogEvent(&event);
	unlock();
}

void
CPGPnetAppLog::logSysEvent(PGPUInt32 dataCode, 
						   PGPUInt32 ipAddress,
						   char *file,
						   PGPUInt32 line)
{
	logErrorEvent(kPGPnetLogEvent_System, 
		dataCode, 
		ipAddress,
		file,
		line);
}

void
CPGPnetAppLog::logPGPEvent(PGPError err, 
						   PGPUInt32 ipAddress,
						   char *file,
						   PGPUInt32 line)
{
	if ((err >= kPGPIPsecError_FirstError) &&
		(err <= kPGPIPsecError_LastError))
		logErrorEvent(kPGPnetLogEvent_IPSEC,
			err,
			ipAddress,
			file,
			line);
	else
		logErrorEvent(kPGPnetLogEvent_PGPError, 
			err,
			ipAddress,
			file,
			line);
}

void
CPGPnetAppLog::logServiceEvent(PGPnetServiceError err, 
							   PGPUInt32 ipAddress,
							   PGPKeyID *keyID,
							   char *file,
							   PGPUInt32 line)
{
	lock();
	PGPnetLogEvent 	event;

	event.timeOfEvent = 0;	// gets filled in automatically
	event.typeOfEvent = kPGPnetLogEvent_Service;
	event.ipaddress   = ipAddress;

	event.info.svcerror.error = err;

	if (keyID == NULL) {
		pgpFillMemory(&event.info.svcerror.keyID, 0, sizeof(PGPKeyID));
	} else {
		pgpCopyMemory (keyID, &event.info.svcerror.keyID, sizeof(PGPKeyID));
	}

	event.info.svcerror.lineno = line;

	char *tstr = strrchr(file, '\\');
	if (tstr) {
		strncpy(event.info.svcerror.filename, 
			tstr+1, 
			pgpMin(strlen(tstr), sizeof(event.info.svcerror.filename)));
	}

	privateLogEvent(&event);
	unlock();
}

void
CPGPnetAppLog::logIKEAlert(PGPikeMTAlert *pAlert)
{
	lock();
	PGPnetLogEvent event;

	event.timeOfEvent = 0;	// gets filled in automatically
	event.typeOfEvent = kPGPnetLogEvent_PGPike;
	event.ipaddress	  = pAlert->ipAddress;

	event.info.ikealert.alert = pAlert->alert;
	event.info.ikealert.value = pAlert->value;
	event.info.ikealert.remoteGenerated = pAlert->remoteGenerated;

	privateLogEvent(&event);
	unlock();
}

void
CPGPnetAppLog::logEvent(PGPnetLogEvent *p_Event)
{
	lock();
	privateLogEvent(p_Event);
	unlock();
}

void
CPGPnetAppLog::privateLogEvent(PGPnetLogEvent *p_Event)
{
	PGPnetLogEventToFile (m_logRef, p_Event);
	PGPnetFlushLogBuffers (m_logRef);

	sendEventToApp(p_Event);
}

void
CPGPnetAppLog::sendEventToApp(PGPnetLogEvent *p_Event)
{
	if (m_bLogToApp) {
		// want to send the event to the app
		HWND hWnd = FindWindow(PGPNET_APPWINDOWCLASS, PGPNET_APPWINDOWTITLE);
		if (hWnd) {
			COPYDATASTRUCT cds;
			
			cds.dwData	= PGPNET_LOGUPDATE;
			cds.cbData	= sizeof(PGPnetLogEvent);
			cds.lpData	= p_Event;
			
			SendMessage(hWnd,
						WM_COPYDATA,
						(WPARAM) m_hSendingWnd,
						(LPARAM) &cds);
		}
	}
}	

void
CPGPnetAppLog::clearLog()
{
	lock();
	PGPnetClearLogFile(m_logRef);
	unlock();
}

//
// static functions
//
CPGPnetAppLog *
CPGPnetAppLog::instance()
{
	if (!sm_instance) {
		sm_lock.lock();
		if (!sm_instance) {
			sm_instance = new CPGPnetAppLog();
			sm_destroyer.SetDoomed(sm_instance);
		}
		sm_lock.unlock();
	}
	return sm_instance;
}
