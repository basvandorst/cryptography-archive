/*____________________________________________________________________________
    groups.c

    Copyright 1998,1999 by Network Associates Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    Functions for maintenance and key checking.

    $Id: groups.c,v 1.11 1999/05/12 21:01:03 sluu Exp $
____________________________________________________________________________*/

#include <string.h>
#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpSDKPrefs.h"
#include "pgpUtilities.h"

#include "language.h"
#include "globals.h"
#include "prototypes.h"

/*
   General purpose functions for working with groups.
 */

/*
   Search for the specified group by Name.
 */
PGPError pgpGetGroupByName( PGPGroupSetRef groupSet, char *groupName,
        PGPGroupID *group )
{
    PGPSize i,ngroups;
    PGPGroupID groupID;
    PGPGroupInfo info;
    PGPError err;

    err = PGPCountGroupsInSet( groupSet, &ngroups );
    pgpAssertNoErr(err);

    for( i=0; i< ngroups; i++ ) {
        err = PGPGetIndGroupID( groupSet, i, &groupID );
        pgpAssertNoErr(err);
        err = PGPGetGroupInfo( groupSet, groupID, &info );
        pgpAssertNoErr(err);

        if( strncmp( groupName, info.name, kPGPMaxGroupNameLength ) == 0 )
        {
            *group = info.id;
            return kPGPError_NoErr;
        }
    }

    return kPGPError_ItemNotFound;
}

/* Item type: Group Returns: True
              Key            False
*/
PGPBoolean isGroup(const PGPGroupItem *item)
{
    if (item->type == kPGPGroupItem_Group) return TRUE;
    else return FALSE;
}

PGPError getGroupIDFromItem(const PGPGroupItem *item, PGPGroupID *groupID)
{
    if (!isGroup(item)) return -1;

    *groupID = item->u.group.id;
    return kPGPError_NoErr;
}

PGPError getKeyIDFromItem(const PGPGroupItem *item, PGPKeyID *keyID)
{
    if (isGroup(item)) return -1;

    *keyID = item->u.key.keyID;
    return kPGPError_NoErr;
}

/*
   Functions to initialize and finalize the working group set.
 */

/*
   Get the default group set. Try not to fail.
 */
PGPError pgpInitializeWorkingGroupSet( struct pgpmainBones *mainbPtr )
{
    PGPContextRef context = mainbPtr->pgpContext;
    PGPFileSpecRef groupSpec = NULL;
    PGPError err;

    err = PGPsdkPrefGetFileSpec( context, kPGPsdkPref_GroupsFile,
            &groupSpec );

    if( IsntPGPError(err) )
        err = PGPNewGroupSetFromFile( context, groupSpec,
                &mainbPtr->workingGroupSet );

    if( IsPGPError(err) )
        err = PGPNewGroupSet( context, &mainbPtr->workingGroupSet );

    if(groupSpec)
        PGPFreeFileSpec(groupSpec);

    return err;
}

PGPError pgpFinalizeWorkingGroupSet( struct pgpmainBones *mainbPtr )
{
    PGPContextRef context = mainbPtr->pgpContext;
    PGPError err = kPGPError_NoErr;

    if( mainbPtr->workingGroupSet ) {
        if( PGPGroupSetNeedsCommit( mainbPtr->workingGroupSet ) ) {
            PGPFileSpecRef groupSpec = NULL;

            err = PGPsdkPrefGetFileSpec( context, kPGPsdkPref_GroupsFile,
                    &groupSpec );
            pgpAssertNoErr(err);

            err = PGPSaveGroupSetToFile( mainbPtr->workingGroupSet,
                    groupSpec );
            pgpAssertNoErr(err);

            if(groupSpec)
                PGPFreeFileSpec(groupSpec);
        }
        PGPFreeGroupSet( mainbPtr->workingGroupSet );
        mainbPtr->workingGroupSet=NULL;
    }
    return err;
}

/* modify these to return proper 'status' error-codes.*/


/*
   Functions to display nested group structure, and keys in those groups.
   For pgp -gv[v]
*/
const PGPUInt32 nameLength = 20; /* a reasonable name length, not the max. */

