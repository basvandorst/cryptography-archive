/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpX509Utils.c,v 1.5 1999/03/11 07:16:40 jason Exp $
____________________________________________________________________________*/

#include "pgpKeys.h"
#include "pgpMem.h"
#include "pgpUtilities.h"
#include "pgpClientLib.h"
#include "pgpEndianConversion.h"

#include <ctype.h>
#include <string.h>

#define CKERR	if( IsPGPError( err ) ) goto done	

	PGPError
PGPX509CertFromExport(
	PGPContextRef	context,
	PGPUInt32		uX509Alg,
	PGPByte*		expkeyidX509,
	PGPByte*		pIASNX509,
	PGPUInt32		uX509IASNLength,
	PGPKeySetRef	keyset,
	PGPKeyRef*		pkeyX509,
	PGPSigRef*		psigX509 )
{
	PGPKeyListRef	keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef	keyiter			= kInvalidPGPKeyIterRef;
	PGPKeySetRef	keysetX509		= kInvalidPGPKeySetRef;
	PGPKeyRef		key				= kInvalidPGPKeyRef;
	PGPUserIDRef	userid			= kInvalidPGPUserIDRef;
	PGPSigRef		sig				= kInvalidPGPSigRef;
	PGPError		err				= kPGPError_NoErr;
	PGPByte*		pIASN			= NULL;

	PGPKeyID		keyid;
	PGPSize			size;

	PGPValidatePtr (pkeyX509);
	PGPValidatePtr (psigX509);

	*pkeyX509 = kInvalidPGPKeyRef;
	*psigX509 = kInvalidPGPSigRef;

	// get X.509 key 
	if ((pIASNX509 != NULL) &&
		(uX509Alg != kPGPPublicKeyAlgorithm_Invalid))
	{
		err = PGPImportKeyID (expkeyidX509, &keyid); CKERR;
		err = PGPGetKeyByKeyID (
				keyset, &keyid, (PGPPublicKeyAlgorithm)uX509Alg,
				pkeyX509); CKERR;
	}

	// get X.509 sig
	if (PGPKeyRefIsValid (*pkeyX509))
	{
		err = PGPNewSingletonKeySet (*pkeyX509, &keysetX509); CKERR;
		err = PGPOrderKeySet (keysetX509, kPGPAnyOrdering, &keylist); CKERR;
		err = PGPNewKeyIter (keylist, &keyiter); CKERR;

		err = PGPKeyIterNext (keyiter, &key); CKERR;
		err = PGPKeyIterNextUserID (keyiter, &userid); CKERR;
		while (userid) 
		{
			PGPKeyIterNextUIDSig (keyiter, &sig);

			while (	(sig != kInvalidPGPSigRef) &&
					(*psigX509 == kInvalidPGPSigRef))
			{
				size = 0;
				PGPGetSigPropertyBuffer (sig,
							kPGPSigPropX509IASN, 0, NULL, &size);
				if (size == uX509IASNLength)
				{
					pIASN = (PGPByte *)PGPNewData (
								PGPGetContextMemoryMgr (context),
								size, kPGPMemoryMgrFlags_Clear);
					if (pIASN)
					{
						err = PGPGetSigPropertyBuffer (sig, 
							kPGPSigPropX509IASN, size, pIASN, &size); CKERR;

						if (pgpMemoryEqual (pIASN, pIASNX509, size))
						{
							*psigX509 = sig;
						}

						PGPFreeData (pIASN);
						pIASN = NULL;
					}
				}
				PGPKeyIterNextUIDSig (keyiter, &sig);
			}
			PGPKeyIterNextUserID (keyiter, &userid);
		}
	}

done :
	if (pIASN != NULL)
		PGPFreeData (pIASN);
	if (PGPKeySetRefIsValid (keysetX509))
		PGPFreeKeySet (keysetX509);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);

	return err;
}

	PGPError
