/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetCConfig.h,v 1.17 1999/04/08 03:04:13 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetCConfig_h
#define _Included_pgpNetCConfig_h

// PGP includes
#include "pgpPubTypes.h"

// PGPnet includes
#include "pgpNetConfig.h"

class CPGPnetCConfig {
private:
	PNCONFIG		m_config;
	char *			m_pszPGPPassphrase;
	char *			m_pszX509Passphrase;
	PGPKeyRef		m_PGPkeyRef;
	PGPKeyRef		m_X509keyRef;
	PGPSigRef		m_X509sigRef;
	PGPKeySetRef	m_keySetRef;
	PGPBoolean		m_isDriverUp;
	
	void		sendConfigToKernel();
	void		sendConfigToIKE();
	PGPBoolean	x509NeedsPassphrase();
	PGPBoolean	pgpNeedsPassphrase();

	// private and undefined assignment and copy
	CPGPnetCConfig& operator=(const CPGPnetCConfig &);
	CPGPnetCConfig(const CPGPnetCConfig &);
	
public:
	CPGPnetCConfig();
	~CPGPnetCConfig();

	PGPError	loadConfiguration();
	PGPError	reloadConfiguration();
	PGPError	saveConfiguration();
	PGPError	reloadKeyring();
	void		propogateConfig();
	void		clearPassphrases();
	PGPError	getKeyRefs();
	PGPBoolean	needsPassphrase();
	PGPBoolean	isValidPassphrases();

	PGPNetHostEntry *	findHost(PGPUInt32 ipAddress);
	PGPNetHostEntry *	findHostByIndex(PGPUInt32 index);

	// access
	const PNCONFIG * const config()		{ return &m_config; };
	const char *		pgpPassphrase()	{ return m_pszPGPPassphrase; };
	const char *		x509Passphrase(){ return m_pszX509Passphrase; };
	const PGPKeyRef		pgpKeyRef()		{ return m_PGPkeyRef; };
	const PGPKeyRef		x509KeyRef()	{ return m_X509keyRef; };
	const PGPSigRef		x509SigRef()	{ return m_X509sigRef; };
	const PGPKeySetRef	keySetRef()		{ return m_keySetRef; };
	const PGPBoolean	isDriverUp()	{ return m_isDriverUp; };

	void pgpPassphrase(char *psz)	{ m_pszPGPPassphrase = psz; };
	void x509Passphrase(char *psz)	{ m_pszX509Passphrase = psz; };
	void isDriverUp(PGPBoolean b)	{ m_isDriverUp = b; };
		
protected:
};

#endif // _Included_pgpNetCConfig_h
