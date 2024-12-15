/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pflPrefTypes.c,v 1.16 1997/09/07 00:28:44 lloyd Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pflPrefTypes.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"
#include "pgpDebug.h"
#include "pgpMem.h"

static PGPError pgpUInt32ToByte(PGPUInt32 uintValue, 
								PGPByte *byteArray);

static PGPError pgpByteToUInt32(const PGPByte *byteArray,
								PGPUInt32 *uintValue);


PGPError PGPGetPrefBoolean(PGPPrefRef prefRef,
						   PGPPrefIndex prefIndex,
						   PGPBoolean *data)
{
	void *			tempData;
	PGPSize			dataLength;
	PGPError		err;

	PGPValidatePtr(data);

	err = PGPGetPrefData(prefRef, prefIndex, &dataLength, &tempData);
	if (IsPGPError(err))
		return err;

	pgpAssert(dataLength == sizeof(PGPBoolean));
	if (dataLength != sizeof(PGPBoolean))
	{
		PGPDisposePrefData(prefRef, tempData);
		return kPGPError_BadParams;
	}

	*data = *(PGPBoolean *)tempData;
	PGPDisposePrefData(prefRef, tempData);

	return err;
}


PGPError PGPSetPrefBoolean(PGPPrefRef prefRef,
						   PGPPrefIndex prefIndex,
						   PGPBoolean data)
{
	PGPError err;

	err = PGPSetPrefData(prefRef, prefIndex, sizeof(PGPBoolean), &data);

	return err;
}


PGPError PGPGetPrefNumber(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  PGPUInt32 *data)
{
	void *		tempData;
	PGPSize		dataLength;
	PGPError	err;

	if (IsntNull(data))
		*data = 0;

	PGPValidatePtr(data);

	err = PGPGetPrefData(prefRef, prefIndex, &dataLength, &tempData);
	if (IsPGPError(err))
		return err;

	pgpAssert(dataLength == sizeof(PGPUInt32));
	if (dataLength != sizeof(PGPUInt32))
	{
		PGPDisposePrefData(prefRef, tempData);
		return kPGPError_BadParams;
	}

	pgpByteToUInt32( (PGPByte *)tempData, data);
	PGPDisposePrefData(prefRef, tempData);

	return err;
}


PGPError PGPSetPrefNumber(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  PGPUInt32 data)
{
	PGPError	err;
	PGPByte 	tempData[4];

	pgpUInt32ToByte(data, tempData);
	err = PGPSetPrefData(prefRef, prefIndex, sizeof(PGPUInt32), tempData);
	return err;
}


PGPError PGPGetPrefStringAlloc(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   char **string)
{
	PGPError	err;
	void *		data			= NULL;
	PGPSize		stringLength	= 0;

	if (IsntNull(string))
		*string = NULL;

	PGPValidatePtr(string);

	err = PGPGetPrefData(prefRef, prefIndex, &stringLength, &data);
	if (IsntPGPError(err))
	{
		*string	= (char *)data;
	}
	
	return err;
}


PGPError PGPGetPrefStringBuffer(PGPPrefRef prefRef,
								PGPPrefIndex prefIndex,
								PGPSize maxSize,
								char *string)
{
	PGPError	err;
	void *		tempString;
	PGPSize		tempLength;

	if (IsntNull(string) && (maxSize > 0))
		string[0] = '\0';

	PGPValidatePtr(string);
	PGPValidateParam(maxSize > 0);

	err = PGPGetPrefData(prefRef, prefIndex, &tempLength, &tempString);
	if (IsPGPError(err))
		return err;

	if (tempLength > maxSize)
	{
		tempLength = maxSize;
		string[tempLength-1] = 0;
	}

	pgpCopyMemory(tempString, string, tempLength);
	PGPDisposePrefData(prefRef, tempString);
	
	return err;
}


PGPError PGPSetPrefString(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  const char *string)
{
	PGPError	err;
	PGPSize		stringLength;

	PGPValidatePtr(string);
	
	stringLength = strlen(string)+1;	/* Include null terminator */

	err = PGPSetPrefData(prefRef, prefIndex, stringLength, string);
	return err;
}


static PGPError pgpUInt32ToByte(PGPUInt32 uintValue, 
								PGPByte *byteArray)
{
	PGPValidatePtr(byteArray);

	byteArray[0] = (PGPByte)(uintValue >> 24);
	byteArray[1] = (PGPByte)(uintValue >> 16);
	byteArray[2] = (PGPByte)(uintValue >> 8);
	byteArray[3] = (PGPByte)(uintValue);

	return kPGPError_NoErr;
}


static PGPError pgpByteToUInt32(const PGPByte *byteArray,
								PGPUInt32 *uintValue)
{
	PGPValidatePtr(byteArray);
	PGPValidatePtr(uintValue);

	*uintValue =	(((PGPUInt32)byteArray[0]) << 24) |
		  			(((PGPUInt32)byteArray[1]) << 16) |
					(((PGPUInt32)byteArray[2]) <<  8) |
					(((PGPUInt32)byteArray[3]));
	
	return kPGPError_NoErr;
}
