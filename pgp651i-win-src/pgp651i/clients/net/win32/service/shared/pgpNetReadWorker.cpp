/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetReadWorker.cpp,v 1.17 1999/05/20 03:06:31 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <queue>

// PGP includes
#include "pgpRMWOLock.h"
#include "pgpErrors.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpNetQueueElement.h"
#include "pgpIKE.h"
#include "pgpNetCConfig.h"
#include "pgpRWLockSignal.h"

#include "pgpNetReadWorker.h"

// external globals
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> >	g_IKEQueue;
extern CPGPrwLockSignal<CPGPnetCConfig>	*					g_pConfig;
extern PGPCond_t											g_ShutdownRequest;
extern PGPContextRef										g_Context;

CPGPnetReadWorker::CPGPnetReadWorker(SOCKET s)
: m_socket(s), m_size(65536)
{
	m_pBuffer = new PGPByte[m_size];
}

CPGPnetReadWorker::~CPGPnetReadWorker()
{
	delete [] m_pBuffer;
}

unsigned int
CPGPnetReadWorker::Run()
{
	PGPUInt32		ret = 0;
	MSG				msg;
	const PGPInt16	numHandles = 1;
	HANDLE			lpHandles[numHandles];
	
	if (!registerClass(0))
		return FALSE;
	
	if (!createMyWindow(0))
		return FALSE;

	if (WSAAsyncSelect(m_socket, m_hWnd, WM_APP+145, FD_READ) == SOCKET_ERROR)
		return FALSE;

	lpHandles[0] = g_ShutdownRequest;

	PGPBoolean running = TRUE;
	while (running) {
		// Read all of the messages in this next loop, 
		// removing each message as we read it.
		while (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE)) { 
			// If it's a quit message, we're out of here.
			if (msg.message == WM_QUIT) {
				running = FALSE;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				goto end;
			}
			// Otherwise, dispatch the message.
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} // End of PeekMessage while loop.

		ret = MsgWaitForMultipleObjects(numHandles,
			lpHandles, 
			FALSE, 
			INFINITE,
			QS_ALLINPUT);
		if (ret >= WAIT_ABANDONED_0) {
			CPGPnetDebugLog::instance()->dbgOut(
				"ReadWorker: something abandoned");
			PostQuitMessage(0);
		} else if (ret == WAIT_OBJECT_0 + numHandles) {
			continue;
		} else if (ret >= WAIT_OBJECT_0) {
			switch (ret - WAIT_OBJECT_0) {
			case 0:
				CPGPnetDebugLog::instance()->dbgOut(
					"ReadWorker: ShutdownRequest signaled");
				PostQuitMessage(0);
				break;
			default:
				CPGPnetDebugLog::instance()->dbgOut(
					"ReadWorker: something signaled");
				break;
			}
		}
	}

end:
	return kPGPError_NoErr;
}

void
CPGPnetReadWorker::readUDP()
{
	struct sockaddr_in	fromAddr;
	PGPInt32			fromLen = sizeof(fromAddr);

	if (!m_pBuffer) {
		return;
	}

	PGPInt32 ret = recvfrom(m_socket, 
		(char*)m_pBuffer, 
		m_size, 
		0,
		(struct sockaddr*)&fromAddr,
		&fromLen);

	// check to see if PGPnet is enabled
	g_pConfig->startReading();
	if (!g_pConfig->data().config()->bPGPnetEnabled) {
		g_pConfig->stopReading();
		return;
	}
	g_pConfig->stopReading();

	if (ret == SOCKET_ERROR) {
		PGPInt32 err = WSAGetLastError();
		switch (err) {
		case WSAEWOULDBLOCK:
			return;
			break;
		case WSAEMSGSIZE:
			CPGPnetDebugLog::instance()->dbgOut("ReadWorker: Buffer underrun");
			delete [] m_pBuffer;
			m_pBuffer = new PGPByte[m_size*2];
			m_size *= 2;
			break;
		default:
			CPGPnetAppLog::instance()->logSysEvent(err, 0, __FILE__, __LINE__);
			return;
		}
	} else if (ret > 0) {
		CPGPnetDebugLog::instance()->dbgOut(
			"ReadWorker: Read %d bytes from %s - sending to IKE",
			ret,
			inet_ntoa(fromAddr.sin_addr));

		g_IKEQueue.startWriting();

		g_IKEQueue.data().push(CPGPnetQueueElement(ret,
			m_pBuffer, 
			kPGPike_MT_Packet,
			fromAddr.sin_addr.s_addr));

		g_IKEQueue.stopWriting();
		g_IKEQueue.setEvent();
	}
}

PGPInt32
CPGPnetReadWorker::registerClass(HINSTANCE hinstance) 
{ 
	WNDCLASSEX wcx; 
	
	// Fill in the window class structure with parameters 
	// that describe the main window. 
	
	wcx.cbSize = sizeof(wcx);				// size of structure 
	wcx.style = CS_HREDRAW | CS_VREDRAW;	// redraw if size changes 
	wcx.lpfnWndProc = CPGPnetReadWorker::winProc; // points to window proc
	wcx.cbClsExtra = 0;						// no extra class memory 
	wcx.cbWndExtra = 0;						// no extra window memory 
	wcx.hInstance = hinstance;				// handle of instance 
	wcx.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);	// predefined app. icon 
	wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);	// predefined arrow 
	wcx.hbrBackground = NULL;				// white background brush 
	wcx.lpszMenuName =  NULL;				// name of menu resource 
	wcx.lpszClassName = "PGPnetReadWorker";	// name of window class 
	wcx.hIconSm = NULL;
	
	// Register the window class. 
	return ::RegisterClassEx(&wcx); 
}

PGPBoolean
CPGPnetReadWorker::createMyWindow(HINSTANCE hinstance) 
{ 
	// Create the gui worker window. 
	
	m_hWnd = CreateWindow("PGPnetReadWorker",	// name of window class 
						  "PGPnetReadWorker",	// title-bar string 
						  WS_OVERLAPPEDWINDOW,	// top-level window 
						  CW_USEDEFAULT,		// default horizontal pos
						  CW_USEDEFAULT,		// default vertical pos
						  CW_USEDEFAULT,		// default width 
						  CW_USEDEFAULT,		// default height 
						  (HWND) NULL,			// no owner window 
						  (HMENU) NULL,			// use class menu 
						  hinstance,			// handle of app instance 
						  (LPVOID) NULL);		// no window-creation data 
	
	if (!m_hWnd) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Fatal Error: Unable to create window '%s'",
			"PGPnetReadWorker");
		CPGPnetAppLog::instance()->logSysEvent(GetLastError(),
			0,
			__FILE__,
			__LINE__);
		return FALSE;
	}

	SetLastError(0);
	PGPInt32 err = SetWindowLong(m_hWnd, GWL_USERDATA, (long)this);
	PGPInt32 lasterr = GetLastError();
	if (err == 0 && lasterr != 0) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Fatal Error: Unable to set window '%s' data",
			"PGPnetReadWorker");
		CPGPnetAppLog::instance()->logSysEvent(lasterr, 
			0,
			__FILE__,
			__LINE__);
		return FALSE;
	}
	
	return TRUE; 
}

LRESULT CALLBACK
CPGPnetReadWorker::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)    
{
	CPGPnetReadWorker *me = static_cast<CPGPnetReadWorker*>(
		(void*)::GetWindowLong(hwnd, GWL_USERDATA));

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_APP+145:
		me->readUDP();
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
