/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: pgpNetLogUtils.cpp,v 1.5.6.1 1999/06/14 03:22:38 elowe Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <winsock2.h>
#include <windows.h>

#include "pgpErrors.h"

#include "pgpNetLogUtils.h"

static char *tmpbuf = 0;

// local static functions
static PGPBoolean isWSAerror(PGPInt32 errnum);
static PGPBoolean isANSIerror(PGPInt32 errnum);
static const char *wsaFormatMessage(PGPInt32 errnum, PGPInt32 *len);

void
voutputToDebugger(const char *fmt,
				  va_list args, 
				  PGPBoolean eol /* = TRUE */, 
				  const char *prepend /* = NULL */)
{
	int size = 1024;
	char *buf = new char[size];

	while (_vsnprintf(buf, size-1, fmt, args) < 0) {
		delete [] buf;
		size = size * 2;
		buf = new char[size];
	}
	
	if (eol)
		strcat(buf, "\n");
	if (prepend)
		OutputDebugString(prepend);
	OutputDebugString(buf);
	delete [] buf;
}

void
outputToDebugger(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	voutputToDebugger(fmt, args, FALSE, NULL);
	va_end(args);
}

void
vprogerr(const char *fmt,
		 va_list args,
		 FILE *stream,
		 PGPBoolean eol /* = TRUE */,
		 const char *prepend /* = NULL */)
{	
	if (stream) {
		time_t secs;
		time(&secs);
		fprintf(stream, "%15.15s ", ctime(&secs)+4);
		if (prepend)
			fprintf(stream, prepend);
		vfprintf(stream, fmt, args);
		if (eol)
			fprintf(stream, "\n");
		fflush(stream);
	}
}

PGPError
myStrerror(PGPInt32 errnum, char *buf, PGPInt32 *len)
{
	if (buf && *len)
		memset(buf, '\0', *len);

	char *tmpbuf = 0;
	PGPInt32 tmplen = 0;
	
	if (isWSAerror(errnum)) {
		tmpbuf = const_cast<char*>(wsaFormatMessage(errnum, len));
		if (!tmpbuf || !buf) {
			// buf wasn't big enough for the err message
			return kPGPError_BufferTooSmall;
		}
		strncpy(buf, tmpbuf, *len);
		return kPGPError_NoErr;
	}
	
	if (isANSIerror(errnum)) {
		tmpbuf = strerror(errnum);
		tmplen = strlen(tmpbuf);
		if (tmplen > *len || !buf) {
			*len = tmplen;
			return kPGPError_BufferTooSmall;
		}
		*len = tmplen;
		strncpy(buf, tmpbuf, *len);
		return kPGPError_NoErr;
	}
	
	
	PGPUInt32 rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
								 FORMAT_MESSAGE_ALLOCATE_BUFFER,
								 NULL,
								 errnum,
								 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								 (LPTSTR) &tmpbuf,
								 0,
								 NULL);
	if (rc == 0) {
		if (IsPGPError(errnum)) {
			PGPError err;
			PGPInt16 i = 1;
			do {
				tmplen = *len * i++;
				if (tmpbuf)
					delete [] tmpbuf;
				tmpbuf = new char[tmplen];
				err = PGPGetErrorString(errnum, tmplen, tmpbuf);
			} while (err == kPGPError_BufferTooSmall);

			tmplen = strlen(tmpbuf);
			if (tmplen > *len || !buf) {
				*len = tmplen;
				delete [] tmpbuf;
				return kPGPError_BufferTooSmall;
			}
			*len = tmplen;
			strncpy(buf, tmpbuf, *len);
			delete [] tmpbuf;
			return kPGPError_NoErr;
		}
	}
	
	tmplen = strlen(tmpbuf);
	if (tmplen > *len || !buf) {
		*len = tmplen;
		LocalFree(tmpbuf);
		return kPGPError_BufferTooSmall;
	}
	*len = tmplen;
	strncpy(buf, tmpbuf, *len);
	LocalFree(tmpbuf);
	return kPGPError_NoErr;
}

