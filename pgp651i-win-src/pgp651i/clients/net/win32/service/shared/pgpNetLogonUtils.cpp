/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetLogonUtils.cpp,v 1.30 1999/05/24 16:53:55 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <assert.h>

// PGP includes
#include "pgpErrors.h"
#include "pgpRMWOLock.h"
#include "pgpUserInterface.h"

// PGP client includes
#include "pgpTrayIPC.h"

// PGPnet includes
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpRWLockSignal.h"
#include "pgpNetIPC.h"
#include "pgpNetCConfig.h"

#include "pgpNetLogonUtils.h"

// external globals
extern CPGPrwLockSignal<CPGPnetCConfig> *	g_pConfig;
extern PGPContextRef						g_Context;

// local statics
static void		sSetUserLoggedIn(PGPUInt16 b);
static void		sSendMessage(PGPUInt32 msg, HWND hSendingWnd);
static PGPError sPGPPassphrase(PGPKeyRef pgpKeyRef, HWND hSendingWnd);
static PGPError sX509Passphrase(PGPKeyRef pgpKeyRef, HWND hSendingWnd);

static PGPRMWOLock		s_UserLoggedInLock;
static PGPUInt16		s_isUserLoggedIn;
static PGPBoolean		s_initialized = FALSE;

void
sSetUserLoggedIn(PGPUInt16 b)
{
	PGPRMWOLockStartWriting(&s_UserLoggedInLock);
	s_isUserLoggedIn = b;
	PGPRMWOLockStopWriting(&s_UserLoggedInLock);
}

void
sSendMessage(PGPUInt32 msg, HWND hSendingWnd)
{
	HWND hWnd;
	if ((hWnd = FindWindow(PGPTRAY_WINDOWNAME, PGPTRAY_WINDOWNAME))) {
		SendMessage(hWnd, PGPNET_M_APPMESSAGE, msg, (LPARAM)hSendingWnd);
	}

	if ((hWnd = FindWindow(PGPNET_APPWINDOWCLASS, PGPNET_APPWINDOWTITLE))) {
		SendMessage(hWnd, PGPNET_M_APPMESSAGE, msg, (LPARAM)hSendingWnd);
	}
}

void
pgpNetLogonCleanup()
{
	if (s_initialized) {
		pgpNetClearPassphrases();
		DeletePGPRMWOLock(&s_UserLoggedInLock);
		s_initialized = FALSE;
	}
}

void
pgpNetLogonInit()
{
	InitializePGPRMWOLock(&s_UserLoggedInLock);
	s_initialized = TRUE;
	
	g_pConfig->startWriting();
	PGPError err = g_pConfig->data().getKeyRefs();
	g_pConfig->stopWriting();

	if (IsPGPError(err)) {
		return;
	}
	
	g_pConfig->startReading();
	// check for passphraseless
	PGPBoolean bNeedsPhrase = g_pConfig->data().needsPassphrase();
	g_pConfig->stopReading();

	if (!bNeedsPhrase)
		sSetUserLoggedIn(PGPNET_NOLOGONREQUIRED);
	else	
		sSetUserLoggedIn(PGPNET_LOGGEDOFF);
}

void
pgpNetSendLogonStatus(HWND hSendingWnd)
{
	HWND hWnd;
	COPYDATASTRUCT cds;
	DWORD dwTemp;

	PGPRMWOLockStartReading(&s_UserLoggedInLock);
	dwTemp = s_isUserLoggedIn;
	PGPRMWOLockStopReading(&s_UserLoggedInLock);

	cds.dwData = PGPNET_LOGONSTATUS;
	cds.cbData = sizeof(dwTemp);
	cds.lpData = &dwTemp;

	if (dwTemp == PGPNET_LOGONINPROCESS) {
		sSendMessage(PGPNET_DISABLEGUI, hSendingWnd);
	}

	if ((hWnd = FindWindow(PGPTRAY_WINDOWNAME, PGPTRAY_WINDOWNAME))) {
		SendMessage(hWnd, WM_COPYDATA, (WPARAM) hSendingWnd, (LPARAM) &cds);
	}

	if ((hWnd = FindWindow(PGPNET_APPWINDOWCLASS, PGPNET_APPWINDOWTITLE))) {
		SendMessage(hWnd, WM_COPYDATA, (WPARAM) hSendingWnd, (LPARAM) &cds);
	}
}

void
pgpNetClearPassphrases()
{
	g_pConfig->startWriting();
	g_pConfig->data().clearPassphrases();
	g_pConfig->stopWriting();

	sSetUserLoggedIn(PGPNET_LOGGEDOFF);
}

