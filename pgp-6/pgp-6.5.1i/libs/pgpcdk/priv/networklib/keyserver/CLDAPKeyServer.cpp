/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: CLDAPKeyServer.cpp,v 1.32.6.1 1999/06/13 21:12:31 heller Exp $
____________________________________________________________________________*/

#include <string.h>
#include <ctype.h>
/* <stdlib.h> includs <sys/time.h> which aCC barfs on if <sys/sigevent.h>
   in not included. */
#if PGP_COMPILER_HPUX
#include <sys/sigevent.h>
#endif /* PGP_COMPILER_HPUX */
#include <stdlib.h>

#include "pgpStrings.h"
#include "pgpOptionList.h"
#include "pgpEventPriv.h"
#include "pgpKeys.h"
#include "pgpFeatures.h"
#include "pgpSockets.h"
#include "pgpDebug.h"

#include "CLDAPKeyServer.h"



#define ThrowIfLDAPCanceled_()	if (mCanceled) \
									ThrowPGPError_(kPGPError_UserAbort);
#define ThrowIfPGPErrorOrLDAPCanceled_(x)	{	ThrowIfPGPError_(x); \
												ThrowIfLDAPCanceled_(); }

static const char *	kBaseKeyspaceDN = "baseKeyspaceDN";
static const char *	kBasePendingDN = "basePendingDN";
static const char *	kVersionDN = "version";
static const char *	kPGPServerInfoCN = "cn=PGPServerInfo";
static const char *	kMonitorCN = "cn=monitor";
static const char *	kObjectClassAny = "(objectclass=*)";
static const char *	kPGPKeyAttribute = "pgpKey";
static const char *	kPGPKeyV2Attribute = "pgpKeyV2";
static const char *	kRequestPGPRequest = "PGPrequest";
static const char *	kDefaultPGPGroupspace = 
										"ou=default,o=pgp groupspace,c=us";
static const char *	kPGPSrvGroupFile = "pgpSrvGroupFile";
static const char *	kRequestRequest = "Request: ";
static const char * kRequestLocation = "Location: ";
static const char * kRequestPending = "pending";
static const char *	kRequestActive = "active";
static const char *	kAddAndRequestDNPrefix = "pgpcertid=virtual, ";
static const char *	kAdd = "add";
static const char *	kDelete = "delete";
static const char *	kDisable = "disable";
static const char *	kObjectClass = "objectclass";
static const char * kPGPSrvGroupFileClass = "pgpsrvgroupfile";



CLDAPKeyServer::CLDAPKeyServer(
	PGPContextRef			inContext,
	const char *			inHostName,
	PGPUInt32				inHostAddress,
	PGPUInt16				inHostPort,
	PGPKeyServerProtocol		inType,
	PGPKeyServerAccessType	inAccessType,
	PGPKeyServerKeySpace	inKeySpace)
	: CKeyServer(inContext, inHostName, inHostAddress, inHostPort, 0, inType),
	  mAccessType(inAccessType), mKeySpace(inKeySpace), mLDAP(0),
	  mBaseKeySpaceDN(0), mBasePendingDN(0),
	  mExportFormat(kPGPExportFormat_Basic)
{
}



CLDAPKeyServer::~CLDAPKeyServer()
{
}



	void