PGPError showGroupInfo(struct pgpfileBones *filebPtr, PGPGroupSetRef
        groupSet, PGPGroupID groupID, PGPUInt32 depth)
{
    PGPGroupInfo info;
    PGPError err;

    err = PGPGetGroupInfo( groupSet, groupID, &info );
    pgpAssertNoErr(err);

    fprintf( filebPtr->pgpout, LANG("%*s> "), (2 * depth), " ");
    fprintf( filebPtr->pgpout, "%-*s", nameLength-2*depth, info.name );
    if(strlen(info.description)>0)
        fprintf( filebPtr->pgpout, " \"%s\"\n", info.description );
    else
        fprintf( filebPtr->pgpout, "\n");

    return kPGPError_NoErr;
}


/* For -gvv, display a key. */
PGPError showKeyGroupFormat(struct pgpfileBones *filebPtr, PGPKeySetRef
        keySet, PGPGroupSetRef groupSet, PGPKeyID keyID, PGPUInt32 depth,
        PGPBoolean vOption)
{
    char useridstr[ kPGPMaxUserIDSize ];
    PGPKeyRef key;
    char kstr[kPGPMaxKeyIDStringSize];

    PGPError err;

    err = PGPGetKeyByKeyID( keySet, &keyID,
            kPGPPublicKeyAlgorithm_Invalid, &key);
    pgpAssertNoErr(err);

    err = pgpGetUserIDStringFromKey( key, useridstr );
    pgpAssertNoErr(err);

    err = pgpGetKeyIDStringCompatFromKey( key, TRUE, FALSE, kstr );
    pgpAssertNoErr(err);

    if (vOption) {

        fprintf( filebPtr->pgpout, "%*s   ", 2*depth + nameLength, " ");

        fprintf( filebPtr->pgpout, "%8s %s\n", kstr, useridstr);
    }
    else {
        fprintf( filebPtr->pgpout, "%8s %s", kstr, useridstr);
    }
    return kPGPError_NoErr;
}


/*  Display a group's member keys.  */

PGPError walkGroup(struct pgpfileBones *filebPtr, PGPKeySetRef keySet,
        PGPGroupSetRef groupSet, PGPGroupID groupID, PGPUInt32 depth)
{
    PGPGroupItemIterRef iter;
    PGPGroupItem item;
    PGPError err;
    PGPKeyID keyID;

    err = PGPNewGroupItemIter(groupSet, groupID,
            kPGPGroupIterFlags_Keys, &iter);

    pgpAssertNoErr( err );

    err = PGPGroupItemIterNext( iter, &item );
    if (err) return kPGPError_NoErr;  /* empty */

    while (1) {
        err = getKeyIDFromItem(&item, &keyID);
        pgpAssertNoErr(err);

        err = showKeyGroupFormat(filebPtr, keySet, groupSet,
                keyID, depth, TRUE);
        pgpAssertNoErr(err);

        err = PGPGroupItemIterNext( iter, &item );

        if ( IsPGPError(err) ) /* End of Group. */
        {
            err = PGPFreeGroupItemIter( iter );
            pgpAssertNoErr(err);

            fprintf(filebPtr->pgpout, "\n");

            return kPGPError_NoErr;
        }
    }
}  /* walkGroup */


/* Walk a group set displaying each member group.
   If showKeys=TRUE, also show keys.
*/
PGPError walkGroupSet(struct pgpfileBones *filebPtr, PGPGroupSetRef
        groupSet, PGPKeySetRef keySet, PGPBoolean showKeys)
{
    PGPUInt32 i, count;
    PGPError err;
    PGPGroupID groupID;

    err = PGPSortGroupSetStd( groupSet, keySet);
    pgpAssertNoErr( err );

    err = PGPCountGroupsInSet( groupSet, &count);
    pgpAssertNoErr( err );

    if (count == 0) {
        fprintf(filebPtr->pgpout, LANG("No groups found.\n"));
    } else {
        for (i = 0; i < count; i++) {
            err = PGPGetIndGroupID( groupSet, i, &groupID);
            pgpAssertNoErr( err );

            err = showGroupInfo(filebPtr, groupSet, groupID, 0);
            pgpAssertNoErr(err);

            if (showKeys) {

                fprintf(filebPtr->pgpout, "\n");

                err = walkGroup(filebPtr, keySet, groupSet,
                        groupID, 0);
                pgpAssertNoErr( err );
            }
        }
    }
    return kPGPError_NoErr;
}

PGPError viewOneGroup(struct pgpfileBones *filebPtr, PGPGroupSetRef
        groupSet, PGPGroupID groupID, PGPKeySetRef keySet, PGPBoolean
        showKeys)

