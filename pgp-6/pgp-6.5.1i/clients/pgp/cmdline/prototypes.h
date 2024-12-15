/*____________________________________________________________________________
    prototypes.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    $Id: prototypes.h,v 1.15.8.1.2.1 1999/07/29 23:10:35 heller Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include "pgpBase.h"
#include "pgpContext.h"
#include "pgpKeys.h"
#include "pgpGroups.h"

#include "globals.h"

/* doencode.c, dodecode.c */

PGPError pgpUnionKeysAskInvalid( struct pgpfileBones *filebPtr,
        PGPKeySetRef fromSet, PGPKeySetRef toSet, PGPKeySetRef *resultset );
int pgpDoEncode( struct pgpmainBones *mainbPtr, char *workfilename,
        char **resultfilename, int *perrorLvl );
int pgpDoDecode( struct pgpmainBones *mainbPtr, char *workfilename,
        int *perrorLvl );

PGPError pgpNewUserIDIterFromKey( PGPKeyRef key, PGPKeyIterRef *newiter );
PGPError pgpCountKeyUserIDs( PGPKeyIterRef keyiter, PGPUInt32 *numuserids );
PGPError pgpGetKeySetFromUserIDString( PGPContextRef context, PGPKeySetRef
        keyringset, const char *useridstr, PGPKeySetRef *poutSet );

PGPError dechandler( PGPContextRef context, struct PGPEvent *event,
        PGPUserValue uservalue );

int encryptFile( struct pgpmainBones *mainbPtr, const char **recipients,
        char *infile, char *outfile, PGPBoolean attempt_compression );

int conventionalEncryptFile( struct pgpmainBones *mainbPtr, char *infile,
        char *outfile, PGPBoolean attempt_compression );

/* pgpAcquireEntropy.c */
PGPError pgpAcquireEntropy( struct pgpfileBones *filebPtr,
        unsigned bitsneeded );

/* keygen.c */
void GetRSAStatus(PGPBoolean *haveRSAAlgorithm, 
				  PGPBoolean *canGenerateKeys,
				  char *copyright);

PGPError pgpPassphraseDialogCmdline( struct pgpmainBones *mainbPtr,
        PGPBoolean confirm, const char *prompt, char **passphrase );

int dokeygen( struct pgpmainBones *mainbPtr, char *sigbitsstr,
        char *encbitsstr );

/* keyadd.c */
PGPError addToWorkingRingSetFinish( struct pgpmainBones *mainbPtr );
PGPError addToWorkingRingSet( struct pgpmainBones *mainbPtr,
        PGPKeySetRef keyset );

PGPError pgpAddKeyToKeySet( PGPKeyRef key, PGPKeySetRef keyset );
PGPError pgpMarkKey( PGPKeyIterRef iterContext, PGPKeyRef key );
PGPError pgpMarkKeySet( PGPKeySetRef keyset );

PGPError pgpShowKeyUnmarked( struct pgpfileBones *filebPtr,
        PGPKeyIterRef iterContext, PGPKeyRef key,
        PGPSize *newKeyCount, PGPSize *newUIDCount, PGPSize *newSigCount,
        PGPSize *newRevokeCount );

PGPError pgpShowKeySetUnmarked( struct pgpfileBones *filebPtr,
        PGPKeySetRef keyset, PGPSize *newKeyCount, PGPSize *newUIDCount,
        PGPSize *newSigCount, PGPSize *newRevokeCount );

int addToKeyring( struct pgpmainBones *mainbPtr, char *keyfile,
        PGPFileSpecRef ringfilespec, char *origRingFileName );

/* keyedit.c */
PGPError pgpGetValidPassphrase( struct pgpmainBones *mainbPtr, PGPKeyRef
        key, char **passphrasePtr, PGPBoolean *needsfree );

PGPError pgpGetCorrespondingSecretRingName( struct pgpfileBones *filebPtr,
        const char *pubringfile, char *secringfile );

PGPError pgpGetCorrespondingPublicRingName( struct pgpfileBones *filebPtr,
        const char *secringfile, char *pubringfile );

PGPError pgpOpenKeyringsFromPubringSpec( struct pgpmainBones *mainbPtr,
        PGPFileSpecRef pubfilespec, PGPKeySetRef *keyringset, PGPUInt32 openFlags );

PGPError pgpOpenKeyringsIfSecringSpec( struct pgpmainBones *mainbPtr,
        PGPFileSpecRef secfilespec, PGPKeySetRef *keyringset, PGPBoolean
        *isprivate, PGPUInt32 openFlags );

PGPError pgpEditPublicTrustParameter( struct pgpfileBones *filebPtr,
        const char *useridstr, PGPKeyRef key);

int doKeyEdit( struct pgpmainBones *mainbPtr, const char *useridstr,
        PGPFileSpecRef ringfilespec );