CLDAPKeyServer::Open(
	PGPtlsSessionRef	inTLSSession)
{
	StPreserveSocketsEventHandler	preserve(this);
	LDAPMessage *					message = 0;
	char **							value = 0;

	SetErrorString(0);
	try {
		PGPError			pgpError = kPGPError_NoErr;
		int					ldapResult = 0;
		LDAPMessage *		nextMessage = 0;
		char *				attribute;
		BerElement *		iterAttribute;
		static const char *	attr[4] = { kBaseKeyspaceDN,
									kBasePendingDN, kVersionDN, 0 };

		CKeyServer::Open(inTLSSession);
		
		// Open ldap connection
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Opening);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		mLDAP = ldap_open(mHostName, mHostPort);
		if (mLDAP == 0) {
			ThrowPGPError_(kPGPError_ServerOpenFailed);
		}
		ThrowIfLDAPCanceled_();
		
		// Secure connection for HTTPS
		if (mType == kPGPKeyServerProtocol_LDAPS) {
			if ((mTLSSession == kInvalidPGPtlsSessionRef)
			|| (PGPSocketsEstablishTLSSession(
					(PGPSocketRef) mLDAP->ld_sb.sb_sd,
					mTLSSession) != kPGPError_NoErr)) {
				pgpError = pgpEventKeyServerTLS(
								mContext,
								mEventHandler,
								mEventHandlerData,
								(PGPKeyServerRef) this,
								kPGPKeyServerState_TLSUnableToSecureConnection,
								mTLSSession);
			} else {
				pgpError = pgpEventKeyServerTLS(
								mContext,
								mEventHandler,
								mEventHandlerData,
								(PGPKeyServerRef) this,
								kPGPKeyServerState_TLSConnectionSecured,
								mTLSSession);
				mSecured = true;
				mLDAP->tlsSession = mTLSSession;
			}
			ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		}
		
		// Retrieve key space distinguished names
		ldapResult = ldap_search_s(	mLDAP,
									(char *) kPGPServerInfoCN,
									LDAP_SCOPE_BASE,
									(char *) kObjectClassAny,
									(char **) attr,
									0,
									&message);
		ThrowIfLDAPCanceled_();
		if (ldapResult != LDAP_SUCCESS) {
			ThrowPGPError_(kPGPError_ServerOpenFailed);
		}
		nextMessage = ldap_first_entry(mLDAP, message);
		while (nextMessage != 0) {
			attribute = ldap_first_attribute(	mLDAP,
												nextMessage,
												&iterAttribute);
			while (attribute != 0) {
				value = ldap_get_values(	mLDAP,
											nextMessage,
											attribute);
				if ((value != 0) && (*value != 0)) {
					if (pgpCompareStringsIgnoreCase(attribute,
					attr[0]) == 0) {
						mBaseKeySpaceDN = new char[strlen(*value) + 1];
						if (mBaseKeySpaceDN == 0) {
							ThrowPGPError_(kPGPError_OutOfMemory);
						}
						strcpy(mBaseKeySpaceDN, *value);
					} else if (pgpCompareStringsIgnoreCase(attribute,
					attr[1]) == 0) {
						mBasePendingDN = new char[strlen(*value) + 1];
						if (mBasePendingDN == 0) {
							ThrowPGPError_(kPGPError_OutOfMemory);
						}
						strcpy(mBasePendingDN, *value);
					} else if (pgpCompareStringsIgnoreCase(attribute,
					attr[2]) == 0) {
						if (atol(*value) > 1) {
							mExportFormat = kPGPExportFormat_Complete;
						}
					}
					ldap_value_free(value);
					value = 0;
				} else if ((value == 0)
				&& (mLDAP->ld_errno != LDAP_SUCCESS)) {
					if (mLDAP->ld_errno == LDAP_INSUFFICIENT_ACCESS) {
						ThrowPGPError_(kPGPError_ServerAuthorizationFailed);
					} else {
						ThrowPGPError_(kPGPError_ServerOpenFailed);
					}
				}
				attribute = ldap_next_attribute(	mLDAP,	
													nextMessage,
													iterAttribute);
			}
pgpFixBeforeShip(am getting errors here)
/*
			if (mLDAP->ld_errno != LDAP_SUCCESS) {
				ThrowPGPError_(kPGPError_ServerOpenFailed);
			}
*/			nextMessage = ldap_next_entry(	mLDAP,
											nextMessage);
		}
		if (/*(mLDAP->ld_errno != LDAP_SUCCESS) ||*/ (mBaseKeySpaceDN == 0)
		|| (mBasePendingDN == 0)) {
			ThrowPGPError_(kPGPError_ServerOpenFailed);
		}
		ldap_msgfree(message);
	}
	
	catch (...) {
		delete[] mBaseKeySpaceDN;
		mBaseKeySpaceDN = 0;
		delete[] mBasePendingDN;
		mBasePendingDN = 0;
		if (value != 0) {
			ldap_value_free(value);
		}
		if (message != 0) {
			ldap_msgfree(message);
		}
		mTLSSession = kInvalidPGPtlsSessionRef;
		mIsOpen = false;
		mSecured = false;
		if (mLDAP != 0) {
			if (!mCanceled) {
				SetErrorString(mLDAP->ld_error);
			}
			ldap_unbind(mLDAP);
			mLDAP = 0;
		}
		if (mCanceled) {
			mCanceled = false;
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			throw;
		}
	}
}



	void
CLDAPKeyServer::Close()
{
	StPreserveSocketsEventHandler	preserve(this);

	pgpEventKeyServer(	mContext,
						mEventHandler,
						mEventHandlerData,
						(PGPKeyServerRef) this,
						kPGPKeyServerState_Closing);
						
	delete[] mBaseKeySpaceDN;
	mBaseKeySpaceDN = 0;
	delete[] mBasePendingDN;
	mBasePendingDN = 0;
	if (mLDAP != 0) {
		// Both the TLS close and the socket close are done in the LDAP unbind
		ldap_unbind(mLDAP);
		mSecured = false;
		mLDAP = 0;
	}
	CKeyServer::Close();
}



	void
CLDAPKeyServer::Cancel()
{
	CKeyServer::Cancel();
	if (mIsBusy && (mLDAP != 0)
	&& PGPSocketRefIsValid( (PGPSocketRef) mLDAP->ld_sb.sb_sd)) {
		PGPCloseSocket((PGPSocketRef) mLDAP->ld_sb.sb_sd);
		mLDAP->ld_sb.sb_sd = 0;
#if PGP_WIN32
		if (PGPAsyncHostEntryRefIsValid(mLDAP->ld_sb.ref)) {
			PGPCancelAsyncHostEntryRef(mLDAP->ld_sb.ref);
			mLDAP->ld_sb.ref = kPGPInvalidAsyncHostEntryRef;
		}
#endif
	}
}



	void
