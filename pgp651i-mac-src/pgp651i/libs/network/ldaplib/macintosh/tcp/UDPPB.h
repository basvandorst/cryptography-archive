/*	File:		UDPPB.h	Copyright:	(C) 1984-1993 by Apple Computer, Inc., all rights reserved.	WARNING	This file was auto generated by the interfacer tool. Modifications	must be made to the master file.*/#ifndef __UDPPB__#define __UDPPB__#ifndef __MACTCPCOMMONTYPES__#include <MacTCPCommonTypes.h>#endif#define UDPCreate		20#define UDPRead			21#define UDPBfrReturn	22#define UDPWrite		23#define UDPRelease		24#define UDPMaxMTUSize	25#define UDPStatus		26#define UDPMultiCreate	27#define UDPMultiSend	28#define UDPMultiRead	29#define UDPCtlMax		29enum UDPEventCode {	UDPDataArrival		= 1,	UDPICMPReceived,	lastUDPEvent		= 32767};typedef enum UDPEventCode UDPEventCode;typedef pascal void (*UDPNotifyProcPtr)(StreamPtr udpStream, unsigned short eventCode, Ptr userDataPtr, struct ICMPReport *icmpMsg);enum {	uppUDPNotifyProcInfo = kPascalStackBased		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(StreamPtr)))		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(unsigned short)))		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Ptr)))		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(struct ICMPReport*)))};#if TARGET_RT_MAC_CFMtypedef UniversalProcPtr UDPNotifyUPP;#define CallUDPNotifyProc(userRoutine, udpStream, eventCode, userDataPtr, icmpMsg)		\		CallUniversalProc((UniversalProcPtr)userRoutine, uppUDPNotifyProcInfo, udpStream, eventCode, userDataPtr, icmpMsg)#define NewUDPNotifyProc(userRoutine)		\		(UDPNotifyUPP) NewRoutineDescriptor((ProcPtr)userRoutine, uppUDPNotifyProcInfo, GetCurrentISA())#elsetypedef UDPNotifyProcPtr UDPNotifyUPP;#define CallUDPNotifyProc(userRoutine, udpStream, eventCode, userDataPtr, icmpMsg)		\		(*userRoutine)(udpStream, eventCode, userDataPtr, icmpMsg)#define NewUDPNotifyProc(userRoutine)		\		(UDPNotifyUPP)(userRoutine)#endiftypedef UDPNotifyProcPtr UDPNotifyProc;typedef void (*UDPIOCompletionProcPtr)(struct UDPiopb *iopb);enum {	uppUDPIOCompletionProcInfo = kCStackBased		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(struct UDPiopb*)))};#if TARGET_RT_MAC_CFMtypedef UniversalProcPtr UDPIOCompletionUPP;#define CallUDPIOCompletionProc(userRoutine, iopb)		\		CallUniversalProc((UniversalProcPtr)userRoutine, uppUDPIOCompletionProcInfo, iopb)#define NewUDPIOCompletionProc(userRoutine)		\		(UDPIOCompletionUPP) NewRoutineDescriptor((ProcPtr)userRoutine, uppUDPIOCompletionProcInfo, GetCurrentISA())#elsetypedef UDPIOCompletionProcPtr UDPIOCompletionUPP;#define CallUDPIOCompletionProc(userRoutine, iopb)		\		(*userRoutine)(iopb)#define NewUDPIOCompletionProc(userRoutine)		\		(UDPIOCompletionUPP)(userRoutine)#endiftypedef UDPIOCompletionProcPtr UDPIOCompletionProc;typedef unsigned short udp_port;#if defined(powerc) || defined (__powerc)#pragma options align=mac68k#endifstruct UDPCreatePB {		/* for create and release calls */	Ptr							rcvBuff;	unsigned long				rcvBuffLen;	UDPNotifyProc				notifyProc;	unsigned short				localPort;	Ptr							userDataPtr;	udp_port					endingPort;};#if defined(powerc) || defined(__powerc)#pragma options align=reset#endiftypedef struct UDPCreatePB UDPCreatePB;#if defined(powerc) || defined (__powerc)#pragma options align=mac68k#endifstruct UDPSendPB {	unsigned short				reserved;	ip_addr						remoteHost;	udp_port					remotePort;	Ptr							wdsPtr;	Boolean						checkSum;	unsigned short				sendLength;	Ptr							userDataPtr;	udp_port					localPort;};#if defined(powerc) || defined(__powerc)#pragma options align=reset#endiftypedef struct UDPSendPB UDPSendPB;#if defined(powerc) || defined (__powerc)#pragma options align=mac68k#endifstruct UDPReceivePB {		/* for receive and buffer return calls */	unsigned short				timeOut;	ip_addr						remoteHost;	udp_port					remotePort;	Ptr							rcvBuff;	unsigned short				rcvBuffLen;	unsigned short				secondTimeStamp;	Ptr							userDataPtr;	ip_addr						destHost;		/* only for use with multi rcv */	udp_port					destPort;		/* only for use with multi rcv */};#if defined(powerc) || defined(__powerc)#pragma options align=reset#endiftypedef struct UDPReceivePB UDPReceivePB;#if defined(powerc) || defined (__powerc)#pragma options align=mac68k#endifstruct UDPMTUPB {	unsigned short				mtuSize;	ip_addr						remoteHost;	Ptr							userDataPtr;};#if defined(powerc) || defined(__powerc)#pragma options align=reset#endiftypedef struct UDPMTUPB UDPMTUPB;#if defined(powerc) || defined (__powerc)#pragma options align=mac68k#endifstruct UDPiopb {	char						fill12[12];	UDPIOCompletionProc			ioCompletion;	short						ioResult;	char						*ioNamePtr;	short						ioVRefNum;	short						ioCRefNum;	short						csCode;	StreamPtr					udpStream;	union {		struct UDPCreatePB		create;		struct UDPSendPB		send;		struct UDPReceivePB		receive;		struct UDPMTUPB			mtu;	} csParam;};#if defined(powerc) || defined(__powerc)#pragma options align=reset#endiftypedef struct UDPiopb UDPiopb;#endif