/* match.c */
#define kMatch_NotDisabled  1
#define kMatch_NotExpired   2
#define kMatch_NotKeyServer 4
PGPError pgpGetMatchingKeySet( struct pgpmainBones *mainbPtr, const char
        *useridstr, PGPFlags matchFlags, PGPKeySetRef *resultset );

PGPError pgpGetMatchingKeyList( struct pgpmainBones *mainbPtr, const
        char *useridstr, PGPFlags matchFlags, PGPKeyListRef *resultlist );

PGPError pgpGetMySigningKey( struct  pgpmainBones *mainbPtr, PGPKeyRef
        *myKey );

PGPError pgpGetSigningKey(struct pgpmainBones *mainbPtr, PGPKeyRef	*pKey);


PGPError pgpBuildKeySetFromUserIDStringList( struct pgpmainBones
        *mainbPtr, const char **ustrList, PGPKeySetRef *resultsetRef );

PGPBoolean pgpLocationIsURL( const char *locationName );

/* keyview.c */
void pgpShowError( struct pgpfileBones *filebPtr, PGPError err,
        char *file, int line );

PGPError pgpGetKeyTrustString( PGPKeyRef key, char **outStr );
PGPError pgpGetKeyValidityString( PGPKeyRef key, char **outStr );

PGPError pgpShowKeyTrust( struct pgpfileBones *filebPtr, PGPKeyRef key );
PGPError pgpShowKeyValidity( struct pgpfileBones *filebPtr, PGPKeyRef key );

PGPError pgpGetKeyIDStringCompat( PGPKeyID const *kid, PGPBoolean abbrev,
        PGPBoolean compat, char *kidstr );

PGPError pgpGetKeyIDStringCompatFromKey( PGPKeyRef key, PGPBoolean abbrev,
        PGPBoolean compat, char *kidstr );

PGPError pgpShowKeyIDFromKey( struct pgpfileBones *filebPtr, PGPKeyRef key );
PGPError pgpShowKeyID( struct pgpfileBones *filebPtr, PGPKeyID const *kid );

PGPError pgpGetUserIDStringFromKey( PGPKeyRef key, char *useridstr );
PGPError pgpShowKeyUserID( struct pgpfileBones *filebPtr, PGPKeyRef key );

PGPError pgpShowKeyBrief( struct pgpfileBones *filebPtr, PGPKeyRef key );
PGPError pgpShowKeyChanges( struct pgpfileBones *filebPtr, PGPKeyRef
        existingkey, PGPKeyRef newkey );

PGPError pgpShowKeyTrustAndValidity( struct pgpfileBones *filebPtr,
        PGPKeySetRef ringset, PGPKeyIterRef iterContext, PGPKeyRef key,
        PGPBoolean show_fingerprints );

PGPError pgpShowKeySetTrustAndValidity( struct pgpfileBones *filebPtr,
        PGPKeySetRef ringSet, PGPKeySetRef keyset );

PGPError pgpShowTrustAndValidityList( struct pgpmainBones *mainbPtr,
        char *useridStr, PGPFileSpecRef ringFileSpec );

#define kShow_Sigs    1
#define kShow_Checks  2
#define kShow_Subkeys 4
#define kShow_Hashes  8
PGPError pgpShowKeySig( struct pgpfileBones *filebPtr, PGPKeySetRef
        ringset, PGPSigRef sig, PGPFlags showFlags );
PGPError pgpShowKeyFingerprint( struct pgpfileBones *filebPtr,
        PGPKeyRef key );

PGPError pgpShowKeyListFormat( struct pgpfileBones *filebPtr, PGPKeySetRef
        ringset, PGPKeyIterRef iterContext, PGPKeyRef key,
        PGPFlags showFlags );

PGPError viewKeySet( struct pgpmainBones *mainbPtr, PGPKeySetRef keyset,
        PGPFlags showFlags );

int viewKeyring( struct pgpmainBones *mainbPtr, char *mcguffin,
        PGPFileSpecRef ringfilespec, char *origRingFileName,
        PGPFlags showFlags );

int viewKeyServer( struct pgpmainBones *mainbPtr, char *mcguffin,
        char *keyServerURL, PGPFlags showFlags );

/* keysign.c */
int signKeyInFileSpec( struct pgpmainBones *mainbPtr, char *keyguffin,
        PGPFileSpecRef keyfilespec );

int pgpSignKey( struct pgpmainBones *mainbPtr, PGPKeyRef sigkey,
        PGPKeyRef key, const char *useridstr );


/* keymaint.c */
PGPError pgpDoCheckKeySet( struct pgpmainBones *mainbPtr,
        PGPKeySetRef checkSet );

PGPError pgpDoCheckKeyRing( struct pgpmainBones *mainbPtr, char *useridStr );

PGPError dokeycheck( struct pgpmainBones *mainbPtr, char *useridStr,
        PGPFileSpecRef ringFileSpec );