CLDAPKeyServer::Query(
	PGPFilterRef	inFilterRef,
	PGPKeySetRef *	outFoundKeys)
{
	StPreserveSocketsEventHandler	preserve(this);
	char *							query = 0;
	LDAPMessage *					message = 0;
	char **							value = 0;
	PGPKeySetRef					foundKeys = kInvalidPGPKeySetRef;
	PGPKeySetRef					singleKeySet = kInvalidPGPKeySetRef;
	PGPBoolean						partialResults = false;
	PGPBoolean						receivedBadKeys = false;
	int								ldapResult 	= 0;
	PGPError						pgpError 	= kPGPError_NoErr;
	LDAPMessage *					nextMessage = 0;
	
	SetErrorString(0);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}
	
	try {
		char *				attribute;
		BerElement *		iterAttribute;
		const char *		attr[2] =
								{ (mExportFormat == kPGPExportFormat_Basic) ?
									kPGPKeyAttribute : kPGPKeyV2Attribute,
										0 };

		pgpError = PGPLDAPQueryFromFilter(inFilterRef, &query);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);

		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Querying);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		
		// Send the query
		
		ldapResult = ldap_search_s(	mLDAP,
									(mKeySpace == 
										kPGPKeyServerKeySpace_Pending) ?
										mBasePendingDN : mBaseKeySpaceDN,
									LDAP_SCOPE_SUBTREE,
									query,
									(char **) attr,
									0,
									&message);
		ThrowIfLDAPCanceled_();
		switch (ldapResult) {
			case LDAP_SUCCESS:
			{
			}
			break;
			
			
			case LDAP_INSUFFICIENT_ACCESS:
			{
				ThrowPGPError_(kPGPError_ServerAuthorizationFailed);
			}
			break;
			
			
			case LDAP_SIZELIMIT_EXCEEDED:
			case LDAP_TIMELIMIT_EXCEEDED:
			{
				partialResults = true;
			}
			break;
			
			
			default:
			{
				ThrowPGPError_(kPGPError_ServerSearchFailed);
			}
			break;
		}
		PGPFreeData(query);
		query = 0;
		
		// Get the results
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_ProcessingResults);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		pgpError = PGPNewKeySet(mContext, &foundKeys);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		nextMessage = ldap_first_entry(mLDAP, message);
		while (nextMessage != 0) {
			attribute = ldap_first_attribute(	mLDAP,
												nextMessage,
												&iterAttribute);
			while (attribute != 0) {
				value = ldap_get_values(	mLDAP,
											nextMessage,
											attribute);
				if ((value != 0) && (*value != 0)) {
					if (pgpCompareStringsIgnoreCase(attribute,
					attr[0]) == 0) {
						pgpError = PGPImportKeySet(
										mContext,
										&singleKeySet,
										PGPOInputBuffer(	mContext,
															*value,
															strlen(*value)),
										PGPOLastOption(mContext));
						if (IsPGPError(pgpError)) {
							receivedBadKeys = true;
						} else { 
							pgpError = PGPAddKeys(singleKeySet, foundKeys);
							ThrowIfPGPError_(pgpError);
							pgpError = PGPCommitKeyRingChanges(foundKeys);
							ThrowIfPGPError_(pgpError);
							PGPFreeKeySet(singleKeySet);
							singleKeySet = kInvalidPGPKeySetRef;
						}
					}
					ldap_value_free(value);
					value = 0;
				} else if ((value == 0)
				&& (mLDAP->ld_errno != LDAP_SUCCESS)) {
					ThrowPGPError_(kPGPError_ServerSearchFailed);
				}
				attribute = ldap_next_attribute(	mLDAP,	
													nextMessage,
													iterAttribute);
			}
pgpFixBeforeShip(am getting errors here)
/*
			if (mLDAP->ld_errno != LDAP_SUCCESS) {
				ThrowPGPError_(kPGPError_ServerSearchFailed);
			}
			
*/			
			nextMessage = ldap_next_entry(	mLDAP,
											nextMessage);
		}
/*		if (mLDAP->ld_errno != LDAP_SUCCESS) {
			ThrowPGPError_(kPGPError_ServerSearchFailed);
		}
*/		ldap_msgfree(message);
		*outFoundKeys = foundKeys;
	}
	
	catch (...) {
		if (singleKeySet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(singleKeySet);
		}
		if (foundKeys != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(foundKeys);
		}
		if (value != 0) {
			ldap_value_free(value);
		}
		if (message != 0) {
			ldap_msgfree(message);
		}
		if (query != 0) {
			PGPFreeData(query);
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			SetErrorString(mLDAP->ld_error);
			throw;
		}
	}
	
	if (receivedBadKeys) {
		ThrowPGPError_(kPGPError_ServerBadKeysInSearchResults);
	}
	if (partialResults) {
		ThrowPGPError_(kPGPError_ServerPartialSearchResults);
	}
}



	void