PGPX509CertToExport(
	PGPContextRef	context,
	PGPKeyRef		keyCert,
	PGPSigRef		sigCert,
	PGPUInt32*		puX509Alg,
	PGPByte*		pexpkeyidX509,
	PGPByte**		ppIASNX509,
	PGPUInt32*		puX509IASNLength )
{
	PGPError		err		= kPGPError_NoErr;
	PGPSize			size;
	PGPKeyID		keyid;

	PGPValidatePtr (puX509Alg);
	PGPValidatePtr (pexpkeyidX509);
	PGPValidatePtr (ppIASNX509);
	PGPValidatePtr (puX509IASNLength);

	*puX509Alg = kPGPPublicKeyAlgorithm_Invalid;
	*ppIASNX509 = NULL;
	*puX509IASNLength = 0;

	if (PGPKeyRefIsValid (keyCert))
	{
		err = PGPGetKeyNumber (keyCert, 
						kPGPKeyPropAlgID,
						(PGPInt32 *)puX509Alg); CKERR;

		err = PGPGetKeyIDFromKey (keyCert, &keyid); CKERR;
		err = PGPExportKeyID (&keyid, pexpkeyidX509, &size); CKERR;

		PGPGetSigPropertyBuffer (sigCert, 
					kPGPSigPropX509IASN, 0, NULL,
					(PGPSize *)puX509IASNLength);
		if (*puX509IASNLength > 0)
		{
			*ppIASNX509 = (PGPByte *)PGPNewData (
						PGPGetContextMemoryMgr (context),
						*puX509IASNLength,
						kPGPMemoryMgrFlags_Clear );

			if (*ppIASNX509)
			{
				PGPGetSigPropertyBuffer (sigCert, 
						kPGPSigPropX509IASN, *puX509IASNLength, 
						*ppIASNX509, &size);
			}
		}
	}

done:
	return err;
}


enum PGPAVAttributeType_
{
	kPGPAVAttributeType_FirstType = 0,
	kPGPAVAttributeType_Invalid	= kPGPAVAttributeType_FirstType,
	kPGPAVAttributeType_Boolean,
	kPGPAVAttributeType_Long,
	kPGPAVAttributeType_Pointer,

	PGP_ENUM_FORCE( PGPAVAttributeType_ )
};
PGPENUM_TYPEDEF( PGPAVAttributeType_, PGPAVAttributeType );


	static PGPAVAttributeType
sGetAttributeType(PGPAVAttribute attribute)
{
	switch (attribute)
	{
	case kPGPAVAttribute_CommonName:
	case kPGPAVAttribute_Email:
	case kPGPAVAttribute_OrganizationName:
	case kPGPAVAttribute_OrganizationalUnitName:
	case kPGPAVAttribute_SurName:
	case kPGPAVAttribute_SerialNumber:
	case kPGPAVAttribute_Country:
	case kPGPAVAttribute_Locality:
	case kPGPAVAttribute_State:
	case kPGPAVAttribute_StreetAddress:
	case kPGPAVAttribute_Title:
	case kPGPAVAttribute_Description:
	case kPGPAVAttribute_PostalCode:
	case kPGPAVAttribute_POBOX:
	case kPGPAVAttribute_PhysicalDeliveryOfficeName:
	case kPGPAVAttribute_TelephoneNumber:
	case kPGPAVAttribute_X121Address:
	case kPGPAVAttribute_ISDN:
	case kPGPAVAttribute_DestinationIndicator:
	case kPGPAVAttribute_Name:
	case kPGPAVAttribute_GivenName:
	case kPGPAVAttribute_Initials:
	case kPGPAVAttribute_HouseIdentifier:
	case kPGPAVAttribute_DirectoryManagementDomain:
	case kPGPAVAttribute_DomainComponent:
	case kPGPAVAttribute_UnstructuredName:
	case kPGPAVAttribute_UnstructuredAddress:
	case kPGPAVAttribute_RFC822Name:
	case kPGPAVAttribute_DNSName:
	case kPGPAVAttribute_AnotherName:
	case kPGPAVAttribute_IPAddress:
	case kPGPAVAttribute_CertificateExtension:
	case kPGPAVAttribute_Challenge:
	case kPGPAVAttribute_CertType:
    case kPGPAVAttribute_MailFirstName:
    case kPGPAVAttribute_MailMiddleName:
    case kPGPAVAttribute_MailLastName:
    case kPGPAVAttribute_EmployeeID:
    case kPGPAVAttribute_MailStop:
    case kPGPAVAttribute_AdditionalField4:
    case kPGPAVAttribute_AdditionalField5:
    case kPGPAVAttribute_AdditionalField6:
    case kPGPAVAttribute_Authenticate:
		return kPGPAVAttributeType_Pointer;

    case kPGPAVAttribute_EmbedEmail:
		return kPGPAVAttributeType_Boolean;

	default:
		pgpDebugMsg("Unknown AV attribute");
		return kPGPAVAttributeType_Invalid;
	}
}


/* Caller must free data with PGPFreeData */

	PGPError
