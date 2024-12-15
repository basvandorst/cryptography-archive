/*____________________________________________________________________________
	pgpKeyServer.h
	
	Copyright(C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGP Key Server LDAP/HTTP 

	$Id: pgpKeyServer.h,v 1.19 1997/09/24 17:12:41 lloyd Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServer_h
#define Included_pgpKeyServer_h

#include "pgpTypes.h"
#include "pgpPubTypes.h"
#include "pgpKeyServerTypes.h"
#include "pgpEncode.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


typedef struct PGPKeyServer *			PGPKeyServerRef;
typedef const struct PGPKeyServer *		PGPConstKeyServerRef;

#define	kInvalidPGPKeyServerRef			((PGPKeyServerRef) NULL)
#define PGPKeyServerRefIsValid( ref )	( (ref) != kInvalidPGPKeyServerRef )


/* these flags are returned from PGPQueryKeyServer() in 'extraInfo' */
enum
{
	kPGPKeyServerQuery_PartialResults = ( ((PGPFlags)1) << 0 )
};


PGPError 		PGPKeyServerInit( void );

PGPError 		PGPKeyServerCleanup( void );

PGPError 		PGPKeyServerOpen( PGPKeyServerRef keyServer );

PGPError 		PGPKeyServerClose( PGPKeyServerRef keyServer );

PGPError 		PGPNewKeyServerFromURL( PGPContextRef thecontext, 
							char const * url, 
							PGPKeyServerAccessType accessType,
							PGPKeyServerKeySpace keySpace,
							PGPKeyServerRef *keyServer );

PGPError 		PGPFreeKeyServer( PGPKeyServerRef keyServer );

PGPError 		PGPQueryKeyServer( PGPKeyServerRef keyServer, 
							PGPFilterRef filter, 
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg, 
							PGPKeySetRef *resultSet,
							PGPFlags *	resultInfo /* may be NULL */
							);

PGPError 		PGPUploadToKeyServer( PGPKeyServerRef keyServer, 
							PGPKeySetRef keysToUpload, 
							PGPEventHandlerProcPtr callBack, 
							PGPUserValue callBackArg,
							PGPKeySetRef *keysThatFailed);

PGPError		PGPDeleteFromKeyServer( PGPKeyServerRef	keyServer,
							PGPKeySetRef keysToDelete,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg,
							PGPKeySetRef *keysThatFailed);

PGPError		PGPDisableFromKeyServer( PGPKeyServerRef keyServer,
							PGPKeySetRef keysToDisable,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue callBackArg,
							PGPKeySetRef *keysThatFailed);

PGPError		PGPGetKeyServerErrorString( PGPKeyServerRef keyServer, 
							char **errorString );


PGPError		PGPLDAPNewServerMonitor( PGPKeyServerRef keyServer,
							PGPEventHandlerProcPtr callBack,
							PGPUserValue		callBackArg,
							PGPKeyServerMonitorRef *monitor );

PGPError		PGPLDAPFreeServerMonitor( PGPKeyServerRef keyServer,
							PGPKeyServerMonitorRef monitor );


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