CLDAPKeyServer::Upload(
	PGPKeyUploadPreference inSendPrivateKeys,
	PGPKeySetRef	inKeysToUpload,
	PGPKeySetRef *	outKeysThatFailed)
{
	StPreserveSocketsEventHandler	preserve(this);
	
	char *			keySpace = 0;
	char *			exportedKey = 0;
	PGPKeySetRef	errorKeySet = kInvalidPGPKeySetRef;
	PGPKeyListRef	keyList = kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyIter = kInvalidPGPKeyIterRef;
	PGPKeySetRef	singleKeySet = kInvalidPGPKeySetRef;
	PGPBoolean		sendRequest = false;
	PGPBoolean		authorizationFailed = false;
	PGPBoolean		partialFailure = false;
	PGPBoolean		keyAlreadyExists = false;
	PGPBoolean		failedPolicy = false;
	PGPError		pgpError = kPGPError_NoErr;
	char *			keySpaceSuffix = 0;
	PGPBoolean		exportPrivateKeys = false;
	
	if( mKeySpace == kPGPKeyServerKeySpace_Pending )
	{
		keySpaceSuffix = mBasePendingDN;
	}
	else
	{
		keySpaceSuffix = mBaseKeySpaceDN;
	}
	
	SetErrorString(0);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}

	if (inSendPrivateKeys == kPGPPrivateKeyAllowed) {

		if (mType != kPGPKeyServerProtocol_LDAPS) {
			ThrowPGPError_(kPGPError_ServerOperationRequiresTLS);
		}

		exportPrivateKeys = true;
	}

	try {
		PGPKeyRef			key;
		PGPSize				bufSize;
		int					ldapResult;
		char *				strVals[2] = { 0, 0 };
		LDAPMod				mod = { 0,
								(char *) ((mExportFormat ==
								kPGPExportFormat_Basic) ?
								kPGPKeyAttribute : kPGPKeyV2Attribute),
								{strVals},
								0 };
		LDAPMod *			attrs[2] = { &mod, 0 };
		
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Uploading);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
			
		// Create error set
		pgpError = PGPNewKeySet(mContext, &errorKeySet);
		ThrowIfPGPError_(pgpError);
		
		// Create keyspace
		keySpace = new char[strlen(kAddAndRequestDNPrefix) + strlen(
					keySpaceSuffix) + 1];
		if (keySpace == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		sprintf(keySpace, "%s%s", kAddAndRequestDNPrefix, keySpaceSuffix);
		
		// Iterate through keys uploading them one at a time
		pgpError = PGPOrderKeySet(	inKeysToUpload,
									kPGPAnyOrdering,
									&keyList);
		ThrowIfPGPError_(pgpError);
		pgpError = PGPNewKeyIter(keyList, &keyIter);
		ThrowIfPGPError_(pgpError);
		while ((pgpError = PGPKeyIterNext(keyIter, &key)) == kPGPError_NoErr) {
			// Send to server
			pgpError = PGPNewSingletonKeySet(key, &singleKeySet);
			ThrowIfPGPError_(pgpError);
			if (! sendRequest) {
				pgpError = PGPExportKeySet(
								singleKeySet,
								PGPOExportPrivateKeys(mContext,
													exportPrivateKeys),
								PGPOAllocatedOutputBuffer(
													mContext,
													(void **) &exportedKey,
													MAX_PGPSize,
													&bufSize),
								PGPOExportFormat(mContext,
												mExportFormat),
								PGPOLastOption(mContext));

				ThrowIfPGPError_(pgpError);
				strVals[0] = exportedKey;
				ldapResult = ldap_add_s(	mLDAP,
											keySpace,
											attrs);
				PGPFreeData(exportedKey);
				exportedKey = 0;
				if (ldapResult != LDAP_SUCCESS) {
					partialFailure = true;
					PGPAddKeys(singleKeySet, errorKeySet);
					switch (ldapResult) {
						case LDAP_ALREADY_EXISTS:
						{
							keyAlreadyExists = true;
						}
						break;
						
						
						case LDAP_INSUFFICIENT_ACCESS:
						{
							sendRequest = true;
						}
						break;
						
						
						case LDAP_INVALID_CREDENTIALS:
						{
							failedPolicy = true;
						}
						break;
					}
				}
			}
			
			// Need to send a signed request
			if (sendRequest && (! mSecured)) {
				pgpError = SendRequest(kAdd, singleKeySet, 0);
				if (IsPGPError(pgpError)) {
					partialFailure = true;
					PGPAddKeys(singleKeySet, errorKeySet);
					switch (pgpError) {
						case kPGPError_ServerKeyAlreadyExists:
						{
							keyAlreadyExists = true;
						}
						break;
						
						
						case kPGPError_ServerAuthorizationFailed:
						{
							authorizationFailed = true;
						}
						break;
						
						
						case kPGPError_ServerKeyFailedPolicy:
						{
							failedPolicy = true;
						}
						break;
					}
				}
			}
			PGPFreeKeySet(singleKeySet);
			singleKeySet = kInvalidPGPKeySetRef;
		}
		delete[] keySpace;
		keySpace = 0;
		PGPFreeKeyIter(keyIter);
		keyIter = kInvalidPGPKeyIterRef;
		PGPFreeKeyList(keyList);
		keyList = kInvalidPGPKeyListRef;
		if (partialFailure) {
			*outKeysThatFailed = errorKeySet;
			errorKeySet = kInvalidPGPKeySetRef;
			if (failedPolicy) {
				ThrowPGPError_(kPGPError_ServerKeyFailedPolicy);
			} else if (authorizationFailed) {
				ThrowPGPError_(kPGPError_ServerAuthorizationFailed);
			} else if (keyAlreadyExists) {
				ThrowPGPError_(kPGPError_ServerKeyAlreadyExists);
			} else {
				ThrowPGPError_(kPGPError_ServerPartialAddFailure);
			}
		} else {
			PGPFreeKeySet(errorKeySet);
		}
	}
	
	catch (...) {
		delete[] keySpace;
		if (exportedKey != 0) {
			PGPFreeData(exportedKey);
		}
		if (singleKeySet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(singleKeySet);
		}
		if (keyIter != kInvalidPGPKeyIterRef) {
			PGPFreeKeyIter(keyIter);
		}
		if (keyList != kInvalidPGPKeyListRef) {
			PGPFreeKeyList(keyList);
		}
		if (errorKeySet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(errorKeySet);
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			SetErrorString(mLDAP->ld_error);
			throw;
		}
	}
}



	void