void
pgpNetPromptForPassphrase(HWND hInitiatorWnd,
						  HWND hSendingWnd,
						  PGPBoolean bForce)
{
	PGPKeyRef pgpKeyRef = kInvalidPGPKeyRef;
	PGPKeyRef x509KeyRef = kInvalidPGPKeyRef;
	PGPBoolean bPGPNeedsPhrase = TRUE;
	PGPBoolean bX509NeedsPhrase = TRUE;

	g_pConfig->startWriting();
	PGPError err = g_pConfig->data().getKeyRefs();
	g_pConfig->stopWriting();

	if (IsPGPError(err)) {
		CPGPnetAppLog::instance()->logServiceEvent(
			kPGPnetSrvcError_NoAuthKey,
			0,
			0,
			__FILE__,
			__LINE__);
		return;
	}

	
	if (!bForce) {
		PGPRMWOLockStartReading(&s_UserLoggedInLock);

		if (s_isUserLoggedIn == PGPNET_LOGGEDON ||
			s_isUserLoggedIn == PGPNET_LOGONINPROCESS ||
			s_isUserLoggedIn == PGPNET_NOLOGONREQUIRED) {
			PGPRMWOLockStopReading(&s_UserLoggedInLock);
			return;
		}

		PGPRMWOLockStopReading(&s_UserLoggedInLock);
	}

	pgpNetClearPassphrases();

	g_pConfig->startReading();
	pgpKeyRef = g_pConfig->data().pgpKeyRef();
	x509KeyRef = g_pConfig->data().x509KeyRef();
	// check for passphraseless
	PGPBoolean bNeedsPhrase = g_pConfig->data().needsPassphrase();
	g_pConfig->stopReading();

	if (!bNeedsPhrase) {
		sSetUserLoggedIn(PGPNET_NOLOGONREQUIRED);
		return;
	}

	// Ok check pgpKeyRef
	if (pgpKeyRef != kInvalidPGPKeyRef) {
		sSetUserLoggedIn(PGPNET_LOGONINPROCESS);
		err = sPGPPassphrase(pgpKeyRef, hSendingWnd);
		if (IsPGPError(err))
			goto fail;
	}

	if (x509KeyRef != kInvalidPGPKeyRef) {
		sSetUserLoggedIn(PGPNET_LOGONINPROCESS);
		err = sX509Passphrase(x509KeyRef, hSendingWnd);
		if (IsPGPError(err))
			goto fail;
	}

	if (hInitiatorWnd)
		SetForegroundWindow(hInitiatorWnd);

	sSetUserLoggedIn(PGPNET_LOGGEDON);
	sSendMessage(PGPNET_ENABLEGUI, hSendingWnd);

	return;

fail:
	if (hInitiatorWnd)
		SetForegroundWindow(hInitiatorWnd);

	sSetUserLoggedIn(PGPNET_LOGGEDOFF);
	sSendMessage(PGPNET_ENABLEGUI, hSendingWnd);

	if (err == kPGPError_UserAbort) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Unable to obtain correct passphrase for key");
		CPGPnetAppLog::instance()->logServiceEvent(
			kPGPnetSrvcError_NoAuthPassphrase,
			0,
			0,
			__FILE__,
			__LINE__);
	} else {
		CPGPnetDebugLog::instance()->dbgOut(
			"Unable to obtain correct passphrase for key");
		CPGPnetAppLog::instance()->logPGPEvent(
			err,
			0,
			__FILE__,
			__LINE__);
	}
}

PGPError
sPGPPassphrase(PGPKeyRef pgpKeyRef, HWND hSendingWnd)
{
	if (pgpKeyRef == kInvalidPGPKeyRef)
		return kPGPError_BadParams;

	// check for passphraseless
	PGPBoolean bPGPNeedsPhrase = TRUE;
	PGPError err = PGPGetKeyBoolean (pgpKeyRef,
		kPGPKeyPropNeedsPassphrase,
		&bPGPNeedsPhrase);

	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Unable to get kPGPKeyPropNeedsPassphrase");
		CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
		return err;
	}

	if (bPGPNeedsPhrase) {
		char *	pszPGPPassphrase = 0;

		sSendMessage(PGPNET_DISABLEGUI, hSendingWnd);
		sSetUserLoggedIn(PGPNET_LOGONINPROCESS);
		err = PGPKeyPassphraseDialog(g_Context,
			pgpKeyRef,
			PGPOUIOutputPassphrase(g_Context, &pszPGPPassphrase),
			PGPOUIWindowTitle(g_Context, "PGPnet Logon Passphrase"),
			PGPOLastOption(g_Context));

		if (IsPGPError(err))
			return err;

		g_pConfig->startWriting();
		g_pConfig->data().pgpPassphrase(pszPGPPassphrase);
		g_pConfig->stopWriting();
	}

	return kPGPError_NoErr;
}

PGPError
sX509Passphrase(PGPKeyRef x509KeyRef, HWND hSendingWnd)
{
	if (x509KeyRef == kInvalidPGPKeyRef)
		return kPGPError_BadParams;

	// check for passphraseless
	PGPBoolean bX509NeedsPhrase = TRUE;
	PGPError err = PGPGetKeyBoolean (x509KeyRef,
		kPGPKeyPropNeedsPassphrase,
		&bX509NeedsPhrase);

	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Unable to get kPGPKeyPropNeedsPassphrase");
		CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
		return err;
	}

	if (bX509NeedsPhrase) {
		char *	pszX509Passphrase = 0;

		sSendMessage(PGPNET_DISABLEGUI, hSendingWnd);
		err = PGPKeyPassphraseDialog(g_Context,
			x509KeyRef,
			PGPOUIOutputPassphrase(g_Context, &pszX509Passphrase),
			PGPOUIWindowTitle(g_Context, "PGPnet Logon Passphrase"),
			PGPOLastOption(g_Context));

		if (IsPGPError(err))
			return err;

		g_pConfig->startWriting();
		g_pConfig->data().x509Passphrase(pszX509Passphrase);
		g_pConfig->stopWriting();
	}

	return kPGPError_NoErr;
}