{
    PGPError err;

    err = showGroupInfo(filebPtr, groupSet, groupID, 0);
    pgpAssertNoErr(err);

    if (showKeys) {

        fprintf(filebPtr->pgpout, "\n");

        err =  walkGroup(filebPtr, keySet, groupSet, groupID, 0);

        pgpAssertNoErr( err );
    }
    return err;
}


/* Called by -gv[v]
  -gv displays group hierarchy
  -gvv also shows keys in each group
*/

int viewGroups(struct pgpmainBones *mainbPtr, char *groupName,
        PGPFileSpecRef ringfilespec, char *origRingFileName, PGPBoolean
        showKeys)

{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPGroupSetRef groupSet = mainbPtr->workingGroupSet;
    PGPContextRef context = mainbPtr->pgpContext;
    PGPKeySetRef keySet;
    PGPGroupID groupID;
    PGPError err, er2;
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    PGPInt32 pri;
    char * ringfile = NULL;


    err = PGPGetFullPathFromFileSpec( ringfilespec, &ringfile );
    pgpAssertNoErr(err);

    err = PGPOpenKeyRing( context, kPGPKeyRingOpenFlags_Mutable,
    ringfilespec, &keySet);

    if( IsPGPError(err) ) {
        fprintf(filebPtr->pgpout, LANG("\nCan't open key ring file '%s'\n"),
                origRingFileName);
        goto done;
    }

    /* -gv[v] [groupName] views only one group. */

    if ((groupName != NULL) && (groupName[0] != '\0')) {

        err = pgpGetGroupByName( mainbPtr->workingGroupSet, groupName,
                &groupID );

        if ( IsPGPError(err) ) {
            fprintf(filebPtr->pgpout,
                LANG("Group \'%s\' not found.\n"), groupName );
        } else {
           err = viewOneGroup(filebPtr, groupSet, groupID, keySet, showKeys);
        }

    } else {
        err = walkGroupSet(filebPtr, groupSet, keySet, showKeys);
        pgpAssertNoErr(err);
    }

done:
    if (keySet) {
        err = PGPFreeKeySet( keySet );
        pgpAssertNoErr(err);
    }

    if (IsPGPError(err)) {
        if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2 ) )
            pgpShowError(filebPtr, err, __FILE__,__LINE__);
        return -1; /* check proper status for keyringfile open fail
                      and group not found */
    } else {
        return kPGPError_NoErr;
    }
}


/*
   Functions to add a group, or add keys to a group.
   For pgp -ga
 */

PGPError createNewKeyItem( PGPKeyRef key, PGPGroupItem *item )
{
    PGPError err;
    PGPUInt32 algorithm;
    PGPKeyID keyID;

    item->type =  kPGPGroupItem_KeyID;
    item->userValue = 0;

    err = PGPGetKeyNumber( key, kPGPKeyPropAlgID, &algorithm);
    item->u.key.algorithm = (PGPPublicKeyAlgorithm)algorithm;

    err = PGPGetKeyIDFromKey(key, &keyID);
    pgpAssertNoErr(err);

    item->u.key.keyID = keyID;

    return kPGPError_NoErr;
}


PGPError addKeySetToGroup (struct pgpmainBones *mainbPtr, PGPGroupID
        groupID, char * groupName, char * useridstr, PGPFileSpecRef
        ringfilespec)

