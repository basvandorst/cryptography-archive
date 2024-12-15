/*____________________________________________________________________________	Copyright (C) 1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: PGPDiskFile.h,v 1.7 1999/03/15 17:43:44 heller Exp $____________________________________________________________________________*/#pragma once#include "CComboError.h"#include "pgpBase.h"#include "PGPDiskFileFormat.h"#include "pgpKeys.h"typedef struct PGPDiskFile				*PGPDiskFileRef;typedef struct PGPDiskKey				*PGPDiskKeyRef;#define	kInvalidPGPDiskFileRef			((PGPDiskFileRef) NULL)#define PGPDiskFileRefIsValid( ref )	( (ref) != kInvalidPGPDiskFileRef )#define	kInvalidPGPDiskKeyRef			((PGPDiskKeyRef) NULL)#define PGPDiskKeyRefIsValid( ref )		( (ref) != kInvalidPGPDiskKeyRef )enum PGPDiskKeyProperty_{	/* Boolean key properties */	kPGPDiskKeyProperty_MountReadOnly	= 1,	kPGPDiskKeyProperty_CanRemove		= 2,	kPGPDiskKeyProperty_IsMasterKey		= 3,	PGP_ENUM_FORCE( PGPDiskKeyProperty_ )};PGPENUM_TYPEDEF( PGPDiskKeyProperty_, PGPDiskKeyProperty );OSStatus	AddPassphrasePGPDiskKey(PGPDiskFileRef diskFileRef,					ConstStr255Param masterPassphrase,					ConstStr255Param keyPassphrase, Boolean readOnly);OSStatus	ChangePGPDiskKeyPassphrase(PGPDiskKeyRef keyRef,					ConstStr255Param oldPassphrase,					ConstStr255Param newPassphrase);void		ClosePGPDiskFile(PGPDiskFileRef diskFileRef);CComboError	DecryptPGPDiskKey(PGPDiskKeyRef keyRef,					ConstStr255Param passphrase, CASTKey *decryptedKey);void		DisposePGPDiskKey(PGPDiskKeyRef keyRef);OSStatus	FindPassphrasePGPDiskKey(PGPDiskFileRef diskFileRef,					ConstStr255Param passphrase, PGPDiskKeyRef *keyRefPtr);CComboError	FindPGPDiskKey(PGPDiskFileRef diskFileRef,					ConstStr255Param passphrase, PGPDiskKeyRef *keyRefPtr);OSStatus	GetIndPassphrasePGPDiskKey(PGPDiskFileRef diskFileRef,					UInt32 index, PGPDiskKeyRef *keyRefPtr);OSStatus	GetIndPGPDiskKey(PGPDiskFileRef diskFileRef,					UInt32 index, PGPDiskKeyRef *keyRefPtr);OSStatus	GetMasterPGPDiskKey(PGPDiskFileRef diskFileRef,					PGPDiskKeyRef *keyRefPtr);OSStatus	GetPGPDiskKeyBoolean(PGPDiskKeyRef keyRef,					PGPDiskKeyProperty property, Boolean *value);OSStatus	OpenPGPDiskFile(PGPContextRef context, const FSSpec *fileSpec,					PGPDiskFileRef *diskFileRefPtr);OSStatus	RemovePGPDiskKey(PGPDiskKeyRef keyRef);OSStatus	SavePGPDiskFile(PGPDiskFileRef diskFileRef);OSStatus	NewPGPDiskFile(PGPContextRef context, const FSSpec *fileSpec,					PGPUInt32 diskSizeInK,					PGPDiskEncryptionAlgorithm algorithm,					ConstStr255Param masterPassphrase);OSStatus	VerifyPGPDiskKeyPassphrase(PGPDiskKeyRef keyRef,					ConstStr255Param passphrase);const PGPDiskFileHeader *PeekPGPDiskFileHeader(PGPDiskFileRef diskFileRef);CComboError	AddPublicPGPDiskKey(PGPDiskFileRef diskFileRef, PGPKeyRef theKey,					ConstStr255Param masterPassphrase, Boolean readOnly,					Boolean locked);OSStatus	GetIndPublicPGPDiskKey(PGPDiskFileRef diskFileRef,					UInt32 index, PGPDiskKeyRef *keyRefPtr);UInt32		GetPublicPGPDiskKeyCount(PGPDiskFileRef diskFileRef);CComboError	GetPublicPGPDiskKeyInfo(PGPDiskKeyRef keyRef, PGPKeyID *keyID,					PGPPublicKeyAlgorithm *algorithm, PGPBoolean *locked);