/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: IKETestMain.cp,v 1.42 1999/05/26 10:17:02 wprice Exp $
____________________________________________________________________________*/
#include <stddef.h>
#include <string.h>
#include <OpenTptInternet.h>

#include "IKETestMain.h"
#include "IKETestKey.h"

#include <LGrowZone.h>
#include <LWindow.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <PPobClasses.h>
#include <UDrawingState.h>
#include <UMemoryMgr.h>
#include <URegistrar.h>
#include <LEditField.h>
#include <LAttachable.h>
#include <LSIOUXAttachment.h>
#include <UThread.h>

#include <Sound.h>

#include "pgpMem.h"


const CommandT		cmd_IKERequestSA		=	1000;
const CommandT		cmd_IKESelfTest			=	1001;
const CommandT		cmd_IKEKillSA1			=	1002;
const CommandT		cmd_IKEKillSA2			=	1003;
const CommandT		cmd_IKERekeySA1			=	1004;
const CommandT		cmd_IKERekeySA2			=	1005;

const PGPUInt16		kPGPike_TestPort		=	500;

const PGPInt32		kIKETestInPacketSize	=	65535L;

const PGPUInt32		kIKEFakeLatencyTicks	=	0;

static PGPUInt32			sLastC1Process = 0;
static PGPUInt32			sLastC2Process = 0;

/* draft-ietf-ipsec-internet-key-00.txt */
const char *		kTestSharedKey			= "test";

PGPikeTransform kPGPike_TestIKETransforms[] =
	{
		{
			kPGPike_AM_PreSharedKey,
			kPGPike_HA_MD5,
			kPGPike_SC_3DES_CBC,
			TRUE, kPGPike_GR_MODPTwo
		},
		{
			kPGPike_AM_PreSharedKey,
			kPGPike_HA_SHA1,
			kPGPike_SC_CAST_CBC,
			TRUE, kPGPike_GR_MODPFive
		},
		{
			kPGPike_AM_DSS_Sig,
			kPGPike_HA_SHA1,
			kPGPike_SC_CAST_CBC,
			TRUE, kPGPike_GR_MODPFive
		},
		{
			kPGPike_AM_DSS_Sig,
			kPGPike_HA_SHA1,
			kPGPike_SC_3DES_CBC,
			TRUE, kPGPike_GR_MODPTwo
		},
		{
			kPGPike_AM_RSA_Sig,
			kPGPike_HA_SHA1,
			kPGPike_SC_CAST_CBC,
			TRUE, kPGPike_GR_MODPTwo
		},
		{
			kPGPike_AM_RSA_Sig,
			kPGPike_HA_MD5,
			kPGPike_SC_3DES_CBC,
			TRUE, kPGPike_GR_MODPTwo
		},
	};

PGPipsecTransform kPGPike_TestIPSECTransforms[] =
	{
		{	// ESP, 3DES, MD5
			TRUE, kPGPike_ET_NULL, kPGPike_AA_HMAC_MD5, kPGPike_PM_None,
			FALSE, kPGPike_AH_None, kPGPike_AA_None, kPGPike_PM_None,
			FALSE, kPGPike_IC_None,
			kPGPike_GR_MODPFive
		},
		{	// ESP, 3DES, SHA, DEFLATE, MODP 5
			TRUE, kPGPike_ET_3DES, kPGPike_AA_HMAC_SHA, kPGPike_PM_None,
			TRUE, kPGPike_AH_SHA, kPGPike_AA_HMAC_SHA, kPGPike_PM_None,
			TRUE, kPGPike_IC_Deflate,
			kPGPike_GR_MODPFive
		},
		{	// ESP, CAST, SHA1
			TRUE, kPGPike_ET_NULL, kPGPike_AA_HMAC_SHA, kPGPike_PM_None,
			TRUE, kPGPike_AH_SHA, kPGPike_AA_HMAC_SHA, kPGPike_PM_None,
			TRUE, kPGPike_IC_LZS,
			kPGPike_GR_MODPFive
		},
	};