CLDAPKeyServer::Delete(
	PGPKeySetRef	inKeysToDelete,
	PGPKeySetRef *	outKeysThatFailed)
{
	DeleteOrDisable(true, inKeysToDelete, outKeysThatFailed);
}



	void
CLDAPKeyServer::Disable(
	PGPKeySetRef	inKeysToDisable,
	PGPKeySetRef *	outKeysThatFailed)
{
	DeleteOrDisable(false, inKeysToDisable, outKeysThatFailed);
}



	void
CLDAPKeyServer::DeleteOrDisable(
	PGPBoolean		inDelete,
	PGPKeySetRef	inKeySet,
	PGPKeySetRef *	outKeysThatFailed)
{
	StPreserveSocketsEventHandler	preserve(this);
	const char *action = 0;
	PGPError	pgpError = kPGPError_NoErr;

	SetErrorString(0);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}

	try {
		PGPKeyServerState	state;
		
		if (mAccessType != kPGPKeyServerAccessType_Administrator) {
			ThrowPGPError_(kPGPError_ServerAuthorizationRequired);
		}
		if (inDelete) {
			action = kDelete;
			state = kPGPKeyServerState_Deleting;
		} else {
			action = kDisable;
			state = kPGPKeyServerState_Disabling;
		}
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this, 
										state);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		pgpError = SendRequest(	action,
								inKeySet,
								outKeysThatFailed);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
	}
	
	catch (...) {
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			SetErrorString(mLDAP->ld_error);
			throw;
		}
	}
}



	PGPError
