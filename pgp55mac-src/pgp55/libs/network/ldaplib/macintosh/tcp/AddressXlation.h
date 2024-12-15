/*
	File:		AddressXlation.h

	Copyright:	(C) 1984-1993 by Apple Computer, Inc., all rights reserved.

	WARNING
	This file was auto generated by the interfacer tool. Modifications
	must be made to the master file.

*/

#ifndef __ADDRESSXLATION__
#define __ADDRESSXLATION__

#ifndef __MACTCPCOMMONTYPES__
#include "MacTCP.h"
#endif

#define NUM_ALT_ADDRS 4

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct hostInfo {
	long						rtnCode;
	char						cname[255];
	unsigned long				addr[NUM_ALT_ADDRS];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct hostInfo hostInfo;

enum AddrClasses {
	A							= 1,
	NS,
	CNAME						= 5,
	HINFO						= 13,
	MX							= 15,
	lastClass					= 32767
};

typedef enum AddrClasses AddrClasses;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct HInfoRec {
	char						cpuType[30];
	char						osType[30];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct HInfoRec HInfoRec;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct MXRec {
	unsigned short				preference;
	char						exchange[255];
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct MXRec MXRec;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct returnRec {
	long						rtnCode;
	char						cname[255];
	union {
		unsigned long				addr[NUM_ALT_ADDRS];
		struct HInfoRec				hinfo;
		struct MXRec				mx;
	} rdata;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct returnRec returnRec;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct cacheEntryRecord {
	char						*cname;
	unsigned short				type;
	unsigned short				cacheClass;
	unsigned long				ttl;
	union {
		char						*name;
		ip_addr						addr;
	} rdata;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct cacheEntryRecord cacheEntryRecord;

#ifdef __cplusplus
extern "C" {
#endif

typedef pascal void (*EnumResultProcPtr)(struct cacheEntryRecord *cacheEntryRecordPtr, char *userDataPtr);

enum {
	uppEnumResultProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(struct cacheEntryRecord*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr EnumResultUPP;

#define CallEnumResultProc(userRoutine, cacheEntryRecordPtr, userDataPtr)		\
		CallUniversalProc((UniversalProcPtr)userRoutine, uppEnumResultProcInfo, cacheEntryRecordPtr, userDataPtr)
#define NewEnumResultProc(userRoutine)		\
		(EnumResultUPP) NewRoutineDescriptor((ProcPtr)userRoutine, uppEnumResultProcInfo, GetCurrentISA())
#else
typedef EnumResultProcPtr EnumResultUPP;

#define CallEnumResultProc(userRoutine, cacheEntryRecordPtr, userDataPtr)		\
		(*userRoutine)(cacheEntryRecordPtr, userDataPtr)
#define NewEnumResultProc(userRoutine)		\
		(EnumResultUPP)(userRoutine)
#endif

typedef pascal void (*ResultProcPtr)(struct hostInfo *hostInfoPtr, char *userDataPtr);

enum {
	uppResultProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(struct hostInfo*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ResultUPP;

#define CallResultProc(userRoutine, hostInfoPtr, userDataPtr)		\
		CallUniversalProc((UniversalProcPtr)userRoutine, uppResultProcInfo, hostInfoPtr, userDataPtr)
#define NewResultProc(userRoutine)		\
		(ResultUPP) NewRoutineDescriptor((ProcPtr)userRoutine, uppResultProcInfo, GetCurrentISA())
#else
typedef ResultProcPtr ResultUPP;

#define CallResultProc(userRoutine, hostInfoPtr, userDataPtr)		\
		(*userRoutine)(hostInfoPtr, userDataPtr)
#define NewResultProc(userRoutine)		\
		(ResultUPP)(userRoutine)
#endif

typedef pascal void (*ResultProc2ProcPtr)(struct returnRec *returnRecPtr, char *userDataPtr);

enum {
	uppResultProc2ProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(struct returnRec*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(char*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ResultProc2UPP;

#define CallResultProc2Proc(userRoutine, returnRecPtr, userDataPtr)		\
		CallUniversalProc((UniversalProcPtr)userRoutine, uppResultProc2ProcInfo, returnRecPtr, userDataPtr)
#define NewResultProc2Proc(userRoutine)		\
		(ResultProc2UPP) NewRoutineDescriptor((ProcPtr)userRoutine, uppResultProc2ProcInfo, GetCurrentISA())
#else
typedef ResultProc2ProcPtr ResultProc2UPP;

#define CallResultProc2Proc(userRoutine, returnRecPtr, userDataPtr)		\
		(*userRoutine)(returnRecPtr, userDataPtr)
#define NewResultProc2Proc(userRoutine)		\
		(ResultProc2UPP)(userRoutine)
#endif

typedef ResultProc2ProcPtr ResultProc2Ptr;

extern OSErr OpenResolver(char *fileName);
extern OSErr StrToAddr(char *hostName, struct hostInfo *hostInfoPtr, ResultUPP ResultProc, char *userDataPtr);
extern OSErr AddrToStr(unsigned long addr, char *addrStr);
extern OSErr EnumCache(EnumResultUPP enumResultProc, char *userDataPtr);
extern OSErr AddrToName(ip_addr addr, struct hostInfo *hostInfoPtr, ResultUPP ResultProc, char *userDataPtr);
extern OSErr HInfo(char *hostName, struct returnRec *returnRecPtr, ResultProc2Ptr resultProc, char *userDataPtr);
extern OSErr MXInfo(char *hostName, struct returnRec *returnRecPtr, ResultProc2Ptr resultProc, char *userDataPtr);
extern OSErr CloseResolver(void);
#ifdef __cplusplus
}
#endif

#endif