void main(void)
{
	SetDebugThrow_(debugAction_SourceDebugger);
	SetDebugSignal_(debugAction_SourceDebugger);

	InitializeHeap(3);
	UQDGlobals::InitializeToolbox(&qd);
	
	new LGrowZone(20000);
	
	pgpLeaksSuspend();
	
	CIKETestApp	theApp;
	theApp.Run();
	
}

CIKETestApp::CIKETestApp()
{
	LThread *myMainThread = new UMainThread;

	RegisterAllPPClasses();

	AddAttachment(new LSIOUXAttachment);
	AddAttachment(new LYieldAttachment(-1));
	
	ThrowIf_(PGPSocketsInit());
	mSelfTestMode = FALSE;
	mPacketC1 = NULL;
	mPacketC2 = NULL;
}

CIKETestApp::~CIKETestApp()
{
	PGPError	err;
	
	PGPCloseSocket( mSocket );
	err = PGPFreeIKEContext( mIKEContext1 );
	pgpAssertNoErr( err );
	err = PGPFreeIKEContext( mIKEContext2 );
	pgpAssertNoErr( err );
	err = PGPFreeKeySet( mTestKeySet );
	pgpAssertNoErr( err );
	err = PGPFreeContext( mPGPContext );
	pgpAssertNoErr( err );
	PGPSocketsCleanup();
	if( IsntNull( mPacketC1 ) )
		(void)DisposePtr( (char *)mPacketC1 );
	if( IsntNull( mPacketC2 ) )
		(void)DisposePtr( (char *)mPacketC2 );
	
	PGPSocketsDisposeThreadStorage( mSocketStorage );
	
	DisposePtr( (char *)mIncomingPacket );
}

	void
CIKETestApp::StartUp()
{
	PGPError					err;
	OSStatus					otErr;
    InetInterfaceInfo			ifaceInfo;
	PGPSocketAddressInternet	theAddress;
	PGPInt32					result;
	PGPikeMTPref				ikePref;
	
	PGPSocketsCreateThreadStorage( &mSocketStorage );
	
	err = PGPNewContext( kPGPsdkAPIVersion, &mPGPContext );
	pgpAssertNoErr( err );
	otErr = OTInetGetInterfaceInfo(&ifaceInfo, kDefaultInetInterface);
	pgpAssertNoErr( otErr );
	mLocalIP = ifaceInfo.fAddress;

	mTestKeyX509 = TRUE;
	if( mTestKeyX509 )
	{
		err = PGPImportKeySet( mPGPContext, &mTestKeySet,
				PGPOInputBuffer( mPGPContext,
								kPGPIKEX509TestKey, sizeof(kPGPIKEX509TestKey) ),
				PGPOLastOption( mPGPContext ) );
		pgpAssertNoErr( err );
	}
	else
	{
		err = PGPImportKeySet( mPGPContext, &mTestKeySet,
				PGPOInputBuffer( mPGPContext, kPGPIKETestKey, sizeof(kPGPIKETestKey) ),
				PGPOLastOption( mPGPContext ) );
	}
	err = PGPNewIKEContext( mPGPContext, IKECallback, this, &mIKEContext1 );
	pgpAssertNoErr(err);
	err = PGPNewIKEContext( mPGPContext, IKECallback, this, &mIKEContext2 );
	pgpAssertNoErr(err);
	
	ikePref.pref = kPGPike_PF_Expiration;
	ikePref.u.expiration.kbLifeTimeIKE = 256;
	ikePref.u.expiration.kbLifeTimeIPSEC = 3072;
	ikePref.u.expiration.secLifeTimeIKE = 120;
	ikePref.u.expiration.secLifeTimeIPSEC = 340;
	err = PGPikeProcessMessage( mIKEContext2, kPGPike_MT_Pref, &ikePref );
	ikePref.u.expiration.kbLifeTimeIKE = 256;
	ikePref.u.expiration.kbLifeTimeIPSEC = 2048;
	ikePref.u.expiration.secLifeTimeIKE = 120;
	ikePref.u.expiration.secLifeTimeIPSEC = 120;
	err = PGPikeProcessMessage( mIKEContext1, kPGPike_MT_Pref, &ikePref );
	ikePref.pref = kPGPike_PF_IKEProposals;
	ikePref.u.ikeProposals.numTransforms = sizeof(kPGPike_TestIKETransforms)
											/ sizeof(PGPikeTransform);
	ikePref.u.ikeProposals.t = &kPGPike_TestIKETransforms[0];
	err = PGPikeProcessMessage( mIKEContext1, kPGPike_MT_Pref, &ikePref );
	err = PGPikeProcessMessage( mIKEContext2, kPGPike_MT_Pref, &ikePref );
	ikePref.pref = kPGPike_PF_IPSECProposals;
	ikePref.u.ipsecProposals.numTransforms = sizeof(kPGPike_TestIPSECTransforms)
											/ sizeof(PGPipsecTransform);
	ikePref.u.ipsecProposals.t = &kPGPike_TestIPSECTransforms[0];
	err = PGPikeProcessMessage( mIKEContext1, kPGPike_MT_Pref, &ikePref );
	err = PGPikeProcessMessage( mIKEContext2, kPGPike_MT_Pref, &ikePref );
	
	mIncomingPacket = (PGPByte *)NewPtr( kIKETestInPacketSize );
	mSocket = PGPOpenSocket(	kPGPAddressFamilyInternet,
								kPGPSocketTypeDatagram,
								kPGPUDPProtocol );
	pgpAssert( mSocket != kInvalidPGPSocketRef );
	theAddress.sin_addr.s_addr = kPGPInternetAddressAny;
	theAddress.sin_port = (ushort) PGPHostToNetShort(kPGPike_TestPort);
	theAddress.sin_family = kPGPAddressFamilyInternet;
	result = PGPBindSocket( mSocket, (PGPSocketAddress *) &theAddress,
							sizeof(theAddress) );
	pgpAssert(result == 0);
	
	mIPSECSA1 = mIPSECSA2 = NULL;
	
	StartRepeating();
}

