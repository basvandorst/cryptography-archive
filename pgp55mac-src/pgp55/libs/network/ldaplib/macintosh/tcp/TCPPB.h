/*
	File:		TCPPB.h

	Copyright:	(C) 1984-1993 by Apple Computer, Inc., all rights reserved.

	WARNING
	This file was auto generated by the interfacer tool. Modifications
	must be made to the master file.

*/

#ifndef __TCPPB__
#define __TCPPB__

#ifndef __MACTCPCOMMONTYPES__
#include <MacTCPCommonTypes.h>
#endif

#define TCPCreate			30
#define TCPPassiveOpen		31
#define TCPActiveOpen		32
#define TCPSend				34
#define TCPNoCopyRcv		35
#define TCPRcvBfrReturn		36
#define TCPRcv				37
#define TCPClose			38
#define TCPAbort			39
#define TCPStatus			40
#define TCPExtendedStat		41
#define TCPRelease			42
#define TCPGlobalInfo		43
#define TCPCtlMax			49

enum TCPEventCode {
	TCPClosing				= 1,
	TCPULPTimeout,
	TCPTerminate,
	TCPDataArrival,
	TCPUrgent,
	TCPICMPReceived,
	lastEvent				= 32767
};

typedef enum TCPEventCode TCPEventCode;

enum TCPTerminationReason {
	TCPRemoteAbort			= 2,
	TCPNetworkFailure,
	TCPSecPrecMismatch,
	TCPULPTimeoutTerminate,
	TCPULPAbort,
	TCPULPClose,
	TCPServiceError,
	lastReason				= 32767
};

// typedef TCPTerminationReason TCPTerminationReason;

typedef pascal void (*TCPNotifyProcPtr)(StreamPtr tcpStream, unsigned short eventCode, Ptr userDataPtr, unsigned short terminReason, struct ICMPReport *icmpMsg);

