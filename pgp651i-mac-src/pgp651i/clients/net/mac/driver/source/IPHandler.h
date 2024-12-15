/*____________________________________________________________________________	Copyright (C) 1999 Network Associates, Inc.	All rights reserved.	$Id: IPHandler.h,v 1.7 1999/03/17 02:05:36 jason Exp $____________________________________________________________________________*/#pragma once#include "pgpIPsecBuffer.h"#include "PGPnetModuleCommon.h"/*PGPBoolean			IPEncodeAndPutNext(queue_t * inQ, const mblk_t * inHeaderBlock,						const mblk_t * inPacketMessage, SGlobals * inGlobals);PGPBoolean			IPDecodeAndPutNext(queue_t * inQ, const mblk_t * inHeaderBlock,						const mblk_t * inPacketMessage, SGlobals * inGlobals);*/PGPBoolean			IPProcessAndPutNext(PGPBoolean inIncoming, queue_t * inQ, const mblk_t * inHeaderBlock,						const mblk_t * inPacketMessage, SGlobals * inGlobals,						SIPReassemblyList ** inReassemblyList);PGPError			IPApplySAToPacket(queue_t * inQ, PGPBoolean inIncoming,						PGPBoolean inTunnel, PGPikeSA * inSA, PGPIPsecBuffer * inBuffers,						mblk_t ** outPacketMessage);void				IPGetSourceAddressAndSPI(PGPIPsecBuffer * inBuffers,						PGPUInt32 * outAddress, PGPUInt32 * outSPI);PGPIPsecBuffer *	IPMessageToIPsecBuffers(const mblk_t * inMessage);