Boolean
CIKETestApp::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;
	PGPError	err;

	switch (inCommand)
	{
		case cmd_IKERequestSA:
		{
			PGPikeMTSASetup		saSetup;
			//PGPUInt32			remoteIP = PGPDottedToInternetAddress("10.0.1.92");
			//PGPUInt32			remoteIP = PGPDottedToInternetAddress("161.69.56.29");
			//PGPUInt32			remoteIP = PGPDottedToInternetAddress("161.69.47.132");
			//PGPUInt32			remoteIP = PGPDottedToInternetAddress("194.100.55.1");
			//PGPUInt32			remoteIP = PGPDottedToInternetAddress("204.254.155.77");
			PGPUInt32			remoteIP = PGPDottedToInternetAddress("205.178.102.85");
			
			printf("BEGIN REQUEST SA...\n");
			pgpClearMemory( &saSetup, sizeof( PGPikeMTSASetup ) );
			saSetup.ipAddress			= remoteIP;
			saSetup.localIPAddress		= mLocalIP;
			saSetup.doi					= kPGPike_DOI_IPSEC;
			saSetup.u.ipsec.packetMode	= kPGPike_PM_Tunnel;
			/* draft-ietf-ipsec-internet-key-00.txt */
			saSetup.sharedKey			= (PGPByte *) kTestSharedKey;
			saSetup.sharedKeySize		= strlen( kTestSharedKey );
			saSetup.u.ipsec.idData		= (PGPByte *)&mLocalIP;
			saSetup.u.ipsec.idDataSize	= sizeof(mLocalIP);
			saSetup.u.ipsec.idType		= kPGPike_ID_IPV4_Addr;
			saSetup.u.ipsec.destIsRange = FALSE;
			//saSetup.u.ipsec.ipAddrStart = 0x0A004202;	//10.0.66.2
			saSetup.u.ipsec.ipAddrStart = 0xCD626655;	//205.178.102.85
			saSetup.u.ipsec.ipMaskEnd	= 0xFFFFFFFF;
			err = PGPikeProcessMessage( mIKEContext1, kPGPike_MT_SARequest, &saSetup );
			pgpAssertNoErr( err );
			break;
		}
		case cmd_IKESelfTest:
		{
			PGPikeMTSASetup		saSetup;
			
			mSelfTestMode = TRUE;
			sLastC1Process = sLastC2Process = LMGetTicks();
			printf("BEGIN SELF TEST...\n");
			pgpClearMemory( &saSetup, sizeof( PGPikeMTSASetup ) );
			saSetup.ipAddress			= mLocalIP;
			saSetup.localIPAddress		= mLocalIP;
			saSetup.doi					= kPGPike_DOI_IPSEC;
			saSetup.u.ipsec.packetMode	= kPGPike_PM_Tunnel;
			/* draft-ietf-ipsec-internet-key-00.txt */
			/*saSetup.sharedKey			= (PGPByte *) kTestSharedKey;
			saSetup.sharedKeySize		= strlen( kTestSharedKey );*/
			saSetup.u.ipsec.idData		= (PGPByte *)&mLocalIP;
			saSetup.u.ipsec.idDataSize	= sizeof(mLocalIP);
			saSetup.u.ipsec.idType		= kPGPike_ID_IPV4_Addr;
			saSetup.u.ipsec.destIsRange = FALSE;
			saSetup.u.ipsec.ipAddrStart = mLocalIP;
			saSetup.u.ipsec.ipMaskEnd	= 0xFFFFFFFF;
			err = PGPikeProcessMessage( mIKEContext2, kPGPike_MT_SARequest, &saSetup );
			pgpAssertNoErr( err );
			break;
		}
		case cmd_IKERekeySA1:
		case cmd_IKERekeySA2:
		{
			PGPikeSA *	sa;
			
			if( inCommand == cmd_IKERekeySA1 )
				sa = mIPSECSA1;
			else
				sa = mIPSECSA2;
			
			if( IsNull( sa ) )
				printf( "Requested SA does not exist!\n");
			else
			{
				err = PGPikeProcessMessage( ( inCommand == cmd_IKERekeySA1 ) ?
							mIKEContext1 : mIKEContext2, kPGPike_MT_SARekey, sa );
				pgpAssertNoErr( err );
			}
			break;
		}
		case cmd_IKEKillSA1:
		case cmd_IKEKillSA2:
		{
			PGPikeSA *	sa;
			
			if( inCommand == cmd_IKEKillSA1 )
				sa = mIPSECSA1;
			else
				sa = mIPSECSA2;
			
			if( IsNull( sa ) )
				printf( "Requested SA does not exist!\n");
			else
			{
				err = PGPikeProcessMessage( ( inCommand == cmd_IKEKillSA1 ) ?
							mIKEContext1 : mIKEContext2, kPGPike_MT_SADied, sa );
				pgpAssertNoErr( err );
			}
			break;
		}
		default:
			cmdHandled = LApplication::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

	void
CIKETestApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch (inCommand)
	{
		case cmd_IKERequestSA:
		case cmd_IKESelfTest:
		case cmd_IKEKillSA1:
		case cmd_IKEKillSA2:
		case cmd_IKERekeySA1:
		case cmd_IKERekeySA2:
			outEnabled = true;
			break;
		default:
			LApplication::FindCommandStatus(inCommand, outEnabled,
											outUsesMark, outMark, outName);
			break;
	}
}

	void
