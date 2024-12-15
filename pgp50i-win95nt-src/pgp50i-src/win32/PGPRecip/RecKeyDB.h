/*
 * RecKeyDB.h  Routines needed to fetch and match keys
 *
 * Most of the calls to PGPKeyDB lib are isolated here. The keys are
 * read in from disk, and matches are attempted.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

BOOL AddUsersToLists(PUSERKEYINFO *HeadUserLL,
                     DWORD NumRecipients, char* RecipientList,
                     BOOL EncSelfOn);
BOOL AddLinkedListtoListView(HWND hwndList,PUSERKEYINFO Head);
void KMConvertStringKeyID (LPSTR sz);
PUSERKEYINFO AddLink(PUSERKEYINFO head,PUSERKEYINFO pui);
int UserIsARecipient(PUSERKEYINFO pui,int *MultipleFound,
                     char* RecipientList, DWORD NumRecipients,
                     PUSERKEYINFO *RecipientCheck,
					 PUSERKEYINFO *HeadUserLL);
