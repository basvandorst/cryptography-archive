/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDkeyDB.h,v 1.12 1997/10/16 15:08:49 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RDKEYDB_h	/* [ */
#define Included_RDKEYDB_h

/*
 * RecKeyDB.h  Routines needed to fetch and match keys
 *
 * Most of the calls to PGPKeyDB lib are isolated here. The keys are
 * read in from disk, and matches are attempted.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

#define ADDUSER_OK 0
#define ADDUSER_NORECIPIENTSTOSEARCH 1
#define ADDUSER_ADKSINCLUDED 2

// anything keysnotvalid and over gives warning message
#define ADDUSER_KEYSNOTVALID 4
#define ADDUSER_KEYSNOTFOUND 8
#define ADDUSER_MULTIPLEMATCH 16
#define ADDUSER_KEYSNOTCORPSIGNED 32
#define ADDUSER_ADKMISSING 64

UINT AddUsersToLists(HWND hwndLoad,PUSERKEYINFO *HeadUserLL,
					 PRECIPIENTDIALOGSTRUCT prds,
					 PPREFS Prefs,
					 UINT *ReturnValue);
BOOL AddLinkedListtoListView(HWND hwndList,PUSERKEYINFO Head);

PGPError RDGroupItemToKey(
	PGPGroupItem const *	item,
	PGPKeySetRef			keySet,
	PGPKeyRef *				outKey );
PUSERKEYINFO KeyIsEncryptable(PUSERKEYINFO headpui,PGPKeyRef key);
int MoveSelectedKeysAndADKs(HWND hdlg,
							PUSERKEYINFO headpui,
							BOOL RecToUser);

#endif /* ] Included_RDKEYDB_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