CLDAPKeyServer::SendRequest(
	const char *	inAction,
	PGPKeySetRef	inKeySetRef,
	PGPKeySetRef *	outKeysThatFailed)
{
	PGPError		result = kPGPError_NoErr;
	char *			request = 0;
	char *			signedRequest = 0;
	PGPKeyListRef	keyList = kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyIter = kInvalidPGPKeyIterRef;
	char *			keySpace = 0;
	char *			error = 0;
	PGPKeySetRef	singleKeySet = kInvalidPGPKeySetRef;
	
	try {
		PGPUInt32			requestSize = 0;
		PGPKeyID			keyID;
		char				keyIDString[kPGPMaxKeyIDStringSize];
		const char *		location = 0;
		PGPUInt32			total;
		PGPError			pgpError = kPGPError_NoErr;
		PGPOptionListRef	options;
		PGPKeyRef			key;
		PGPSize				dummy;
		char *				strVals[2] = { 0, 0 };
		LDAPMod				mod = { 0, (char *) kRequestPGPRequest,
											{strVals}, 0 };
		LDAPMod *			attrs[2] = { &mod, 0 };
		int					ldapResult = 0;
		PGPUInt32			numAlgorithms;
		char *				nextItem = 0;
		PGPUInt32 			i = 0;
		PGPPublicKeyAlgorithmInfo	algorithmInfo;
		
		if( mKeySpace == kPGPKeyServerKeySpace_Pending )
		{
			location = kRequestPending;
		}
		else
		{
			location = kRequestActive;
		}
		
		// Allocate request buffer and add prefix
		pgpError = PGPCountKeys(inKeySetRef, &total);
		ThrowIfPGPError_(pgpError);
		request = new char[strlen(kRequestRequest) + strlen(inAction)
							+ strlen(kRequestLocation) + strlen(location)
							+ ((kPGPMaxKeyIDStringSize + 2) * total) + 7];
		if (request == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		requestSize = sprintf(	request,
								"%s%s\r\n%s%s\r\n\r\n",
								kRequestRequest,
								inAction,
								kRequestLocation,
								location);
		
		// Iterate through keys creating the request
		pgpError = PGPOrderKeySet(inKeySetRef, kPGPAnyOrdering, &keyList);
		ThrowIfPGPError_(pgpError);
		pgpError = PGPNewKeyIter(keyList, &keyIter);
		ThrowIfPGPError_(pgpError);
		while ((pgpError = PGPKeyIterNext(keyIter, &key)) == kPGPError_NoErr) {
			pgpError = PGPGetKeyIDFromKey(key, &keyID);
			ThrowIfPGPError_(pgpError);
			pgpError = PGPGetKeyIDString(	&keyID,
											kPGPKeyIDString_Full,
											keyIDString);
			ThrowIfPGPError_(pgpError);
			requestSize += sprintf(	request + requestSize,
									"%s\r\n",
									keyIDString);
		}
		PGPFreeKeyIter(keyIter);
		keyIter = kInvalidPGPKeyIterRef;
		PGPFreeKeyList(keyList);
		keyList = kInvalidPGPKeyListRef;
		if (pgpError != kPGPError_EndOfIteration) {
			ThrowIfPGPError_(pgpError);
		}
		
		// Now sign the request if we do not have a secure connection
		if (! mSecured) {
			do {
				pgpError = pgpEventKeyServerSign(	mContext,
													&options,
													mEventHandler,
													mEventHandlerData,
													(PGPKeyServerRef) this);
				ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
				pgpError = PGPEncode(	mContext,
										PGPOInputBuffer(mContext,
													request,
													requestSize),
										PGPOAllocatedOutputBuffer(
													mContext,
													(void**)&signedRequest,
													MAX_PGPSize,
													&dummy),
										options,
										PGPOLastOption(mContext));
			} while (pgpError == kPGPError_BadPassphrase);
			ThrowIfPGPError_(pgpError);
			delete[] request;
			request = 0;
		}
		
		// Send the request
		mod.mod_values[0] = (mSecured) ? request : signedRequest;
		keySpace = new char[strlen(kAddAndRequestDNPrefix)
							+ strlen(mBaseKeySpaceDN) + 1];
		if (keySpace == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		sprintf(keySpace, "%s%s", kAddAndRequestDNPrefix, mBaseKeySpaceDN);
		ldapResult = ldap_add_s(	mLDAP,
									keySpace,
									attrs);
		ThrowIfLDAPCanceled_();
		delete[] keySpace;
		keySpace = 0;
		if (mSecured) {
			delete[] request;
			request = 0;
		} else {
			PGPFreeData(signedRequest);
			signedRequest = 0;
		}
	
		// Scan for error keys
		if ((outKeysThatFailed != 0) && (mLDAP->ld_error != 0)
		&& (*mLDAP->ld_error != '\0')) {
			char *	currentItem = 0;
		
			error = new char[strlen(mLDAP->ld_error) + 1];
			if (error == 0) {
				ThrowPGPError_(kPGPError_OutOfMemory);
			}
			strcpy(error, mLDAP->ld_error);
			currentItem = strchr(error, ':');
			if (currentItem != 0) {
				currentItem++;
				while ((*currentItem != '\0') && isspace(*currentItem)) {
					currentItem++;
				}
				while (*currentItem != '\0') {
					nextItem = currentItem;
					while ((*nextItem != '\0') && (! isspace(*nextItem))) {
						nextItem++;
					}
					if (*nextItem != '\0') {
						*nextItem++ = '\0';
					}
					pgpError = PGPGetKeyIDFromString(currentItem, &keyID);
					ThrowIfPGPError_(pgpError);
					pgpError = PGPCountPublicKeyAlgorithms(&numAlgorithms);
					ThrowIfPGPError_(pgpError);
					for (i = 0; i < numAlgorithms; i++) {
						pgpError = PGPGetIndexedPublicKeyAlgorithmInfo(	i,
													&algorithmInfo);
						ThrowIfPGPError_(pgpError);
						pgpError = PGPGetKeyByKeyID(
										inKeySetRef,
													&keyID,
													algorithmInfo.algID,
													&key);
						if (IsntPGPError(pgpError)
						&& (key != kInvalidPGPKeyRef)) {
							break;
						}
					}
					if (*outKeysThatFailed == kInvalidPGPKeySetRef) {
						pgpError = PGPNewKeySet(mContext, outKeysThatFailed);
						ThrowIfPGPError_(pgpError);
					}
					pgpError = PGPNewSingletonKeySet(key, &singleKeySet);
					ThrowIfPGPError_(pgpError);
					pgpError = PGPAddKeys(singleKeySet, *outKeysThatFailed);
					ThrowIfPGPError_(pgpError);
					PGPFreeKeySet(singleKeySet);
					singleKeySet = kInvalidPGPKeySetRef;
					currentItem = nextItem;
					while ((*currentItem != '\0') && isspace(*currentItem)) {
						currentItem++;
					}
				}
			}
			delete[] error;
			error = 0;
		}
			
		switch (ldapResult) {
			case LDAP_SUCCESS:
			{
			}
			break;
			
			
			case LDAP_ALREADY_EXISTS:
			{
				ThrowPGPError_(kPGPError_ServerKeyAlreadyExists);
			}
			break;
			
			
			case LDAP_INSUFFICIENT_ACCESS:
			{
				ThrowPGPError_(kPGPError_ServerAuthorizationFailed);
			}
			break;
			
			
			case LDAP_INVALID_CREDENTIALS:
			{
				ThrowPGPError_(kPGPError_ServerKeyFailedPolicy);
			}
			break;
			
			
			default:
			{
				ThrowPGPError_(kPGPError_ServerRequestFailed);
			}
			break;
		}
	}
	
	catch (PGPError exception) {
		delete[] error;
		delete[] request;
		delete[] keySpace;
		if (singleKeySet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(singleKeySet);
		}
		if (signedRequest != 0) {
			PGPFreeData(signedRequest);
		}
		if (keyIter != kInvalidPGPKeyIterRef) {
			PGPFreeKeyIter(keyIter);
		}
		if (keyList != kInvalidPGPKeyListRef) {
			PGPFreeKeyList(keyList);
		}
		result = exception;
	}
	
	return result;
}



	void
CLDAPKeyServer::SendGroups(
	PGPGroupSetRef	inGroupSetRef)
{
	StPreserveSocketsEventHandler	preserve(this);
	void *							groupsBuffer = 0;
	PGPError						pgpError = kPGPError_NoErr;
	int								ldapResult = 0;
	
	SetErrorString(0);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}
	try {
		struct berval		berVal = { 0, 0 };
		struct berval *		berVals[2] = { &berVal, 0 };
		const char *		strVals[2] = { kPGPSrvGroupFileClass, 0 };
		LDAPMod				mod = { LDAP_MOD_BVALUES,
									(char *) kPGPSrvGroupFile,
									{ (char **) berVals }, 0 };
		LDAPMod *			attrs[2] = { &mod, 0 };
		PGPSize				groupsBufferSize;
		
		
		// Export the groups
		pgpError = PGPExportGroupSetToBuffer(	inGroupSetRef,
												&groupsBuffer,
												&groupsBufferSize);
		ThrowIfPGPError_(pgpError);
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Uploading);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		
		// Send them to the server
		berVals[0]->bv_len = groupsBufferSize;
		berVals[0]->bv_val = (char *) groupsBuffer;
		ldapResult = ldap_add_s(	mLDAP,
									(char *) kDefaultPGPGroupspace,
									attrs);
		ThrowIfLDAPCanceled_();
		if (ldapResult != LDAP_SUCCESS) {
			switch (ldapResult) {
				case LDAP_ALREADY_EXISTS:
				{
					ThrowPGPError_(kPGPError_ServerKeyAlreadyExists);
				}
				break;
				
				
				case LDAP_INSUFFICIENT_ACCESS:
				{
					ThrowPGPError_(kPGPError_ServerAuthorizationFailed);
				}
				break;
				
				
				case LDAP_INVALID_CREDENTIALS:
				{
					ThrowPGPError_(kPGPError_ServerKeyFailedPolicy);
				}
				break;
				
				
				default:
				{
					ThrowPGPError_(kPGPError_ServerRequestFailed);
				}
				break;
			}
		}
		PGPFreeData(groupsBuffer);
		groupsBuffer = 0;
		
		// Add the objectclass
		mod.mod_op = 0;
		mod.mod_type = (char *) kObjectClass;
		mod.mod_values = (char **) strVals;
		ldapResult = ldap_add_s(	mLDAP,
									(char *) kDefaultPGPGroupspace,
									attrs);
		ThrowIfLDAPCanceled_();
		switch (ldapResult) {
			case LDAP_SUCCESS:
			{
			}
			break;
			
			
			case LDAP_ALREADY_EXISTS:
			{
				ThrowPGPError_(kPGPError_ServerKeyAlreadyExists);
			}
			break;
			
			
			case LDAP_INSUFFICIENT_ACCESS:
			{
				ThrowPGPError_(kPGPError_ServerAuthorizationFailed);
			}
			break;
			
			
			case LDAP_INVALID_CREDENTIALS:
			{
				ThrowPGPError_(kPGPError_ServerKeyFailedPolicy);
			}
			break;
			
			
			default:
			{
				ThrowPGPError_(kPGPError_ServerRequestFailed);
			}
			break;
		}
	}
	
	catch (...) {
		if (groupsBuffer != 0) {
			PGPFreeData(groupsBuffer);
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			SetErrorString(mLDAP->ld_error);
			throw;
		}
	}
}



	void
