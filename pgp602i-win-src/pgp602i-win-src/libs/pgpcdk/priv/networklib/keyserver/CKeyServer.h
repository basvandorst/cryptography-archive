/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: CKeyServer.h,v 1.23.10.1 1998/11/12 03:22:17 heller Exp $
____________________________________________________________________________*/

#ifndef Included_CKeyServer_h
#define Included_CKeyServer_h

#if PGP_WIN32
#include <windows.h>
#include "PGPAsyncDNS.h"
#endif

#if PGP_WIN32 || PGP_UNIX
#include "pgpRMWOLock.h"
#endif

#include "pgpException.h"

#include "pgpKeyServer.h"

/* Key ordering */
enum PGPKeyUploadPreference_
{
	kPGPInvalid							= 0,
	kPGPPublicKeyOnly 					= 1,
	kPGPPrivateKeyAllowed				= 2,
	
	PGP_ENUM_FORCE( PGPKeyUploadPreference_ )
} ;
PGPENUM_TYPEDEF( PGPKeyUploadPreference_, PGPKeyUploadPreference );



class StPreserveSocketsEventHandler;

struct SThreadContext;

class CKeyServer {
	friend class StPreserveSocketsEventHandler;
	
public:
	static void						Initialize();
	static void						Cleanup();
		
	static void						SetIdleEventHandler(
										PGPEventHandlerProcPtr inCallback,
										PGPUserValue inUserData);
	static void						GetIdleEventHandler(
										PGPEventHandlerProcPtr * outCallback,
										PGPUserValue * outUserData);
	
	static void						CreateThreadStorage(
										SThreadContext ** outContext);
	static void						DisposeThreadStorage(
										SThreadContext * inContext);

	static CKeyServer *				NewKeyServerFromURL(
										PGPContextRef inContext, 
										const char * inURL, 
										PGPKeyServerAccessType inAccessType,
										PGPKeyServerKeySpace inKeySpace);
	static CKeyServer *				NewKeyServerFromHostAddress(
										PGPContextRef inContext,
										PGPUInt32 inHostAddress,
										PGPUInt16 inHostPort,
										PGPKeyServerType inType,
										PGPKeyServerAccessType inAccessType,
										PGPKeyServerKeySpace inKeySpace);
	static CKeyServer *				NewKeyServerFromHostName(
										PGPContextRef inContext,
										const char * inHostName,
										PGPUInt16 inHostPort,
										PGPKeyServerType inType,
										PGPKeyServerAccessType inAccessType,
										PGPKeyServerKeySpace inKeySpace);
										
	virtual PGPBoolean				Free();
	virtual void					IncRefCount() { mRefCount++; }
										
									CKeyServer(PGPContextRef inContext,
										const char * inHostName,
										PGPUInt32 inHostAddress,
										PGPUInt16 inHostPort,
										PGPKeyServerType inType);
	virtual							~CKeyServer();
	
	virtual void					SetEventHandler(
										PGPEventHandlerProcPtr inCallback,
										PGPUserValue inUserData)
										{ mEventHandler = inCallback;
										  mEventHandlerData = inUserData; }
	virtual void					GetEventHandler(
										PGPEventHandlerProcPtr & outCallback,
										PGPUserValue & outUserData)
										{ outCallback = mEventHandler;
										  outUserData = mEventHandlerData; }
										  
	virtual void					Open(PGPtlsSessionRef inTLSSession);
	virtual void					Close();
	
	virtual void					Cancel();
	
	virtual void					GetTLSSession(
										PGPtlsSessionRef & outTLSSession)
										{ outTLSSession = mTLSSession; }
	virtual void					GetType(PGPKeyServerType & outType)
										{ outType = mType; }
	virtual void					GetAccessType(PGPKeyServerAccessType & 
											outAccessType)
										{ 
											(void) outAccessType;
											ThrowPGPError_(
										kPGPError_ServerOperationNotSupported);
										}
	virtual void					GetKeySpace(
										PGPKeyServerKeySpace & outKeySpace)
										{ 
											(void) outKeySpace;
											ThrowPGPError_(
										kPGPError_ServerOperationNotSupported);
										}
	virtual void					GetPort(PGPUInt16 & outPort)
										{ outPort = mHostPort; }
	virtual void					GetHostName(char ** outHostName);
	virtual void					GetAddress(PGPUInt32 & outAddress);
	virtual void					GetErrorString(char ** outErrorString);
										
