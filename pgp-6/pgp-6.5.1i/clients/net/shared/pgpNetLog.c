/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	pgpNetLog.c - PGPnet log file/events
	

	$Id: pgpNetLog.c,v 1.10 1999/04/01 11:25:38 wprice Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>

#include "pgpMemoryMgr.h"
#include "pgpMem.h"
#include "pgpPFLErrors.h"
#include "pgpFileUtilities.h"
#include "pgpEndianConversion.h"
#include "pgpUtilities.h"

#include "pgpNetLog.h"

typedef struct PGPnetLog	PGPnetLog;

struct PGPnetLog 
{
	PGPMemoryMgrRef	memoryMgr;
	PFLFileSpecRef	logFileSpec;
	PGPIORef		logIO;

	PGPUInt32		recLength;
	PGPUInt32		numRecords;
	PGPUInt32		dataLength;
	PGPUInt32		fileLength;
	PGPUInt32		currentFilePosition;
};


#define kPGPNetLogHeaderLength		sizeof(PGPnetLogFileHeader)
#define PGPValidateLog(logRef)		PGPValidateParam(logRef != NULL)


static PGPError 
sFreeLog (
		PGPnetLogRef	logRef)
{
	PGPValidateLog (logRef);

	if (logRef)
	{
		if (pflFileSpecIsValid (logRef->logFileSpec))
			PFLFreeFileSpec (logRef->logFileSpec);

		if (PGPIOIsValid (logRef->logIO))
			PGPFreeIO (logRef->logIO);

		PGPFreeData (logRef);
		return kPGPError_NoErr;
	}
	else
		return kPGPError_BadParams;
}


static PGPError 
sIOWriteEvent (
		PGPIORef		fileIO, 
		PGPnetLogEvent* pevent)
{
	PGPnetLogEvent	event;

	event.timeOfEvent	= pevent->timeOfEvent;
	event.typeOfEvent	= pevent->typeOfEvent;
	event.ipaddress		= pevent->ipaddress;
	pgpCopyMemory (pevent->info.buffer, event.info.buffer, sizeof(event.info));

	return (PGPIOWrite (fileIO, sizeof(PGPnetLogEvent), &event));
}


static PGPError 
sIOReadEvent (
		PGPIORef		fileIO, 
		PGPnetLogEvent* pevent)
{
	PGPnetLogEvent	event;
	PGPError		err;
	PGPSize			bytesRead;

	PGPValidateIO (fileIO);
	PGPValidatePtr (pevent);

	err = PGPIORead (fileIO, sizeof(PGPnetLogEvent), &event, &bytesRead);

	if (IsntPGPError (err))
	{
		pevent->timeOfEvent = event.timeOfEvent;
		pevent->typeOfEvent = event.typeOfEvent;
		pevent->ipaddress	= event.ipaddress;
		pgpCopyMemory (event.info.buffer, 
				pevent->info.buffer, sizeof(event.info));
	}
	return err;
}



PGPError 
PGPnetFlushLogBuffers (
		PGPnetLogRef 	logRef)
{
	PGPValidateLog (logRef);

	if (logRef)
		return PGPIOFlush (logRef->logIO);
	else
		return kPGPError_BadParams;
}



PGPError 
PGPnetClearLogFile (
		PGPnetLogRef 	logRef)
{
	PGPnetLogEvent	event;
	PGPUInt32		u;

	PGPValidateLog (logRef);

	if (logRef)
	{
		PGPIOSetPos (logRef->logIO, kPGPNetLogHeaderLength);
		pgpClearMemory (&event, sizeof(PGPnetLogEvent));
		for (u=0; u<logRef->numRecords; u++)
		{
			PGPIOWrite (logRef->logIO, sizeof(PGPnetLogEvent), &event);
		}

		PGPIOSetPos (logRef->logIO, kPGPNetLogHeaderLength);
		logRef->currentFilePosition	= kPGPNetLogHeaderLength;

		return PGPIOFlush (logRef->logIO);
	}
	else
		return kPGPError_BadParams;
}


