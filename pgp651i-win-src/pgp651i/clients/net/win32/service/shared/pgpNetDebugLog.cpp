/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Implements class CPGPnetDebugLog, a thread-safe singleton
	class for debugging output.

	$Id: pgpNetDebugLog.cpp,v 1.6.6.1 1999/06/14 03:22:37 elowe Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <winsock2.h>
#include <windows.h>
#include <string>

#include "pgpNetLogUtils.h"
#include "pgpNetDebugLog.h"

//
// assignments of static members
//
char 							CPGPnetDebugLog::sm_filename[_MAX_PATH] = "";
CPGPnetDebugLog *				CPGPnetDebugLog::sm_instance = 0;
CPGPlockable 					CPGPnetDebugLog::sm_lock;
CPGPdestroyer<CPGPnetDebugLog>	CPGPnetDebugLog::sm_destroyer;
PGPBoolean						CPGPnetDebugLog::sm_debug = FALSE;

//
// implementation of CPGPnetDebugLog
//
CPGPnetDebugLog::CPGPnetDebugLog(const char *filename /* = 0 */)
	: m_bOpenAndClose(TRUE)
{
	m_filename = filename;
	if (0 == filename || filename[0] == '\0') {
		m_errStream = stderr;
		m_bOpenAndClose = FALSE;
		return;
	}
	m_errStream = fopen (m_filename, "w");
	if (NULL == m_errStream) {
		m_errStream = fopen (m_filename, "w");
		if (NULL == m_errStream) {
			m_errStream = stderr;
			m_bOpenAndClose = FALSE;
		} else {
			fclose (m_errStream);
		}
	}
}

CPGPnetDebugLog::~CPGPnetDebugLog()
{ 
}

void
CPGPnetDebugLog::vprogerr(const char *fmt, va_list args, PGPBoolean eol, const char *prepend)
{
	lock();
	
	if (!sm_debug)
		return;

#ifdef _DEBUG
	voutputToDebugger(fmt, args, eol, prepend);
#endif
	
	if (m_bOpenAndClose) {
		m_errStream = fopen (m_filename, "a");
	}
	
	::vprogerr(fmt, args, m_errStream, eol, prepend);
	
	if (m_bOpenAndClose) {
		fclose (m_errStream);
	}
	unlock();
}

void
CPGPnetDebugLog::vdbgOut(const char *fmt, va_list args, const char *prepend)
{
	if (!sm_debug)
		return;
	vprogerr(fmt, args, TRUE, prepend);
}

void
CPGPnetDebugLog::dbgOut(const char *fmt, ...)
{
	if (!sm_debug)
		return;

	va_list args;
	va_start(args, fmt);
	vprogerr(fmt, args);
	va_end(args);
}

void
CPGPnetDebugLog::dbgOutFL(const char *fmt, ...)
{
	if (!sm_debug)
		return;

	va_list args;
	va_start(args, fmt);
	vprogerr(fmt, args, FALSE);
	va_end(args);
}

//
// static functions
//
CPGPnetDebugLog *
CPGPnetDebugLog::instance()
{
	if (!sm_instance) {
		sm_lock.lock();
		if (!sm_instance) {
			sm_instance = new CPGPnetDebugLog(sm_filename);
			sm_destroyer.SetDoomed(sm_instance);
		}
		sm_lock.unlock();
	}
	return sm_instance;
}

PGPBoolean
CPGPnetDebugLog::Initialize(PGPBoolean cmdLine)
{
	sm_lock.lock();
		
#if PGP_DEBUG
	sm_debug = TRUE;
#else
	if (cmdLine)
		sm_debug = TRUE;
	else {
		HKEY hKey;
		std::string sKey = "SYSTEM\\CurrentControlSet\\Services\\PGPnetIKE";

		PGPInt32 ret;
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,sKey.c_str(),0,KEY_READ,&hKey);
		if (ret == ERROR_SUCCESS) {
			PGPUInt32 debug = 0;
			DWORD dwSize = sizeof(debug);
		
			ret = RegQueryValueEx(hKey,
								"Debug", 
								0, 
								0, 
								(PGPByte*)&debug, 
								&dwSize);
			if (ret != ERROR_SUCCESS) {
				sm_debug = FALSE;
			} else {
				sm_debug = (PGPBoolean)debug;
			}
		}
	}
#endif

	if (sm_debug) {
		// Get the executable file path for the debug log file
		char szFilePath[_MAX_PATH];
		std::string dbgFile;
		GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
		dbgFile = szFilePath;
		dbgFile = dbgFile.substr(0, dbgFile.length()-4);
		dbgFile += ".dbg";
		strcpy(sm_filename, dbgFile.c_str());
	}

	sm_lock.unlock();

	return sm_debug;
}