	virtual PGPBoolean				IsBusy() { return mIsBusy; }
	virtual void					SetBusy(PGPBoolean inIsBusy)
										{ mIsBusy = inIsBusy; }
	
	virtual PGPBoolean				IsCanceled() { return mCanceled; }
	virtual void					ResetCanceled()
										{ mCanceled = false; }
										
	virtual void		Query(PGPFilterRef inFilterRef, 
							PGPKeySetRef * outFoundKeys)
							{ 	(void) inFilterRef;
								(void) outFoundKeys;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }
	virtual void		Upload(PGPKeyUploadPreference inSendPrivateKeys, 
							PGPKeySetRef inKeysToUpload,
							PGPKeySetRef * outKeysThatFailed)
							{ 	(void) inSendPrivateKeys;
								(void) inKeysToUpload;
								(void) outKeysThatFailed;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }
	virtual void		Delete(PGPKeySetRef inKeysToDelete,
							PGPKeySetRef * outKeysThatFailed)
							{ 	(void) inKeysToDelete;
								(void) outKeysThatFailed;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }
	virtual void		Disable(PGPKeySetRef inKeysToDisable,
							PGPKeySetRef * outKeysThatFailed)
							{ 	(void) inKeysToDisable;
								(void) outKeysThatFailed;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }
	virtual void		SendGroups(
							PGPGroupSetRef inGroupSetRef)
							{ 	(void) inGroupSetRef;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }
	virtual void		RetrieveGroups(
							PGPGroupSetRef * outGroupSetRef)
							{ 	(void) outGroupSetRef;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }
	virtual void		NewMonitor(
							PGPKeyServerMonitor ** outMonitor)
							{ 	(void) outMonitor;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }
	virtual void		FreeMonitor(
							PGPKeyServerMonitor * inMonitor)
							{ 	(void) inMonitor;
								ThrowPGPError_(
								kPGPError_ServerOperationNotSupported); }

protected:
#if PGP_WIN32
	static PGPRMWOLock				sThreadLock;
	static PGPInt32					sIdleEventHandlerIndex;
	static PGPInt32					sIdleEventHandlerDataIndex;
	PGPAsyncHostEntryRef			mAsyncHostEntryRef;
#endif
	static PGPBoolean				sIsInitialized;

	PGPContextRef					mContext;
	char *							mHostName;
	PGPUInt16						mHostPort;
	PGPUInt32						mHostAddress;
	PGPKeyServerType				mType;
	PGPEventHandlerProcPtr			mEventHandler;
	PGPUserValue					mEventHandlerData;
	PGPtlsSessionRef				mTLSSession;
	char *							mErrorString;
	PGPBoolean						mSecured;
	PGPBoolean						mIsBusy;
	PGPBoolean						mIsOpen;
	PGPBoolean						mCanceled;
	PGPInt32						mRefCount;
	
	virtual void					InitializeHostNameAndAddress();
	virtual void					SetErrorString(
										const char * inErrorString);
	static void						GetThreadIdleEventHandler(
										PGPEventHandlerProcPtr * outCallback,
										PGPUserValue * outUserData);
	static void						SetThreadIdleEventHandler(
										PGPEventHandlerProcPtr inCallback,
										PGPUserValue inUserData);
	static PGPError					KeyServerIdleEventHandler(
										PGPContextRef inContext,
										PGPEvent * inEvent,
										PGPUserValue callBackArg);

private:
	static void						CheckInitialization();
};


class StPreserveSocketsEventHandler {
public:
								StPreserveSocketsEventHandler(
									CKeyServer * inKeyServer);
	virtual						~StPreserveSocketsEventHandler();
	
protected:
	StPGPPreserveSocketsStorage	mSocketsStorage;
};

#endif