static PGPError
sCheckForCurrentLogFile (
		PGPnetLogRef	logRef,
		PGPBoolean		bWriteAccess)
{
	PGPBoolean			bFileExists;
	PGPError			err;
	PGPIORef			logIO;
	PGPnetLogFileHeader	header;
	PGPSize				bytesRead;

	/* first check if file exists */
	err = PFLFileSpecExists (logRef->logFileSpec, &bFileExists);
	if (IsntPGPError (err) && !bFileExists)
		err = kPGPError_FileNotFound;

	if (IsPGPError (err))
		return err;

	/* now check if file header corresponds */
	err = PGPOpenFileSpec(	logRef->logFileSpec, 
							kPFLFileOpenFlags_ReadOnly, 
							(PGPFileIORef*)&logIO);

	if (IsntPGPError (err))
	{
		err = PGPIORead (logIO, 
				sizeof(PGPnetLogFileHeader), &header, &bytesRead);

		if (IsntPGPError (err) && (bytesRead != sizeof(PGPnetLogFileHeader)))
			err = kPGPError_CantOpenFile;

		if (IsntPGPError (err))
		{
			if( header.logfileFormatVersion != kPGPnetLogFileFormatVersion)
				err = kPGPError_CantOpenFile;

			if( header.recordLengthBytes != logRef->recLength)
				err = kPGPError_CantOpenFile;

			/* if opening for write access, then the file must have the
			   the expected number of records */
			if (bWriteAccess)
			{
				if( header.fileLengthRecords != logRef->numRecords )
					err = kPGPError_CantOpenFile;
			}
			/* for read access, just use the number of records specified
			   in the header */
			else
			{
				logRef->numRecords = header.fileLengthRecords;
				logRef->dataLength = logRef->numRecords * 
										logRef->recLength;
				logRef->fileLength = logRef->dataLength +
										kPGPNetLogHeaderLength;
			}
		}

		PGPFreeIO (logIO);
	}

	return err;
}