{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPGroupSetRef groupSet = mainbPtr->workingGroupSet;
    PGPUInt32 matchFlags = 0;
    PGPKeySetRef resultSet;
    PGPKeyListRef resultList;
    PGPKeyRef key;
    PGPKeyIterRef iter;
    PGPKeyID keyID;
    PGPError err;
    PGPGroupItem item;

    err = pgpGetMatchingKeySet(mainbPtr, useridstr, matchFlags, &resultSet);
    pgpAssertNoErr(err);

    err = PGPOrderKeySet( resultSet, kPGPAnyOrdering, &resultList );
    pgpAssertNoErr(err);

    err = PGPNewKeyIter( resultList, &iter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( iter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( iter, &key);

    if( IsPGPError(err)) {
        fprintf(filebPtr->pgpout, LANG("No matching keys found.\n"));
        goto done;
    }

    while( key != NULL ) {

        err = createNewKeyItem( key, &item );
        pgpAssertNoErr(err);

        err = getKeyIDFromItem(&item, &keyID);
        pgpAssertNoErr(err);

        err = showKeyGroupFormat(filebPtr, resultSet, groupSet,
                keyID, 0, FALSE);

        err = PGPAddItemToGroup(groupSet, &item, groupID);

        if ( IsPGPError (err)) {

            fprintf(filebPtr->pgpout,
                    LANG(" already in group \'%s\'.\n"), groupName);
        }
        else {

            fprintf(filebPtr->pgpout,
                    LANG(" added to group \'%s\'.\n"), groupName);
        }

        err = PGPKeyIterNext( iter, &key);
    }

done:
    if(iter)
        PGPFreeKeyIter( iter );
    if(resultList)
        PGPFreeKeyList( resultList );
    if(resultSet)
        PGPFreeKeySet( resultSet );

    return kPGPError_NoErr;
}


/*
   Create a new group, pgp -ga groupname
   add [a] member/s to a group, pgp -ga groupname [userids]
   or add all members of group 2 to group 1.
   pgp -ga groupname1 groupname2
 */
int addToGroup(struct pgpmainBones *mainbPtr, char *groupName, char
        **mcguffins, PGPFileSpecRef ringfilespec)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPGroupName groupDescription;
    PGPUInt32 i = 0;
    PGPError err;
    PGPGroupID groupID;

    err = pgpGetGroupByName( mainbPtr->workingGroupSet, groupName,
            &groupID );

    if ( IsPGPError(err) ) /* New Group */
    {
        if( filebPtr->pgpout ) {

            fputs( LANG("\nGroup Description: "), filebPtr->pgpout);
            fflush( filebPtr->pgpout );
        }

        pgpTtyGetString( groupDescription,
                kPGPMaxGroupNameLength, filebPtr->pgpout );
        err = PGPNewGroup( mainbPtr->workingGroupSet,
                groupName, groupDescription, &groupID);
        pgpAssertNoErr( err);

        fprintf(filebPtr->pgpout,
                LANG("Group \'%s\' added.\n"), groupName);
    } else {
        if (mcguffins[0] == NULL)
        {
            fprintf(filebPtr->pgpout,
                    LANG("Group \'%s\' already exists.\n"), groupName );

            return kPGPError_NoErr;

        }
    }

    while (mcguffins[i] != NULL) {
        /* Add groupname or userid. If ambiguous, does both. */
        err =addKeySetToGroup (mainbPtr, groupID, groupName, mcguffins[i],
                ringfilespec);
        pgpAssertNoErr(err);
        i++;
    }

    return kPGPError_NoErr;
}

/*
   Functions to remove a group, or delete keys from a group.
   For pgp -gr

   XXX NOTE: THESE FUNCTIONS REQUIRE PATCH IN
    /libs/pgpcdk/priv/utilities/pgpGroups.c
*/

/* Remove keys in keySetToRemove. */
PGPError removeKeySetFromGroup(struct pgpfileBones *filebPtr,
        PGPGroupSetRef groupSet, PGPGroupID groupID, char *groupName,
        PGPKeySetRef keySetToRemove, PGPKeySetRef keySet )

{
    PGPKeyRef key;
    PGPKeyID keyID;
    PGPError err;
    PGPGroupItem item;
    PGPUInt32 countKeys, countItems, i;
    PGPBoolean done = FALSE;

    err = PGPCountGroupItems(groupSet, groupID, FALSE,
            &countKeys, &countItems);
    pgpAssertNoErr( err );

    if (countItems == 0) {

        fprintf(filebPtr->pgpout,
                LANG("Empty group: \'%s\'.\n"), groupName);
        return kPGPError_NoErr;
    }

    for (i=0; i<countItems; i++) {
        err = PGPGetIndGroupItem( groupSet, groupID, i, &item);
        pgpAssertNoErr(err);

        err = getKeyIDFromItem(&item, &keyID);
        pgpAssertNoErr(err);

        err = PGPGetKeyByKeyID( keySet, &keyID,
                kPGPPublicKeyAlgorithm_Invalid, &key);
        pgpAssertNoErr(err);

        if ( PGPKeySetIsMember(key, keySetToRemove) ) {
            err = PGPDeleteIndItemFromGroup(groupSet, groupID, i);
            pgpAssertNoErr(err);

            /* Deletion auto-decrements indices. */
            i = i - 1;
            countItems = countItems - 1;

            if (IsntPGPError(err)) done = TRUE;

            showKeyGroupFormat(filebPtr, keySet,
                    groupSet, keyID, 0, FALSE);
            pgpAssertNoErr(err);

            fprintf(filebPtr->pgpout,
                    LANG(" removed from group \'%s\'.\n"), groupName);
        }
    }

    if (!done) fprintf(filebPtr->pgpout,
            LANG("Key(s) not found in group \'%s\'.\n"), groupName);

    return kPGPError_NoErr;
}


PGPError removeKeySetFromGroupSet(struct pgpmainBones *mainbPtr,
        PGPGroupSetRef groupSet, PGPGroupID targetGroupID, char *groupName,
        PGPKeySetRef keySetToRemove, PGPFileSpecRef ringfilespec)

{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPUInt32 i, count;
    PGPError err;
    PGPKeySetRef keySet;
    PGPGroupID groupID;


    /* For removeKeySetFromGroup, open the groups' keyset. */

    err = PGPOpenKeyRing( context, kPGPKeyRingOpenFlags_Mutable,
            ringfilespec, &keySet);
    pgpAssertNoErr(err);

    err = PGPCountGroupsInSet( groupSet, &count);
    pgpAssertNoErr( err );

    if (count == 0)
        fprintf(filebPtr->pgpout, LANG("No groups found.\n"));

    for (i = 0; i < count; i++) {

        err = PGPGetIndGroupID( groupSet, i, &groupID);
        pgpAssertNoErr( err );

        if (targetGroupID == groupID) {

            err = removeKeySetFromGroup(filebPtr, groupSet,
                    groupID, groupName, keySetToRemove, keySet);
            pgpAssertNoErr( err );
        }
    }

    if (keySet) {
        err = PGPFreeKeySet( keySet );
        pgpAssertNoErr(err);
    }
    return kPGPError_NoErr;
}


/*
   Remove a key or keys from a group, pgp -gr groupname [userids]
   remove the contents of group 2 from group 1:
   pgp -gr groupname1 groupname2
   or remove a group. pgp -gr groupname
*/
int removeFromGroup(struct pgpmainBones *mainbPtr, char *groupName,
        char **mcguffins, PGPFileSpecRef ringfilespec)
{
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPGroupSetRef groupSet = mainbPtr->workingGroupSet;
    PGPUInt32 i = 0, countKeys, countItems;
    PGPInt32 pri;
    PGPKeySetRef keySetToRemove;
    PGPError err;
    PGPGroupID groupID;
    PGPBoolean boo = TRUE;

    err = pgpGetGroupByName( mainbPtr->workingGroupSet, groupName,
            &groupID );

    if ( IsPGPError(err) ) {
        fprintf(filebPtr->pgpout,
                LANG("Group \'%s\' not found.\n"), groupName );
    } else {
        if (mcguffins[0] == NULL) {
            err = PGPCountGroupItems(groupSet, groupID,
                    FALSE, &countKeys, &countItems);
            pgpAssertNoErr( err );

            if (countItems != 0) {

                if( filebPtr->pgpout ) {

                    fprintf(filebPtr->pgpout, LANG(
                        "Group '%s' not empty. Remove it anyway? (y/N)\n"),
                        groupName);
                    fflush( filebPtr->pgpout );
                }
                boo =  getyesno(filebPtr,'n', pgpenvGetInt( env,
                        PGPENV_BATCHMODE, &pri, &err ));

            }

            if (boo) {
                err = PGPDeleteGroup(groupSet, groupID);
                pgpAssertNoErr(err);

                if (!IsPGPError(err))
                    fprintf(filebPtr->pgpout,
                            LANG("Group '%s' removed.\n"), groupName);
            }
        }
        while (mcguffins[i] != NULL) {
            err = pgpGetMatchingKeySet(mainbPtr, mcguffins[i],
                    0, &keySetToRemove);
            pgpAssertNoErr(err);

            /* This removes either keys or groups,
               due to pgpGetMatchingKeySet's strategy. */

            err = removeKeySetFromGroupSet(mainbPtr, groupSet,
                    groupID, groupName, keySetToRemove, ringfilespec );
            pgpAssertNoErr(err);

            i++;
        }
    }
    return kPGPError_NoErr;
}























