CIKETestApp::SpendTime(
	const EventRecord&	inMacEvent)
{
	PGPError					err;
	PGPikeMTPacket				pkt;
	PGPInt32					sockResult;
	PGPSocketAddressInternet	sourceIP;
	PGPInt32					sourceSize;
	PGPSocketSet				sockSet;
	PGPSocketsTimeValue			sockTime;
	IKETestPacket *				savePkt;
	
	if( IsntNull( mPacketC1 ) && ( sLastC1Process + kIKEFakeLatencyTicks < LMGetTicks() ) )
	{
		pkt.ipAddress			= mLocalIP;
		pkt.packetSize			= mPacketC1->dataSize;
		pkt.packet				= mPacketC1->data;
		err = PGPikeProcessMessage( mIKEContext1, kPGPike_MT_Packet, &pkt );
		pgpAssertNoErr( err );
		savePkt = mPacketC1->next;
		(void)DisposePtr( (char *)mPacketC1 );
		mPacketC1 = savePkt;
		sLastC1Process = LMGetTicks();
	}
	if( IsntNull( mPacketC2 ) && ( sLastC2Process + kIKEFakeLatencyTicks < LMGetTicks() ) )
	{
		pkt.ipAddress			= mLocalIP;
		pkt.packetSize			= mPacketC2->dataSize;
		pkt.packet				= mPacketC2->data;
		err = PGPikeProcessMessage( mIKEContext2, kPGPike_MT_Packet, &pkt );
		pgpAssertNoErr( err );
		savePkt = mPacketC2->next;
		(void)DisposePtr( (char *)mPacketC2 );
		mPacketC2 = savePkt;
		sLastC2Process = LMGetTicks();
	}
	
	err = PGPikeProcessMessage( mIKEContext1, kPGPike_MT_Idle, NULL );
	pgpAssertNoErr( err );
	err = PGPikeProcessMessage( mIKEContext2, kPGPike_MT_Idle, NULL );
	pgpAssertNoErr( err );
	
	sockTime.tv_sec = 0;
	sockTime.tv_usec = 0;
	sockSet.fd_count = 1;
	sockSet.fd_array[0] = mSocket;
	if( PGPSelect( 1, &sockSet, NULL, NULL, &sockTime ) > 0 )
	{
		sourceSize = sizeof(sourceIP);
		sockResult = PGPReceiveFrom( mSocket, mIncomingPacket, kIKETestInPacketSize,
									kPGPReceiveFlagNone, (PGPSocketAddress *) &sourceIP,
									&sourceSize );
		if( sockResult > 0 )
		{
			pkt.ipAddress			= sourceIP.sin_addr.s_addr;
			pkt.packetSize			= sockResult;
			pkt.packet				= mIncomingPacket;
			err = PGPikeProcessMessage( mIKEContext1, kPGPike_MT_Packet, &pkt );
			pgpAssertNoErr( err );
		}
	}
}

	PGPError
