/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pflPrefTypes.c,v 1.19 1999/03/10 02:46:49 heller Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pflPrefTypes.h"
#include "pgpPFLErrors.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpEndianConversion.h"

#define kPGPPrefsEndian		kPGPBigEndian


PGPError PGPGetPrefBoolean(PGPPrefRef prefRef,
						   PGPPrefIndex prefIndex,
						   PGPBoolean *data)
{
	void *			tempData;
	PGPSize			dataLength;
	PGPError		err;

	if (IsntNull(data))
		*data = FALSE;
		
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

	*data = PGPEndianToUInt32(kPGPPrefsEndian, (PGPByte *)tempData);
	PGPDisposePrefData(prefRef, tempData);

	return err;
}


PGPError PGPSetPrefNumber(PGPPrefRef prefRef,
						  PGPPrefIndex prefIndex,
						  PGPUInt32 data)
{
	PGPError	err;
	PGPByte 	tempData[sizeof(PGPUInt32)];

	PGPUInt32ToEndian(data, kPGPPrefsEndian, tempData);
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


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


