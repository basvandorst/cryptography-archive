/*____________________________________________________________________________
	pgpKeyServerPriv.h
	
	Copyright(C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGP Key Server LDAP/HTTP Private Header File 

	$Id: pgpKeyServerPriv.h,v 1.27 1997/09/24 17:12:38 lloyd Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServerPriv_h
#define Included_pgpKeyServerPriv_h

#include "pgpKeyServer.h"
#include "pgpKeyServerTypes.h"
#include "pgpKeyFilterPriv.h"
#include "pgpEncode.h"
#include "ldap.h"
#include "PGPSockets.h"


enum PGPKeyServerType_
{
	kPGPKeyServerInvalid			= 1,
	kPGPKeyServerLDAP				= 2,
	kPGPKeyServerHTTP				= 3,
	
	PGP_ENUM_FORCE( PGPKeyServerType_ )
};

PGPENUM_TYPEDEF( PGPKeyServerType_, PGPKeyServerType );

struct PGPKeyServer
{
	PGPContextRef			myContext;
	PGPKeyServerType		serverType;
	PGPKeyServerAccessType	accessType;
	PGPKeyServerKeySpace	keySpace;
	PGPSocketRef			http;
	char					*host;
	unsigned int			port;

	LDAP					*ldap;
	LDAPMessage				*ldapResult;
	int						errorNum;
	char					*error;
	char					*baseKeySpaceDN;
	char					*basePendingDN;

} ;
typedef struct PGPKeyServer	PGPKeyServer;

typedef struct PGPKeyServerCallbackData_
{
	PGPContextRef			myContext;
	PGPEventHandlerProcPtr	callBack;
	PGPUserValue			callBackArg;
	PGPSocketRef			socket;
	PGPUInt32				callBackState;
	PGPUInt32				soFar;
	PGPUInt32				total;
} PGPKeyServerCallbackData;

typedef PGPKeyServerCallbackData* PGPKeyServerCallbackDataRef;



#define kPGPKeyServerPortInvalid	0
#define kPGPKeyServerPortLDAP		389
#define	kPGPKeyServerPortHTTP		11371


PGPError		PGPGetKeyServerErrorString( PGPKeyServerRef keyServer, 
							char **errorString );

char			*pgpGrowQueryString(char **query, 
							uint *maxsize, 
							uint growthfactor );

void			pgpKeyServerSocketCallback( void * callBackArg );

/*
 * LDAP functions
 */

PGPError			pgpLDAPUploadToKeyServer( PGPKeyServerRef keyServer,
  							PGPKeySetRef keysToUpload,
							PGPEventHandlerProcPtr callBack, 
							PGPUserValue callBackArg,
							PGPKeySetRef *keysThatFailed);

PGPError			pgpQueryLDAPKeyServer( PGPKeyServerRef keyServer, 
							PGPFilterRef filter,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg, 
							PGPKeySetRef *resultSet,
							PGPFlags *resultFlags );

PGPError			pgpLDAPbind( PGPKeyServerRef keyServer,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue		callBackArg );

PGPError			pgpLDAPDeleteFromKeyServer( PGPKeyServerRef keyServer,
							PGPKeySetRef keysToDelete,
							PGPBoolean disableKeys,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg,
							PGPKeySetRef *keysThatFailed );

PGPError			pgpLDAPOpenKeyServer( PGPKeyServerRef keyServer,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg );

PGPError			pgpLDAPCloseKeyServer( PGPKeyServerRef keyServer,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg );

PGPError			pgpLDAPGetBaseDNFromKeyServer( PGPKeyServerRef	keyServer,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg );


/*
 * HKS functions
 */

PGPError			pgpHKPUploadToKeyServer( PGPKeyServerRef keyServer,
  							PGPKeySetRef keysToUpload,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg,
							PGPKeySetRef *keysThatFailed);

PGPError			pgpQueryHKSKeyServer( PGPKeyServerRef keyServer, 
							PGPFilterRef filter,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg, 
							PGPKeySetRef *resultSet );



PGPBoolean		pgpKeyServerIsValid( PGPKeyServer const * keyServer );
#define PGPValidateKeyServer( ks )	\
	PGPValidateParam( pgpKeyServerIsValid( ks ) )

#endif

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
