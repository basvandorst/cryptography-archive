/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetCConfig.cpp,v 1.29 1999/04/23 13:59:09 pbj Exp $
____________________________________________________________________________*/

#include <assert.h>
#include <stdio.h>
#include <queue>

// PGP includes
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpUserInterface.h"
#include "pgpEndianConversion.h"
#include "pgpSDKPrefs.h"

// PGPnet includes
#include "pgpNetQueueElement.h"
#include "pgpNetKernelXChng.h"
#include "pgpRWLockSignal.h"
#include "pgpNetAppLog.h"
#include "pgpNetDebugLog.h"
#include "pgpNetPaths.h"

#include "pgpNetCConfig.h"

extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_KernelQueue;
extern CPGPrwLockSignal< std::queue<CPGPnetQueueElement> > g_IKEQueue;

extern PGPContextRef g_Context;

CPGPnetCConfig::CPGPnetCConfig()
: m_pszPGPPassphrase(0),
 m_pszX509Passphrase(0),
 m_PGPkeyRef(kInvalidPGPKeyRef),
 m_X509keyRef(kInvalidPGPKeyRef),
 m_X509sigRef(kInvalidPGPSigRef),
 m_keySetRef(kInvalidPGPKeySetRef),
 m_isDriverUp(FALSE)
{
	pgpClearMemory(&m_config, sizeof(PNCONFIG));
}

CPGPnetCConfig::~CPGPnetCConfig()
{
	PGPFreeKeySet(m_keySetRef);
	m_keySetRef = kInvalidPGPKeySetRef;

	PGPnetFreeConfiguration(&m_config);
}

void
CPGPnetCConfig::clearPassphrases()
{
	if (m_pszPGPPassphrase) {
		FillMemory ((char *)m_pszPGPPassphrase,
			lstrlen((char *)m_pszPGPPassphrase),
			'\0');
		PGPFreeData ((char *)m_pszPGPPassphrase);
		m_pszPGPPassphrase = NULL;
	}
	if (m_pszX509Passphrase) {
		FillMemory ((char *)m_pszX509Passphrase,
			lstrlen((char *)m_pszX509Passphrase),
			'\0');
		PGPFreeData ((char *)m_pszX509Passphrase);
		m_pszX509Passphrase = NULL;
	}
}

PGPError
CPGPnetCConfig::getKeyRefs()
{
	PGPError err;

	// open keyring files
	if (m_keySetRef == kInvalidPGPKeySetRef) {
		err = PGPnetLoadSDKPrefs(g_Context);
		if (IsPGPError(err)) {
			CPGPnetDebugLog::instance()->dbgOut(
				"Unable to load default sdk prefs");
			CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
			return err;
		}

		err = PGPOpenDefaultKeyRings(g_Context, 0, &m_keySetRef);
		if (IsPGPError(err)) {
			CPGPnetDebugLog::instance()->dbgOut(
				"Unable to open default key ring");
			CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
			return err;
		}
	}

	err = PGPnetGetConfiguredAuthKeys(g_Context,
		&m_config,
		m_keySetRef,
		&m_PGPkeyRef,
		&m_X509keyRef,
		&m_X509sigRef);

	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut("Unable to load authentication keys");
		CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
		return err;
	}

	return kPGPError_NoErr;
}

PGPBoolean
CPGPnetCConfig::needsPassphrase()
{
	if (pgpNeedsPassphrase())
		return TRUE;
	else
		return x509NeedsPassphrase();
}

PGPBoolean
CPGPnetCConfig::x509NeedsPassphrase()
{
	if (m_X509keyRef == kInvalidPGPKeyRef)
		return FALSE;

	// check for passphraseless
	PGPBoolean bNeedsPhrase = FALSE;
	PGPError err = PGPGetKeyBoolean (m_X509keyRef,
		kPGPKeyPropNeedsPassphrase,
		&bNeedsPhrase);

	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Unable to get kPGPKeyPropNeedsPassphrase");
		CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
		return FALSE;
	}

	return bNeedsPhrase;
}

