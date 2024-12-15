/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
**
*/

#pragma once

typedef enum FindAppSearchType
{
	kFindAppOnLocalVolumes	= 1,
	kFindAppOnRemoteVolumes,
	kFindAppOnAllVolumes
	
} FindAppSearchType;


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


Boolean		FindApplicationOnVolume(short vRefNum, OSType applicationType,
						OSType applicationCreator, FSSpec *appSpec);
Boolean		FindApplication(OSType applicationType, OSType applicationCreator,
						FindAppSearchType searchType, FSSpec *appSpec);
						
OSStatus	BringAppToFront(OSType type, OSType creator,
						FindAppSearchType searchType, Boolean launch,
						AppParametersPtr params, Boolean * wasLaunched);


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