/*
 * Formats a WSAGetLastError() code into a string.
 * Returns -  error string on success, NULL on failure.
 *  Author: Alfredo Botelho
 *  Date: 22/Oct/'97
 */
const char *
wsaFormatMessage(PGPInt32 errnum, PGPInt32 *len)
{
	const char *errStr = 0;
	PGPInt32 tmplen = 0;
	
    switch (errnum)
    {
    case WSAEACCES:
		errStr = "Permission denied.";
		break;

    case WSAEADDRINUSE:
		errStr =  "Address already in use.";
		break;

    case WSAEADDRNOTAVAIL:
		errStr =  "Cannot assign requested address.";
		break;

    case WSAEAFNOSUPPORT:
		errStr =  "Address family not supported by protocol family.";
		break;
	
    case WSAEALREADY:
		errStr =  "Operation already in progress.";
		break;
		
    case WSAECONNABORTED:
		errStr =  "Software caused connection abort.";
		break;
		
    case WSAECONNREFUSED:
		errStr =  "Connection refused.";
		break;
		
    case WSAECONNRESET:
		errStr =  "Connection reset by peer.";
		break;

    case WSAEDESTADDRREQ:
		errStr =  "Destination address required.";
		break;

    case WSAEFAULT:
		errStr =  "Bad address.";
		break;

    case WSAEHOSTDOWN:
		errStr =  "Host is down.";
		break;

    case WSAEHOSTUNREACH:
		errStr =  "No route to host.";
		break;

    case WSAEINPROGRESS:
		errStr =  "Operation now in progress.";
		break;

    case WSAEINTR:
		errStr =  "Interrupted function call.";
		break;

    case WSAEINVAL:
		errStr =  "Invalid argument.";
		break;

    case WSAEISCONN:
		errStr =  "Socket is already connected.";
		break;

    case WSAEMFILE:
		errStr =  "Too many open files.";
		break;

    case WSAEMSGSIZE:
		errStr =  "Message too long.";
		break;

    case WSAENETDOWN:
		errStr =  "Network is down.";
		break;

    case WSAENETRESET:
		errStr =  "Network dropped connection on reset.";
		break;

    case WSAENETUNREACH:
		errStr =  "Network is unreachable.";
		break;

    case WSAENOBUFS:
		errStr =  "No buffer space available.";
		break;

    case WSAENOPROTOOPT:
		errStr =  "Bad protocol option.";
		break;

    case WSAENOTCONN:
		errStr =  "Socket is not connected.";
		break;

    case WSAENOTSOCK:
		errStr =  "Socket operation on non-socket.";
		break;

    case WSAEOPNOTSUPP:
		errStr =  "Operation not supported.";
		break;

    case WSAEPFNOSUPPORT:
		errStr =  "Protocol family not supported.";
		break;

    case WSAEPROCLIM:
		errStr =  "Too many processes.";
		break;

    case WSAEPROTONOSUPPORT:
		errStr =  "Protocol not supported.";
		break;

    case WSAEPROTOTYPE:
		errStr =  "Protocol wrong type for socket.";
		break;

    case WSAESHUTDOWN:
		errStr =  "Cannot send after socket shutdown.";
		break;

    case WSAESOCKTNOSUPPORT:
		errStr =  "Socket type not supported.";
		break;

    case WSAETIMEDOUT:
		errStr =  "Connection timed out.";
		break;

    case WSAEWOULDBLOCK:
		errStr =  "Resource temporarily unavailable.";
		break;

    case WSAHOST_NOT_FOUND:
		errStr =  "Host not found.";
		break;

    case WSA_INVALID_HANDLE:
		errStr =  "Specified event object handle is invalid.";
		break;

    case WSA_INVALID_PARAMETER:
		errStr =  "One or more parameters are invalid.";
		break;

    case WSA_IO_PENDING:
		errStr =  "Overlapped operations will complete later.";
		break;

    case WSA_IO_INCOMPLETE:
		errStr =  "Overlapped I/O event object not in signaled state.";
		break;

    case WSA_NOT_ENOUGH_MEMORY:
		errStr =  "Insufficient memory available.";
		break;

    case WSANOTINITIALISED:
		errStr =  "Successful WSAStartup not yet performed.";
		break;

    case WSANO_DATA:
		errStr =  "Valid name, no data record of requested type.";
		break;

    case WSANO_RECOVERY:
		errStr =  "This is a non-recoverable error.";
		break;

    case WSASYSCALLFAILURE:
		errStr =  "System call failure.";
		break;

    case WSASYSNOTREADY:
		errStr =  "Network subsystem is unavailable.";
		break;

    case WSATRY_AGAIN:
		errStr =  "Non-authoritative host not found.";
		break;

    case WSAVERNOTSUPPORTED:
		errStr =  "WINSOCK.DLL version out of range.";
		break;

    case WSAEDISCON:
		errStr =  "Graceful shutdown in progress.";
		break;

    case WSA_OPERATION_ABORTED:
		errStr =  "Overlapped operation aborted.";
		break;

    default:
        errStr =  "unknown network error";
		break;
    }
	tmplen = strlen(errStr);
	if (tmplen > *len) {
		errStr = NULL;
	}
	*len = tmplen;
	return errStr;
}

