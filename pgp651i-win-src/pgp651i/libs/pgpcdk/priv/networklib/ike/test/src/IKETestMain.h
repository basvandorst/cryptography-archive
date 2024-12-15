/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: IKETestMain.h,v 1.7 1999/01/18 09:37:38 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include <LApplication.h>
#include <LPeriodical.h>
#include <LSimpleThread.h>

#include "PGPSockets.h"
#include "pgpIKE.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"

typedef struct IKETestPacket
{
	IKETestPacket *		next;
	PGPByte				data[2048];
	PGPSize				dataSize;
} IKETestPacket;

class	CIKETestApp :	public LApplication,
						public LPeriodical
{
public:
						CIKETestApp();
	virtual 			~CIKETestApp();
	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);	
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
protected:
	virtual void		StartUp();
	virtual void		SpendTime(const EventRecord&		inMacEvent);

	static PGPError		SocketsCallback(PGPContextRef context,
							struct PGPEvent *	event,
							PGPUserValue		callBackArg);
	static PGPError		IKECallback( PGPikeContextRef ike,
								void *inUserData,
								PGPikeMessageType	msg,
								void *data );
	void				AddPacket(
								PGPBoolean			c1,
								PGPByte *			data,
								PGPSize				dataSize );

	PGPContextRef		mPGPContext;
	PGPikeContextRef	mIKEContext1,
						mIKEContext2;
	PGPikeSA	*		mIPSECSA1,
				*		mIPSECSA2;
	PGPBoolean			mSelfTestMode;
	Uint32				mLocalIP;
	
	PGPByte *			mIncomingPacket;
	IKETestPacket *		mPacketC1;
	IKETestPacket *		mPacketC2;
	
	PGPKeySetRef		mTestKeySet;
	PGPBoolean			mTestKeyX509;
	
	PGPSocketRef		mSocket;
	PGPSocketsThreadStorageRef	mSocketStorage;
};