PGPError 
PGPnetOpenLogFile (
		PFLFileSpecRef	logFileSpec, 
		PGPBoolean		bWriteAccess,
		PGPBoolean		bZeroOutFile,
		PGPnetLogRef*	plogRef)
{
	PGPMemoryMgrRef	memoryMgr	= NULL;
	PGPError		err;
	PGPnetLogEvent	event;

	if (IsntNull (plogRef))
		*plogRef = NULL;

	PGPValidatePtr (plogRef);
	PFLValidateFileSpec (logFileSpec);

	memoryMgr = PFLGetFileSpecMemoryMgr (logFileSpec);
	
	pgpAssert (PGPMemoryMgrIsValid (memoryMgr));
	if (!PGPMemoryMgrIsValid (memoryMgr))
		return kPGPError_BadParams;

	*plogRef = (PGPnetLogRef) PGPNewData (memoryMgr, 
								sizeof(PGPnetLog), 
								kPGPMemoryMgrFlags_Clear);

	pgpAssert (IsntNull (*plogRef));
	if (IsNull (*plogRef))
		return kPGPError_OutOfMemory;

	(*plogRef)->memoryMgr			= memoryMgr;
	(*plogRef)->logFileSpec			= NULL;
	(*plogRef)->logIO				= NULL;
	(*plogRef)->recLength			= sizeof(PGPnetLogEvent);

	/* to be changed? */
	(*plogRef)->numRecords			= kPGPnetLogMaxNumberOfEvents;

	(*plogRef)->dataLength			= (*plogRef)->numRecords * 
										(*plogRef)->recLength;

	(*plogRef)->fileLength			= (*plogRef)->dataLength +
										kPGPNetLogHeaderLength;

	(*plogRef)->currentFilePosition	= 0;

	err = PFLCopyFileSpec (logFileSpec, &((*plogRef)->logFileSpec));
	if (IsPGPError (err))
	{
		sFreeLog (*plogRef);
		*plogRef = NULL;
		return err;
	}

	/* Create the file if it does not exist or 
	   if it is a different version and we have write access */
	err = sCheckForCurrentLogFile (*plogRef, bWriteAccess);
	if ((err == kPGPError_FileNotFound) ||
		(IsPGPError (err) && bWriteAccess))
	{
		PGPnetLogFileHeader header;

		if( err == kPGPError_FileNotFound )
			PFLFileSpecCreate ((*plogRef)->logFileSpec);

		err = PGPSetFileSize ((*plogRef)->logFileSpec, 
								(*plogRef)->fileLength);

		if (IsntPGPError (err))
		{

			err = PGPOpenFileSpec((*plogRef)->logFileSpec, 
						kPFLFileOpenFlags_ReadWrite, 
						(PGPFileIORef*)(&((*plogRef)->logIO)));

			if (IsntPGPError (err))
			{
				/* initialize header and write it */
				pgpClearMemory (&header, sizeof(header));

				header.logfileFormatVersion = kPGPnetLogFileFormatVersion;
				header.recordLengthBytes = sizeof(PGPnetLogEvent);
				header.fileLengthRecords = (*plogRef)->numRecords;

				PGPIOSetPos ((*plogRef)->logIO, 0);

				err = PGPIOWrite ((*plogRef)->logIO, 
									kPGPNetLogHeaderLength, &header);

				/* we will need to wipe the entire file */
				bZeroOutFile = TRUE;
			}
		}

		PGPFreeIO ((*plogRef)->logIO);
	}
	
	pgpAssertNoErr (err);
	if (IsPGPError (err))
	{
		sFreeLog (*plogRef);
		*plogRef = NULL;
		return err;
	}


	/* open the file */
	if (bWriteAccess)
		err = PGPOpenFileSpec(	(*plogRef)->logFileSpec, 
							kPFLFileOpenFlags_ReadWrite, 
							(PGPFileIORef*)(&((*plogRef)->logIO)));
	else
		err = PGPOpenFileSpec(	(*plogRef)->logFileSpec, 
							kPFLFileOpenFlags_ReadOnly, 
							(PGPFileIORef*)(&((*plogRef)->logIO)));

	if (IsPGPError(err))
	{
		sFreeLog (*plogRef);
		*plogRef = NULL;
		return err;
	}
	
	pgpAssert (PGPIOIsValid ((*plogRef)->logIO));
	if (!PGPIOIsValid ((*plogRef)->logIO))
	{
		sFreeLog (*plogRef);
		*plogRef = NULL;
		return kPGPError_OutOfMemory;
	}

	/* zero-out file */
	if (bZeroOutFile)
		PGPnetClearLogFile (*plogRef);

	/* otherwise find starting position */
	else
	{
		PGPUInt32		currPos				= kPGPNetLogHeaderLength;
		PGPTime			timeOfPrevEvent;

		PGPIOSetPos ((*plogRef)->logIO, kPGPNetLogHeaderLength);

		event.timeOfEvent = 0;
		do {
			timeOfPrevEvent = event.timeOfEvent;

			err = sIOReadEvent ((*plogRef)->logIO, &event);
			if (IsPGPError (err))
				return err;
			currPos += sizeof(PGPnetLogEvent);

		} while ((event.timeOfEvent != 0) &&
				 (event.timeOfEvent >= timeOfPrevEvent) &&
				 (currPos < (*plogRef)->fileLength));

		if (currPos >= (*plogRef)->fileLength)
			currPos = kPGPNetLogHeaderLength;
		else
			currPos -= sizeof(PGPnetLogEvent);

		PGPIOSetPos ((*plogRef)->logIO, currPos);
		(*plogRef)->currentFilePosition	= currPos;
	}

	return err;
}


PGPError 
PGPnetLogEventToFile (
		PGPnetLogRef 	logRef,
		PGPnetLogEvent*	pevent)
{
	PGPTime		time			= 0;
	PGPError	err;

	PGPValidateLog (logRef);
	PGPValidatePtr (pevent);

	if (logRef)
	{
		if (pevent->timeOfEvent == 0)
			pevent->timeOfEvent = PGPGetTime ();

		err = sIOWriteEvent (logRef->logIO, pevent);
		if (IsPGPError (err))
			return err;

		logRef->currentFilePosition += sizeof(PGPnetLogEvent);
		if (logRef->currentFilePosition >= logRef->fileLength)
		{
			logRef->currentFilePosition = kPGPNetLogHeaderLength;
			PGPIOSetPos (logRef->logIO, kPGPNetLogHeaderLength);
		}

		return err;
	}
	else 
		return kPGPError_BadParams;
}


