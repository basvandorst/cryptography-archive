/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpRecipientDialogCommon.h,v 1.15.10.1 1998/11/12 03:23:03 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpRecipientDialogCommon_h	/* [ */
#define Included_pgpRecipientDialogCommon_h

#include "pgpMemoryMgr.h"
#include "pgpPubTypes.h"
#include "pgpUserInterface.h"

#define kPGPNumKeyListKeys		20
#define kPGPNumUserListUsers	30

typedef enum PGPRecipientUserKind
{
	kPGPRecipientUserKind_Invalid = 0,
	kPGPRecipientUserKind_Key,
	kPGPRecipientUserKind_MissingRecipient,
	kPGPRecipientUserKind_Group
	
} PGPRecipientUserKind;

typedef enum PGPRecipientUserLocation
{
	kPGPRecipientUserLocation_Invalid = 0,
	kPGPRecipientUserLocation_UserList,
	kPGPRecipientUserLocation_RecipientList,
	kPGPRecipientUserLocation_Hidden
	
} PGPRecipientUserLocation;

typedef enum PGPRecipientEventType
{
	kPGPRecipientEvent_InvalidEvent	= 0,
	kPGPRecipientEvent_MovedUserEvent,
	kPGPRecipientEvent_MoveUserWarningEvent,
	kPGPRecipientEvent_MoveUserFailedEvent,
	kPGPRecipientEvent_MovedARRWarningEvent
	
} PGPRecipientEventType;

typedef struct PGPRecipientEvent
{
	PGPRecipientEventType		type;
	struct PGPRecipientUser		*user;

} PGPRecipientEvent;

typedef PGPError	(*PGPRecipientEventHandlerProcPtr)(
							struct PGPRecipientsList *recipients,
							PGPRecipientEvent *event, PGPUserValue userValue );

typedef struct PGPRecipientUser
{
	struct PGPRecipientUser		*nextUser;
	struct PGPRecipientsList	*recipients;
	
	PGPRecipientUserKind		kind;
	PGPRecipientUserLocation	location;
	PGPValidity					validity;
	PGPUInt32					nameOffset;
	PGPUInt32					markValue;			/* Temp use only */
	PGPUInt16					lockRefCount;		/* Cannot be moved  */
	
	PGPBoolean					movedManually	: 1;
	PGPBoolean					multipleMatch	: 1;
	
	union
	{
		struct
		{
			struct PGPRecipientKey	*key;
			PGPUserIDRef			userID;
			PGPUInt16				arrRefCount;
			PGPUInt16				enforcedARRRefCount;
			
			PGPBoolean				isPrimaryUser	: 1;
		
		} userInfo;
		
		struct
		{
			PGPGroupID				groupID;
			PGPUInt32				numKeys;
			PGPUInt32				numMissingKeys;
			PGPUInt16				numARRKeys;
			PGPBoolean				haveMissingARRs;
			
		} groupInfo;
		
		struct
		{
			PGPRecipientSpecType	type;
			
			const PGPKeyID			*keyID;
			PGPPublicKeyAlgorithm	algorithm;
			
		} missingUser;
	};

} PGPRecipientUser;

typedef struct PGPRecipientKeyARRInfo
{
	PGPKeyID				keyID;
	struct PGPRecipientKey	*key;
	PGPBoolean				keyMissing;
	PGPByte					arrClass;

} PGPRecipientKeyARRInfo;

typedef struct PGPRecipientKey
{
	struct PGPRecipientKey			*nextKey;
	struct PGPRecipientUser			*users;
	struct PGPRecipientUser			*primaryUser;

	PGPUInt16						numARRKeys;
	struct PGPRecipientKeyARRInfo	*arrKeys;
	
	PGPKeyRef						keyRef;
	PGPUInt16						keyBits;
	PGPUInt16						subKeyBits;
	PGPPublicKeyAlgorithm			algorithm;
	PGPUInt32						markValue;			/* Temp use only */
	
	PGPBoolean						isDefaultKey	: 1;
	PGPBoolean						isSecretKey		: 1;
	PGPBoolean						isAxiomaticKey	: 1;
	PGPBoolean						canEncrypt		: 1;
	PGPBoolean						isNewKey		: 1;
	PGPBoolean						isVisible		: 1;
	PGPBoolean						haveMissingARRs	: 1;

} PGPRecipientKey;

typedef struct PGPRecipientUserList
{
	struct PGPRecipientUserList	*nextUserList;
	struct PGPRecipientUser		users[kPGPNumUserListUsers];

} PGPRecipientUserList;

typedef struct PGPRecipientKeyList
{
	struct PGPRecipientKeyList	*nextKeyList;
	struct PGPRecipientKey		keys[kPGPNumKeyListKeys];

} PGPRecipientKeyList;

typedef struct PGPRecipientsList
{
	PGPContextRef 			context;
	PGPMemoryMgrRef			memoryMgr;
	PGPKeySetRef			keySet;
	
	struct PGPRecipientKey	*keys;
	struct PGPRecipientUser	*missingRecipients;
	struct PGPRecipientUser	*groups;

	struct PGPRecipientKey	*freeKeys;
	struct PGPRecipientUser	*freeUsers;

	char					*nameList;
	PGPUInt32				nameListSize;
	PGPUInt32				nextNameOffset;
	
	PGPGroupSetRef			groupSet;
	const PGPKeyServerSpec	*serverList;
	PGPUInt32 				serverCount;
	PGPtlsContextRef		tlsContext;
	
	PGPRecipientKeyList		*keyLists;
	PGPRecipientUserList	*userLists;
	
	PGPRecipientEventHandlerProcPtr				eventHandler;
	PGPUserValue								eventUserValue;
	
	PGPAdditionalRecipientRequestEnforcement	arrEnforcement;
	
} PGPRecipientsList;

PGP_BEGIN_C_DECLARATIONS

PGPError		PGPBuildRecipientsList(void *hwndParent,PGPContextRef context,
						PGPKeySetRef allKeys, PGPGroupSetRef groupSet,
						PGPUInt32 numDefaultRecipients,
						const PGPRecipientSpec *defaultRecipients,
						PGPUInt32 serverCount,
						const PGPKeyServerSpec *serverList,
						PGPtlsContextRef tlsContext,
						PGPBoolean syncUnknownKeys,
						PGPAdditionalRecipientRequestEnforcement arrEnforcement,
						PGPRecipientEventHandlerProcPtr eventHandler,
						PGPUserValue eventUserValue,
						PGPRecipientsList *recipients,
						PGPBoolean *haveDefaultARRs);
void			PGPDisposeRecipientsList(PGPRecipientsList *recipients);
PGPError		PGPGetRecipientKeys(PGPRecipientsList *recipients,
						PGPKeySetRef *keySetPtr, PGPKeySetRef *newKeysPtr,
						PGPUInt32 *keyListCountPtr,
						PGPRecipientSpec **keyListPtr);
void			PGPGetRecipientUserName(const PGPRecipientUser *user, 
						char name[256]);
const char *	PGPGetRecipientUserNamePtr(const PGPRecipientUser *user);
PGPError		PGPMoveRecipients(PGPRecipientsList *recipients,
						PGPRecipientUserLocation destinationList,
						PGPUInt32 numUsers, PGPRecipientUser **userList,
						PGPUInt32 *numMovedUsers, PGPBoolean *movedARRs);
PGPError		PGPUpdateMissingRecipients(void *hwndParent,
						PGPRecipientsList *recipients,
						PGPBoolean *haveNewKeys);
						
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpRecipientDialogCommon_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