CLDAPKeyServer::RetrieveGroups(
	PGPGroupSetRef *	outGroupSetRef)
{
	StPreserveSocketsEventHandler	preserve(this);
	LDAPMessage *					message = 0;
	struct berval **				value = 0;
	PGPGroupSetRef					groupSetRef = kInvalidPGPGroupSetRef;
	PGPError						pgpError = kPGPError_NoErr;
	int								ldapResult = 0;

	SetErrorString(0);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}
	try {
		LDAPMessage *		nextMessage;
		char *				attribute;
		BerElement *		iterAttribute;
		static const char *	attr[2] = { kPGPSrvGroupFile, 0 };
		
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Querying);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		
		ldapResult = ldap_search_s(	mLDAP,
									(char *) kDefaultPGPGroupspace,
									LDAP_SCOPE_BASE,
									(char *) kObjectClassAny,
									(char **) attr,
									0,
									&message);
		ThrowIfLDAPCanceled_();
		if (ldapResult != LDAP_SUCCESS) {
			ThrowPGPError_(kPGPError_ServerSearchFailed);
		}
		nextMessage = ldap_first_entry(mLDAP, message);
		if (nextMessage != 0) {
			attribute = ldap_first_attribute(	mLDAP,
												nextMessage,
												&iterAttribute);
			if (attribute != 0) {
				value = ldap_get_values_len(	mLDAP,
												nextMessage,
												attribute);
				if ((value != 0) && (*value != 0)) {
					pgpError = PGPImportGroupSetFromBuffer(	mContext,
															(*value)->bv_val,
															(*value)->bv_len,
															&groupSetRef);
					ldap_value_free_len(value);
					value = 0;
				}
			}
pgpFixBeforeShip(am getting errors here)
/*
			if (mLDAP->ld_errno != LDAP_SUCCESS) {
				ThrowPGPError_(kPGPError_ServerOpenFailed);
			}
*/		}
		if (groupSetRef == kInvalidPGPGroupSetRef) {
			ThrowPGPError_(kPGPError_ServerSearchFailed);
		}
		ldap_msgfree(message);
		*outGroupSetRef = groupSetRef;
	}
	
	catch (...) {
		if (groupSetRef != kInvalidPGPGroupSetRef) {
			PGPFreeGroupSet(groupSetRef);
		}
		if (value != 0) {
			ldap_value_free_len(value);
		}
		if (message != 0) {
			ldap_msgfree(message);
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			SetErrorString(mLDAP->ld_error);
			throw;
		}
	}
}



	void
