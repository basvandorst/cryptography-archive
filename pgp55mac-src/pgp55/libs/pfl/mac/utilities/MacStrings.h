/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacStrings.h,v 1.15 1997/09/18 01:34:56 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include "pgpBase.h"
#include "pgpTypes.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#ifdef __cplusplus
	inline unsigned char
StrSize(ConstStr255Param string) 
{
	return ( 1UL + string[ 0 ] );
}
#else
#define StrSize(string) ( 1UL + string[ 0 ] )
#endif

void	NumVersionToString(const NumVersion *version, StringPtr versionString);
void	VersionToString(UInt16 version, StringPtr versionString);

void	CopyPString(ConstStr255Param srcStr, StringPtr destStr);
void	CopyPStringMax(ConstStr255Param srcStr, StringPtr destStr,
			UInt32 maxBytesToCopy);
void	AppendPString(ConstStr255Param	appendStr, StringPtr destStr);
void	AppendPStringMax(ConstStr255Param	appendStr, StringPtr destStr,
			UInt32 maxBytesToAppend);
void	InsertInPString(const uchar *source, uchar offset, uchar *dest);
void	DeleteFromPString(uchar *string, uchar offset, uchar count);
void	PrintPString(uchar *dest, const uchar *formatStr,
			const uchar *stringToPut);
Boolean	PStringsAreEqual(const uchar *str1, const uchar *str2);

Boolean	PStringHasSuffix( ConstStr255Param str,
			ConstStr255Param suffix, Boolean caseSensitive );
	
void	CToPString(const char *cString, StringPtr pString);
void	PToCString(ConstStr255Param pString, char * cString);

void	GetIndCString(char theString[256], ResID stringListResID,
					short stringListIndex);
	
const char *	FindSubStringNoCase( const char * mainString,
					const char * subString);



/* Use this if you don't want to link in ANSI */
char *	CopyCString( const char *	src, char *	 dest );

OSErr	AppendStringToHandle( Handle theHandle, ConstStr255Param	string );


/* str1, str2 and str3 may be null */
void	InsertPStrings( ConstStr255Param baseString,
			ConstStr255Param str1, ConstStr255Param str2,
			ConstStr255Param str3, StringPtr resultString );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS




