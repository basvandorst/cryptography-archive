/*____________________________________________________________________________
	MacDebug.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDebug.h,v 1.13 1997/09/18 01:34:23 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#include <AppleEvents.h>

#include "MacBasics.h"


#include "pgpDebug.h"



PGP_BEGIN_C_DECLARATIONS


/*____________________________________________________________________________
	some compilers don't support #pragma mark, which allows nice markers in the
	source file routine name popup menu.
____________________________________________________________________________*/
#ifdef __MWERKS__

#if ! PGP_DEBUG
	#if powerc
		#pragma traceback off
	#else
		#pragma macsbug off
	#endif
#endif
	
#endif



#if PGP_DEBUG	/* [ */
/*____________________________________________________________________________
	OSErrString tables
____________________________________________________________________________*/

/* a single entry in an error table.  An array of these constitutes a table. */
typedef struct DebugOSErrStringTableEntry
{
	OSStatus	err;
	const char	*cString;	/* a C string */
} DebugOSErrStringTableEntry;

void	DebugAddOSErrStringTable( const DebugOSErrStringTableEntry * entries,
			UInt16 numEntries);
	
#endif	/* ] */




/*____________________________________________________________________________
	basic asserts and their variants...
____________________________________________________________________________*/
#if	PGP_DEBUG	/* [ */

	#define DebugMsgIf( cond, failStr)		\
		do {if ( (cond) ) { pgpDebugMsg(failStr);}} while ( FALSE )
	
#else	/* ] PGP_DEBUG [ */

	#define DebugMsgIf( cond, failStr)
	
#endif	/* ] PGP_DEBUG */




/*____________________________________________________________________________
	more complex asserts
____________________________________________________________________________*/
#if	PGP_DEBUG	/* [ */

#ifdef _cplusplus
	#define EQ_NIL	= nil
#else
	#define EQ_NIL
#endif

	void	_AssertHandleIsValid(const void *theHandle,
					const char *varName, const char * optionalMsg,
					const char *fileName, const UInt32 lineNumber);
					
	void	_AssertResourceIsValid(const void *theHandle,
					const char *varName, const char * optionalMsg,
					const char *fileName, const UInt32 lineNumber);
					
	void	_AssertFileRefNumIsValid( short refNum, const char *varName,
					const char * optionalMsg EQ_NIL,
					const char *fileName, const UInt32 lineNumber);
					
	void	_AssertAEDescIsValid( const AEDesc *desc, const char *varName,
					const char * optionalMsg EQ_NIL,
					const char *fileName, const UInt32 lineNumber);
					
	void	_AssertSpecIsValid( const FSSpec * spec, const char *varName,
					const char * optionalMsg EQ_NIL,
					const char *fileName, const UInt32 lineNumber);
					
	void	_AssertUPPIsValid( UniversalProcPtr upp, const char *varName,
					const char * optionalMsg EQ_NIL,
					const char *fileName, const UInt32 lineNumber);
					
	void	_AssertNoErr( OSStatus err, const char * optionalMsg,
					const char *fileName, const UInt32 lineNumber);

	
#define AssertHandleIsValid(h, msg)				\
	_AssertHandleIsValid( (h), #h, msg, __FILE__, __LINE__)
	
#define AssertResourceIsValid(r, msg)			\
	_AssertResourceIsValid( (r), #r, msg, __FILE__, __LINE__)
	
#define AssertFileRefNumIsValid( r, msg)		\
	_AssertFileRefNumIsValid( r, #r, msg, __FILE__, __LINE__)
	
#define AssertAEDescIsValid( d, msg )			\
	_AssertAEDescIsValid( d, #d, msg, __FILE__, __LINE__)
	
#define AssertSpecIsValid( s, msg)				\
	_AssertSpecIsValid( s, #s, msg, __FILE__, __LINE__)
	
#define AssertUPPIsNullOrValid( upp, msg )		\
	do\
	{\
		if ( IsntNull( upp ) )\
			_AssertUPPIsValid( (UniversalProcPtr)upp, \
			#upp, msg, __FILE__, __LINE__ );\
	} while ( FALSE )
	
#define AssertNoErr( err, msg)					\
	_AssertNoErr( err, msg, __FILE__, __LINE__)

#define AssertAddrNullOrValid( addr, type, msg)	\
	do\
	{\
		if ( IsntNull( addr ) ) \
		pgpAssertAddrValidMsg( addr, type, msg );\
	} while ( FALSE )

#undef EQ_NIL

#else	/* ] PGP_DEBUG [ */

	#define AssertHandleIsValid(h, msg)	
	#define AssertResourceIsValid(r, msg)			
	#define AssertNoErr(err, msg)					
	#define AssertFileRefNumIsValid( r, msg)
	#define AssertAEDescIsValid( d, msg )
	#define AssertUPPIsNullOrValid( upp, msg )	
	#define AssertSpecIsValid( r, msg)
	
	#define AssertAddrNullOrValid( addr, type, msg)
	
#endif	/* ] PGP_DEBUG */





/*____________________________________________________________________________
	'USE_MAC_DEBUG_TRAPS' controls whether debugging traps are used.
	It can be set before including this file.  Otherwise, it defaults
	to the same status as PGP_DEBUG
____________________________________________________________________________*/
#ifndef USE_MAC_DEBUG_TRAPS
	#define USE_MAC_DEBUG_TRAPS		( PGP_DEBUG )
#endif

#if USE_MAC_DEBUG_TRAPS
	#include "MacDebugTraps.h"
#endif

#include "MacDebugPatches.h"


/*____________________________________________________________________________
	'USE_MAC_DEBUG_LEAKS' controls whether leaks code compiles in.
	it can be set before #including this file. Otherwise, it defaults
	to the same status as USE_MAC_DEBUG_TRAPS.
____________________________________________________________________________*/
#if ! USE_MAC_DEBUG_TRAPS
	#undef USE_MAC_DEBUG_LEAKS
#endif
 
#ifndef USE_MAC_DEBUG_LEAKS
	#define USE_MAC_DEBUG_LEAKS		( USE_MAC_DEBUG_TRAPS )
#else
	/* USE_MAC_DEBUG_LEAKS must be off if USE_MAC_DEBUG_TRAPS is off */
	#if ! USE_MAC_DEBUG_LEAKS
		#undef USE_MAC_DEBUG_LEAKS
		#define USE_MAC_DEBUG_LEAKS	0
	#endif
#endif

#include "MacDebugLeaks.h"


#if PGP_DEBUG	/* [ */

/* routines used by PGP_DEBUG code */
void	MacDebug_FillWithGarbage( void	*ptr, UInt32 byteCount);
void	MacDebug_WhackAllFreeSpace( void );

#else

#define MacDebug_FillWithGarbage( ptr, cnt)

#endif	/* ] PGP_DEBUG */




PGP_END_C_DECLARATIONS



