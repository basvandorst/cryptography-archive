/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Defines class CPGPnetDebugLog. This is a thread-safe singleton
	class that is used to send debug information to a file

	$Id: pgpNetDebugLog.h,v 1.4.6.1 1999/06/14 03:22:37 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetDebugLog_h
#define _Included_pgpNetDebugLog_h

#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "pgpLockable.h"
#include "pgpSingletonDestroyer.h"

#define _l  __LINE__		// int
#define _f  __FILE__		// string
#define _t  __TIME__		// string
#define _ts __TIMESTAMP__	// string
#define _c  __DATE__		// string

class CPGPnetDebugLog : public CPGPlockable
{
private:
	// data members
	PGPBoolean		m_bOpenAndClose;
	const char *	m_filename;
	FILE *			m_errStream;

	// static members
	static char 				sm_filename[_MAX_PATH];
	static CPGPnetDebugLog *	sm_instance;
	static CPGPlockable			sm_lock;
	static PGPBoolean			sm_debug;
	static CPGPdestroyer<CPGPnetDebugLog> sm_destroyer;

	// private functions
	void vprogerr(const char *fmt, 
		va_list args,
		PGPBoolean eol = TRUE,
		const char *prepend = NULL);
	CPGPnetDebugLog(const CPGPnetDebugLog &);
	CPGPnetDebugLog & operator=(const CPGPnetDebugLog &);
	
protected:
	CPGPnetDebugLog(const char *filename = 0);
	virtual ~CPGPnetDebugLog();
	
	friend class CPGPdestroyer<CPGPnetDebugLog>;

public:
	static CPGPnetDebugLog *instance();
	static PGPBoolean Initialize(PGPBoolean cmdLine);
	static PGPBoolean isDebug() { return sm_debug; };
	static void DbgOut(const char *fmt, ...) {
		CPGPnetDebugLog::instance()->dbgOut(fmt);
	};
	static void DbgOutFL(int lineno, char *fname) {
		CPGPnetDebugLog::instance()->dbgOutFL(lineno, fname);
	};

	void dbgOutFL(const char *fmt, ...);
	void dbgOutFL(int lineno, char *fname) {
		dbgOutFL("[%s : line %d] ", fname, lineno);
	};
	void dbgOut(const char *fmt, ...);
	void vdbgOut(const char *fmt, 
		va_list args, 
		const char *prepend = NULL);
};

#endif // _Included_pgpNetDebugLog_h