PGPAVPairsToData(
	PGPMemoryMgrRef		memoryMgr, 
	PGPAttributeValue*	avPairs, 
	PGPUInt32			numAV, 
	PGPByte**			data,
	PGPSize*			dataSize)
{
	PGPUInt32 avIndex;
	PGPUInt32 dataIndex;
	PGPByte *newData;
	PGPError err = kPGPError_NoErr;

	if (IsntNull(data))
		*data = NULL;
	if (IsntNull(dataSize))
		*dataSize = 0;

	PGPValidateMemoryMgr(memoryMgr);
	PGPValidatePtr(avPairs);
	PGPValidatePtr(data);
	PGPValidatePtr(dataSize);
	PGPValidateParam(numAV > 0);

	*dataSize = (numAV * sizeof(PGPUInt32) * 2) + sizeof(PGPUInt32);

	for (avIndex=0; avIndex<numAV; avIndex++)
		*dataSize += avPairs[avIndex].size;

	newData = (PGPByte *) PGPNewData(memoryMgr, *dataSize, 
							kPGPMemoryMgrFlags_Clear);

	if (IsNull(newData))
	{
		err = kPGPError_OutOfMemory;
		*dataSize = 0;
	}

	if (IsntPGPError(err))
	{
		PGPUInt32ToStorage(numAV, newData);
		dataIndex = sizeof(PGPUInt32);

		for (avIndex=0; avIndex<numAV; avIndex++)
		{
			PGPUInt32ToStorage(avPairs[avIndex].attribute, 
				&(newData[dataIndex]));

			dataIndex += sizeof(PGPUInt32);

			PGPUInt32ToStorage(avPairs[avIndex].size, 
				&(newData[dataIndex]));

			dataIndex += sizeof(PGPUInt32);

			switch (sGetAttributeType(avPairs[avIndex].attribute))
			{
			case kPGPAVAttributeType_Boolean:
				newData[dataIndex] = avPairs[avIndex].value.booleanvalue;
				break;

			case kPGPAVAttributeType_Long:
				PGPUInt32ToStorage(avPairs[avIndex].value.longvalue,
					&(newData[dataIndex]));
				break;

			case kPGPAVAttributeType_Pointer:
				pgpCopyMemory(avPairs[avIndex].value.pointervalue,
					&(newData[dataIndex]), avPairs[avIndex].size);
				break;

			default:
				PGPFreeData(newData);
				newData = NULL;
				*dataSize = 0;
				err = kPGPError_ImproperInitialization;
			}

			dataIndex += avPairs[avIndex].size;
		}
	}

	*data = newData;
	return err;
}


/* Caller must supply function to free data allocated for the avPairs
   array */

	PGPError 
PGPDataToAVPairs(
	PGPMemoryMgrRef		memoryMgr, 
	PGPByte*			data,
	PGPSize				dataSize, 
	PGPAttributeValue**	avPairs, 
	PGPUInt32*			numAV)
{
	PGPUInt32 avIndex;
	PGPUInt32 dataIndex;
	PGPUInt32 freeIndex;
	PGPAttributeValue *newAV;
	PGPError err = kPGPError_NoErr;

	if (IsntNull(avPairs))
		*avPairs = NULL;
	if (IsntNull(numAV))
		*numAV = 0;

	PGPValidateMemoryMgr(memoryMgr);
	PGPValidatePtr(data);
	PGPValidatePtr(avPairs);
	PGPValidatePtr(numAV);
	PGPValidateParam(dataSize >= sizeof(PGPUInt32));

	*numAV = PGPStorageToUInt32(data);
	if (*numAV > 0)
	{
		newAV = (PGPAttributeValue *) PGPNewData(memoryMgr, 
										sizeof(PGPAttributeValue) * (*numAV),
										kPGPMemoryMgrFlags_Clear);

		if (IsNull(newAV))
		{
			err = kPGPError_OutOfMemory;
			*numAV = 0;
		}

		if (IsntPGPError(err))
		{
			dataIndex = sizeof(PGPUInt32);

			for (avIndex=0; avIndex<(*numAV); avIndex++)
			{
				newAV[avIndex].attribute = (PGPAVAttribute)
					PGPStorageToUInt32(&(data[dataIndex]));

				dataIndex += sizeof(PGPUInt32);

				newAV[avIndex].size = 
					PGPStorageToUInt32(&(data[dataIndex]));

				dataIndex += sizeof(PGPUInt32);
				
				switch (sGetAttributeType(newAV[avIndex].attribute))
				{
				case kPGPAVAttributeType_Boolean:
					newAV[avIndex].value.booleanvalue = data[dataIndex];
					break;
					
				case kPGPAVAttributeType_Long:
					newAV[avIndex].value.longvalue = 
						PGPStorageToUInt32(&(data[dataIndex]));
					break;
					
				case kPGPAVAttributeType_Pointer:
					newAV[avIndex].value.pointervalue = 
						PGPNewData(memoryMgr, newAV[avIndex].size,
							kPGPMemoryMgrFlags_Clear);

					if (IsNull(newAV[avIndex].value.pointervalue))
						err = kPGPError_OutOfMemory;
					else
						pgpCopyMemory(&(data[dataIndex]), 
							newAV[avIndex].value.pointervalue,
							newAV[avIndex].size);
					break;
					
				default:
					err = kPGPError_CorruptData;
				}
				
				if (IsPGPError(err))
				{
					for (freeIndex=0; freeIndex<avIndex; freeIndex++)
						PGPFreeData(newAV[freeIndex].value.pointervalue);

					PGPFreeData(newAV);
					avIndex = *numAV;
					newAV = NULL;
					*numAV = 0;
				}
				else
					dataIndex += newAV[avIndex].size;

				if (IsntPGPError(err) && (dataIndex > dataSize))
				{
					for (freeIndex=0; freeIndex<avIndex; freeIndex++)
						PGPFreeData(newAV[freeIndex].value.pointervalue);

					PGPFreeData(newAV);
					avIndex = *numAV;
					newAV = NULL;
					*numAV = 0;

					err = kPGPError_CorruptData;
				}
			}
		}

		*avPairs = newAV;
	}

	return err;
}


	PGPError 