CLDAPKeyServer::NewMonitor(
	PGPKeyServerMonitor **	outMonitor)
{
	StPreserveSocketsEventHandler	preserve(this);
	LDAPMessage *					message = 0;
	char **							value = 0;
	PGPKeyServerMonitor *			monitor = 0;
	PGPError						pgpError = kPGPError_NoErr;
	int								ldapResult = 0;
	PGPKeyServerMonitorValues *		currentMonitorValue = 0;
	LDAPMessage *					nextMessage = 0;
	char *							attribute = 0;
	int								numValues = 0;
	int 							i = 0;

	SetErrorString(0);
	if (! mIsOpen) {
		ThrowPGPError_(kPGPError_ServerNotOpen);
	}
	try {
		BerElement *				iterAttribute;
		static const char *			attr[1] = { 0 };

		if (mAccessType != kPGPKeyServerAccessType_Administrator) {
			ThrowPGPError_(kPGPError_ServerAuthorizationRequired);
		}
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_Querying);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		ldapResult = ldap_search_s(	mLDAP,
									(char *) kMonitorCN,
									LDAP_SCOPE_BASE,
									(char *) kObjectClassAny,
									(char **) attr,
									0,
									&message);
		ThrowIfLDAPCanceled_();
		if (ldapResult != LDAP_SUCCESS) {
			ThrowPGPError_(kPGPError_ServerSearchFailed);
		}
		
		pgpError = pgpEventKeyServer(	mContext,
										mEventHandler,
										mEventHandlerData,
										(PGPKeyServerRef) this,
										kPGPKeyServerState_ProcessingResults);
		ThrowIfPGPErrorOrLDAPCanceled_(pgpError);
		monitor = new PGPKeyServerMonitor;
		if (monitor == 0) {
			ThrowPGPError_(kPGPError_OutOfMemory);
		}
		monitor->keyServerRef = (PGPKeyServerRef) this;
		monitor->valuesHead = 0;
		nextMessage = ldap_first_entry(mLDAP, message);
		while (nextMessage != 0) {
			attribute = ldap_first_attribute(	mLDAP,
												nextMessage,
												&iterAttribute);
			while (attribute != 0) {
				if (currentMonitorValue == 0) {
					monitor->valuesHead = currentMonitorValue = 
											new PGPKeyServerMonitorValues;
				} else {
					currentMonitorValue->next =
											new PGPKeyServerMonitorValues;
					currentMonitorValue = currentMonitorValue->next;
				}
				if (currentMonitorValue == 0) {
					ThrowPGPError_(kPGPError_OutOfMemory);
				}
				currentMonitorValue->values = 0;
				currentMonitorValue->next = 0;
				currentMonitorValue->name = new char[strlen(attribute) + 1];
				if (currentMonitorValue->name == 0) {
					ThrowPGPError_(kPGPError_OutOfMemory);
				}
				/* copy the attribute */
				strncpy( currentMonitorValue->name, attribute,
						strlen(attribute) + 1);
				
				value = ldap_get_values(	mLDAP,
											nextMessage,
											attribute);
				if (value != 0) {
					numValues = ldap_count_values(value);
					currentMonitorValue->values = new char*[numValues + 1];
					if (currentMonitorValue->values == 0) {
						ThrowPGPError_(kPGPError_OutOfMemory);
					}
					for (i = 0; i < numValues; i++) {
						currentMonitorValue->values[i] =
											new char[strlen(value[i]) + 1];
						if (currentMonitorValue->values[i] == 0) {
							ThrowPGPError_(kPGPError_OutOfMemory);
						}
						strcpy(currentMonitorValue->values[i], value[i]);
					}
					currentMonitorValue->values[numValues] = 0;
					ldap_value_free(value);
					value = 0;
				}
				attribute = ldap_next_attribute(	mLDAP,	
													nextMessage,
													iterAttribute);
			}
pgpFixBeforeShip(am getting errors here)
/*
			if (mLDAP->ld_errno != LDAP_SUCCESS) {
				ThrowPGPError_(kPGPError_ServerOpenFailed);
			}
*/			nextMessage = ldap_next_entry(	mLDAP,
											nextMessage);
		}
		ldap_msgfree(message);
		*outMonitor = monitor;
	}
	
	catch (...) {
		if (monitor != 0) {
			FreeMonitor(monitor);
		}
		if (value != 0) {
			ldap_value_free(value);
		}
		if (message != 0) {
			ldap_msgfree(message);
		}
		if (mCanceled) {
			ThrowPGPError_(kPGPError_UserAbort);
		} else {
			SetErrorString(mLDAP->ld_error);
			throw;
		}
	}
}



	void
CLDAPKeyServer::FreeMonitor(
	PGPKeyServerMonitor *	inMonitor)
{
	PGPKeyServerMonitorValues *	currentMonitorValue = inMonitor->valuesHead;
	PGPKeyServerMonitorValues * tempMonitorValue;
	char **						tempValue;
	
	while (currentMonitorValue != 0) {
		delete[] currentMonitorValue->name;
		if (currentMonitorValue->values != 0) {
			tempValue = currentMonitorValue->values;
			while (*tempValue != 0) {
				delete[] *tempValue;
				tempValue++;
			}
			delete[] currentMonitorValue->values;
		}
		tempMonitorValue = currentMonitorValue;
		currentMonitorValue = currentMonitorValue->next;
		delete tempMonitorValue;
	}
	delete[] inMonitor;
}
