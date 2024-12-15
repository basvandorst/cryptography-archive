/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CLDAPKeyServer.h,v 1.9 1999/05/03 18:15:09 heller Exp $
____________________________________________________________________________*/

#ifndef Included_CLDAPKeyServer_h
#define Included_CLDAPKeyServer_h

/* HP-UX aCC will not compile this file unless the ANSI C prototypes are used,
   otherwise it thinks that all of the functions defined in ldap.h have no
   arguments */
#if PGP_COMPILER_HPUX
#define NEEDPROTOS 1
#endif /* PGP_COMPILER_HPUX */

#include "ldap.h"

#include "CKeyServer.h"


class CLDAPKeyServer : public CKeyServer {
public:
							CLDAPKeyServer(PGPContextRef inContext,
								const char * inHostName,
								PGPUInt32 inHostAddress,PGPUInt16 inHostPort,
								PGPKeyServerProtocol inType,
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
	virtual void			GetAccessType(PGPKeyServerAccessType &outAccessType)
							{ 
								outAccessType = mAccessType;
							}
	virtual void			GetKeySpace(PGPKeyServerKeySpace &outKeySpace)
							{ 
								outKeySpace = mKeySpace;
							}
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