PGPBoolean
CPGPnetCConfig::pgpNeedsPassphrase()
{
	if (m_PGPkeyRef == kInvalidPGPKeyRef)
		return FALSE;

	// check for passphraseless
	PGPBoolean bNeedsPhrase = FALSE;
	PGPError err = PGPGetKeyBoolean (m_PGPkeyRef,
		kPGPKeyPropNeedsPassphrase,
		&bNeedsPhrase);

	if (IsPGPError(err)) {
		CPGPnetDebugLog::instance()->dbgOut(
			"Unable to get kPGPKeyPropNeedsPassphrase");
		CPGPnetAppLog::instance()->logPGPEvent(err, 0, __FILE__, __LINE__);
		return FALSE;
	}

	return bNeedsPhrase;
}

//
// XXX should this be more sophisticated?
//
PGPBoolean
CPGPnetCConfig::isValidPassphrases()
{
	if (m_PGPkeyRef != kInvalidPGPKeyRef) {
		if (!m_pszPGPPassphrase)
			return FALSE;
	}

	if (m_X509keyRef != kInvalidPGPKeyRef) {
		if (!m_pszX509Passphrase)
			return FALSE;
	}
	return TRUE;
}

PGPError
CPGPnetCConfig::reloadKeyring()
{
	PGPFreeKeySet(m_keySetRef);
	m_keySetRef = kInvalidPGPKeySetRef;

	return getKeyRefs();
}

PGPError
CPGPnetCConfig::loadConfiguration()
{
	pgpClearMemory(&m_config, sizeof(PNCONFIG));
	PGPError err = PGPnetLoadConfiguration(g_Context, &m_config);
	sendConfigToKernel();
	sendConfigToIKE();
	return err;
}

PGPError
CPGPnetCConfig::reloadConfiguration()
{
	return PGPnetReloadConfiguration(g_Context, &m_config);
}

PGPError
CPGPnetCConfig::saveConfiguration()
{
	return PGPnetSaveConfiguration(g_Context, &m_config);
}

void
CPGPnetCConfig::propogateConfig()
{
	sendConfigToKernel();
	sendConfigToIKE();
}

PGPNetHostEntry *
CPGPnetCConfig::findHost(PGPUInt32 ipAddress)
{
	PGPNetHostEntry	*host;
	PGPUInt32		i = 0;

	for (i = 0; i < m_config.uHostCount; i++) {
		host = &(m_config.pHostList[i]);
		if (ipAddress == host->ipAddress) {
			return host;
		}
	}

	for (i = 0; i < m_config.uHostCount; i++) {
		host = &(m_config.pHostList[i]);
		if ((ipAddress & host->ipMask) == (host->ipAddress & host->ipMask)) {
			return host;
		}
	}

	return 0;
}

PGPNetHostEntry *
CPGPnetCConfig::findHostByIndex(PGPUInt32 index)
{
	if (index < 0 || index >= m_config.uHostCount)
		return 0;

	return &(m_config.pHostList[index]);
}

void
CPGPnetCConfig::sendConfigToKernel()
{
	PGPnetKernelConfig kConfig;
	
	kConfig.bPGPnetEnabled = m_config.bPGPnetEnabled;
	kConfig.bAllowUnconfigHost = m_config.bAllowUnconfigHost;
	kConfig.bAttemptUnconfigHost = m_config.bRequireSecure;

	// put onto kernel Queue
	g_KernelQueue.startWriting();

	g_KernelQueue.data().push(CPGPnetQueueElement(
		sizeof(kConfig),
		&kConfig, 
		OID_PGP_NEWCONFIG,
		0));

	if (m_config.uHostCount > 0 ) {
		g_KernelQueue.data().push(CPGPnetQueueElement(
			m_config.uHostCount * sizeof(PGPNetHostEntry),
			m_config.pHostList,
			OID_PGP_ALLHOSTS,
			0));
	}

	g_KernelQueue.stopWriting();
	g_KernelQueue.setEvent();
}

void
CPGPnetCConfig::sendConfigToIKE()
{
	// put onto kernel Queue
	g_IKEQueue.startWriting();
	g_IKEQueue.data().push(CPGPnetQueueElement(0,0,kPGPike_MT_Pref,0));
	g_IKEQueue.stopWriting();
	g_IKEQueue.setEvent();
}