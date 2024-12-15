/*	File:		MoreNetworkSetup.h	Contains:	Simple wrappers to make Network Setup easier	Written by:	Quinn	Copyright:	Copyright (C) 1998 by Apple Computer, Inc., all rights reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):         <4>     2/19/99    wprice  MODIFIED BY NETWORK ASSOCIATES         <3>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".         <2>     5/11/98    Quinn   Fix header.         <1>     5/11/98    Quinn   First checked in.*/#pragma once/////////////////////////////////////////////////////////////////// MoreIsBetter Setup#include "MoreSetup.h"/////////////////////////////////////////////////////////////////// Mac OS Interfaces#include <MacTypes.h>#include <NetworkSetup.h>/////////////////////////////////////////////////////////////////struct MNSDatabadeRef {	CfgDatabaseRef dbRef;	CfgAreaID area;	CfgAreaID originalArea;};typedef struct MNSDatabadeRef MNSDatabadeRef;typedef MNSDatabadeRef *MNSDatabadeRefPtr;#ifdef __cplusplusextern "C" {#endifextern pascal Boolean IsNetworkSetupAvailable(void);	// This routine tests whether the Network Setup API is available.	// It may be called by both classic and CFM code.  If called by	// CFM code, it uses a weak link to see if the library is present,	// See DTS Technote 1083 "Weak-Linking to a Code Fragment Manager-based	// Shared Library" for details.	//	// <http://developer.apple.com/technotes/tn/tn1083.html>	//	// If called by classic 68K code, it uses explicit CFM calls to determine	// whether the library is present.	//	// If Network Setup is not available, calling any of these routines	// will cause you to crash.  Also, if building classic 68K, calling any	// other routine in this library will cause a link error because there's	// no Mixed Mode glue for the Network Setup library.extern pascal Boolean  MNSValidDatabase(const MNSDatabadeRef *ref);	// Returns true if the database reference passed in is reasonably	// valid.	extern pascal Boolean  MNSDatabaseWritable(const MNSDatabadeRef *ref);	// Returns true if referenced database is writable.extern pascal OSStatus MNSOpenDatabase(MNSDatabadeRef *ref, Boolean forWriting);	// Opens the Network Setup database, filling out MNSDatabadeRef to	// contain the information needed to access it.  This also opens	// the current area, either for reading or writing depending on	// the forWriting parameter.	extern pascal OSStatus MNSCloseDatabase(MNSDatabadeRef *ref, Boolean commit);	// Closes the Network Setup database specified by MNSDatabadeRef.	// If commit is true, it will attempt to commit any changes made	// to the database.  The database must have been opened for writing.	// If commit is false, it will throw away changes (if any) made to the	// database.extern pascal OSStatus MNSGetFixedSizePref(const MNSDatabadeRef *ref,						const CfgEntityRef *entityID,						OSType prefType,						void *buffer, ByteCount prefSize);	// This routine gets a fixed size preference out of	// the configuration database described by ref.  entityID	// is the entity containing the preference.  prefType is the	// type of preference within the entity.  buffer is the address	// where the preference data should be put.  prefSize is the size	// of the buffer, and the routine validates that the preference	// is exactly that size.extern pascal OSStatus MNSGetPref(const MNSDatabadeRef *ref,						const CfgEntityRef *entityID,						OSType prefType,						void **buffer, ByteCount *prefSize);	// This routine gets a variable size preference out of	// the configuration database described by ref.  entityID	// is the entity containing the preference.  prefType is the	// type of preference within the entity.  buffer is the address	// a pointer where the address of the newly allocated preference	// buffer should be put.  prefSize is the address of a variable	// where the size of the newly allocated preference should be.	// 	// The caller is responsible for disposing of the preference buffer	// using DisposePtr.  If the routine fails, no preference buffer is 	// returned.extern pascal OSStatus MNSSetPref(const MNSDatabadeRef *ref,						const CfgEntityRef *entityID,						OSType prefType,						void *prefData, ByteCount prefSize);	// This routine sets a preference in the database specified by ref.	// entityID is the entity containing the preference.  prefType is the	// type of preference within the entity.  buffer is the address	// of the new preference data.  prefSize is the size of the new	// preference data.	extern pascal OSStatus MNSGetEntitiesList(const MNSDatabadeRef *ref,								OSType entityClass, OSType entityType,								ItemCount *entityCount,								CfgEntityRef **entityIDs,								CfgEntityInfo **entityInfos);	// This routine gets a list of all the entities that match	// class and type in the specified area of the specified database.	// It allocates buffer- (using NewPtr) to hold the CfgEntityRef's	// and CfgEntityInfo's and sets *entityIDs and *entityInfos to	// the buffers.  Pass nil if you don't want to get that particular	// information.  It sets entityCount to the number of entities found.extern pascal OSStatus MNSFindActiveSet(const MNSDatabadeRef *ref, CfgEntityRef *activeSet);	// This routine finds the entity ref of the active set entity	// in the database.  It works by finding all the set entities	// (there is generally only one in the current OT implementation)	// and checks each one for the active bit set in its flags.	// It returns the first set that claims to be active.typedef pascal void (*MNSSetIterator)(const MNSDatabadeRef *ref, CfgSetsElement *thisElement, void *refcon);	// A callback for MNSIterateSet.  ref and refcon are as	// specified in the call to MNSIterateSet.  thisElement is	// a specified set element in the set.  It has fields	// fEntityRef (the entity ref) and fEntityInfo (the entity info).	// Your callback can modify these fields and the changes will	// take effect if writeAfterIterate is specified in the call	// to MNSIterateSet.extern pascal OSStatus MNSIterateSet(const MNSDatabadeRef *ref,									const CfgEntityRef *setEntity,									MNSSetIterator iteratorProc,									void *refcon,									Boolean writeAfterIterate);	// This routine iterates through all the entities in the specified	// set in the database specified by ref.  Pass a set entity	// in the setEntity parameter.  Pass a pointer to your callback	// routine in iteratorProc.  refcon is ignored by the routine and	// simply passed to the iterator.  If writeAfterIterate is true,	// the set preference is written back to the database after	// all entities have been enumerated.	//	// Typically one would iterate through the active set, found using	// MNSFindActiveSet.	#ifdef __cplusplus}#endif