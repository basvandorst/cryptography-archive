/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: pgpNetLog.h,v 1.18 1999/04/23 08:00:13 wprice Exp $
____________________________________________________________________________*/

#ifndef Included_pgpNetLog_h
#define Included_pgpNetLog_h

// includes
#include "pgpPubTypes.h"
#include "pgpIKE.h"
#include "pgpNetServiceErrors.h"
#include "pflTypes.h"

// defines
#define kPGPnetLogFileFormatVersion		3		/* 1=>6.5; 3=>6.5.1 */
#define kPGPnetLogMaxNumberOfEvents		256

// log event types
enum PGPnetLogEventType_
{
	kPGPnetLogEvent_First			= 0,
	kPGPnetLogEvent_None			= kPGPnetLogEvent_First,
	kPGPnetLogEvent_DebugText,
	kPGPnetLogEvent_PGPError,
	kPGPnetLogEvent_PGPike,
	kPGPnetLogEvent_System,
	kPGPnetLogEvent_Service,
	kPGPnetLogEvent_IPSEC,

	PGP_ENUM_FORCE(PGPnetLogEventType_)
};
PGPENUM_TYPEDEF(PGPnetLogEventType_, PGPnetLogEventType);

// typedefs
typedef struct
{
	PGPUInt32		logfileFormatVersion;
	PGPUInt32		recordLengthBytes;
	PGPUInt32		fileLengthRecords;
} PGPnetLogFileHeader;

typedef struct PGPnetLog *	PGPnetLogRef;

typedef struct
{
	PGPTime				timeOfEvent;
	PGPnetLogEventType	typeOfEvent;
	PGPUInt32			ipaddress;
	union {
		// for debugging strings
		struct {
			char				sztext[64];
		} debug;

		// for PGPError values
		struct {
			PGPError			error;
			PGPUInt32			lineno;
			char				filename[32];
		} pgperror;

		// for IKE alerts
		struct {
			PGPikeAlert			alert;
			PGPUInt32			value;
			PGPBoolean			remoteGenerated;
		} ikealert;
		
		// for IPSEC messages
		struct {
			PGPError			error;
		} ipsec;

		// for system errors
		struct {
			PGPUInt32			error;
			PGPUInt32			lineno;
			char				filename[32];
		} syserror;
		
		// for PGPnetServiceError values
		struct {
			PGPnetServiceError	error;
			PGPKeyID			keyID;
			PGPUInt32			lineno;
			char				filename[32];
		} svcerror;

		// for convenience
		PGPByte					buffer[1];
	} info;
} PGPnetLogEvent;

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

// prototypes
PGPError 
PGPnetOpenLogFile (
		PFLFileSpecRef	logFileSpec, 
		PGPBoolean		bWriteAccess,
		PGPBoolean		bZeroOutFile,
		PGPnetLogRef*	plogRef);

PGPError 
PGPnetLogEventToFile (
		PGPnetLogRef 	logRef,
		PGPnetLogEvent*	pevent);

PGPError 
PGPnetClearLogFile (
		PGPnetLogRef 	logRef);

PGPError 
PGPnetFlushLogBuffers (
		PGPnetLogRef 	logRef);

PGPError 
PGPnetCloseLogFile (
		PGPnetLogRef	logRef);

PGPError 
PGPnetReadLogData (
		PGPnetLogRef	logRef, 
		PGPSize*		pnumberEvents, 
		void**			ppEventBuffer);

PGPError 
PGPnetFreeLogData (
		void*			pEventData);


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif //Included_pgpNetLog_h

