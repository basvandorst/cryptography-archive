/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDkeyDB.h,v 1.4.10.1 1998/11/12 03:24:25 heller Exp $
____________________________________________________________________________*/
#ifndef Included_RDKEYDB_h	/* [ */
#define Included_RDKEYDB_h

#define ADDUSER_OK 0
#define ADDUSER_NORECIPIENTSTOSEARCH 1
#define ADDUSER_ADKSINCLUDED 2

// anything keysnotvalid and over gives warning message
#define ADDUSER_KEYSNOTVALID 4
#define ADDUSER_KEYSNOTFOUND 8
#define ADDUSER_MULTIPLEMATCH 16
#define ADDUSER_KEYSNOTCORPSIGNED 32
#define ADDUSER_ADKMISSING 64

	PGPError
BuildTable(
	HWND						hwndTable,
	PGPGroupSetRef				mGroupSet,
	PUSERKEYINFO				*UserLinkedList,
	PGPRecipientsList			*mRecipients,
	PGPRecipientUserLocation 	location);

void BuildTables(PRECGBL prg);

#endif /* ] Included_RDKEYDB_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
