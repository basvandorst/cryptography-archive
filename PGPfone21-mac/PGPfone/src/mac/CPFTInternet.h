/*____________________________________________________________________________	Copyright (C) 1996-1999 Network Associates, Inc.	All rights reserved.	$Id: CPFTInternet.h,v 1.6 1999/03/10 02:36:18 heller Exp $____________________________________________________________________________*/#pragma once#include "CPFTransport.h"#include <OpenTransport.h>#include <OpenTptInternet.h>#include <MacTCP.h>#include <LThread.h>#define CONTROLPORTNUMBER		17447	// PGPfone's conference control port#define RTPPORTNUMBER			7448	// RTP's default port#define RTCPPORTNUMBER			7449	// RTCP's default port#define	MAXINCBUFFERS			32		// Rotating packet buffers#define MAXPACKETBUFSIZE		1152#define UDPRCVBUFLEN			8192#define MAXPGPFUDPSIZE			1024	//arbitrary, to avoid fragmentation#define MAXUDPRINGS				6		//note: this number and the next must be synchronized#define MAXUDPCALLTRIES			17		//to avoid overringing.class CPFWindow;class CPFTInternet;typedef struct EndpointCallbackInfo{	CPFTInternet *cpfi;	EndpointRef ep;} EndpointCallbackInfo;class CPFTInternet		:	public CPFTransport{public:					CPFTInternet();					CPFTInternet(CPFWindow *cpfWindow, LThread *thread, short *result);					~CPFTInternet();	PGErr			Connect(ContactEntry *con, short *connectResult);	PGErr			Disconnect();	PGErr			Listen(Boolean answer);	PGErr			Reset();	PGErr			BindMediaStreams();	PGErr			WriteBlock(void *buffer, long *count, short channel);	PGErr			WriteBlockTo(void *buffer, long *count, InetAddress *addr);	PGErr			WriteAsync(long count, short channel, AsyncTransport *async);	long			Read(void *data, long max, short *channel);	void			PrepRead();	void			ReceiveUDPMsg(uchar *msg, long len);	static void		CleanUp();		OTResult		mOTResult;	LThread			*mCThread;	static EndpointRef	sCEndpoint;		// PGPfone conference control protocol	EndpointRef		mMEndpoint,			// Media protocol (RTP)					mMCEndpoint;		// Media control protocol (RTCP)	uchar			mIncoming[MAXINCBUFFERS][MAXPACKETBUFSIZE];	Boolean			mPktReady[MAXINCBUFFERS], mFlowOff;	short			mPktDest[MAXINCBUFFERS], mPktLen[MAXINCBUFFERS], mNextBuffer, mLastBuffer;	InetAddress		mPktSrc[MAXINCBUFFERS];private:	OSErr			MakeOTEndpoint(EndpointRef *ep, EndpointCallbackInfo *eci);	static OSErr	CloseOTEndpoint(EndpointRef *ep);	OSStatus		ClearLookErr(EndpointRef endpoint);		EndpointCallbackInfo mCECI, mMECI, mMCECI, mISECI;	InetAddress		mAddress,			// Our address					mRemoteAddress,		// Other party's address					mLastAddress,		// Address we last received a packet from					mMRemoteAddress,	// Address for remote RTP					mMCRemoteAddress;	// Address for remote RTCP	Boolean			mUseOT;	InetSvcRef 		mInetSvc;	UDPiopb			mPB, mRPB;	uchar			mRcvBuf[UDPRCVBUFLEN];	static short	sIPDriverRef;};