enum {
	uppTCPNotifyProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(StreamPtr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(unsigned short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(unsigned short)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(struct ICMPReport*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr TCPNotifyUPP;

#define CallTCPNotifyProc(userRoutine, tcpStream, eventCode, userDataPtr, terminReason, icmpMsg)		\
		CallUniversalProc((UniversalProcPtr)userRoutine, uppTCPNotifyProcInfo, tcpStream, eventCode, userDataPtr, terminReason, icmpMsg)
#define NewTCPNotifyProc(userRoutine)		\
		(TCPNotifyUPP) NewRoutineDescriptor((ProcPtr)userRoutine, uppTCPNotifyProcInfo, GetCurrentISA())
#else
typedef TCPNotifyProcPtr TCPNotifyUPP;

#define CallTCPNotifyProc(userRoutine, tcpStream, eventCode, userDataPtr, terminReason, icmpMsg)		\
		(*userRoutine)(tcpStream, eventCode, userDataPtr, terminReason, icmpMsg)
#define NewTCPNotifyProc(userRoutine)		\
		(TCPNotifyUPP)(userRoutine)
#endif

typedef TCPNotifyProcPtr TCPNotifyProc;

typedef void (*TCPIOCompletionProcPtr)(struct TCPiopb *iopb);

enum {
	uppTCPIOCompletionProcInfo = kCStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(struct TCPiopb*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr TCPIOCompletionUPP;

#define CallTCPIOCompletionProc(userRoutine, iopb)		\
		CallUniversalProc((UniversalProcPtr)userRoutine, uppTCPIOCompletionProcInfo, iopb)
#define NewTCPIOCompletionProc(userRoutine)		\
		(TCPIOCompletionUPP) NewRoutineDescriptor((ProcPtr)userRoutine, uppTCPIOCompletionProcInfo, GetCurrentISA())
#else
typedef TCPIOCompletionProcPtr TCPIOCompletionUPP;

#define CallTCPIOCompletionProc(userRoutine, iopb)		\
		(*userRoutine)(iopb)
#define NewTCPIOCompletionProc(userRoutine)		\
		(TCPIOCompletionUPP)(userRoutine)
#endif

typedef TCPIOCompletionProcPtr TCPIOCompletionProc;

typedef unsigned short tcp_port;

typedef unsigned char byte;

enum  {					/* ValidityFlags */
	timeoutValue				= 0x80,
	timeoutAction				= 0x40,
	typeOfService				= 0x20,
	precedence					= 0x10
};

enum  {					/* TOSFlags */
	lowDelay					= 0x01,
	throughPut					= 0x02,
	reliability					= 0x04
};

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPCreatePB {
	Ptr							rcvBuff;
	unsigned long				rcvBuffLen;
	TCPNotifyUPP				notifyProc;
	Ptr							userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPCreatePB TCPCreatePB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPOpenPB {
	byte						ulpTimeoutValue;
	byte						ulpTimeoutAction;
	byte						validityFlags;
	byte						commandTimeoutValue;
	ip_addr						remoteHost;
	tcp_port					remotePort;
	ip_addr						localHost;
	tcp_port					localPort;
	byte						tosFlags;
	byte						precedence;
	Boolean						dontFrag;
	byte						timeToLive;
	byte						security;
	byte						optionCnt;
	byte						options[40];
	Ptr							userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPOpenPB TCPOpenPB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPSendPB {
	byte						ulpTimeoutValue;
	byte						ulpTimeoutAction;
	byte						validityFlags;
	Boolean						pushFlag;
	Boolean						urgentFlag;
	Ptr							wdsPtr;
	unsigned long				sendFree;
	unsigned short				sendLength;
	Ptr							userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPSendPB TCPSendPB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPReceivePB {
	byte						commandTimeoutValue;
	byte						filler;
	Boolean						markFlag;
	Boolean						urgentFlag;
	Ptr							rcvBuff;
	unsigned short				rcvBuffLen;
	Ptr							rdsPtr;
	unsigned short				rdsLength;
	unsigned short				secondTimeStamp;
	Ptr							userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPReceivePB TCPReceivePB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPClosePB {
	byte						ulpTimeoutValue;
	byte						ulpTimeoutAction;
	byte						validityFlags;
	Ptr							userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPClosePB TCPClosePB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct HistoBucket {
	unsigned short				value;
	unsigned long				counter;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct HistoBucket HistoBucket;

#define NumOfHistoBuckets 7

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPConnectionStats {
	unsigned long				dataPktsRcvd;
	unsigned long				dataPktsSent;
	unsigned long				dataPktsResent;
	unsigned long				bytesRcvd;
	unsigned long				bytesRcvdDup;
	unsigned long				bytesRcvdPastWindow;
	unsigned long				bytesSent;
	unsigned long				bytesResent;
	unsigned short				numHistoBuckets;
	struct HistoBucket			sentSizeHisto[NumOfHistoBuckets];
	unsigned short				lastRTT;
	unsigned short				tmrSRTT;
	unsigned short				rttVariance;
	unsigned short				tmrRTO;
	byte						sendTries;
	byte						sourchQuenchRcvd;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPConnectionStats TCPConnectionStats;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPStatusPB {
	byte						ulpTimeoutValue;
	byte						ulpTimeoutAction;
	long						unused;
	ip_addr						remoteHost;
	tcp_port					remotePort;
	ip_addr						localHost;
	tcp_port					localPort;
	byte						tosFlags;
	byte						precedence;
	byte						connectionState;
	unsigned short				sendWindow;
	unsigned short				rcvWindow;
	unsigned short				amtUnackedData;
	unsigned short				amtUnreadData;
	Ptr							securityLevelPtr;
	unsigned long				sendUnacked;
	unsigned long				sendNext;
	unsigned long				congestionWindow;
	unsigned long				rcvNext;
	unsigned long				srtt;
	unsigned long				lastRTT;
	unsigned long				sendMaxSegSize;
	struct TCPConnectionStats	*connStatPtr;
	Ptr							userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPStatusPB TCPStatusPB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPAbortPB {
	Ptr							userDataPtr;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPAbortPB TCPAbortPB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPParam {
	unsigned long				tcpRtoA;
	unsigned long				tcpRtoMin;
	unsigned long				tcpRtoMax;
	unsigned long				tcpMaxSegSize;
	unsigned long				tcpMaxConn;
	unsigned long				tcpMaxWindow;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPParam TCPParam;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPStats {
	unsigned long				tcpConnAttempts;
	unsigned long				tcpConnOpened;
	unsigned long				tcpConnAccepted;
	unsigned long				tcpConnClosed;
	unsigned long				tcpConnAborted;
	unsigned long				tcpOctetsIn;
	unsigned long				tcpOctetsOut;
	unsigned long				tcpOctetsInDup;
	unsigned long				tcpOctetsRetrans;
	unsigned long				tcpInputPkts;
	unsigned long				tcpOutputPkts;
	unsigned long				tcpDupPkts;
	unsigned long				tcpRetransPkts;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPStats TCPStats;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPGlobalInfoPB {
	struct TCPParam				*tcpParamPtr;
	struct TCPStats				*tcpStatsPtr;
	StreamPtr					*tcpCDBTable[1];
	Ptr							userDataPtr;
	unsigned short				maxTCPConnections;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPGlobalInfoPB TCPGlobalInfoPB;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct TCPiopb {
	char						fill12[12];
	TCPIOCompletionProc			ioCompletion;
	short						ioResult;
	char						*ioNamePtr;
	short						ioVRefNum;
	short						ioCRefNum;
	short						csCode;
	StreamPtr					tcpStream;
	union {
		struct TCPCreatePB			create;
		struct TCPOpenPB			open;
		struct TCPSendPB			send;
		struct TCPReceivePB			receive;
		struct TCPClosePB			close;
		struct TCPAbortPB			abort;
		struct TCPStatusPB			status;
		struct TCPGlobalInfoPB		globalInfo;
	} csParam;
};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct TCPiopb TCPiopb;

#endif