PGPError 
PGPnetCloseLogFile (
		PGPnetLogRef	logRef)
{
	PGPValidateLog (logRef);

	if (logRef)
	{
		sFreeLog (logRef);
		return kPGPError_NoErr;
	}
	else
		return kPGPError_BadParams;
}



PGPError 
PGPnetReadLogData (
		PGPnetLogRef	logRef, 
		PGPSize*		pnumberEvents, 
		void**			ppEventBuffer)
{
	PGPUInt32		currPos			= kPGPNetLogHeaderLength;
	PGPUInt32		startPos		= kPGPNetLogHeaderLength;
	PGPUInt32		endPos			= kPGPNetLogHeaderLength;
	PGPUInt32		bufferPos		= 0;

	PGPSize			sizeBuffer;
	PGPByte*		pBuffer;
	PGPnetLogEvent	event;
	PGPTime			timeOfPrevEvent;
	PGPError		err;

	PGPValidatePtr (pnumberEvents);
	PGPValidatePtr (ppEventBuffer);

	*pnumberEvents = 0;
	*ppEventBuffer = NULL;

	PGPValidateLog (logRef);

	if (!logRef)
		return kPGPError_BadParams;

	PGPIOSetPos (logRef->logIO, kPGPNetLogHeaderLength);

	/* search for decreasing time, marking end of log data */
	event.timeOfEvent = 0;
	do {
		timeOfPrevEvent = event.timeOfEvent;

		err = sIOReadEvent (logRef->logIO, &event);
		if (IsPGPError (err))
			return err;
		currPos += sizeof(PGPnetLogEvent);

	} while ((event.timeOfEvent != 0) &&
			 (event.timeOfEvent >= timeOfPrevEvent) &&
			 (currPos < logRef->fileLength));

	if (event.timeOfEvent == 0)
	{
		startPos = kPGPNetLogHeaderLength;
		endPos = currPos;
	}
	else if (event.timeOfEvent < timeOfPrevEvent)
	{
		currPos -= sizeof(PGPnetLogEvent);
		startPos = currPos;
		endPos = currPos;
	}
	else 
	{
		startPos = kPGPNetLogHeaderLength;
		endPos = kPGPNetLogHeaderLength;
	}

	/* calculate size of buffer needed and allocate it */
	if (startPos < endPos)
		sizeBuffer = endPos - startPos - sizeof(PGPnetLogEvent);
	else
		sizeBuffer = logRef->numRecords * sizeof(PGPnetLogEvent);

	if (sizeBuffer == 0)
		return kPGPError_NoErr;

	pBuffer = (PGPByte *)PGPNewData (logRef->memoryMgr, 
								sizeBuffer, 
								kPGPMemoryMgrFlags_Clear);

	/* read the data from the file */
	PGPIOSetPos (logRef->logIO, startPos);
	currPos = startPos;

	while (bufferPos < sizeBuffer)
	{
		err = sIOReadEvent (logRef->logIO, 
						(PGPnetLogEvent*)&pBuffer[bufferPos]);
		if (IsPGPError (err))
			return err;
		(*pnumberEvents)++;
		bufferPos += sizeof(PGPnetLogEvent);
		currPos += sizeof(PGPnetLogEvent);
		if (currPos >= logRef->fileLength) 
		{
			PGPIOSetPos (logRef->logIO, kPGPNetLogHeaderLength);
			currPos = kPGPNetLogHeaderLength;
		}
	}

	*ppEventBuffer = pBuffer;

	return kPGPError_NoErr;
}


PGPError 
PGPnetFreeLogData (
		void*	pData)
{
	PGPValidatePtr (pData);

	PGPFreeData (pData);
	return kPGPError_NoErr;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/



