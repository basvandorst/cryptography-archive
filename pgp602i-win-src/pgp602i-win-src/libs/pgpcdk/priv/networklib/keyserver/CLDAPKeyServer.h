/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: CLDAPKeyServer.h,v 1.4.10.1 1998/11/12 03:22:18 heller Exp $
____________________________________________________________________________*/

#ifndef Included_CLDAPKeyServer_h
#define Included_CLDAPKeyServer_h

#include "ldap.h"

#include "CKeyServer.h"


class CLDAPKeyServer : public CKeyServer {
public:
							CLDAPKeyServer(PGPContextRef inContext,
								const char * inHostName,
								PGPUInt32 inHostAddress,PGPUInt16 inHostPort,
								PGPKeyServerType inType,
								PGPKeyServerAccessType inAccessType,
								PGPKeyServerKeySpace inKeySpace);
	virtual					~CLDAPKeyServer();
	
	virtual void			Cancel();

	virtual void			Open(PGPtlsSessionRef inTLSSession);
	virtual void			Close();

	virtual void			Query(PGPFilterRef inFilterRef,
								PGPKeySetRef * outFoundKeys);
	virtual void			Upload(PGPKeyUploadPreference inSendPrivateKeys,
								PGPKeySetRef inKeysToUpload,
								PGPKeySetRef * outKeysThatFailed);
	virtual void			Delete(PGPKeySetRef inKeysToDelete,
								PGPKeySetRef * outKeysThatFailed);
	virtual void			Disable(PGPKeySetRef inKeysToDelete,
								PGPKeySetRef * outKeysThatFailed);
	virtual void			SendGroups(PGPGroupSetRef inGroupSetRef);
	virtual void			RetrieveGroups(PGPGroupSetRef * outGroupSetRef);
	virtual void			NewMonitor(PGPKeyServerMonitor ** outMonitor);
	virtual void			FreeMonitor(PGPKeyServerMonitor * inMonitor);
								
protected:
	PGPKeyServerAccessType	mAccessType;
	PGPKeyServerKeySpace	mKeySpace;
	LDAP *					mLDAP;
	char *					mBaseKeySpaceDN;
	char *					mBasePendingDN;
	PGPExportFormat			mExportFormat;
	
	void					DeleteOrDisable(PGPBoolean inDelete,
								PGPKeySetRef inKeySet,
								PGPKeySetRef * outKeysThatFailed);
	PGPError				SendRequest(const char * inAction,
								PGPKeySetRef inKeySetRef,
								PGPKeySetRef * outKeysThatFailed);
};



#endif