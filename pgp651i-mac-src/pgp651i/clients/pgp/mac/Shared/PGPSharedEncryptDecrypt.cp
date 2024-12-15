/*_____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved._____________________________________________________________________________*/#include <stdio.h>#include <string.h>#include "MacFiles.h"#include "pgpMacMemory.h"#include "MacStrings.h"#include "MacProcesses.h"#include "MacErrors.h"#include "pgpEncode.h"#include "pgpErrors.h"#include "pgpKeys.h"#include "pgpUtilities.h"#include "CComboError.h"#include "PGPSharedEncryptDecrypt.h"namespace {	const OSType	kPGPkeysType				=	'APPL';	const OSType	kPGPkeysCreator				=	'pgpK';	const DescType	kAEPGPClass					=	'PGP ';	const DescType	kAEImportKey				=	'IKey';}	PGPErrorPGPSharedForEachKeyDo(	PGPContextRef					context,	PGPKeyOrdering 					keyOrdering,	PGPSharedForEackKeyDoProcPtr	callbackProc,	void							*callbackData){	PGPError		err = kPGPError_NoErr;	PGPKeySetRef	keySetRef;			err = PGPOpenDefaultKeyRings( context, 0, &keySetRef );	if( IsntPGPError( err ) )	{		PGPKeyListRef	keyListRef;				err = PGPOrderKeySet( keySetRef, keyOrdering, &keyListRef );		if( IsntPGPError( err ) )		{			PGPKeyIterRef	keyIterator;						err = PGPNewKeyIter( keyListRef, &keyIterator );			if( IsntPGPError( err ) )			{				PGPKeyRef	theKey;								err = PGPKeyIterNext( keyIterator, &theKey );				while( IsntErr( err ) )				{					err = (*callbackProc)( keyIterator, theKey,								callbackData );					if( IsPGPError( err ) )						break;											err = PGPKeyIterNext( keyIterator, &theKey );				}								if( err == kPGPError_EndOfIteration )					err = kPGPError_NoErr;									PGPFreeKeyIter( keyIterator );			}						PGPFreeKeyList( keyListRef );		}			PGPFreeKeySet( keySetRef );	}		if( err == kPGPError_UserAbort )		err = kPGPError_NoErr;	return( err );}typedef struct KeyCounts{	UInt32		publicKeys;	UInt32		privateKeys;} KeyCounts;	static PGPErrorCountKeysInDefaultKeyringCallbackProc(	PGPKeyIterRef	keyIterator,	PGPKeyRef 		theKey,	void 			*userData){	PGPBoolean	isPrivateKey = FALSE;		#pragma unused( keyIterator )		++((KeyCounts *) userData)->publicKeys;		PGPGetKeyBoolean( theKey, kPGPKeyPropIsSecret, &isPrivateKey );	if( isPrivateKey )	{		++((KeyCounts *) userData)->privateKeys;	}		return( kPGPError_NoErr );}	PGPErrorCountKeysInDefaultKeyring(	PGPContextRef	context,	PGPUInt32 		*numPublicKeys,	PGPUInt32 		*numPrivateKeys){	PGPError	err;	KeyCounts	keyCounts;		AssertAddrNullOrValid( numPublicKeys, UInt32, "CountKeysInDefaultKeyring" );	AssertAddrNullOrValid( numPrivateKeys, UInt32, "CountKeysInDefaultKeyring");		keyCounts.publicKeys 	= 0;	keyCounts.privateKeys	= 0;		err = PGPSharedForEachKeyDo( context, kPGPAnyOrdering,				CountKeysInDefaultKeyringCallbackProc, &keyCounts );	if( IsntNull( numPublicKeys ) )		*numPublicKeys = keyCounts.publicKeys;	if( IsntNull( numPrivateKeys ) )		*numPrivateKeys = keyCounts.privateKeys;			return( err );}	static PGPErrorGetKeySimpleProperties(	PGPKeyRef				theKey,	PGPSharedKeyProperties	*properties){	PGPError	err;		pgpAssertAddrValid( theKey, VoidAlign );	pgpAssertAddrValid( properties, PGPSharedKeyProperties );		pgpClearMemory( properties, sizeof( *properties ) );		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsSecret,				&properties->isSecret );		if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsAxiomatic,					&properties->isAxiomatic );	}	if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsSecretShared,					&properties->isSplit );	}	if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsRevoked,					&properties->isRevoked );	}	if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsDisabled,					&properties->isDisabled );	}		if( IsntErr( err ) )	{		PGPBoolean	isNotCorrupt;				err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsNotCorrupt,					&isNotCorrupt );		properties->isCorrupt = ! isNotCorrupt;	}	if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsExpired,					&properties->isExpired );	}	if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropCanSign,					&properties->canSign );	}	if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropCanEncrypt,					&properties->canEncrypt );	}	if( IsntErr( err ) )	{		err = PGPGetKeyBoolean( theKey, kPGPKeyPropCanEncrypt,					&properties->canEncrypt );	}	return( err );}	PGPErrorPGPSharedGetKeyProperties(	PGPKeyRef				theKey,	const char				*userID,	PGPSharedKeyProperties	*properties){	PGPError	err;		AssertAddrNullOrValid( userID, char, "PGPSharedGetKeyProperties" );	pgpAssertAddrValid( properties, PGPSharedKeyProperties );		err = GetKeySimpleProperties( theKey, properties );	if( IsntPGPError( err ) )	{		if( IsntNull( userID ) )		{			PGPKeySetRef	keySet;						properties->userValidity = kPGPValidity_Unknown;									err = PGPNewSingletonKeySet( theKey, &keySet );			if( IsntPGPError( err ) )			{				PGPKeyListRef	keyList;								err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyList );				if( IsntPGPError( err ) )				{					PGPKeyIterRef	iterator;										err = PGPNewKeyIter( keyList, &iterator );					if( IsntPGPError( err ) )					{						PGPUserIDRef	curUserID;						Boolean			foundUserID = FALSE;						PGPKeyRef		keyRef;												/* must advance to first key */						err	= PGPKeyIterNext( iterator, &keyRef );						pgpAssertNoErr( err );												while( IsntPGPError(							err = PGPKeyIterNextUserID(iterator, &curUserID)))						{							char		checkUserID[256];							PGPSize		totalSize;														err = PGPGetUserIDStringBuffer( curUserID,										kPGPUserIDPropName,										sizeof( checkUserID ),										checkUserID, &totalSize );							if( IsntPGPError( err ) )							{								if( strcmp( checkUserID, userID ) == 0 )								{									PGPInt32	validity;																		err = PGPGetUserIDNumber( curUserID,											kPGPUserIDPropValidity,											&validity );																		properties->userValidity =										(PGPValidity) validity;																		foundUserID = TRUE;									break;								}							}						}												if( err == kPGPError_EndOfIteration )							err = kPGPError_NoErr;													(void) PGPFreeKeyIter( iterator );					}										(void) PGPFreeKeyList( keyList );				}								(void) PGPFreeKeySet( keySet );			}		}		else		{			err = PGPGetPrimaryUserIDValidity( theKey,						&properties->userValidity );		}	}	return( err );}	PGPErrorPGPSharedAddKeysToDefaultKeyring(	PGPKeySetRef keysToAdd){	PGPError			result;	CComboError			comboErr;	ProcessSerialNumber	keysPSN;	PGPContextRef		context = PGPGetKeySetContext(keysToAdd);		if ( FindProcess(kPGPkeysType, kPGPkeysCreator,	&keysPSN, nil, nil, nil) )	{		AEDesc		theAddress = { typeNull, nil };		AEDesc		theKeys = { typeNull, nil };		AppleEvent	theEvent = { typeNull, nil };		void *		buffer = nil;		PGPSize		bufferSize;			comboErr.err = ::AECreateDesc(	typeProcessSerialNumber,										(Ptr) &keysPSN,										sizeof(keysPSN),										&theAddress);		if (comboErr.IsntError())		{			comboErr.pgpErr = PGPExportKeySet(	keysToAdd,												PGPOAllocatedOutputBuffer(													context,													&buffer,													::MaxBlock(),													&bufferSize),												PGPOLastOption(context));			if (comboErr.IsntError())			{				comboErr.err = ::AECreateDesc(	typeChar,												buffer,												bufferSize,												&theKeys);				if (comboErr.IsntError())				{					comboErr.err = ::AECreateAppleEvent(kAEPGPClass,														kAEImportKey,														&theAddress,														kAutoGenerateReturnID,														kAnyTransactionID,														&theEvent);					if (comboErr.IsntError())					{						comboErr.err = ::AEPutParamDesc(	&theEvent,															keyDirectObject,															&theKeys);						if (comboErr.IsntError())						{							AEDesc	theReply;														comboErr.err = ::AESend(&theEvent,													&theReply,													kAENoReply,													kAENormalPriority,													kAEDefaultTimeout,													nil,													nil);						}					}				}			}		}				::AEDisposeDesc(&theAddress);		::AEDisposeDesc(&theKeys);		::AEDisposeDesc(&theEvent);		if (buffer != nil)		{			PGPFreeData(buffer);		}	}	else	{		PGPKeySetRef	defaultKeySet = kInvalidPGPKeySetRef;				comboErr.pgpErr = PGPOpenDefaultKeyRings(context,								kPGPKeyRingOpenFlags_Mutable,								&defaultKeySet);		if (comboErr.IsntError())		{			comboErr.pgpErr = PGPAddKeys(keysToAdd, defaultKeySet);			if (comboErr.IsntError())			{				comboErr.pgpErr = PGPCommitKeyRingChanges(defaultKeySet);			}		}		if( PGPKeySetRefIsValid( defaultKeySet ) )		{			PGPFreeKeySet(defaultKeySet);		}	}		if (comboErr.HavePGPError())	{		result = comboErr.pgpErr;	}	else	{		result = MacErrorToPGPError(comboErr.err);	}		return result;}	PGPErrorPGPSharedGetIndKeyFromKeySet(	PGPKeySetRef 	keySet,	PGPKeyOrdering 	keyOrdering,	PGPUInt32		searchIndex,	PGPKeyRef		*theKey){	PGPError		err;	PGPKeyListRef	keyListRef;		*theKey = kInvalidPGPKeyRef;	err = PGPOrderKeySet( keySet, keyOrdering, &keyListRef );	if( IsntPGPError( err ) )	{		PGPKeyIterRef	keyIterator;				err = PGPNewKeyIter( keyListRef, &keyIterator );		if( IsntPGPError( err ) )		{			PGPUInt32	keyIndex;						for( keyIndex = 1; keyIndex <= searchIndex; ++keyIndex )			{				err = PGPKeyIterNext( keyIterator, theKey );				if( IsPGPError( err ) )				{					*theKey = kInvalidPGPKeyRef;					break;				}			}			PGPFreeKeyIter( keyIterator );		}				PGPFreeKeyList( keyListRef );	}		if( err == kPGPError_EndOfIteration )		err = kPGPError_ItemNotFound;			return( err );}