PGPParseCommonName(
	PGPMemoryMgrRef	memoryMgr, 
	char*			commonname, 
	char**			pfirstname,
	char**			pmiddlename,
	char**			plastname)
{
	char*		pFirstStart		= NULL;
	char*		pFirstEnd		= NULL;
	char*		pMiddleStart	= NULL;
	char*		pMiddleEnd		= NULL;
	char*		pLastStart		= NULL;
	char*		pLastEnd		= NULL;
	PGPSize		size;

	PGPValidateMemoryMgr(memoryMgr);
	PGPValidatePtr(commonname);
	PGPValidatePtr(pfirstname);
	PGPValidatePtr(pmiddlename);
	PGPValidatePtr(plastname);

	// initialize 
	*pfirstname = NULL;
	*pmiddlename = NULL;
	*plastname = NULL;

	// find the "first" name
	pFirstStart = commonname;
	while (*pFirstStart && isspace(*pFirstStart))
		++pFirstStart;

	if (*pFirstStart)
	{
		pFirstEnd = pFirstStart;
		while (*pFirstEnd && !isspace(*pFirstEnd))
			++pFirstEnd;

		size = pFirstEnd - pFirstStart;
		if (size > 0)
		{
			*pfirstname = (char *) PGPNewData(memoryMgr, size+1,
									kPGPMemoryMgrFlags_Clear);
			strncpy(*pfirstname, pFirstStart, size);
		}
	}
	else
		return kPGPError_NoErr;


	// find the "last" name
	pLastEnd = &(commonname[strlen(commonname)]);
	while ((pLastEnd > commonname) && isspace(*pLastEnd))
		--pLastEnd;

	if (pLastEnd > commonname)
	{
		pLastStart = pLastEnd;
		while ((pLastStart > commonname) && !isspace(*pLastStart))
			--pLastStart;

		if (pLastStart >= pFirstEnd)
		{
			size = pLastEnd - pLastStart;
			*plastname = (char *) PGPNewData(memoryMgr, size+1,
									kPGPMemoryMgrFlags_Clear);
			strncpy(*plastname, pLastStart+1, size);
		}
	}

	// middle name
	if (pLastStart > pFirstEnd)
	{
		pMiddleStart = pFirstEnd;
		while ((pMiddleStart < pLastStart) && isspace(*pMiddleStart))
			++pMiddleStart;

		if (pMiddleStart < pLastStart)
		{
			pMiddleEnd = pLastStart;
			while ((pMiddleEnd >= pMiddleStart) && isspace(*pMiddleEnd))
				--pMiddleEnd;

			if (pMiddleEnd >= pMiddleStart)
			{
				size = pMiddleEnd - pMiddleStart +1;
				*pmiddlename = (char *) PGPNewData(memoryMgr, size+1,
											kPGPMemoryMgrFlags_Clear);
				strncpy(*pmiddlename, pMiddleStart, size);
			}
		}
	}
	
	return kPGPError_NoErr;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