/*
 * Formats a WSAGetLastError() code into a string.
 * Returns -  error string on success, NULL on failure.
 *  Author: Alfredo Botelho                  Date: 22/Oct/'97
 */
PGPBoolean
isWSAerror(PGPInt32 errnum)
{
    switch (errnum) {
	case WSAEACCES:
    case WSAEADDRINUSE: 
    case WSAEADDRNOTAVAIL: 
    case WSAEAFNOSUPPORT: 
    case WSAEALREADY: 
    case WSAECONNABORTED: 
    case WSAECONNREFUSED: 
    case WSAECONNRESET:
    case WSAEDESTADDRREQ: 
    case WSAEFAULT: 
    case WSAEHOSTDOWN: 
    case WSAEHOSTUNREACH: 
    case WSAEINPROGRESS: 
    case WSAEINTR:
    case WSAEINVAL: 
    case WSAEISCONN: 
    case WSAEMFILE: 
    case WSAEMSGSIZE: 
    case WSAENETDOWN: 
    case WSAENETRESET: 
    case WSAENETUNREACH: 
    case WSAENOBUFS: 
    case WSAENOPROTOOPT: 
    case WSAENOTCONN: 
    case WSAENOTSOCK: 
    case WSAEOPNOTSUPP: 
    case WSAEPFNOSUPPORT: 
    case WSAEPROCLIM: 
    case WSAEPROTONOSUPPORT: 
    case WSAEPROTOTYPE: 
    case WSAESHUTDOWN: 
    case WSAESOCKTNOSUPPORT:
    case WSAETIMEDOUT: 
    case WSAEWOULDBLOCK: 
    case WSAHOST_NOT_FOUND:
    case WSA_INVALID_HANDLE: 
    case WSA_INVALID_PARAMETER:
    case WSA_IO_PENDING:
    case WSA_IO_INCOMPLETE:
    case WSA_NOT_ENOUGH_MEMORY:
    case WSANOTINITIALISED:
    case WSANO_DATA:
    case WSANO_RECOVERY:
    case WSASYSCALLFAILURE:
    case WSASYSNOTREADY: 
    case WSATRY_AGAIN: 
    case WSAVERNOTSUPPORTED: 
    case WSAEDISCON:
    case WSA_OPERATION_ABORTED:
		return TRUE;

    default:
        return FALSE;
    }
}

PGPBoolean
isANSIerror(PGPInt32 errnum)
{
	switch (errnum) {
	case 2:
	case 7:
	case 8:
	case 9:
	case 12:
	case 13:
	case 17:
	case 18:
	case 22:
	case 24:
	case 28:
	case 33:
	case 34:
	case 36:
		return TRUE;
	default:
		return FALSE;
	}
}