PGPError doMaintenance( struct pgpmainBones *mainbPtr, PGPFileSpecRef
        ringFileSpec, char * ringfilename );

/* pgp.c: */
void initSignals( void );
void checkExpirationDate( void );

void pgpTearDown( struct pgpmainBones *mainbPtr, int *perrorLvl );
PGPBoolean fileCompressible( char *filename );

/* args.c: */
void signonMsg( struct pgpmainBones *mainbPtr );
int pgpParseArgs( struct pgpmainBones *mainbPtr, int argc, char *argv[],
        int *perrorLvl );

int pgpProcessArgs( struct pgpmainBones *mainbPtr, int *perrorLvl );

void keyUsage( struct pgpfileBones *filebPtr, int *perrorLvl );
void groupUsage( struct pgpfileBones *filebPtr, int *perrorLvl );

int doKeyOpt( struct pgpmainBones *mainbPtr, char keyChar, int *perrorLvl );
int doGroupOpt( struct pgpmainBones *mainbPtr, char groupChar,
        int *perrorLvl );

/* misc.c: strlwr */
#if !defined(MSDOS)
char *strlwr( char *s );
char *strupr( char *s );
#endif
int toLower( int c );
int toUpper( int c );
char *ctdate( PGPUInt32 *tstamp );

/* more.c */
void clearScreen(struct pgpfileBones *filebPtr);
PGPError moreStart( struct pgpfileBones *filebPtr );
PGPError moreBuffer(struct pgpfileBones *filebPtr, const char *textbuffer,
        PGPSize length);

PGPError moreFinish( struct pgpfileBones *filebPtr );
int moreFile( struct pgpfileBones *filebPtr, char *fileName );

/* fileio.c */
int getHeaderInfoFromFile( char *infile,  PGPByte *header, int count );
PGPBoolean getyesno( struct pgpfileBones *filebPtr, char default_answer,
        PGPBoolean batchmode );

PGPError touchFile( char *filename, int mode );

/* keyexport.c */
int extractFromKeyring( struct pgpmainBones *mainbPtr, char *useridstr,
        char *keyfile, PGPFileSpecRef ringfilespec,
        char *origRingFileName, PGPBoolean transflag );

int extractFromKeyServer(struct pgpmainBones *mainbPtr, char *useridstr,
        char *keyfile, char *keyServerURL, PGPBoolean transflag);

/* keyremove.c */
int removeFromKeyring( struct pgpmainBones *mainbPtr, char *mcguffin,
        char *ringFileName, char *origRingFileName);

int removeFromKeyServer( struct pgpmainBones *mainbPtr, char *mcguffin,
        char *keyServerURL );

int removeSigs( struct pgpmainBones *mainbPtr, char *mcguffin, char*
        ringfile, char *origRingFileName );

PGPError srvhandler(PGPContextRef context, struct PGPEvent *event,
        PGPUserValue userValue);

/* keyrevoke.c */
int revokeSigs( struct pgpmainBones *mainbPtr, char *mcguffin,
        char* ringfile );

int revokeOrDisableKey( struct pgpmainBones *mainbPtr, char *keyguffin,
        PGPFileSpecRef keyfilespec );

int disableKeyOnKeyServer( struct pgpmainBones *mainbPtr, char *keyguffin,
        char *keyServerURL );

/* groups.c */
PGPError pgpGetGroupByName( PGPGroupSetRef groupSet, char *groupName,
        PGPGroupID *group );

PGPError pgpInitializeWorkingGroupSet( struct pgpmainBones *mainbPtr );
PGPError pgpFinalizeWorkingGroupSet( struct pgpmainBones *mainbPtr );

int addToGroup(struct pgpmainBones *mainbPtr, char *groupname,
        char **mcguffins, PGPFileSpecRef ringfilespec);

int removeFromGroup(struct pgpmainBones *mainbPtr, char *groupname,
        char **mcguffins, PGPFileSpecRef ringfilespec);

int viewGroups(struct pgpmainBones *mainbPtr, char *groupname,
        PGPFileSpecRef ringfilespec, char *origRingFileName,
        PGPBoolean showFlags);

/* stubs.c */
int set_CONVERSION( );
int get_CONVERSION( );

int maintCheck( PGPFileSpecRef ringfilespec, int options );
struct newkey;
int maintUpdate( PGPFileSpecRef ringfilespec, struct newkey const *nkeys );
void init_charset( void );
void CONVERT_TO_CANONICAL_CHARSET( char *s );
char EXT_C( char c );
char INT_C( char c );

struct IdeaCfbContext;
int cryptRandOpen( struct IdeaCfbContext *cfb );
void cryptRandSave( struct IdeaCfbContext *cfb );

int cryptRandWriteFile( char const *name, struct IdeaCfbContext *cfb,
        unsigned bytes );

char * get_ext_c_ptr( void );

/* see libPGPui.a */
int pgpTtyGetString( char *strbuf, int maxlen, FILE *echo );