CIKETestApp::IKECallback(
	PGPikeContextRef	ike,
	void *				inUserData,
	PGPikeMessageType	msg,
	void *				data )
{
	PGPError			err = kPGPError_NoErr;
	CIKETestApp *		app = (CIKETestApp *)inUserData;
	
	switch( msg )
	{
		case kPGPike_MT_SARequestFailed:
			if( ike == app->mIKEContext1 )
				printf( "(1) " );
			else
				printf( "(2) " );
			printf( "SA REQUEST FAILED\n" );
			SysBeep(1);
			break;
		case kPGPike_MT_SAEstablished:
		{
			PGPikeSA *	sa = (PGPikeSA *)data;
			PGPUInt16	tInx;
			PGPUInt16	kInx;
			
			if( ike == app->mIKEContext1 )
				app->mIPSECSA1 = sa;
			else
				app->mIPSECSA2 = sa;
			printf( "SA Established - IP: %u.%u.%u.%u   Life: S: %u    K: %u",
					( sa->ipAddress >> 24 ), ( sa->ipAddress >> 16 ) & 0xFF,
					( sa->ipAddress >> 8 ) & 0xFF, ( sa->ipAddress & 0xFF ),
					sa->secLifeTime, sa->kbLifeTime );
			printf( "   # Protocols: %u\n", sa->numTransforms );
			printf("    Dest - IP: %u.%u.%u.%u -> %u.%u.%u.%u (%s)\n",
					( sa->ipAddrStart >> 24 ), ( sa->ipAddrStart >> 16 ) & 0xFF,
					( sa->ipAddrStart >> 8 ) & 0xFF, ( sa->ipAddrStart & 0xFF ),
					( sa->ipMaskEnd >> 24 ), ( sa->ipMaskEnd >> 16 ) & 0xFF,
					( sa->ipMaskEnd >> 8 ) & 0xFF, ( sa->ipMaskEnd & 0xFF ),
					sa->destIsRange ? "Range" : "Mask" );
			for( tInx = 0; tInx < sa->numTransforms; tInx++ )
			{
				printf( "    SPI: %x:%x\n",
						*(PGPUInt32 *)&sa->transform[tInx].u.ipsec.inSPI,
						*(PGPUInt32 *)&sa->transform[tInx].u.ipsec.outSPI );
				printf( "    (%u) Prot:", tInx );
				switch( sa->transform[tInx].u.ipsec.protocol )
				{
					case kPGPike_PR_AH:
						printf( "AH" );
						switch( sa->transform[tInx].u.ipsec.u.ah.t.authAttr )
						{
							case kPGPike_AA_HMAC_MD5:
								printf("/HMAC-MD5" );
								break;
							case kPGPike_AA_HMAC_SHA:
								printf("/HMAC-SHA-1" );
								break;
							default:
								printf( "/?AUTH" );
								break;
						}
						switch( sa->transform[tInx].u.ipsec.u.ah.t.mode )
						{
							case kPGPike_PM_Tunnel:
								printf( "/Tunnel\n" );
								break;
							case kPGPike_PM_Transport:
								printf( "/Transport\n" );
								break;
							default:
								printf( "/?MODE\n" );
								break;
						}
						printf( "    inAutKey (first 4): " );
						for( kInx = 0; kInx < 8; kInx++ )
						{
							printf( "%02x",
								(PGPUInt32)sa->transform[tInx].u.ipsec.u.ah.inAuthKey[kInx] );
						}
						printf( "  outAutKey (first 4): " );
						for( kInx = 0; kInx < 8; kInx++ )
						{
							printf( "%02x",
							(PGPUInt32)sa->transform[tInx].u.ipsec.u.ah.outAuthKey[kInx] );
						}
						printf( "\n" );
						break;
					case kPGPike_PR_ESP:
						printf( "ESP" );
						switch( sa->transform[tInx].u.ipsec.u.esp.t.cipher )
						{
							case kPGPike_ET_3DES:
								printf("/3DES" );
								break;
							case kPGPike_ET_CAST:
								printf("/CAST5" );
								break;
							case kPGPike_ET_DES:
								printf( "/DES" );
								break;
							case kPGPike_ET_DES_IV64:
								printf( "/DES_IV64" );
								break;
							case kPGPike_ET_NULL:
								printf( "/NULL" );
								break;
							default:
								printf( "/?CIPHER" );
								break;
						}
						switch( sa->transform[tInx].u.ipsec.u.esp.t.authAttr )
						{
							case kPGPike_AA_HMAC_MD5:
								printf("/HMAC-MD5" );
								break;
							case kPGPike_AA_HMAC_SHA:
								printf("/HMAC-SHA-1" );
								break;
							case kPGPike_AA_None:
								printf("/NOAUTH" );
								break;
							default:
								printf( "/?AUTH" );
								break;
						}
						switch( sa->transform[tInx].u.ipsec.u.esp.t.mode )
						{
							case kPGPike_PM_Tunnel:
								printf( "/Tunnel\n" );
								break;
							case kPGPike_PM_Transport:
								printf( "/Transport\n" );
								break;
							default:
								printf( "/?MODE\n" );
								break;
						}
						printf( "    inESPKey: " );
						for( kInx = 0; kInx < 24; kInx++ )
						{
							printf( "%02x",
								(PGPUInt32)sa->transform[tInx].u.ipsec.u.esp.inESPKey[kInx] );
						}
						printf( "\n    outESPKey: " );
						for( kInx = 0; kInx < 24; kInx++ )
						{
							printf( "%02x",
							(PGPUInt32)sa->transform[tInx].u.ipsec.u.esp.outESPKey[kInx] );
						}
						printf( "\n    inAutKey: " );
						for( kInx = 0; kInx < 8; kInx++ )
						{
							printf( "%02x",
								(PGPUInt32)sa->transform[tInx].u.ipsec.u.esp.inAuthKey[kInx] );
						}
						printf( " outAutKey: " );
						for( kInx = 0; kInx < 8; kInx++ )
						{
							printf( "%02x",
							(PGPUInt32)sa->transform[tInx].u.ipsec.u.esp.outAuthKey[kInx] );
						}
						printf( "\n" );
						break;
					case kPGPike_PR_IPCOMP:
						printf( "IPCOMP\n" );
						break;
					default:
						printf( "UNKNOWN\n" );
						break;
				}
				
			}
			printf( "\n" );
			SysBeep(1);
			break;
		}
		case kPGPike_MT_SADied:
			if( ike == app->mIKEContext1 )
				printf( "(1) " );
			else
				printf( "(2) " );
			printf( "SA DIED\n" );
			break;
		case kPGPike_MT_SAUpdate:
			if( ike == app->mIKEContext1 )
				printf( "(1) " );
			else
				printf( "(2) " );
			printf( "SA Update\n" );
			break;
		case kPGPike_MT_PolicyCheck:
		{
			PGPikeMTSASetup *	saSetup = (PGPikeMTSASetup *)data;
			
			saSetup->approved			= TRUE;
			saSetup->localIPAddress		= app->mLocalIP;
			saSetup->doi				= kPGPike_DOI_IPSEC;
			saSetup->u.ipsec.packetMode	= kPGPike_PM_Tunnel;
			if( app->mSelfTestMode )
			{
			}
				/* draft-ietf-ipsec-internet-key-00.txt */
			saSetup->sharedKey				= (PGPByte *) kTestSharedKey;
			saSetup->sharedKeySize			= strlen( kTestSharedKey );
			saSetup->u.ipsec.idData			= (PGPByte *) &app->mLocalIP;
			saSetup->u.ipsec.idDataSize		= sizeof(app->mLocalIP);
			saSetup->u.ipsec.idType			= kPGPike_ID_IPV4_Addr;
			break;
		}
		case kPGPike_MT_ClientIDCheck:
		{
			PGPikeMTClientIDCheck *	cidCheck = (PGPikeMTClientIDCheck *)data;
			
			cidCheck->approved			= TRUE;
			break;
		}
		case kPGPike_MT_LocalPGPCert:
		{
			PGPikeMTCert *	cert = (PGPikeMTCert *)data;
			PGPKeyRef		testKey;
			PGPKeyListRef	impKeyList;
			PGPKeyIterRef	impKeyIter;
			
			if( !app->mTestKeyX509 )
			{
				err = PGPOrderKeySet( app->mTestKeySet, kPGPAnyOrdering, &impKeyList );
				pgpAssertNoErr(err);
				err = PGPNewKeyIter( impKeyList, &impKeyIter );
				pgpAssertNoErr(err);
				err = PGPKeyIterNext( impKeyIter, &testKey );
				pgpAssertNoErr(err);
				err = PGPFreeKeyIter( impKeyIter );
				pgpAssertNoErr(err);
				err = PGPFreeKeyList( impKeyList );
				pgpAssertNoErr(err);
				cert->authKey = testKey;
				cert->isPassKey = FALSE;
				cert->pass = NULL;
				cert->passLength = 0;
			}
			break;
		}
		case kPGPike_MT_LocalX509Cert:
		{
			PGPikeMTCert *	cert = (PGPikeMTCert *)data;
			PGPKeyRef		testKey;
			PGPUserIDRef	testUID;
			PGPSigRef		testCert = kInvalidPGPSigRef;
			PGPKeyListRef	impKeyList;
			PGPKeyIterRef	impKeyIter;
			PGPBoolean		x509;
			
			if( app->mTestKeyX509 )
			{
				err = PGPOrderKeySet( app->mTestKeySet, kPGPAnyOrdering, &impKeyList );
				pgpAssertNoErr(err);
				err = PGPNewKeyIter( impKeyList, &impKeyIter );
				pgpAssertNoErr(err);
				err = PGPKeyIterNext( impKeyIter, &testKey );
				pgpAssertNoErr(err);
				err = PGPKeyIterNextUserID( impKeyIter, &testUID );
				pgpAssertNoErr(err);
				while( IsntPGPError( PGPKeyIterNextUIDSig( impKeyIter, &testCert ) ) )
				{
					PGPGetSigBoolean( testCert, kPGPSigPropIsX509, &x509 );
					if( x509 )
						break;
				}
				err = PGPFreeKeyIter( impKeyIter );
				pgpAssertNoErr(err);
				err = PGPFreeKeyList( impKeyList );
				pgpAssertNoErr(err);
				cert->authKey = testKey;
				cert->authCert = testCert;
				cert->authCertChain = NULL;
				cert->isPassKey = FALSE;
				cert->pass = NULL;
				cert->passLength = 0;
			}
			break;
		}
		case kPGPike_MT_RemoteCert:
		{
			PGPikeMTRemoteCert *	rcert = (PGPikeMTRemoteCert *)data;
			
			rcert->approved = TRUE;
			printf( "Approved remote key\n" );
			if( rcert->remoteCert )
				printf( "   -- key is X.509\n" );
			break;
		}
		case kPGPike_MT_Packet:
		{
			PGPikeMTPacket *	pkt = (PGPikeMTPacket *)data;
			static PGPUInt16	dropper = 0;
			
			//if( ++dropper == 2 )
			//	break;
			if( app->mSelfTestMode )
			{
				app->AddPacket( ( ike == app->mIKEContext1 ) ?
							FALSE : TRUE,
							pkt->packet, pkt->packetSize );
			}
			else
			{
				PGPSocketAddressInternet	theAddress;
				PGPInt32					sockResult;
				
				theAddress.sin_family			= kPGPAddressFamilyInternet;
				theAddress.sin_addr.s_addr		= pkt->ipAddress;
				theAddress.sin_port				= kPGPike_TestPort;
				sockResult = PGPSendTo( app->mSocket, pkt->packet, pkt->packetSize, kPGPSendFlagNone,
							(PGPSocketAddress *) &theAddress, sizeof(theAddress) );
				//pgpAssert( sockResult > 0 );
			}
			break;
		}
		case kPGPike_MT_Alert:
		{
			PGPikeMTAlert *		alt = (PGPikeMTAlert *)data;
			
			if( ike == app->mIKEContext1 )
				printf( "(1)" );
			else
				printf( "(2)" );
			printf( "IKE ALERT: %d ", alt->alert );
			if( alt->remoteGenerated )
				printf( "(remote)" );
			printf("\n");
			break;
		}
		case kPGPike_MT_DebugLog:
		{
			char *	debugStr = (char *)data;
			
			printf( "%s", debugStr );
			break;
		}
	}
	return err;
}

	void
CIKETestApp::AddPacket(
	PGPBoolean			c1,
	PGPByte *			data,
	PGPSize				dataSize )
{
	IKETestPacket	*	pkt,
					*	walk;
	
	pkt = (IKETestPacket *)NewPtr( sizeof(IKETestPacket) );
	pkt->next = NULL;
	pkt->dataSize = dataSize;
	pgpCopyMemory( data, pkt->data, dataSize );
	
	if( c1 )
	{
		if( IsNull( mPacketC1 ) )
			mPacketC1 = pkt;
		else
		{
			for( walk = mPacketC1; IsntNull( walk->next ); walk = walk->next )
				;
			walk->next = pkt;
		}
	}
	else
	{
		if( IsNull( mPacketC2 ) )
			mPacketC2 = pkt;
		else
		{
			for( walk = mPacketC2; IsntNull( walk->next ); walk = walk->next )
				;
			walk->next = pkt;
		}
	}
}

	PGPError
CIKETestApp::SocketsCallback(
	PGPContextRef		context,
	struct PGPEvent *	event,
	PGPUserValue		callBackArg)
{
	#pragma unused(callBackArg)
	#pragma unused(event)
	#pragma unused(context)
	
	LThread::Yield();
	
	return noErr;
}

