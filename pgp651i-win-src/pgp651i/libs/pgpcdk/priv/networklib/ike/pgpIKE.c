/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	Platform independent state machine based implementation of
	IETF RFC 2409: Internet Key Exchange protocol

	$Id: pgpIKE.c,v 1.115.2.3 1999/06/16 03:31:20 wprice Exp $
____________________________________________________________________________*/

#include "pgpIKEPriv.h"

#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpMem.h"
#include "pgpEndianConversion.h"
#include "pgpHash.h"
#include "pgpHMAC.h"
#include "pgpPublicKey.h"
#include "pgpSymmetricCipher.h"
#include "pgpCBC.h"
#include "pgpKeys.h"
#include "pgpFeatures.h"
#include "pgpMilliseconds.h"

#include <string.h>
#include <stdio.h>

#if PGP_DEBUG
#define PGPIKE_DEBUG	1
#endif

const PGPikeTransform kPGPike_DefaultIKETransforms[] =
	{
		{
			kPGPike_AM_PreSharedKey,
			kPGPike_HA_SHA1,
			kPGPike_SC_CAST_CBC,
			TRUE, kPGPike_GR_MODPFive
		},
		{
			kPGPike_AM_PreSharedKey,
			kPGPike_HA_MD5,
			kPGPike_SC_3DES_CBC,
			TRUE, kPGPike_GR_MODPTwo
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
			TRUE, kPGPike_GR_MODPFive
		},
		{
			kPGPike_AM_RSA_Sig,
			kPGPike_HA_MD5,
			kPGPike_SC_3DES_CBC,
			TRUE, kPGPike_GR_MODPTwo
		},
	};

const PGPipsecTransform kPGPike_DefaultIPSECTransforms[] =
	{
		{	/* ESP, CAST, SHA1 */
			TRUE, kPGPike_ET_CAST, kPGPike_AA_HMAC_SHA, kPGPike_PM_None,
			FALSE, kPGPike_AH_None, kPGPike_AA_None, kPGPike_PM_None,
			FALSE, kPGPike_IC_None,
			kPGPike_GR_None
		},
		{	/* ESP, 3DES, MD5 */
			TRUE, kPGPike_ET_3DES, kPGPike_AA_HMAC_MD5, kPGPike_PM_None,
			FALSE, kPGPike_AH_None, kPGPike_AA_None, kPGPike_PM_None,
			FALSE, kPGPike_IC_None,
			kPGPike_GR_None
		},
		{	/* ESP, 3DES, SHA, DEFLATE */
			TRUE, kPGPike_ET_3DES, kPGPike_AA_HMAC_SHA, kPGPike_PM_None,
			FALSE, kPGPike_AH_None, kPGPike_AA_None, kPGPike_PM_None,
			TRUE, kPGPike_IC_Deflate,
			kPGPike_GR_None
		},
	};



const PGPByte				dhGenerator[1] = { 2 };
const char					kPGPike_PGPVendorString1[] = "OpenPGP1";
const char					kPGPike_PGPVendorString2[] = "0171";
		
	PGPError
PGPNewIKEContext(
	PGPContextRef			context,
	PGPikeMessageProcPtr	ikeMessageProc,
	void *					inUserData,
	PGPikeContextRef *		outRef )
{
	PGPError				err	= kPGPError_NoErr;
	PGPikeContextPriv *		pContext;

	*outRef = NULL;
	PGPValidatePtr( context );
	if( IsNull( ikeMessageProc ) )
		return kPGPError_BadParams;
	
	pContext = (PGPikeContextPriv *) pgpContextMemAlloc( context,
											sizeof(PGPikeContextPriv),
											kPGPMemoryMgrFlags_Clear );
	if( IsntNull( pContext ) )
	{
		pContext->pgpContext		= context;
		pContext->memMgr			= PGPGetContextMemoryMgr( context );
		pContext->msgProc			= ikeMessageProc;
		pContext->userData			= inUserData;
		pContext->pending			= NULL;
		pContext->cookieDough		= FALSE;
		err = PGPContextGetRandomBytes( context, &pContext->cookieSecret,
									kPGPike_CookieSize );
		if( IsntPGPError( err ) )
			pContext->cookieDough = TRUE;
		err = kPGPError_NoErr;	/* no entropy error here will be retried later */
		
		pContext->secLifeTimeIKE	= kPGPike_DefaultSecLife;
		pContext->kbLifeTimeIKE		= kPGPike_DefaultKBLife;
		pContext->secLifeTimeIPSEC	= kPGPike_DefaultSecLife;
		pContext->kbLifeTimeIPSEC	= kPGPike_DefaultKBLife;
		
		pContext->defaultIKEProps = (PGPikeTransform *) pgpContextMemAlloc(
				context, sizeof(kPGPike_DefaultIKETransforms),
				kPGPMemoryMgrFlags_Clear );
		if( IsNull( pContext->defaultIKEProps ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		pContext->numIKEProps = sizeof(kPGPike_DefaultIKETransforms) /
								sizeof(PGPikeTransform);
		pgpCopyMemory( &kPGPike_DefaultIKETransforms[0], pContext->defaultIKEProps,
						sizeof(kPGPike_DefaultIKETransforms) );

		pContext->defaultIPSECProps = (PGPipsecTransform *) pgpContextMemAlloc(
				context, sizeof(kPGPike_DefaultIPSECTransforms),
				kPGPMemoryMgrFlags_Clear );
		if( IsNull( pContext->defaultIPSECProps ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		pContext->numIPSECProps = sizeof(kPGPike_DefaultIPSECTransforms) /
								sizeof(PGPipsecTransform);
		pgpCopyMemory( &kPGPike_DefaultIPSECTransforms[0],
						pContext->defaultIPSECProps,
						sizeof(kPGPike_DefaultIPSECTransforms) );
		
		pContext->allowedAlgorithms.cast5			= TRUE;
		pContext->allowedAlgorithms.tripleDES		= TRUE;
		pContext->allowedAlgorithms.singleDES		= FALSE;
		pContext->allowedAlgorithms.espNULL			= FALSE;
		pContext->allowedAlgorithms.sha1			= TRUE;
		pContext->allowedAlgorithms.md5				= TRUE;
		pContext->allowedAlgorithms.noAuth			= FALSE;
		pContext->allowedAlgorithms.lzs				= TRUE;
		pContext->allowedAlgorithms.deflate			= TRUE;
		pContext->allowedAlgorithms.modpOne768		= TRUE;
		pContext->allowedAlgorithms.modpTwo1024		= TRUE;
		pContext->allowedAlgorithms.modpFive1536	= TRUE;
		
		*outRef = ( PGPikeContextRef ) pContext;
	}
	else
		err = kPGPError_OutOfMemory;
	
done:
	if( IsPGPError( err ) && IsntNull( *outRef ) )
	{
		pgpContextMemFree( context, pContext );
		*outRef = NULL;
	}
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}

	PGPError
PGPFreeIKEContext(
	PGPikeContextRef		ref )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeContextPriv *		ike;
	
	PGPValidatePtr( ref );
	ike = (PGPikeContextPriv *)ref;
	
	if( IsntNull( ike->pending ) )
	{
		PGPikeDestination	*	walkD,
							*	nextD;
		
		pgpAssert( 0 );
		/* REPORT THIS ASSERTION AND SUBMIT LOG */
		for( walkD = ike->pending; IsntNull( walkD ); walkD = nextD )
		{
			nextD = walkD->nextD;
			(void)PGPFreeData( walkD );
		}
	}
	while( IsntNull( ike->partner ) )
		(void)pgpIKEFreePartner( ike->partner );
	while( IsntNull( ike->sa ) )
	{
		PGPikeSA *	oldHead = ike->sa;
		
		ike->sa = oldHead->nextSA;
		(void)PGPFreeData( oldHead );
	}
	
	err = pgpContextMemFree( ike->pgpContext, ike->defaultIKEProps ); CKERR;
	err = pgpContextMemFree( ike->pgpContext, ike->defaultIPSECProps ); CKERR;
	err = pgpContextMemFree( ike->pgpContext, ref ); CKERR;
done:
	return err;
}

	PGPError
pgpIKEFreePartner(
	PGPikePartner *			partner )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner	*		cp,
					*		lp;

	if( partner->ready )
	{
		err = pgpIKELocalAlert( partner->ike, partner->ipAddress,
				kPGPike_AL_None, kPGPike_IA_DeadPhase1SA, FALSE );
	}
	/* delink ourself */
	cp = partner->ike->partner;	/* get head of list */
	lp = NULL;
	while( IsntNull( cp ) )
	{
		if( cp == partner )
		{
			if( IsntNull( lp ) )
				lp->nextPartner = cp->nextPartner;
			else
				partner->ike->partner = cp->nextPartner;
			break;
		}
		lp = cp;
		cp = cp->nextPartner;
	}
	while( IsntNull( partner->exchanges ) )
	{
		err = pgpIKEFreeExchange( partner->exchanges );	CKERR;
	}
	if( IsntNull( partner->sharedKey ) )
	{
		err = PGPFreeData( partner->sharedKey ); CKERR;
	}
	if( PGPKeySetRefIsValid( partner->pgpAuthKey.baseKeySet ) )
	{
		err = PGPFreeKeySet( partner->pgpAuthKey.baseKeySet ); CKERR;
	}
	if( PGPKeySetRefIsValid( partner->x509AuthKey.baseKeySet ) )
	{
		err = PGPFreeKeySet( partner->x509AuthKey.baseKeySet ); CKERR;
	}
	if( IsntNull( partner->pgpAuthKey.pass ) )
	{
		err = PGPFreeData( partner->pgpAuthKey.pass ); CKERR;
	}
	if( IsntNull( partner->x509AuthKey.pass ) )
	{
		err = PGPFreeData( partner->x509AuthKey.pass ); CKERR;
	}
	if( IsntNull( partner->ipsecOpts.idData ) )
	{
		err = PGPFreeData( partner->ipsecOpts.idData ); CKERR;
	}
	if( PGPCBCContextRefIsValid( partner->cbc ) )
	{
		err = PGPFreeCBCContext( partner->cbc );	CKERR;
	}
	if( PGPKeySetRefIsValid( partner->remoteKeySet ) )
	{
		err = PGPFreeKeySet( partner->remoteKeySet );	CKERR;
	}
	
	err = PGPFreeData( partner );
done:
	return err;
}

	PGPError
pgpIKEFreeExchange(
	PGPikeExchange *		exchange )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeExchange	*	cn,
						*	ln;
	PGPikeProposal		*	cp,
						*	lp = NULL;

	if( IsNull( exchange ) )
		goto done;
	exchange->ike->numExchanges--;
	/* delink ourself */
	cn = exchange->partner->exchanges;	/* get head of list */
	ln = NULL;
	while( IsntNull( cn ) )
	{
		if( cn == exchange )
		{
			if( IsntNull( ln ) )
				ln->nextExchange = cn->nextExchange;
			else
				exchange->partner->exchanges = cn->nextExchange;
			break;
		}
		ln = cn;
		cn = cn->nextExchange;
	}

	cp = exchange->proposals;
	while( IsntNull( cp ) )
	{
		lp = cp;
		cp = cp->nextProposal;
		(void)PGPFreeData( lp );
	}
	
	if( exchange->dhP != kPGPInvalidBigNumRef )
		(void) PGPFreeBigNum( exchange->dhP );
	if( exchange->dhG != kPGPInvalidBigNumRef )
		(void) PGPFreeBigNum( exchange->dhG );
	if( exchange->dhX != kPGPInvalidBigNumRef )
		(void) PGPFreeBigNum( exchange->dhX );
	if( exchange->dhYi != kPGPInvalidBigNumRef )
		(void) PGPFreeBigNum( exchange->dhYi );
	if( exchange->dhYr != kPGPInvalidBigNumRef )
		(void) PGPFreeBigNum( exchange->dhYr );
	if( IsntNull( exchange->gXY ) )
		(void) PGPFreeData( exchange->gXY );
	
	if( IsntNull( exchange->initSABody ) )
		(void)PGPFreeData( exchange->initSABody );
	if( IsntNull( exchange->idBody ) )
		(void)PGPFreeData( exchange->idBody );
	if( IsntNull( exchange->idRBody ) )
		(void)PGPFreeData( exchange->idRBody );
	if( IsntNull( exchange->lastPkt ) )
		(void)PGPFreeData( exchange->lastPkt );
	
	(void)PGPFreeData( exchange );
done:
	return err;
}

	PGPError
PGPikeProcessMessage(
	PGPikeContextRef		ref,
	PGPikeMessageType		msg,
	void *					data )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeContextPriv *		ike;
	
	PGPValidatePtr( ref );
	ike = (PGPikeContextPriv *)ref;
	
	switch( msg )
	{
		case kPGPike_MT_Idle:
			err = pgpIKEIdle( ike );
			break;
		case kPGPike_MT_SARequest:
			err = pgpIKEHandleSARequest( ike, (PGPikeMTSASetup *) data );
			break;
		case kPGPike_MT_SARekey:
			err = pgpIKEHandleSAEvent( ike, FALSE, (PGPikeSA *) data );
			break;
		case kPGPike_MT_SADied:
			err = pgpIKEHandleSAEvent( ike, TRUE, (PGPikeSA *) data );
			break;
		case kPGPike_MT_Packet:
			err = pgpIKEHandlePacket( ike, (PGPikeMTPacket *) data );
#if PGPIKE_DEBUG
		/*{
			PGPUInt32	saCount = 0;
			PGPikeSA *	sa;
			
			for( sa = ike->sa; IsntNull( sa ); sa = sa->nextSA )
				saCount++;
			pgpIKEDebugLog( ike, "\tsaCount: %d\n", saCount );
		}*/
#endif
			break;
		case kPGPike_MT_Pref:
			err = pgpIKESetPref( ike, (PGPikeMTPref *) data );
			break;
		case kPGPike_MT_SAKillAll:
		{
			PGPikeSA	*	sa,
						*	nextSA = NULL;
			PGPikePartner *	partner;
			
			for( sa = ike->sa; IsntNull( sa ); sa = nextSA )
			{
				nextSA = sa->nextSA;
				sa->activeIn = FALSE;
				if( IsntPGPError( pgpIKEFindSAPartner( ike, sa, TRUE, &partner ) ) )
				{
					err = pgpIKEKillSA( &partner, sa );
				}
				else
				{
					err = (ike->msgProc)(
							(PGPikeContextRef)ike, ike->userData,
							kPGPike_MT_SADied, sa );
					(void)pgpIKEFreeSA( ike, sa );
				}
				if( IsPGPError( err ) )
					break;
			}
			break;
		}
		case kPGPike_MT_PolicyCheck:
		case kPGPike_MT_SAEstablished:
		case kPGPike_MT_LocalPGPCert:
		case kPGPike_MT_LocalX509Cert:
		case kPGPike_MT_RemoteCert:
		case kPGPike_MT_Alert:
		default:
			err = kPGPError_ItemNotFound;/* these msgs are only sent FROM pgpIKE */
			break;
	}
	if( IsPGPError( err ) )
	{
		if( err == kPGPError_UserAbort )
			err = kPGPError_NoErr;
		else
		{
#if PGPIKE_DEBUG
			pgpIKEDebugLog( ike, "pgpIKE PGPError: %ld\n", err );
#endif
		}
	}
	return err;
}

	PGPError
pgpIKEFindSAPartner(
	PGPikeContextPriv *		ike,
	PGPikeSA *				sa,
	PGPBoolean				mustBeReady,
	PGPikePartner **		partnerP )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner;
	
	*partnerP = NULL;
	for( partner = ike->partner; IsntNull( partner );
			partner = partner->nextPartner )
	{
		if( ( sa->ipAddress == partner->ipAddress ) &&
			( partner->ready || !mustBeReady ) &&
			( sa->ipAddrStart == partner->ipsecOpts.ipAddrStart ) &&
			( sa->ipMaskEnd == partner->ipsecOpts.ipMaskEnd ) &&
			( sa->destIsRange == partner->ipsecOpts.destIsRange ) )
		{
			*partnerP = partner;
			goto done;
		}
	}
	err = kPGPError_ItemNotFound;
done:
	return err;
}

	PGPError
pgpIKEFreeSA(
	PGPikeContextPriv *		ike,
	PGPikeSA *				sa )
{	
	PGPError				err = kPGPError_NoErr;
	
	err = pgpIKELocalAlert( ike, sa->ipAddress,
			kPGPike_AL_None, kPGPike_IA_DeadPhase2SA, FALSE );
	if( IsntNull( sa->prevSA ) )
		sa->prevSA->nextSA = sa->nextSA;
	else
		ike->sa = sa->nextSA;
	if( IsntNull( sa->nextSA ) )
		sa->nextSA->prevSA = sa->prevSA;
	(void)PGPFreeData( sa );
	return err;
}

	PGPError
pgpIKEHandleSAEvent(
	PGPikeContextPriv *		ike,
	PGPBoolean				death,
	PGPikeSA *				sa )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner;
	PGPikeExchange *		exchange;
	
	if( IsntPGPError( pgpIKEFindSAPartner( ike, sa, TRUE, &partner ) ) )
	{
		if( death )
		{
			err = pgpIKEKillSA( &partner, sa );	CKERR;
		}
		else if( IsNull( partner->exchanges ) )
		{
			PGPikeSA *		oSA;
			PGPBoolean		found = FALSE;
			
			for( oSA = ike->sa; IsntNull( oSA ); oSA = oSA->nextSA )
			{
				if( ( oSA->ipAddress == sa->ipAddress ) && ( sa != oSA ) &&
					( oSA->ipAddrStart == sa->ipAddrStart ) &&
					( oSA->ipMaskEnd == sa->ipMaskEnd ) &&
					( oSA->destIsRange == sa->destIsRange ) )
				{
					found = TRUE;
					break;
				}
			}
			if( !found )
			{
				err = pgpIKEStartQMExchange( partner, &exchange );	CKERR;
			}
		}
	}
done:
	return err;
}

	PGPError
pgpIKESetPref(
	PGPikeContextPriv *		ike,
	PGPikeMTPref *			pref )
{
	PGPError				err = kPGPError_NoErr;
	
	switch( pref->pref )
	{
		case kPGPike_PF_Expiration:
			ike->secLifeTimeIKE			= pref->u.expiration.secLifeTimeIKE;
			if( ike->secLifeTimeIKE && ( ike->secLifeTimeIKE < 60 ) )
				ike->secLifeTimeIKE = 60;
			ike->kbLifeTimeIKE			= pref->u.expiration.kbLifeTimeIKE;
			if( ike->kbLifeTimeIKE && ( ike->kbLifeTimeIKE < kPGPike_KBLifeMinimum ) )
				ike->kbLifeTimeIKE = kPGPike_KBLifeMinimum;
			ike->secLifeTimeIPSEC		= pref->u.expiration.secLifeTimeIPSEC;
			if( ike->secLifeTimeIPSEC && ( ike->secLifeTimeIPSEC < 60 ) )
				ike->secLifeTimeIPSEC = 60;
			ike->kbLifeTimeIPSEC		= pref->u.expiration.kbLifeTimeIPSEC;
			if( ike->kbLifeTimeIPSEC && ( ike->kbLifeTimeIPSEC < kPGPike_KBLifeMinimum ) )
				ike->kbLifeTimeIPSEC = kPGPike_KBLifeMinimum;
			break;
		case kPGPike_PF_IKEProposals:
			if( IsntNull( ike->defaultIKEProps ) )
			{
				err = pgpContextMemFree( ike->pgpContext,
										ike->defaultIKEProps );CKERR;
			}
			ike->defaultIKEProps = (PGPikeTransform *)
				pgpContextMemAlloc( ike->pgpContext,
				sizeof(PGPikeTransform) * pref->u.ikeProposals.numTransforms,
				kPGPMemoryMgrFlags_Clear );
			if( IsNull( ike->defaultIKEProps ) )
			{
				err = kPGPError_OutOfMemory;
				goto done;
			}
			ike->numIKEProps = pref->u.ikeProposals.numTransforms;
			pgpCopyMemory( pref->u.ikeProposals.t, ike->defaultIKEProps,
							ike->numIKEProps * sizeof(PGPikeTransform) );
			break;
		case kPGPike_PF_IPSECProposals:
			if( IsntNull( ike->defaultIPSECProps ) )
			{
				err = pgpContextMemFree( ike->pgpContext,
										ike->defaultIPSECProps );CKERR;
			}
			ike->defaultIPSECProps = (PGPipsecTransform *)
				pgpContextMemAlloc( ike->pgpContext,
				sizeof(PGPipsecTransform) * pref->u.ipsecProposals.numTransforms,
				kPGPMemoryMgrFlags_Clear );
			if( IsNull( ike->defaultIPSECProps ) )
			{
				err = kPGPError_OutOfMemory;
				goto done;
			}
			ike->numIPSECProps = pref->u.ipsecProposals.numTransforms;
			pgpCopyMemory( pref->u.ipsecProposals.t, ike->defaultIPSECProps,
							ike->numIPSECProps * sizeof(PGPipsecTransform) );
			break;
		case kPGPike_PF_AllowedAlgorithms:
			pgpCopyMemory( &pref->u.allowedAlgorithms, &ike->allowedAlgorithms,
							sizeof(PGPikeAllowedAlgorithms) );
			break;
		default:
			err = kPGPError_FeatureNotAvailable;
			break;
	}
done:
	return err;
}

	PGPError
pgpIKEAddPending(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd,
	PGPBoolean				destIsRange )
{
	PGPikeDestination *		dest;
	PGPError				err = kPGPError_NoErr;
	
	/* The pending list is really just for debugging purposes */
	dest = (PGPikeDestination *)PGPNewData( ike->memMgr, sizeof(PGPikeDestination), 0 );
	if( IsntNull( dest ) )
	{
		dest->ipAddress		= ipAddress;
		dest->ipAddrStart	= ipAddrStart;
		dest->ipMaskEnd		= ipMaskEnd;
		dest->destIsRange	= destIsRange;
		dest->nextD			= ike->pending;
		ike->pending		= dest;
	}
	else
		err = kPGPError_OutOfMemory;
	return err;
}

	PGPBoolean
pgpIKEFindPending(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd,
	PGPBoolean				destIsRange )
{
	PGPBoolean				found = FALSE;
	PGPikeDestination *		dest;
	
	for( dest = ike->pending; IsntNull( dest ); dest = dest->nextD )
	{
		if( ( dest->ipAddress == ipAddress ) &&
			( dest->ipAddrStart == ipAddrStart ) &&
			( dest->ipMaskEnd == ipMaskEnd ) &&
			( dest->destIsRange == destIsRange ) )
		{
			found = TRUE;
			break;
		}
	}
	return found;
}

	PGPError
pgpIKERemovePending(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd,
	PGPBoolean				destIsRange )
{
	PGPikeDestination *		dest;
	PGPikeDestination *		lastDest = NULL;
	PGPError				err = kPGPError_NoErr;
	
	for( dest = ike->pending; IsntNull( dest ); dest = dest->nextD )
	{
		if( ( dest->ipAddress == ipAddress ) &&
			( dest->ipAddrStart == ipAddrStart ) &&
			( dest->ipMaskEnd == ipMaskEnd ) &&
			( dest->destIsRange == destIsRange ) )
		{
			if( IsNull( lastDest ) )
				ike->pending = dest->nextD;
			else
				lastDest->nextD = dest->nextD;
			(void)PGPFreeData( dest );
			break;
		}
		lastDest = dest;
	}
	return err;
}

	PGPError
pgpIKEHandleSARequest(
	PGPikeContextPriv *		ike,
	PGPikeMTSASetup *		saReq )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner = NULL;

	if( IsNull( saReq ) )
		return kPGPError_BadParams;
	if( saReq->doi != kPGPike_DOI_IPSEC )
		return kPGPError_FeatureNotAvailable;
	
	/* currently, if a P2 Tunnel Mode SA exists, and we want to communicate with
		a different destination inside the Tunnel, we establish another P1 and P2
		SA for that.  This is unnecessary, and a good optimization here would be
		to only establish the P2 SA using the existing P1 SA if there is one. */
	
#if PGPIKE_DEBUG
	pgpIKEDebugLog( ike, "SARequest: %u.%u.%u.%u (%u.%u.%u.%u/%u.%u.%u.%u)\n",
		( saReq->ipAddress >> 24 ), ( saReq->ipAddress >> 16 ) & 0xFF,
		( saReq->ipAddress >> 8 ) & 0xFF, ( saReq->ipAddress & 0xFF ),
		( saReq->u.ipsec.ipAddrStart >> 24 ), ( saReq->u.ipsec.ipAddrStart >> 16 ) & 0xFF,
		( saReq->u.ipsec.ipAddrStart >> 8 ) & 0xFF, ( saReq->u.ipsec.ipAddrStart & 0xFF ),
		( saReq->u.ipsec.ipMaskEnd >> 24 ), ( saReq->u.ipsec.ipMaskEnd >> 16 ) & 0xFF,
		( saReq->u.ipsec.ipMaskEnd >> 8 ) & 0xFF, ( saReq->u.ipsec.ipMaskEnd & 0xFF ) );
#endif
	if( pgpIKEFindPending(  ike, saReq->ipAddress, saReq->u.ipsec.ipAddrStart,
			saReq->u.ipsec.ipMaskEnd, saReq->u.ipsec.destIsRange ) )
	{
#if PGPIKE_DEBUG
		pgpIKEDebugLog( ike, "IGNORED: Already in progress\n" );
#endif
		goto done;
	}
	err = pgpIKEAddPending( ike, saReq->ipAddress, saReq->u.ipsec.ipAddrStart,
			saReq->u.ipsec.ipMaskEnd, saReq->u.ipsec.destIsRange );	CKERR;
	err = pgpIKEStartIdentityExchange( ike, saReq->ipAddress, saReq, FALSE, 0, 0 );	CKERR;
done:
	return err;
}

	PGPError
pgpIKECreateProposals(
	PGPikeExchange *		exchange )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeProposal *		proposal = NULL;
	PGPUInt16				transIndex,
							numTransforms;

	proposal = PGPNewData( exchange->ike->memMgr, sizeof(PGPikeProposal),
								kPGPMemoryMgrFlags_Clear );
	if( IsNull( proposal ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	switch( exchange->exchangeT )
	{
		case kPGPike_EX_Identity:
			proposal->number				= 1;
			proposal->protocol				= kPGPike_PR_IKE;
			numTransforms = exchange->ike->numIKEProps;
			if( numTransforms > kPGPike_MaxPropTransforms )
				numTransforms = kPGPike_MaxPropTransforms;
			for( transIndex = 0; transIndex < numTransforms; transIndex++ )
				if( pgpIKEIsTransformValid( exchange->partner, kPGPike_PR_IKE,
					(PGPikeGenericTransform *)
					&exchange->ike->defaultIKEProps[transIndex] ) )
				{
					pgpCopyMemory(	&exchange->ike->defaultIKEProps[transIndex],
									&proposal->t[proposal->numTransforms].u.ike,
									sizeof( PGPikeTransform ) );
					proposal->numTransforms++;
				}
			proposal->secLifeTime			= exchange->ike->secLifeTimeIKE;
			proposal->kbLifeTime			= exchange->ike->kbLifeTimeIKE;
			break;
		case kPGPike_EX_IPSEC_Quick:
		{
			PGPikeProposal	*	nproposal = NULL;
			PGPUInt32			propType,
								propIndex;
			PGPBoolean			useProtocol;
			PGPipsecTransform *	curp;
			
			for( propIndex = exchange->ike->numIPSECProps; propIndex > 0; propIndex-- )
			{
				curp = &exchange->ike->defaultIPSECProps[propIndex - 1];
				for( propType = 0; propType < 3; propType++ )
				{
					useProtocol = FALSE;
					if( IsNull( nproposal ) )
						nproposal = proposal;
					else
					{
						nproposal = PGPNewData( exchange->ike->memMgr,
							sizeof(PGPikeProposal), kPGPMemoryMgrFlags_Clear );
						if( IsNull( nproposal ) )
						{
							err = kPGPError_OutOfMemory;
							goto done;
						}
						nproposal->nextProposal = proposal;
						proposal = nproposal;
					}
					switch( propType )	// in reverse order
					{
						case 0:			// IPCOMP
							if( curp->useIPCOMP )
							{
								useProtocol = TRUE;
								proposal->protocol	= kPGPike_PR_IPCOMP;
								pgpCopyMemory( &curp->ipcomp,
												&proposal->t[0].u.ipsec.ipcomp,
												sizeof(PGPipsecIPCOMPTransform) );
							}
							break;
						case 1:			// ESP
							if( curp->useESP )
							{
								useProtocol = TRUE;
								proposal->protocol	= kPGPike_PR_ESP;
								pgpCopyMemory( &curp->esp, &proposal->t[0].u.ipsec.esp,
												sizeof(PGPipsecESPTransform) );
								proposal->t[0].u.ipsec.esp.mode =
									exchange->partner->ipsecOpts.packetMode;
							}
							break;
						case 2:			// AH
							if( curp->useAH )
							{
								useProtocol = TRUE;
								proposal->protocol	= kPGPike_PR_AH;
								pgpCopyMemory( &curp->ah, &proposal->t[0].u.ipsec.ah,
												sizeof(PGPipsecAHTransform) );
								proposal->t[0].u.ipsec.ah.mode =
									exchange->partner->ipsecOpts.packetMode;
							}
							break;
					}
					if( useProtocol )
					{
						proposal->number		= propIndex;
						proposal->numTransforms	= 1;
						proposal->t[0].u.ipsec.groupID	= curp->groupID;
						proposal->secLifeTime	= exchange->ike->secLifeTimeIPSEC;
						proposal->kbLifeTime	= exchange->ike->kbLifeTimeIPSEC;
						err = PGPContextGetRandomBytes( exchange->ike->pgpContext,
									&proposal->initSPI, sizeof(PGPipsecSPI) ); CKERR;
					}
					else
					{
						nproposal = proposal;
						proposal = proposal->nextProposal;
						err = PGPFreeData( nproposal );	CKERR;
					}
				}
			}
			break;
		}
		default:
			break;
	}
	pgpAssert( IsNull( exchange->proposals ) );
	exchange->proposals = proposal;
	proposal = NULL;
done:
	if( IsntNull( proposal ) )
		(void) PGPFreeData( proposal );
	return err;
}

	PGPError
pgpIKECreateExchange(
	PGPikePartner *			partner,
	PGPikeExchangeT			exchangeT,
	PGPikeMessageID *		messageID,
	PGPikeState				state,
	PGPBoolean				initiator,
	PGPikeExchange **		exchangeP )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeExchange *		exchange = NULL;
	PGPHashContextRef		hash = kInvalidPGPHashContextRef;

	*exchangeP = NULL;
	pgpAssertAddrValid( partner, PGPikePartner );
	
	exchange = PGPNewData( partner->ike->memMgr, sizeof(PGPikeExchange),
							kPGPMemoryMgrFlags_Clear );
	if( IsNull( exchange ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	partner->ike->numExchanges++;
	exchange->ike					= partner->ike;
	exchange->partner				= partner;
	exchange->initiator				= initiator;
	exchange->state					= state;
	exchange->doi					= kPGPike_DOI_IPSEC;
	exchange->exchangeT				= exchangeT;
	
	if( ( exchangeT == kPGPike_EX_IPSEC_Quick ) ||
		( ( exchangeT == kPGPike_EX_Informational ) && partner->ready ) )
	{
		PGPByte		p2iv[kPGPike_MaxHashSize];
		
		if( IsNull( messageID ) )
		{
			err = PGPContextGetRandomBytes( partner->ike->pgpContext,
									&exchange->messageID,
									sizeof(PGPikeMessageID) ); CKERR;
		}
		else
		{
			pgpCopyMemory( messageID, exchange->messageID,
							sizeof(PGPikeMessageID) );
		}
		/* Set the last CBC block to correct value for a new P1/P2 exchange */
		err = PGPNewHashContext( partner->ike->memMgr,
						partner->sdkHashAlg, &hash );	CKERR;
		err = PGPContinueHash( hash, partner->lastP1CBC,
								kPGPike_MaxCipherBlockSize );	CKERR;
		err = PGPContinueHash( hash, &exchange->messageID,
								sizeof(PGPikeMessageID) );	CKERR;
		err = PGPFinalizeHash( hash, p2iv );	CKERR;
		pgpCopyMemory( p2iv, exchange->lastCBC, partner->cipherBlockSize );
	}
	
	if( IsNull( partner->exchanges ) )
		partner->exchanges	= exchange;
	else
	{
		PGPikeExchange *cn = partner->exchanges;
		
		while( IsntNull( cn->nextExchange ) )
			cn = cn->nextExchange;
		cn->nextExchange		= exchange;
	}
	*exchangeP = exchange;
done:
	if( PGPHashContextRefIsValid( hash ) )
		(void)PGPFreeHashContext( hash );
	if( IsPGPError( err ) )
		(void)PGPFreeData( exchange );
	return err;
}

	PGPError
pgpIKECreatePartner(
	PGPikeContextPriv *		ike,
	PGPikeMTSASetup *		setup,
	PGPBoolean				initiator,
	PGPikePartner **		partnerP )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner;

	pgpAssert( IsntNull( setup ) );
	pgpAssert( IsntNull( partnerP ) );
	*partnerP = NULL;
	partner = PGPNewData( ike->memMgr, sizeof(PGPikePartner),
							kPGPMemoryMgrFlags_Clear );
	if( IsntNull( partner ) )
	{
		partner->magic				= kPGPike_Magic;
		partner->rttMillisec		= kPGPike_RoundTripSlack * 2;
		partner->ike				= ike;
		partner->ipAddress			= setup->ipAddress;
		partner->localIPAddress		= setup->localIPAddress;
		partner->initiator			= initiator;
		partner->birthTime			= PGPGetTime();
		if( initiator )
			err = pgpIKEBakeCookie( ike, setup->ipAddress, setup->localIPAddress,
									&partner->initCookie[0] );
		else
			err = pgpIKEBakeCookie( ike, setup->ipAddress, setup->localIPAddress,
									&partner->respCookie[0] );
		CKERR;
		partner->sharedKeySize = setup->sharedKeySize;
		if( IsntNull( setup->sharedKey ) )
		{
			partner->sharedKey = PGPNewSecureData( ike->memMgr,
					setup->sharedKeySize, kPGPMemoryMgrFlags_Clear );
			if( IsntNull( partner->sharedKey ) )
				pgpCopyMemory( setup->sharedKey,
								partner->sharedKey,
								setup->sharedKeySize );
			else
			{
				err = kPGPError_OutOfMemory;
				goto done;
			}
		}
		pgpCopyMemory( &setup->u.ipsec, &partner->ipsecOpts,
						sizeof(PGPipsecDOIOptions) );
		partner->ipsecOpts.ipAddrStart &= partner->ipsecOpts.ipMaskEnd;
		if( IsntNull( setup->u.ipsec.idData ) )
		{
			partner->ipsecOpts.idData = NULL;
			partner->ipsecOpts.idData = PGPNewData( ike->memMgr,
								partner->ipsecOpts.idDataSize,
								kPGPMemoryMgrFlags_Clear );
			if( IsNull( partner->ipsecOpts.idData ) )
			{
				err = kPGPError_OutOfMemory;
				goto done;
			}
			pgpCopyMemory( setup->u.ipsec.idData, partner->ipsecOpts.idData,
							partner->ipsecOpts.idDataSize );
		}
		
		
		err = pgpIKEGetCert( partner, kPGPike_MT_LocalPGPCert ); CKERR;
		err = pgpIKEGetCert( partner, kPGPike_MT_LocalX509Cert ); CKERR;
				
		/* we're all set, hook it into our partners list */
		partner->nextPartner = ike->partner;
		ike->partner = partner;
		*partnerP = partner;
	}
	else
		err = kPGPError_OutOfMemory;

done:
	if( IsPGPError(err) && IsntNull( partner ) )
		pgpIKEFreePartner( partner );
	return err;
}

	PGPError
pgpIKEBakeCookie(
	PGPikeContextPriv *		ike,
	PGPUInt32				destAddress,
	PGPUInt32				srcAddress,
	PGPByte *				outCookie )
{
	PGPError				err = kPGPError_NoErr;
	PGPHashContextRef		hash = kInvalidPGPHashContextRef;
	PGPUInt16				port = kPGPike_CommonPort;
	PGPTime					now = PGPGetTime();
	PGPByte					rawOut[32];

	/* cookie recipe per draft-ietf-ipsec-isakmp-10, 2.5.3 */
	if( !ike->cookieDough )
	{
		err = PGPContextGetRandomBytes( ike->pgpContext, &ike->cookieSecret,
									kPGPike_CookieSize ); CKERR;
		ike->cookieDough = TRUE;
	}
	err = PGPNewHashContext( ike->memMgr, kPGPHashAlgorithm_SHA, &hash); CKERR;
	
	(void)PGPContinueHash( hash, &srcAddress, sizeof(srcAddress) );
	(void)PGPContinueHash( hash, &destAddress, sizeof(destAddress) );
	(void)PGPContinueHash( hash, &port, sizeof(port) );	/* srcPort */
	(void)PGPContinueHash( hash, &port, sizeof(port) );	/* destPort */
	(void)PGPContinueHash( hash, &ike->cookieSecret, sizeof(ike->cookieSecret) );
	(void)PGPContinueHash( hash, &now, sizeof(now) );

	(void)PGPFinalizeHash( hash, rawOut );
	
	pgpCopyMemory( rawOut, outCookie, kPGPike_CookieSize );

done:
	if( PGPHashContextRefIsValid( hash ) )
		PGPFreeHashContext( hash );
	return err;
}

/* The Mighty ISAKMP Packet Interpreter */
	PGPError
pgpIKEHandlePacket(
	PGPikeContextPriv *		ike,
	PGPikeMTPacket *		packet )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner;
	PGPikeExchange *		exchange;
	PGPByte *				mp;
	PGPByte *				ep;
	PGPSize					mLen;
	PGPBoolean				valid = FALSE;
	PGPBoolean				startP1 = FALSE;
	PGPikeCookie			initCookie,
							respCookie;
	PGPikePayload			firstPayload;
	PGPikeExchangeT			exchangeT;
	PGPikeMessageID			messageID;
	PGPUInt8				flags;
	PGPUInt32				thisCksm = 0;
	PGPByte					noMessageID[4] = { 0, 0, 0, 0 };
	
	err = pgpIKEHeaderSniffer( ike, packet, &valid );
	if( !valid )
		goto done;
	mp		= packet->packet;
	mLen	= packet->packetSize;
	ep		= mp + mLen;
	
	pgpCopyMemory( mp, initCookie, sizeof( PGPikeCookie ) );
	mp += sizeof( PGPikeCookie );
	pgpCopyMemory( mp, respCookie, sizeof( PGPikeCookie ) );
	mp += sizeof( PGPikeCookie );
	firstPayload	= (PGPikePayload) *mp++;	*mp++;
	exchangeT		= (PGPikeExchangeT) *mp++;
	flags			= *mp++;
	pgpCopyMemory( mp, messageID, sizeof( PGPikeMessageID ) );
	mp += sizeof( PGPikeMessageID );
	mp += sizeof( PGPUInt32 );		/* length checked by pgpIKEHeaderSniffer */

	if( ( exchangeT == kPGPike_EX_Identity ) &&
		( firstPayload == kPGPike_PY_SA ) &&
		pgpMemoryEqual( &messageID, &noMessageID, sizeof(PGPikeMessageID) ) )
		startP1 = TRUE;
	if( startP1 && ( ike->numExchanges >= KPGPike_MaxExchanges ) )
	{
		err = pgpIKELocalAlert( ike, packet->ipAddress,
				kPGPike_AL_None, kPGPike_IA_TooManyExchanges, FALSE );
		goto done;
	}
	
	for( partner = ike->partner; IsntNull( partner );
			partner = partner->nextPartner )
		if( ( partner->ipAddress == packet->ipAddress ) &&
			pgpMemoryEqual( initCookie, partner->initCookie, sizeof(PGPikeCookie) ) &&
			( startP1 ||
			pgpMemoryEqual( respCookie, partner->respCookie, sizeof(PGPikeCookie) )) )
			break;
	
#if PGPIKE_DEBUG
	pgpIKEDebugLog( ike, "Rcvd: %sexchange=%s, firstPayload=%s%s%s\n",
		( flags & kPGPike_ISAKMPEncryptBit ) ? "(E):" : "",
		pgpIKEExchangeTypeString(exchangeT),
		pgpIKEPayloadTypeString(firstPayload),
		( flags & kPGPike_ISAKMPCommitBit ) ? ", Commit" : "",
		( flags & kPGPike_ISAKMPAuthOnlyBit ) ? ", AuthOnly" : "" );
#endif
	if( IsNull( partner ) )
	{
		if( startP1 )
		{
			PGPikeMTSASetup		msg;
			
			pgpClearMemory( &msg, sizeof(PGPikeMTSASetup) );
			msg.ipAddress = packet->ipAddress;
			err = (ike->msgProc)( (PGPikeContextRef)ike, ike->userData,
						kPGPike_MT_PolicyCheck, &msg );CKERR;
			if( msg.approved )
			{
				err = pgpIKECreatePartner( ike, &msg, FALSE, &partner );CKERR;
				err = pgpIKECreateExchange( partner, kPGPike_EX_Identity, NULL,
												kPGPike_S_MM_WaitSA, FALSE,
												&exchange );CKERR;
				pgpCopyMemory( initCookie, partner->initCookie, kPGPike_CookieSize );
			}
		}
		else
		{
			if( ( exchangeT == kPGPike_EX_Informational ) &&
				( firstPayload == kPGPike_PY_Notification ) &&
				!( flags & kPGPike_ISAKMPEncryptBit ) &&
				( *mp++ == kPGPike_PY_None ) )
			{
				PGPikeNDPayload nd;
				
				mp++;	/* reserved */
				nd.nextND	= NULL;
				nd.payType	= kPGPike_PY_Notification;
				nd.payLen	= PGPEndianToUInt16( kPGPBigEndian, mp ) -
								kPGPike_ISAKMPPayloadHeaderSize;
				mp += sizeof(PGPUInt16);
				nd.pay		= mp;
				err = pgpIKEProcessInformational( ike, packet->ipAddress,
						NULL, &nd );	CKERR;
			}
			else
			{
				err = pgpIKELocalAlert( ike, packet->ipAddress,
						kPGPike_AL_InvalidExchange, kPGPike_IA_None, FALSE );
				goto done;
			}
		}
	}
	else
	{
		if( startP1 )
		{
			if( !partner->ready && partner->initiator &&
				IsntNull( partner->exchanges ) )
			{
				if( partner->exchanges->state == kPGPike_S_MM_WaitSA )
					pgpCopyMemory( respCookie, partner->respCookie, kPGPike_CookieSize );
				else
				{
#if PGPIKE_DEBUG
					pgpIKEDebugLog( ike, "\tDetected repeat packet(1)\n" );
#endif
					partner->rttMillisec *= 2;
					goto done;
				}
			}
			else
			{
				err = pgpIKELocalAlert( ike, partner->ipAddress,
						kPGPike_AL_InvalidCookie, kPGPike_IA_None, FALSE );
				goto done;
			}
		}
		else if( !pgpMemoryEqual( respCookie, partner->respCookie,
					kPGPike_CookieSize ) )
		{
			err = pgpIKELocalAlert( ike, partner->ipAddress,
					kPGPike_AL_InvalidCookie, kPGPike_IA_None, FALSE );
			goto done;
		}
	}
	if( IsntNull( partner ) )
	{
		PGPBoolean				found = FALSE;
		
		/* find the exchange */
		for( exchange = partner->exchanges; IsntNull( exchange );
				exchange = exchange->nextExchange )
		{
			if( ( exchange->exchangeT == exchangeT ) &&
				pgpMemoryEqual( &exchange->messageID, messageID,
					sizeof( PGPikeMessageID ) ) )
			{
				found = TRUE;
				break;
			}
		}
		if( !found )
		{
			if( startP1 )
			{
				err = pgpIKECreateExchange( partner, kPGPike_EX_Identity, NULL,
												kPGPike_S_MM_WaitSA, FALSE,
												&exchange );CKERR;
			}
			else if( ( exchangeT == kPGPike_EX_Informational ) ||
				( ( exchangeT == kPGPike_EX_IPSEC_Quick ) &&
				partner->ready ) )
			{
				err = pgpIKECreateExchange( partner, exchangeT, &messageID,
						( exchangeT == kPGPike_EX_Informational ) ?
						kPGPike_S_ND_Informational : kPGPike_S_QM_WaitSA,
						FALSE, &exchange );	CKERR;
			}
			else
			{
				err = pgpIKELocalAlert( ike,  partner->ipAddress,
							kPGPike_AL_InvalidMessageID, kPGPike_IA_None, FALSE );
				goto done;
			}
		}
		pgpAssertAddrValid( exchange, PGPikeExchange * );
		
		{
			PGPByte *	cp,
					*	ecp;
			
			cp = packet->packet;
			ecp = cp + packet->packetSize;
			while( cp < ecp )
				thisCksm += *cp++;
		}
		/*	Check for repeat packets because this silly protocol
			doesn't have sequence numbers						*/
		if( ( packet->packetSize == exchange->lastRcvdLength ) &&
			( thisCksm == exchange->lastRcvdCksm ) )
		{
			/* just let ourselves time out because we may have already resent */
			partner->rttMillisec *= 2;
#if PGPIKE_DEBUG
			pgpIKEDebugLog( ike, "\tDetected repeat packet(2)\n" );
#endif
			goto done;
		}
		exchange->lastRcvdLength	= packet->packetSize;
		exchange->lastRcvdCksm		= thisCksm;
		
		/* average in estimate of round trip time */
		if( partner->rttMillisec )
		{
			partner->rttMillisec += PGPGetMilliseconds() - exchange->lastTransmit;
			partner->rttMillisec /= 2;
		}
		else if( exchange->lastTransmit )
		{
			partner->rttMillisec = PGPGetMilliseconds() - exchange->lastTransmit;
		}

		if( flags & kPGPike_ISAKMPEncryptBit )
		{
			if( PGPCBCContextRefIsValid( partner->cbc ) )
			{
				err = PGPInitCBC( partner->cbc, partner->cipherKey,
									exchange->lastCBC );	CKERR;
				/* Save the IV for the next packet */
				pgpCopyMemory( ep - kPGPike_MaxCipherBlockSize,
								exchange->lastCBC, kPGPike_MaxCipherBlockSize );
				if( ( exchange->exchangeT == kPGPike_EX_Identity ) ||
					( exchange->exchangeT == kPGPike_EX_Aggressive ) )
					pgpCopyMemory( exchange->lastCBC,
								partner->lastP1CBC, kPGPike_MaxCipherBlockSize );
				/* Decrypt packet */
				err = PGPCBCDecrypt( partner->cbc,
								packet->packet + kPGPike_ISAKMPHeaderSize,
								mLen - kPGPike_ISAKMPHeaderSize,
								packet->packet + kPGPike_ISAKMPHeaderSize );CKERR;
			}
			else
			{
				err = pgpIKEAbortExchange( &exchange, 
						kPGPike_AL_InvalidPayload );CKERR;
			}
		}
		if( flags & kPGPike_ISAKMPCommitBit )
		{
			exchange->needsCommit = TRUE;
		}
		if( flags & kPGPike_ISAKMPAuthOnlyBit )
		{
#if PGPIKE_DEBUG
			pgpIKEDebugLog( ike, "\tDetected auth-only IKE, rejecting\n" );
#endif
			goto done;
		}
		err = pgpIKEPayloadLengthCheck( partner, firstPayload, mp, ep,
										&valid );	CKERR;
		if( !valid )
		{
			err = pgpIKEAbortExchange( &exchange,
						kPGPike_AL_UnequalPayloadLengths );	CKERR;
		}
		
		/* OK, everything looks cool, now on to the payloads */
		err = pgpIKEProcessPayloads( exchange, firstPayload, mp );CKERR;
	}
	
done:
	return err;	
}

	PGPError
pgpIKEProcessPayloads(
	PGPikeExchange *		exchange,
	PGPikePayload			firstPayload,
	PGPByte *				mp )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner = exchange->partner;
	PGPikePayload			payload,
							nextPayload;
	PGPUInt16				payLength;
	PGPikeProposal *		proposal = NULL;
	PGPikeSA *				sa;
	PGPBoolean				rcvdSA = FALSE;
	PGPBoolean				rcvdHash = FALSE;
	PGPBoolean				found = FALSE;
	PGPBoolean				sendRL = FALSE;
	PGPByte					hashPY[kPGPike_MaxHashSize];
	PGPHMACContextRef		p2hmac = kInvalidPGPHMACContextRef;
	PGPikeNDPayload	*		ndPays = NULL,
					*		wNDPays;
	PGPBoolean				sharedKey;
	
	if( ( ( exchange->state == kPGPike_S_MM_WaitSA ) &&
			( firstPayload != kPGPike_PY_SA ) ) ||
		( ( exchange->state == kPGPike_S_QM_WaitSA ) &&
			( ( firstPayload != kPGPike_PY_Hash ) ||
			  ( (PGPikePayload)*mp != kPGPike_PY_SA ) ) ) ||
		  ( ( exchange->state == kPGPike_S_ND_Informational ) &&
		  	partner->ready && ( firstPayload != kPGPike_PY_Hash ) ) )
	{
		err = pgpIKEAbortExchange( &exchange,
						kPGPike_AL_InvalidPayload );
		goto done;
	}
	if( PGPCBCContextRefIsValid( partner->cbc ) &&
		( ( exchange->exchangeT == kPGPike_EX_IPSEC_Quick ) ||
		( ( exchange->exchangeT == kPGPike_EX_Informational ) &&
			( firstPayload == kPGPike_PY_Hash ) ) ) )
	{
		pgpClearMemory( hashPY, kPGPike_MaxHashSize );
		err = PGPNewHMACContext( partner->ike->memMgr,
						partner->sdkHashAlg,
						partner->skeyid_a,
						partner->hashSize,
						&p2hmac );	CKERR;
		if( !exchange->initiator && ( exchange->state == kPGPike_S_QM_WaitHash3 ) )
		{
			err = PGPContinueHMAC( p2hmac, "\0", 1 );	CKERR;
		}
		err = PGPContinueHMAC( p2hmac, exchange->messageID,
								sizeof(PGPikeMessageID) );	CKERR;
		if( exchange->exchangeT == kPGPike_EX_IPSEC_Quick )
		{
			if( ( exchange->initiator && ( exchange->state == kPGPike_S_QM_WaitSA ) ) ||
				( !exchange->initiator && ( exchange->state == kPGPike_S_QM_WaitHash3 ) ) )
				err = PGPContinueHMAC( p2hmac, exchange->initNonce,
											exchange->initNonceLen );CKERR;
			if( !exchange->initiator && ( exchange->state == kPGPike_S_QM_WaitHash3 ) )
				err = PGPContinueHMAC( p2hmac, exchange->respNonce,
											exchange->respNonceLen );CKERR;
		}
	}
	payload = firstPayload;
	while( payload != kPGPike_PY_None )
	{
		PGPByte	*	pp,
				*	ep;
		
		nextPayload = (PGPikePayload)*mp++;
		mp++;	/* reserved */
		payLength = PGPEndianToUInt16( kPGPBigEndian, mp );
		mp += sizeof(PGPUInt16);
		if( PGPHMACContextRefIsValid( p2hmac ) && ( payload != kPGPike_PY_Hash ) )
		{
			err = PGPContinueHMAC( p2hmac, mp - kPGPike_ISAKMPPayloadHeaderSize,
							payLength );	CKERR;
		}
		payLength -= kPGPike_ISAKMPPayloadHeaderSize;
		pp = mp;
		ep = mp + payLength;
		switch( payload )
		{
			case kPGPike_PY_SA:
			{
				PGPikeDOI			doi;
				PGPUInt32			situation;
				PGPikePayload		nextSAPayload;
				PGPUInt16			saPayLength;
				PGPikeAlert			alert = kPGPike_AL_None;
				PGPikeProposal *	nproposal;
				PGPBoolean			valid = FALSE;
				
				if( rcvdSA )	/* we only handle one SA payload */
					break;
				rcvdSA = TRUE;
				if( ( exchange->state != kPGPike_S_QM_WaitSA ) &&
					( exchange->state != kPGPike_S_MM_WaitSA ) )
					goto invalidPayload;
				if( !exchange->initiator &&
					( exchange->state == kPGPike_S_MM_WaitSA ) )
				{
					/* save the SA payload body for HASH_X */
					pgpAssert( IsNull( exchange->initSABody ) );
					exchange->initSABodySize = payLength;
					exchange->initSABody = PGPNewData(
									exchange->ike->memMgr,
									payLength, kPGPMemoryMgrFlags_Clear );
					if( IsNull( exchange->initSABody ) )
					{
						err = kPGPError_OutOfMemory;
						goto done;
					}
					pgpCopyMemory( pp, exchange->initSABody, payLength );
				}
				if( ep - pp < sizeof(PGPUInt32) * 2 )
				{
					err = pgpIKEAbortExchange( &exchange,
								kPGPike_AL_UnequalPayloadLengths );
					goto done;
				}
				/* doi */
				doi = (PGPikeDOI)PGPEndianToUInt32( kPGPBigEndian, pp );
				pp += sizeof( PGPikeDOI );
				if( doi != kPGPike_DOI_IPSEC )
				{
					err = pgpIKEAbortExchange( &exchange,
								kPGPike_AL_DOIUnsupported );
					goto done;
				}
				/* situation */
				situation = PGPEndianToUInt32( kPGPBigEndian, pp );
				pp += sizeof( PGPUInt32 );
				if( ( situation & kPGPike_IPSEC_SIT_Integrity ) ||
					( situation & kPGPike_IPSEC_SIT_Secrecy ) ||
					!( situation & kPGPike_IPSEC_SIT_IdentityOnly ))
				{
					err = pgpIKEAbortExchange( &exchange,
								kPGPike_AL_SituationUnsupported );
					goto done;
				}
				/* Proposals */
				do
				{
					nproposal = NULL;
					
					if( ep - pp < kPGPike_ISAKMPPayloadHeaderSize )
					{
						err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_UnequalPayloadLengths );
						goto done;
					}
					nextSAPayload = (PGPikePayload)*pp++;	pp++;
					saPayLength = PGPEndianToUInt16( kPGPBigEndian, pp ) -
									kPGPike_ISAKMPPayloadHeaderSize;
					pp += sizeof(PGPUInt16);
					
					err = pgpIKEProcessProposal( &exchange, pp, ep,
												&alert, &nproposal );
					if( IsPGPError( err ) || ( alert != kPGPike_AL_None ) )
						goto done;
					if( IsntNull( nproposal ) )
					{
						PGPikeProposal *wproposal = proposal;
						
						if( IsNull( wproposal ) )
							proposal = nproposal;
						else
						{
							while( IsntNull( wproposal->nextProposal ) )
								wproposal = wproposal->nextProposal;
							wproposal->nextProposal = nproposal;
						}
					}
					pp += saPayLength;
				} while( nextSAPayload == kPGPike_PY_Proposal );
				if( IsNull( proposal ) )
				{
					if( alert == kPGPike_AL_None )
						err = pgpIKEAbortExchange( &exchange,
								kPGPike_AL_BadProposal );
					goto done;
				}
				err = pgpIKESelectProposal( exchange, proposal, &valid );CKERR;
				if( !valid )
				{
					err = pgpIKEAbortExchange( &exchange,
							kPGPike_AL_NoProposalChoice );
					goto done;
				}
				break;
			}
			case kPGPike_PY_KeyExchange:
				if( ( exchange->state == kPGPike_S_MM_WaitKE ) ||
					( exchange->state == kPGPike_S_QM_WaitSA ) )
				{
					err = pgpIKEProcessKE( &exchange, pp,
											payLength );CKERR;
				}
				else
					goto invalidPayload;
				break;
			case kPGPike_PY_Identification:
				if( exchange->exchangeT == kPGPike_EX_IPSEC_Quick )
				{
					if( exchange->initiator )
					{
						PGPBoolean	goodID = FALSE;
						
						if( !exchange->checkedIDci )
						{
							if( ( payLength == exchange->idBodySize ) &&
								pgpMemoryEqual( pp, exchange->idBody, payLength ) )
							{
								exchange->checkedIDci = TRUE;
								goodID = TRUE;
							}
						}
						else if( !exchange->checkedIDcr )
						{
							if( ( payLength == exchange->idRBodySize ) &&
								pgpMemoryEqual( pp, exchange->idRBody, payLength ) )
							{
								exchange->checkedIDcr = TRUE;
								goodID = TRUE;
							}
						}
						if( !goodID )
						{
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike, "\tToo many IDs rcvd.\n" );
#endif
							err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_InvalidID );
							goto done;
						}
					}
					else
					{
						if( IsNull( exchange->idBody ) )
						{
							goto saveIDBody;
						}
						if( IsNull( exchange->idRBody ) )
						{
							exchange->idRBodySize = payLength;
							exchange->idRBody = PGPNewData( exchange->ike->memMgr,
								payLength, kPGPMemoryMgrFlags_Clear );
							if( IsNull( exchange->idBody ) )
							{
								err = kPGPError_OutOfMemory;
								goto done;
							}
							pgpCopyMemory( pp, exchange->idRBody, payLength );
						}
						else
						{
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike, "\tToo many IDs rcvd.\n" );
#endif
							err = pgpIKEAbortExchange( &exchange, kPGPike_AL_InvalidID );
							goto done;
						}
					}
				}
				else
				{
					PGPByte		ipProtocol;
					PGPUInt16	ipPort;
					
				saveIDBody:
					/* save the ID payload for HASH_X */
					if( IsntNull( exchange->idBody ) )
					{
						(void)PGPFreeData( exchange->idBody );
						exchange->idBody = NULL;
					}
					exchange->idBodySize = payLength;
					exchange->idBody = PGPNewData( exchange->ike->memMgr,
											payLength, kPGPMemoryMgrFlags_Clear );
					if( IsNull( exchange->idBody ) )
					{
						err = kPGPError_OutOfMemory;
						goto done;
					}
					pgpCopyMemory( pp, exchange->idBody, payLength );
#if PGPIKE_DEBUG
					pgpIKEDebugLog( exchange->ike,
						"\tRemote ID: %s\n", pgpIKEIDTypeString( (PGPipsecIdentity)*pp ) );
#endif
					*pp++;
					ipProtocol = *pp++;
					ipPort = PGPEndianToUInt16( kPGPBigEndian, pp );
					pp += sizeof(PGPUInt16);
					if( ( exchange->exchangeT != kPGPike_EX_IPSEC_Quick ) &&
						( ( ( ipProtocol != 0 ) && (ipProtocol != kPGPike_IPProtocolUDP ) ) ||
						( ( ipPort != 0 ) && ( ipPort != kPGPike_CommonPort ) ) ) )
					{
						err = pgpIKEAbortExchange( &exchange, kPGPike_AL_InvalidID );
						goto done;
					}
				}
				break;
			case kPGPike_PY_Certificate:
				if( ( exchange->state == kPGPike_S_MM_WaitFinal ) &&
					( ( exchange->proposals->t[0].u.ike.authMethod ==
						kPGPike_AM_DSS_Sig ) ||
						( exchange->proposals->t[0].u.ike.authMethod ==
						kPGPike_AM_RSA_Sig ) ) )
				{
					err = pgpIKEProcessCert( &exchange, pp, payLength );CKERR;
				}
				else
					goto invalidPayload;
				break;
			case kPGPike_PY_Signature:
				if( ( exchange->state == kPGPike_S_MM_WaitFinal ) &&
					( ( exchange->proposals->t[0].u.ike.authMethod ==
						kPGPike_AM_DSS_Sig ) ||
						( exchange->proposals->t[0].u.ike.authMethod ==
						kPGPike_AM_RSA_Sig ) ) )
				{
					err = pgpIKEProcessSig( &exchange, pp, payLength );CKERR;
				}
				else
					goto invalidPayload;
				break;
			case kPGPike_PY_CertRequest:
				err = pgpIKEProcessCertRequest( &exchange, pp, payLength );CKERR;
				break;
			case kPGPike_PY_Hash:
				if( !rcvdHash && ( partner->hashSize == payLength ) )
				{
					switch( exchange->state )
					{
						case kPGPike_S_MM_WaitFinal:
							switch( exchange->proposals->t[0].u.ike.authMethod )
							{
								case kPGPike_AM_PreSharedKey:
								case kPGPike_AM_RSA_Encrypt:
								case kPGPike_AM_RSA_Encrypt_R:
								{
									PGPByte		lHash[kPGPike_MaxHashSize];
									
									err = pgpIKEGetAuthHash( exchange,
												(PGPBoolean)!exchange->initiator,
												lHash );	CKERR;
									if( pgpMemoryEqual( lHash, pp,
											partner->hashSize ) )
									{
										partner->authenticated = TRUE;
									}
									else
									{
										err = pgpIKEAbortExchange( &exchange,
													kPGPike_AL_AuthenticationFailed );
										goto done;
									}
									break;
								}
								default:
									goto invalidPayload;
							}
							break;
						case kPGPike_S_QM_WaitSA:
						case kPGPike_S_QM_WaitHash3:
						case kPGPike_S_ND_Informational:
							pgpCopyMemory( pp, hashPY, payLength );
							break;
						default:
							goto invalidPayload;
					}
					rcvdHash = TRUE;
				}
				else
					goto invalidPayload;	
				break;
			case kPGPike_PY_Nonce:
				if( ( exchange->state == kPGPike_S_MM_WaitKE ) ||
					( exchange->state == kPGPike_S_QM_WaitSA ) )
				{
					if( ( payLength >= kPGPike_MinNonceSize ) &&
						( payLength <= kPGPike_MaxNonceSize ) )
					{
						if( exchange->initiator )
						{
							exchange->respNonceLen = payLength;
							pgpCopyMemory( pp, exchange->respNonce,
											payLength );
						}
						else
						{
							exchange->initNonceLen = payLength;
							pgpCopyMemory( pp, exchange->initNonce,
											payLength );
						}
					}
					else
						goto invalidPayload;
				}
				else
					goto invalidPayload;
				break;
			case kPGPike_PY_Notification:
			case kPGPike_PY_Delete:
			{
				PGPikeNDPayload *	newND;
				
				/* save off the notify and delete payloads to make
					sure the QM hash is approved prior to action */
				newND = PGPNewData( exchange->ike->memMgr,
								sizeof(PGPikeNDPayload), kPGPMemoryMgrFlags_Clear );
				if( IsntNull( newND ) )
					newND->pay = PGPNewData( exchange->ike->memMgr,
								payLength, 0 );
				if( IsntNull( newND ) && IsntNull( newND->pay ) )
				{
					pgpCopyMemory( pp, newND->pay, payLength );
					newND->payLen	= payLength;
					newND->payType	= payload;
					newND->nextND	= ndPays;
					ndPays			= newND;
				}
				else
				{
					err = kPGPError_OutOfMemory;
					goto done;
				}
				break;
			}
			case kPGPike_PY_VendorID:
			{
				PGPSize	vidSize = strlen( kPGPike_PGPVendorString1 );
				
				if( pgpMemoryEqual( kPGPike_PGPVendorString1, pp, vidSize ) )
				{
					partner->remotePGPVersion = 1;
					if( payLength > vidSize )
					{
						partner->remotePGPVersion = 2;
#if PGPIKE_DEBUG
						pgpIKEDebugLog( exchange->ike, "\tDetected PGPikeV2\n" );
#endif
					}
				}
				break;
			}
			default:
			invalidPayload:
				err = pgpIKEAbortExchange( &exchange,
								kPGPike_AL_InvalidPayload );
				goto done;
		}
		payload = nextPayload;
		mp += payLength;
	}
	if( PGPHMACContextRefIsValid( p2hmac ) )
	{
		PGPByte *	p2hmacFinal[kPGPike_MaxHashSize];
		
		err = PGPFinalizeHMAC( p2hmac, p2hmacFinal );	CKERR;
		if( !rcvdHash ||
			!pgpMemoryEqual( p2hmacFinal, hashPY, partner->hashSize ) )
		{
			err = pgpIKEAbortExchange( &exchange,
							kPGPike_AL_AuthenticationFailed );
			goto done;
		}
	}
	if( IsntNull( ndPays ) )
	{
		for( wNDPays = ndPays; IsntNull( wNDPays); wNDPays = wNDPays->nextND )
		{
			err = pgpIKEProcessInformational( exchange->ike,
					exchange->partner->ipAddress, &exchange, wNDPays );	CKERR;
			if( IsNull( exchange ) )
				goto done;
			
		}
	}
	if( exchange->initiator )
	{
		switch( exchange->state )
		{
			case kPGPike_S_MM_WaitSA:
				switch( exchange->proposals->t[0].u.ike.authMethod )
				{
					case kPGPike_AM_PreSharedKey:
					case kPGPike_AM_DSS_Sig:
					case kPGPike_AM_RSA_Sig:
						err = pgpIKELoadGroup( exchange );	CKERR;
						err = pgpIKEDoPacket( &exchange,
								kPGPike_PY_KeyExchange,
								kPGPike_PY_Nonce,
								kPGPike_PY_None );	CKERR;
						break;
					case kPGPike_AM_RSA_Encrypt:
					case kPGPike_AM_RSA_Encrypt_R:
					default:
						pgpAssert( 0 );	/* unsupported */
						break;
				}
				exchange->state = kPGPike_S_MM_WaitKE;
				break;
			case kPGPike_S_MM_WaitKE:
			{
				PGPikePartner *	oPartner;
				
				found = FALSE;
				if( ( exchange->respNonceLen == 0 ) ||
					( IsNull( exchange->dhYr ) ) )
				{
					err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_InvalidPayload );
					goto done;
				}
				/* encrypto mondo */
				err = pgpIKEGoSecure( exchange );	CKERR;
				
				for( oPartner = exchange->ike->partner; IsntNull( oPartner );
						oPartner = oPartner->nextPartner )
				{
					if( ( oPartner->ipAddress == partner->ipAddress ) &&
						( oPartner != partner ) )
						found = TRUE;
				}
				exchange->outAlert = kPGPike_AL_InitialContact;
				exchange->outInfoProtocol = kPGPike_PR_IKE;
				exchange->outInfoSPICount = 0;
				
				switch( exchange->proposals->t[0].u.ike.authMethod )
				{
					case kPGPike_AM_RSA_Encrypt:
					case kPGPike_AM_RSA_Encrypt_R:
						err = pgpIKEDoPacket( &exchange,
								kPGPike_PY_Hash,
								!found ? kPGPike_PY_Notification : kPGPike_PY_Skip,
								kPGPike_PY_None );	CKERR;
						break;
					case kPGPike_AM_PreSharedKey:
						err = pgpIKEDoPacket( &exchange,
								kPGPike_PY_Identification,
								kPGPike_PY_Hash,
								!found ? kPGPike_PY_Notification : kPGPike_PY_Skip,
								kPGPike_PY_None );	CKERR;
						break;
					case kPGPike_AM_DSS_Sig:
					case kPGPike_AM_RSA_Sig:
						err = pgpIKEDoPacket( &exchange,
								kPGPike_PY_Identification,
								kPGPike_PY_Certificate,
								kPGPike_PY_Signature,
								kPGPike_PY_CertRequest,
								!found ? kPGPike_PY_Notification : kPGPike_PY_Skip,
								kPGPike_PY_None );	CKERR;
						break;
					default:
						pgpAssert( 0 );	/* unsupported */
						break;
				}
				exchange->state = kPGPike_S_MM_WaitFinal;
				break;
			}
			case kPGPike_S_MM_WaitFinal:
				if( partner->authenticated )
				{
					partner->kbLifeTime		= exchange->proposals->kbLifeTime;
					partner->secLifeTime	= exchange->proposals->secLifeTime;
					partner->birthTime		= PGPGetTime();
					if( PGPKeySetRefIsValid( partner->remoteKeySet ) )
						PGPFreeKeySet( partner->remoteKeySet );
					partner->remoteKeySet	= kInvalidPGPKeySetRef;
					partner->remoteKey		= kInvalidPGPKeyRef;
					partner->remoteCert		= kInvalidPGPSigRef;
					if( IsntNull( partner->pgpAuthKey.pass ) )
						PGPFreeData( partner->pgpAuthKey.pass );
					partner->pgpAuthKey.pass = NULL;
					partner->pgpAuthKey.passLength = 0;
					if( IsntNull( partner->x509AuthKey.pass ) )
						PGPFreeData( partner->x509AuthKey.pass );
					partner->x509AuthKey.pass = NULL;
					partner->x509AuthKey.passLength = 0;
					partner->ready			= TRUE;
					err = pgpIKEFreeExchange( exchange );
					exchange = NULL;
					err = pgpIKELocalAlert( partner->ike, partner->ipAddress,
							kPGPike_AL_None, kPGPike_IA_NewPhase1SA, FALSE );
#if PGPIKE_DEBUG
					pgpIKEDebugLog( partner->ike, "\tPhase 1 SA Negotiated(I)\n" );
#endif
					for( found = FALSE, sa = partner->ike->sa; IsntNull( sa ); sa = sa->nextSA )
						if( ( sa->ipAddress == partner->ipAddress ) &&
							( sa->ipAddrStart == partner->ipsecOpts.ipAddrStart ) &&
							( sa->ipMaskEnd == partner->ipsecOpts.ipMaskEnd ) &&
							( sa->destIsRange == partner->ipsecOpts.destIsRange ) &&
							sa->activeOut )
						{
							found = TRUE;
							break;
						}
					if( !found )
					{
						/* Phase 1 complete, begin Phase 2 if no P2 SAs exist */
						err = pgpIKEStartQMExchange( partner, &exchange );	CKERR;
					}
				}
				else
				{
					err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_AuthenticationFailed );
					goto done;
				}
				break;
			case kPGPike_S_QM_WaitSA:
				if( exchange->respNonceLen > 0 )
				{
					if( IsntNull( exchange->idBody ) &&
						( !exchange->checkedIDci || !exchange->checkedIDcr ) )
					{
#if PGPIKE_DEBUG
						pgpIKEDebugLog( exchange->ike, "\tOnly one client ID rcvd!\n" );
#endif
						err = pgpIKEAbortExchange( &exchange,
										kPGPike_AL_InvalidID );
						goto done;
					}
					if( ( exchange->proposals->t[0].u.ipsec.groupID !=
						kPGPike_GR_None ) && IsNull( exchange->dhYr ) )
					{
						err = pgpIKEAbortExchange( &exchange,
										kPGPike_AL_AuthenticationFailed );
						goto done;
					}
					exchange->state = kPGPike_S_QM_WaitHash3;
					err = pgpIKEDoPacket( &exchange, kPGPike_PY_None );CKERR;
					/* Phase 2 complete.  Generate the SAs */
					if( !exchange->needsCommit )
					{
						err = pgpIKECompletePhase2( exchange, &sa );	CKERR;
						err = pgpIKESAEstablished( exchange->ike, sa );	CKERR;
						err = pgpIKEFreeExchange( exchange );			CKERR;
						exchange = NULL;
					}
					else
						exchange->complete = TRUE;
				}
				else
				{
					err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_InvalidPayload );
					goto done;
				}
				break;
			case kPGPike_S_QM_WaitHash3:
				/* we would only legally receive something in this state
					when the commit bit was set by the responder and we
					are waiting for a connected notify message */
				if( !exchange->needsCommit )
				{
					err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_InvalidPayload );
					goto done;
				}
				break;
		}
	}
	else
	{
		switch( exchange->state )
		{
			case kPGPike_S_MM_WaitSA:
				exchange->outAlert	= kPGPike_AL_ResponderLifetime;
				exchange->outRLSeconds = exchange->outRLKB = FALSE;
				exchange->outInfoProtocol = kPGPike_PR_IKE;
				exchange->outInfoSPICount = 0;
				if( ( ( exchange->proposals->secLifeTime > exchange->ike->secLifeTimeIKE ) &&
					exchange->ike->secLifeTimeIKE ) ||
					( !exchange->proposals->secLifeTime && exchange->ike->secLifeTimeIKE ) )
				{
					exchange->outRLSeconds	= TRUE;
					sendRL = TRUE;
				}
				if( ( ( exchange->proposals->kbLifeTime > exchange->ike->kbLifeTimeIKE ) &&
					exchange->ike->kbLifeTimeIKE ) ||
					( !exchange->proposals->kbLifeTime && exchange->ike->kbLifeTimeIKE ) )
				{
					exchange->outRLKB		= TRUE;
					sendRL = TRUE;
				}
				err = pgpIKEDoPacket( &exchange, 
							kPGPike_PY_SA,
							sendRL ? kPGPike_PY_Notification : kPGPike_PY_Skip,
							kPGPike_PY_VendorID,
							kPGPike_PY_None );CKERR;
				err = pgpIKELoadGroup( exchange );	CKERR;
				exchange->state = kPGPike_S_MM_WaitKE;
				break;
			case kPGPike_S_MM_WaitKE:
				if( ( exchange->initNonceLen == 0 ) ||
					( IsNull( exchange->dhYi ) ) )
				{
					err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_InvalidPayload );
					goto done;
				}
				switch( exchange->proposals->t[0].u.ike.authMethod )
				{
					case kPGPike_AM_PreSharedKey:
					case kPGPike_AM_DSS_Sig:
					case kPGPike_AM_RSA_Sig:
						sharedKey = FALSE;
						if( exchange->proposals->t[0].u.ike.authMethod ==
							kPGPike_AM_PreSharedKey )
							sharedKey = TRUE;
						err = pgpIKEDoPacket( &exchange,
								kPGPike_PY_KeyExchange,
								kPGPike_PY_Nonce,
								sharedKey ? kPGPike_PY_Skip : kPGPike_PY_CertRequest,
								kPGPike_PY_None );CKERR;
						break;
					case kPGPike_AM_RSA_Encrypt:
					case kPGPike_AM_RSA_Encrypt_R:
					default:
						pgpAssert( 0 );	/* unsupported */
						break;
				}
				/* encrypto mondo */
				err = pgpIKEGoSecure( exchange );	CKERR;
				exchange->state = kPGPike_S_MM_WaitFinal;
				break;
			case kPGPike_S_MM_WaitFinal:
				if( partner->authenticated )
				{
					switch( exchange->proposals->t[0].u.ike.authMethod )
					{
						case kPGPike_AM_PreSharedKey:
							err = pgpIKEDoPacket( &exchange,
									kPGPike_PY_Identification,
									kPGPike_PY_Hash,
									kPGPike_PY_None );	CKERR;
							break;
						case kPGPike_AM_DSS_Sig:
						case kPGPike_AM_RSA_Sig:
							err = pgpIKEDoPacket( &exchange,
									kPGPike_PY_Identification,
									kPGPike_PY_Certificate,
									kPGPike_PY_Signature,
									kPGPike_PY_None );	CKERR;
							break;
						case kPGPike_AM_RSA_Encrypt:
						case kPGPike_AM_RSA_Encrypt_R:
							err = pgpIKEDoPacket( &exchange,
									kPGPike_PY_Hash,
									kPGPike_PY_None );	CKERR;
							break;
						default:
							pgpAssert( 0 );	/* unsupported */
							break;
					}
					partner->kbLifeTime		= exchange->proposals->kbLifeTime;
					partner->secLifeTime	= exchange->proposals->secLifeTime;
					partner->birthTime		= PGPGetTime();
					if( PGPKeySetRefIsValid( partner->remoteKeySet ) )
						PGPFreeKeySet( partner->remoteKeySet );
					partner->remoteKeySet	= kInvalidPGPKeySetRef;
					partner->remoteKey		= kInvalidPGPKeyRef;
					partner->remoteCert		= kInvalidPGPSigRef;
					if( IsntNull( partner->pgpAuthKey.pass ) )
						PGPFreeData( partner->pgpAuthKey.pass );
					partner->pgpAuthKey.pass = NULL;
					partner->pgpAuthKey.passLength = 0;
					if( IsntNull( partner->x509AuthKey.pass ) )
						PGPFreeData( partner->x509AuthKey.pass );
					partner->x509AuthKey.pass = NULL;
					partner->x509AuthKey.passLength = 0;
					partner->ready			= TRUE;
					err = pgpIKEFreeExchange( exchange );
					exchange = NULL;
					err = pgpIKELocalAlert( partner->ike, partner->ipAddress,
							kPGPike_AL_None, kPGPike_IA_NewPhase1SA, FALSE );
#if PGPIKE_DEBUG
					pgpIKEDebugLog( partner->ike, "\tPhase 1 SA Negotiated(R)\n" );
#endif
				}
				else
				{
					err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_AuthenticationFailed );
					goto done;
				}
				break;
			case kPGPike_S_QM_WaitSA:
				if( exchange->initNonceLen > 0 )
				{
					PGPBoolean				pfs = FALSE,
											idC = FALSE;
					
					exchange->state = kPGPike_S_QM_WaitHash3;
					if( IsntNull( exchange->idBody ) != IsntNull( exchange->idRBody ) )
					{
#if PGPIKE_DEBUG
						pgpIKEDebugLog( exchange->ike, "\tOnly one client ID rcvd!\n" );
#endif
						err = pgpIKEAbortExchange( &exchange,
										kPGPike_AL_InvalidID );
						goto done;
					}
					if( IsntNull( exchange->idBody ) )
					{
						PGPUInt32				idIP,
												fullMask;
						PGPBoolean				rrange;
						PGPikeMTClientIDCheck	msg;
						
						/* interpret IDci */
						if( IsPGPError( pgpIKEProcessP2ID( exchange,
								exchange->idBody,
								exchange->idBodySize,
								&rrange,
								&idIP,
								&fullMask ) ) )
						{
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike, "\tClient ID Parse error!\n" );
#endif
							err = pgpIKEAbortExchange( &exchange,
											kPGPike_AL_InvalidID );
							goto done;
						}
#if PGPIKE_DEBUG
						pgpIKEDebugLog( exchange->ike, "\tIDci: %u.%u.%u.%u/%u.%u.%u.%u\n",
							( idIP >> 24 ), ( idIP >> 16 ) & 0xFF, ( idIP >> 8 ) & 0xFF,
							( idIP & 0xFF ), ( fullMask >> 24 ), ( fullMask >> 16 ) & 0xFF,
							( fullMask >> 8 ) & 0xFF, ( fullMask & 0xFF ) );
#endif
						pgpClearMemory( &msg, sizeof(PGPikeMTClientIDCheck) );
						msg.ipAddress		= partner->ipAddress;
						msg.destIsRange		= rrange;
						msg.ipAddrStart		= idIP;
						msg.ipMaskEnd		= fullMask;
						err = (exchange->ike->msgProc)( (PGPikeContextRef)exchange->ike,
								exchange->ike->userData, kPGPike_MT_ClientIDCheck, &msg );CKERR;
						if( !msg.approved )
						{
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike, "\tClient ID not approved.\n" );
#endif
							err = pgpIKEAbortExchange( &exchange,
											kPGPike_AL_InvalidID );
							goto done;
						}
						exchange->partner->ipsecOpts.ipAddrStart	= idIP;
						exchange->partner->ipsecOpts.ipMaskEnd		= fullMask;
						exchange->partner->ipsecOpts.destIsRange	= rrange;
						/* make sure IDcr is our local IP here */
						/* ***** change this to support local gateway! */
						if( IsPGPError( pgpIKEProcessP2ID( exchange, exchange->idRBody,
							exchange->idRBodySize, &rrange, &idIP, &fullMask ) ) ||
							( idIP != partner->localIPAddress ) ||
							( fullMask != 0xFFFFFFFF ) || rrange )
						{
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike, "\tIDcr is not our ID!\n" );
#endif
							err = pgpIKEAbortExchange( &exchange,
											kPGPike_AL_InvalidID );
							goto done;
						}
						(void)PGPFreeData( exchange->idBody );
						(void)PGPFreeData( exchange->idRBody );
						exchange->idBody = NULL;
						exchange->idRBody = NULL;
						exchange->idBodySize = 0;
						exchange->idRBodySize = 0;
						idC = TRUE;
					}
					if( exchange->proposals->t[0].u.ipsec.groupID !=
						kPGPike_GR_None )
					{
						if( IsNull( exchange->dhYi ) )
						{
							err = pgpIKEAbortExchange( &exchange,
											kPGPike_AL_AuthenticationFailed );
							goto done;
						}
						pfs = TRUE;
						err = pgpIKELoadGroup( exchange );	CKERR;
					}
					exchange->outAlert	= kPGPike_AL_ResponderLifetime;
					exchange->outRLSeconds = exchange->outRLKB = FALSE;
					exchange->outInfoProtocol = exchange->proposals->protocol;
					exchange->outInfoSPICount = 1;
					pgpCopyMemory( &exchange->proposals->initSPI, &exchange->outInfoSPI[0],
									sizeof(PGPipsecSPI) );
					if( ( ( exchange->proposals->secLifeTime > exchange->ike->secLifeTimeIPSEC ) &&
						exchange->ike->secLifeTimeIPSEC ) ||
						( !exchange->proposals->secLifeTime && exchange->ike->secLifeTimeIPSEC ) )
					{
						exchange->outRLSeconds	= TRUE;
						sendRL = TRUE;
					}
					if( ( ( exchange->proposals->kbLifeTime > exchange->ike->kbLifeTimeIPSEC ) &&
						exchange->ike->kbLifeTimeIPSEC ) ||
						( !exchange->proposals->kbLifeTime && exchange->ike->kbLifeTimeIPSEC ) )
					{
						exchange->outRLKB		= TRUE;
						sendRL = TRUE;
					}
					err = pgpIKEDoPacket( &exchange,
								kPGPike_PY_SA,
								kPGPike_PY_Nonce,
								pfs ? kPGPike_PY_KeyExchange : kPGPike_PY_Skip,
								idC ? kPGPike_PY_Identification : kPGPike_PY_Skip,
								idC ? kPGPike_PY_Identification : kPGPike_PY_Skip,
								sendRL ? kPGPike_PY_Notification : kPGPike_PY_Skip,
								kPGPike_PY_None );	CKERR;
				}
				else
				{
					err = pgpIKEAbortExchange( &exchange,
									kPGPike_AL_InvalidPayload );
					goto done;
				}
				break;
			case kPGPike_S_QM_WaitHash3:
			{
				if( !exchange->needsCommit )
				{
					/* Phase 2 complete.  Generate the SAs */
					err = pgpIKECompletePhase2( exchange, &sa );	CKERR;
					err = pgpIKESAEstablished( exchange->ike, sa );	CKERR;
					err = pgpIKEFreeExchange( exchange );	CKERR;
					exchange = NULL;
				}
				else
				{
					exchange->complete = TRUE;
				}
				break;
			}
		}
	}
done:
	if( PGPHMACContextRefIsValid( p2hmac ) )
		(void)PGPFreeHMACContext( p2hmac );
	while( IsntNull( proposal ) )
	{
		PGPikeProposal *	curP = proposal;
		
		proposal = proposal->nextProposal;
		(void)PGPFreeData( curP );
	}
	while( IsntNull( ndPays ) )
	{
		wNDPays = ndPays;
		ndPays = ndPays->nextND;
		(void)PGPFreeData( wNDPays->pay );
		(void)PGPFreeData( wNDPays );
	}
	if( IsntNull( exchange ) &&
		( exchange->exchangeT == kPGPike_EX_Informational ) )
	{
		(void)pgpIKEFreeExchange( exchange );
	}
	return err;	
}

	PGPError
pgpIKEProcessCertRequest(
	PGPikeExchange **				exchangeP,
	PGPByte *						pp,
	PGPSize							payLen )
{
	PGPError						err = kPGPError_NoErr;
	PGPikeExchange *				exchange = *exchangeP;
	PGPikeCertEncoding				ce = (PGPikeCertEncoding)*pp;
	
	if( payLen < 1 )
	{
		err = pgpIKEAbortExchange( &exchange,
						kPGPike_AL_CertUnavailable );
		goto done;
	}
	switch( ce )
	{
		case kPGPike_CE_PGP:
			exchange->partner->remotePGPVersion = 2;
			break;
		case kPGPike_CE_X509_Sig:
			exchange->partner->remotePGPVersion = 0;
			break;
		default:
			/* we only generate a local alert in this case, because in many cases
				these can be ignored */
			err = pgpIKELocalAlert( exchange->ike, exchange->partner->ipAddress,
						kPGPike_AL_CertUnavailable, kPGPike_IA_None, FALSE );	CKERR;
			break;
	}
	
	/* we ignore the CA DN info */
done:
	return err;
}

	PGPError
pgpIKEProcessSig(
	PGPikeExchange **				exchangeP,
	PGPByte *						pp,
	PGPSize							payLen )
{
	PGPError						err = kPGPError_NoErr;
	PGPikeExchange *				exchange = *exchangeP;
	PGPByte							authHash[kPGPike_MaxHashSize];
	PGPPublicKeyContextRef			pubKeyCon = kInvalidPGPPublicKeyContextRef;
	PGPKeyRef						cert;
	
	cert = exchange->partner->remoteKey;
	if( !PGPKeyRefIsValid( cert ) )
	{
		err = pgpIKEAbortExchange( exchangeP,
						kPGPike_AL_CertUnavailable );
		goto done;
	}
	/* There's only one SIG payload in IKE, it signs HASH_I or HASH_R */
	err = pgpIKEGetAuthHash( exchange, (PGPBoolean)!exchange->initiator,
							authHash );CKERR;

	err = PGPNewPublicKeyContext( cert, kPGPPublicKeyMessageFormat_IKE,
									&pubKeyCon );
	if( IsntPGPError( err ) )
		err = PGPPublicKeyVerifyRaw( pubKeyCon, authHash,
				exchange->partner->hashSize, pp, payLen );
	if( IsPGPError( err ) )
	{
		err = kPGPError_NoErr;		/* dont pass PGPError up to caller */
		err = pgpIKEAbortExchange( exchangeP,
						kPGPike_AL_AuthenticationFailed );
		goto done;
	}
	/* phew!  if this checks out, main mode is complete */
	exchange->partner->authenticated = TRUE;
done:
	if( PGPPublicKeyContextRefIsValid( pubKeyCon ) )
		(void)PGPFreePublicKeyContext( pubKeyCon );
	return err;
}

	PGPError
pgpIKEProcessCert(
	PGPikeExchange **				exchangeP,
	PGPByte *						pp,
	PGPSize							payLen )
{
	PGPError						err = kPGPError_NoErr;
	PGPUInt32						numKeys;
	PGPByte							certType;
	PGPikeExchange *				exchange = *exchangeP;
	PGPikePartner *					partner = exchange->partner;
	PGPKeyListRef					keyList = kInvalidPGPKeyListRef;
	PGPKeyIterRef					keyIter = kInvalidPGPKeyIterRef;

	certType = *pp++;
	switch( certType )
	{
		case kPGPike_CE_PGP:
		case kPGPike_CE_X509_Sig:
			if( PGPKeySetRefIsValid( partner->remoteKeySet ) )
			{
				/* ignore this.  we have received multiple CERT payloads
					which means that a certificate chain being sent.
					we don't support this right now.  If a cert doesn't
					verify from one level, it is bad.  */
#if PGPIKE_DEBUG
				pgpIKEDebugLog( exchange->ike,
					"\tReceived extra CERT payload, ignoring\n" );
#endif
				goto done;
			}
			err = PGPImportKeySet( exchange->ike->pgpContext,
					&partner->remoteKeySet,
					PGPOInputBuffer( exchange->ike->pgpContext,
									pp, payLen - 1 ),
					PGPOX509Encoding( exchange->ike->pgpContext,
						(PGPBoolean) ( certType == kPGPike_CE_X509_Sig ) ),
					PGPOLastOption( exchange->ike->pgpContext ) );
			if( IsntPGPError( err ) )
			{
				err = PGPCountKeys( partner->remoteKeySet, &numKeys );
				/* Get just the one key out of it */
				err = PGPOrderKeySet( partner->remoteKeySet,
							kPGPAnyOrdering, &keyList );	CKERR;
				err = PGPNewKeyIter( keyList, &keyIter ); CKERR;
				err = PGPKeyIterNext( keyIter, &partner->remoteKey ); CKERR;
			}
			if( IsPGPError( err ) || ( numKeys != 1 ) )
			{
				err = pgpIKEAbortExchange( exchangeP,
								kPGPike_AL_InvalidCert );
				goto done;
			}
			if( certType == kPGPike_CE_X509_Sig )
			{
				PGPUserIDRef	userID;
				
				err = PGPKeyIterNextUserID( keyIter, &userID );		CKERR;
				err = PGPKeyIterNextUIDSig( keyIter,
						&partner->remoteCert );	CKERR;
			}
			{
				PGPikeMTRemoteCert			rCert;
				
				pgpClearMemory( &rCert, sizeof(PGPikeMTRemoteCert) );
				rCert.ipAddress		= partner->ipAddress;
				rCert.remoteKey		= partner->remoteKey;
				rCert.remoteCert	= partner->remoteCert;
				err = (partner->ike->msgProc)( (PGPikeContextRef)partner->ike,
												partner->ike->userData,
												kPGPike_MT_RemoteCert,
												&rCert );
				if( IsntPGPError( err ) )
				{
					if( !rCert.approved )
					{
						if( PGPKeyListRefIsValid( keyList ) )
							(void)PGPFreeKeyList( keyList );
						keyList = kInvalidPGPKeyListRef;
						if( PGPKeyIterRefIsValid( keyIter ) )
							(void)PGPFreeKeyIter( keyIter );
						keyIter = kInvalidPGPKeyIterRef;
						err = pgpIKEAbortExchange( exchangeP,
										kPGPike_AL_UnsupportedCert );
					}
				}
			}
			break;
		case kPGPike_CE_DNSSig:
		case kPGPike_CE_X509_PKCS7:
		case kPGPike_CE_X509_Exchange:
		case kPGPike_CE_KerberosToken:
		case kPGPike_CE_CRL:
		case kPGPike_CE_ARL:
		case kPGPike_CE_SPKI:
		case kPGPike_CE_X509_Attrib:
		default:
			err = pgpIKEAbortExchange( exchangeP,
							kPGPike_AL_InvalidCertEncoding );
			goto done;
	}
done:
	if( PGPKeyListRefIsValid( keyList ) )
		(void)PGPFreeKeyList( keyList );
	if( PGPKeyIterRefIsValid( keyIter ) )
		(void)PGPFreeKeyIter( keyIter );
	return err;
}

	PGPError
pgpIKEProcessKE(
	PGPikeExchange **				exchangeP,
	PGPByte *						pp,
	PGPSize							payLen )
{
	PGPError						err = kPGPError_NoErr;
	PGPikeExchange *				exchange = *exchangeP;

	if( exchange->initiator )
	{
		if( exchange->dhYr != kPGPInvalidBigNumRef )
		{
			err = pgpIKEAbortExchange( exchangeP,
							kPGPike_AL_InvalidPayload );
			goto done;
		}
		err = PGPNewBigNum( exchange->ike->memMgr, TRUE,	
							&exchange->dhYr ); CKERR;
		err = PGPBigNumInsertBigEndianBytes( exchange->dhYr, pp,
											0, payLen ); CKERR;
	}
	else
	{
		if( exchange->dhYi != kPGPInvalidBigNumRef )
		{
			err = pgpIKEAbortExchange( exchangeP,
							kPGPike_AL_InvalidPayload );
			goto done;
		}
		err = PGPNewBigNum( exchange->ike->memMgr, TRUE,
							&exchange->dhYi ); CKERR;
		err = PGPBigNumInsertBigEndianBytes( exchange->dhYi, pp,
											0, payLen ); CKERR;
	}
done:
	return err;
}

	PGPError
pgpIKEProcessP2ID(
	PGPikeExchange *		exchange,
	PGPByte *				inIDBody,
	PGPSize					inIDBodySize,
	PGPBoolean *			outIsRange,
	PGPUInt32 *				ipStart,
	PGPUInt32 *				ipEnd )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				pp = inIDBody;
	PGPipsecIdentity		idType;
	PGPByte					ipProtocol;
	PGPUInt16				ipPort;
	
	exchange;
	*outIsRange = FALSE;
	*ipStart = 0;
	*ipEnd = 0;
	idType = (PGPipsecIdentity) *pp++;
	ipProtocol = *pp++;
	ipPort = PGPEndianToUInt16( kPGPBigEndian, pp );
	pp+= sizeof(PGPUInt16);
	if( ipProtocol != 0 )
	{
#if PGPIKE_DEBUG
		pgpIKEDebugLog( exchange->ike,
			"\t\tRemote attempting IP protocol negotiation: %c\n", ipProtocol );
#endif
	}
	if( ipPort != 0 )
	{
#if PGPIKE_DEBUG
		pgpIKEDebugLog( exchange->ike,
			"\t\tRemote attempting IP port negotiation: %u\n", (PGPUInt32)ipPort );
#endif
	}
	switch( idType )
	{
		case kPGPike_ID_IPV4_Addr:
			*ipStart = *(PGPUInt32 *)pp;
			pp += sizeof(PGPUInt32);
			*ipEnd = 0xFFFFFFFF;
			break;
		case kPGPike_ID_IPV4_Addr_Range:
			*outIsRange = TRUE;
		case kPGPike_ID_IPV4_Addr_Subnet:
			*ipStart = *(PGPUInt32 *)pp;
			pp += sizeof(PGPUInt32);
			*ipEnd = *(PGPUInt32 *)pp;
			pp += sizeof(PGPUInt32);
			break;
		default:
			err = kPGPError_ItemNotFound;
			break;
	}
	if( IsntPGPError( err ) && ( inIDBody + inIDBodySize ) != pp )
		err = kPGPError_BufferTooSmall;
	return err;
}

	PGPError
pgpIKESelectProposal(
	PGPikeExchange *				exchange,
	PGPikeProposal *				proposal,
	PGPBoolean *					isValid )
{
	PGPError						err = kPGPError_NoErr;
	PGPBoolean						match = FALSE;
	PGPikeProposal *				curR;
	PGPUInt16						trInx;

	curR = proposal;
	if( exchange->initiator )
	{
		PGPikeProposal *			curLR;
		PGPikeProposal *			lastLR;
		
		/*	We should have received back a matching set of
			proposals.  This could be more than one proposal
			if we sent a pair or triplet proposal. */
		
		for( curLR = exchange->proposals; IsntNull( curLR );
				curLR = curLR->nextProposal )
		{
			/*if( curR->number == curLR->number )
			{*/
			/* The two lines above had to be removed due to lame IKE
				implementations that don't correctly number their
				return proposals. */
			multiProposal:
				if( ( curR->protocol == curLR->protocol ) &&
					( curR->numTransforms == 1 ) &&
					( curR->kbLifeTime == curLR->kbLifeTime ) &&
					( curR->secLifeTime == curLR->secLifeTime ) )
				{
					for( trInx = 0; trInx < curLR->numTransforms; trInx++ )
					{
						if( pgpIKEIsTransformEqual( curR->protocol,
								&curR->t[0], &curLR->t[trInx] ) )
						{
							match = TRUE;
							break;
						}
						else
							curR = proposal;
					}
					if( match )
					{
						*(PGPUInt32 *)curR->initSPI =
								*(PGPUInt32 *)curLR->initSPI;
						if( IsntNull( curR->nextProposal ) )
						{
							match = FALSE;
							if( IsntNull( curLR->nextProposal ) )
								/* && ( curLR->nextProposal->number == curR->number ) )*/
							{
								curR = curR->nextProposal;
								curLR = curLR->nextProposal;
								goto multiProposal;
							}
						}
						/* We have a positive match */
						curLR = exchange->proposals;
						while( IsntNull( curLR ) )
						{
							lastLR = curLR;
							curLR = curLR->nextProposal;
							(void)PGPFreeData( lastLR );
						}
						exchange->proposals = NULL;
#if PGPIKE_DEBUG
						if( exchange->state == kPGPike_S_MM_WaitSA )
						{
							pgpIKEDebugLog( exchange->ike, "\tProposal Selected (I): %s, %s\n",
								pgpIKEAuthMethodString( proposal->t[0].u.ike.authMethod ),
								pgpIKECipherString( proposal->t[0].u.ike.cipher ) );
						}
#endif
						curR = NULL;
						for( curLR = proposal; IsntNull( curLR ); curLR = curLR->nextProposal )
						{
							if( IsNull( exchange->proposals ) )
							{
								exchange->proposals = PGPNewData( exchange->ike->memMgr,
									sizeof(PGPikeProposal), kPGPMemoryMgrFlags_Clear );
								if( IsNull( exchange->proposals ) )
								{
									err = kPGPError_OutOfMemory;
									goto done;
								}
								curR = exchange->proposals;
							}
							else
							{
								curR->nextProposal = PGPNewData( exchange->ike->memMgr,
									sizeof(PGPikeProposal), kPGPMemoryMgrFlags_Clear );
								if( IsNull( curR->nextProposal ) )
								{
									err = kPGPError_OutOfMemory;
									goto done;
								}
								curR = curR->nextProposal;
							}
							pgpCopyMemory( curLR, curR, sizeof( PGPikeProposal ) );
						}
						if( IsntNull( curR ) )
							curR->nextProposal = NULL;
						break;
					}
				}
				else
				{
					PGPUInt8	curPNum = curLR->number;
					
					while( IsntNull( curLR->nextProposal ) &&
							( curLR->nextProposal->number == curPNum ) )
						curLR = curLR->nextProposal;
				}
			/*}*/
		}
	}
	else
	{
		PGPikeProposal *			acceptProposal = NULL;
		PGPUInt16					acceptCount = 0;
		PGPInt16					lastPNum = -1;
		
		/*	we must now select a proposal from the set */
		
#if PGPIKE_DEBUG
		PGPikeProposal *				dbgR = proposal;
		
		pgpIKEDebugLog( exchange->ike, "\tRemote Proposals:\n" );
		if( exchange->state == kPGPike_S_MM_WaitSA )
		{
			for( trInx = 0; trInx < dbgR->numTransforms; trInx++ )
			{
				pgpIKEDebugLog( exchange->ike, "\t\tIKE: (%d) %s, %s, %s, %s, S:%u, KB:%u\n",
					trInx, pgpIKEAuthMethodString( dbgR->t[trInx].u.ike.authMethod ),
					pgpIKECipherString( dbgR->t[trInx].u.ike.cipher ),
					pgpIKEHashTypeString( dbgR->t[trInx].u.ike.hash ),
					pgpIKEGroupTypeString( dbgR->t[trInx].u.ike.groupID ),
					dbgR->secLifeTime, dbgR->kbLifeTime );
			}
		}
		else
		{
			while( IsntNull( dbgR ) )
			{
				pgpIKEDebugLog( exchange->ike, "\t\tIPSEC: (%d) ", dbgR->number );
				switch( dbgR->protocol )
				{
					case kPGPike_PR_AH:
						pgpIKEDebugLog( exchange->ike, "AH: %s, %s",
							pgpIKEipsecAuthTypeString( dbgR->t[0].u.ipsec.ah.authAttr ),
							( dbgR->t[0].u.ipsec.ah.mode == kPGPike_PM_Tunnel ) ?
							"Tunnel" : "Transport" );
						break;
					case kPGPike_PR_ESP:
						pgpIKEDebugLog( exchange->ike, "ESP: %s, %s, %s",
							pgpIKEipsecAuthTypeString( dbgR->t[0].u.ipsec.esp.authAttr ),
							pgpIKEipsecCipherTypeString( dbgR->t[0].u.ipsec.esp.cipher ),
							( dbgR->t[0].u.ipsec.esp.mode == kPGPike_PM_Tunnel ) ?
							"Tunnel" : "Transport" );
						break;
					case kPGPike_PR_IPCOMP:
						pgpIKEDebugLog( exchange->ike, "IPCOMP" );
						break;
				}
				pgpIKEDebugLog( exchange->ike, ", S:%u, KB:%u\n",
					dbgR->secLifeTime, dbgR->kbLifeTime );
				dbgR = dbgR->nextProposal;
			}
		}
#endif
		pgpAssert( IsNull( exchange->proposals ) );
		
		while( IsntNull( curR ) )
		{
			for( trInx = 0; trInx < curR->numTransforms; trInx++ )
			{
				if( pgpIKEIsTransformValid( exchange->partner, curR->protocol,
							&curR->t[trInx] ) )
				{
					/* good transform, select it */
					pgpCopyMemory( &curR->t[trInx], &curR->t[0],
									sizeof(PGPikeGenericTransform) );
#if PGPIKE_DEBUG
					if( exchange->state == kPGPike_S_MM_WaitSA )
					{
						pgpIKEDebugLog( exchange->ike, "\tProposal Selected (R): %s, %s\n",
							pgpIKEAuthMethodString( curR->t[0].u.ike.authMethod ),
							pgpIKECipherString( curR->t[0].u.ike.cipher ) );
					}
#endif
					break;
				}
			}
			if( trInx >= curR->numTransforms )
			{
				if( IsntNull( acceptProposal ) && ( acceptProposal->number == curR->number ) )
				{
					/* partial rejection of SA bundle proposal */
					while( IsntNull( acceptProposal ) )
					{
						PGPikeProposal *	fProp = acceptProposal;
						
						acceptProposal = acceptProposal->nextProposal;
						(void)PGPFreeData( fProp );
						acceptCount--;
					}
				}
				goto unsupportedProposal;
			}
			
			curR->numTransforms = 1;
			if( IsntNull( acceptProposal ) )
			{
				if( acceptProposal->number == curR->number )
				{
					PGPikeProposal *	walkP;
					
					if( acceptCount >= kPGPike_MaxTransforms )
						goto unsupportedProposal;
					for( walkP = acceptProposal; IsntNull( walkP->nextProposal );
						walkP = walkP->nextProposal )
						;
					walkP->nextProposal = PGPNewData( exchange->ike->memMgr,
									sizeof(PGPikeProposal), kPGPMemoryMgrFlags_Clear );
					if( IsNull( walkP->nextProposal ) )
					{
						err = kPGPError_OutOfMemory;
						goto done;
					}
					walkP = walkP->nextProposal;
					pgpCopyMemory( curR, walkP, sizeof( PGPikeProposal ) );
					walkP->nextProposal = NULL;
					acceptCount++;
				}
				else
					break;
			}
			else
			{
				if( lastPNum == (PGPInt16)curR->number )
					goto unsupportedProposal;
				acceptProposal = PGPNewData( exchange->ike->memMgr,
					sizeof(PGPikeProposal), kPGPMemoryMgrFlags_Clear );
				if( IsNull( acceptProposal ) )
				{
					err = kPGPError_OutOfMemory;
					goto done;
				}
				pgpCopyMemory( curR, acceptProposal, sizeof( PGPikeProposal ) );
				acceptProposal->nextProposal = NULL;
				acceptCount++;
			}
			
		unsupportedProposal:
			lastPNum = curR->number;
			curR = curR->nextProposal;
		}
		if( IsntNull( acceptProposal ) )
		{
			PGPikeProposal *	curP;
			
			if( exchange->state == kPGPike_S_QM_WaitSA )
			{
				PGPBoolean	hasESP		= FALSE,
							hasCipher	= FALSE,
							hasAH		= FALSE,
							hasIPCOMP	= FALSE,
							badCompound	= FALSE;
				
				for( curP = acceptProposal; IsntNull( curP ) && !badCompound;
						curP = curP->nextProposal )
				{
					if( curP->protocol == kPGPike_PR_IPCOMP )
					{
						if( !hasIPCOMP )
							hasIPCOMP = TRUE;
						else
							badCompound = TRUE;
					}
					else if( curP->protocol == kPGPike_PR_ESP )
					{
						if( !hasESP )
						{
							hasESP = TRUE;
							if( curP->t[0].u.ipsec.esp.cipher != kPGPike_ET_NULL )
								hasCipher = TRUE;
						}
						else
							badCompound = TRUE;
					}
					else if( curP->protocol == kPGPike_PR_AH )
					{
						if( !hasAH )
							hasAH = TRUE;
						else
							badCompound = TRUE;
					}
				}
				if( !hasCipher && !exchange->ike->allowedAlgorithms.espNULL )
					badCompound = TRUE;
				if( badCompound )
				{
					while( IsntNull( acceptProposal ) )
					{
						curP = acceptProposal;
						
						acceptProposal = acceptProposal->nextProposal;
						(void)PGPFreeData( curP );
						acceptCount--;
					}
					if( IsntNull( curR ) )
						goto unsupportedProposal;
				}
				
				for( curP = acceptProposal; IsntNull( curP ); curP = curP->nextProposal )
				{
					err = PGPContextGetRandomBytes(
							exchange->ike->pgpContext,
							&curP->respSPI,
							sizeof(PGPipsecSPI) ); CKERR;
				}
			}
			if( IsntNull( acceptProposal ) )
			{
				match = TRUE;
				exchange->proposals = acceptProposal;
			}
		}
	}
done:
	*isValid = match;
	return err;
}

	PGPError
pgpIKEProcessProposal(
	PGPikeExchange **		exchangeP,
	PGPByte *				mp,
	PGPByte *				ep,
	PGPikeAlert	*			alert,
	PGPikeProposal **		proposal )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeProposal *		prop = NULL;
	PGPikeExchange *		exchange = *exchangeP;

	*alert = kPGPike_AL_None;
	*proposal = NULL;
	prop = PGPNewData( exchange->ike->memMgr, sizeof(PGPikeProposal),
							kPGPMemoryMgrFlags_Clear );
	if( IsntNull( prop ) )
	{
		PGPUInt8				spiSize;
		PGPUInt8				transIndex;
		PGPBoolean				zeroBased = FALSE;
		PGPUInt8				numTransforms;
		
		if( ep - mp < sizeof(PGPUInt32) )
		{
			*alert = kPGPike_AL_UnequalPayloadLengths;
			goto done;
		}
		prop->number			= *mp++;
		prop->protocol			= (PGPipsecProtocol)*mp++;
		spiSize					= *mp++;
		numTransforms			= *mp++;
		
		switch( prop->protocol )
		{
			case kPGPike_PR_IKE:
				if( spiSize > 16 )
				{
					*alert = kPGPike_AL_InvalidSPI;
					goto done;
				}
				break;
			case kPGPike_PR_IPCOMP:
				if( spiSize == sizeof( PGPipsecSPI ) )
				{
					if( exchange->initiator )
						pgpCopyMemory( mp, &prop->respSPI, spiSize );
					else
						pgpCopyMemory( mp, &prop->initSPI, spiSize );
				}
				else if( spiSize == sizeof( PGPUInt16 ) )
				{
					PGPUInt16	pcpCPI16 = PGPEndianToUInt16( kPGPBigEndian, mp );
					
					if( exchange->initiator )
					{
						prop->respSPI[2] = pcpCPI16 >> 8;
						prop->respSPI[3] = pcpCPI16 & 0xFF;
					}
					else
					{
						prop->initSPI[2] = pcpCPI16 >> 8;
						prop->initSPI[3] = pcpCPI16 & 0xFF;
					}
				}
				else
				{
					*alert = kPGPike_AL_InvalidSPI;
					goto done;
				}
				break;
			case kPGPike_PR_AH:
			case kPGPike_PR_ESP:
				if( spiSize != sizeof( PGPipsecSPI ) )
				{
					*alert = kPGPike_AL_InvalidSPI;
					goto done;
				}
				if( exchange->initiator )
					pgpCopyMemory( mp, &prop->respSPI, spiSize );
				else
					pgpCopyMemory( mp, &prop->initSPI, spiSize );
				break;
			default:
				*alert = kPGPike_AL_InvalidProtocolID;
				goto done;
		}
		if( ep - mp < spiSize )
		{
			*alert = kPGPike_AL_UnequalPayloadLengths;
			goto done;
		}
		mp += spiSize;
		
		/* Transforms */
		for( transIndex = 0; transIndex < numTransforms; transIndex++ )
		{
			PGPByte *				hdr = mp;
			PGPByte *				tep;
			PGPUInt16				transLen;
			PGPikeAttributeType		attribute;
			PGPUInt32				value;
			PGPikeGenericTransform *trx = &prop->t[prop->numTransforms];
			
			if( ep - mp < kPGPike_ISAKMPPayloadHeaderSize )
			{
				*alert = kPGPike_AL_UnequalPayloadLengths;
				goto done;
			}
			*mp++;	*mp++;
			transLen = PGPEndianToUInt16( kPGPBigEndian, mp );
			mp+= sizeof(PGPUInt16);
			tep = mp + transLen - kPGPike_ISAKMPPayloadHeaderSize;
			if( ep < tep )
			{
				*alert = kPGPike_AL_UnequalPayloadLengths;
				goto done;
			}
			if( transIndex < kPGPike_MaxPropTransforms )
			{
				if( !transIndex && ( *mp == 0 ) )
					zeroBased = TRUE;
				if( ( *mp++ == transIndex + ( zeroBased ? 0 : 1 ) ) ||
					( exchange->initiator && ( numTransforms == 1 ) ) )
				{
					PGPUInt16	transformID;
					
					transformID = *mp++;
					*mp++; *mp++;	/* reserved */
					switch( prop->protocol )
					{
						case kPGPike_PR_IKE:
							if( transformID != kPGPike_Key_IKE_Transform )
							{
								*alert = kPGPike_AL_InvalidTransform;
								goto done;
							}
							trx->u.ike.modpGroup		= TRUE;	/* default */
							trx->u.ike.groupID			= kPGPike_GR_MODPOne;
							while( mp < tep )
							{
								pgpIKEGetAttribute( &mp, &attribute, &value );
								switch( attribute )
								{
									case kPGPike_AT_AuthMethod:
										trx->u.ike.authMethod =
											(PGPikeAuthMethod)value;
										break;
									case kPGPike_AT_GroupType:
										if( value != kPGPike_MODPGroupType )
											goto unsupportedTransform;
										trx->u.ike.modpGroup = TRUE;
										break;
									case kPGPike_AT_GroupDesc:
										/* if unspecified, MODP one is used */
										switch( value )
										{
											case kPGPike_GR_MODPOne:
											case kPGPike_GR_MODPTwo:
											case kPGPike_GR_MODPFive:
												trx->u.ike.groupID =
													(PGPikeGroupID)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_EncryptAlg:
										switch( value )
										{
											case kPGPike_SC_3DES_CBC:
											case kPGPike_SC_CAST_CBC:
											case kPGPike_SC_DES_CBC:
												trx->u.ike.cipher =
													(PGPikeCipher)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_HashAlg:
										switch( value )
										{
											case kPGPike_HA_SHA1:
											case kPGPike_HA_MD5:
												trx->u.ike.hash = (PGPikeHash)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_LifeType:
									{
										PGPikeLifeType	lifeType =
															(PGPikeLifeType) value;
										if( ( mp != tep ) &&
											( ( lifeType == kPGPike_LT_Seconds ) ||
											( lifeType == kPGPike_LT_Kilobytes ) ) )
										{
											pgpIKEGetAttribute( &mp, &attribute,
																&value );
											if( attribute == kPGPike_AT_LifeDuration )
											{
												if( lifeType == kPGPike_LT_Seconds )
													prop->secLifeTime	= value;
												else
													prop->kbLifeTime	= value;
											}
											else
											{
												*alert = kPGPike_AL_InvalidTransform;
												goto done;
											}
										}
										else
										{
											*alert = kPGPike_AL_InvalidTransform;
											goto done;
										}
										break;
									}
									default:
										*alert = kPGPike_AL_InvalidAttribute;
										goto done;
								}
							}
							if( ( trx->u.ike.authMethod == kPGPike_AM_None ) ||
								( trx->u.ike.hash		== kPGPike_HA_None ) ||
								( trx->u.ike.cipher		== kPGPike_SC_None ) ||
								!trx->u.ike.modpGroup )
							{
								*alert = kPGPike_AL_InvalidTransform;
								goto done;
							}
							prop->numTransforms++;
							break;
						case kPGPike_PR_AH:
						{
							PGPipsecAHTransformID	ahTID;
							
							switch( ahTID = (PGPipsecAHTransformID)transformID )
							{
								case kPGPike_AH_MD5:
								case kPGPike_AH_SHA:
									trx->u.ipsec.ah.authAlg = ahTID;
									break;
								default:
									goto unsupportedTransform;
							}
							trx->u.ipsec.ah.mode = kPGPike_PM_Transport;
							while( mp < tep )
							{
								pgpIKEGetAttribute( &mp, &attribute, &value );
								switch( attribute )
								{
									case kPGPike_AT_IPSEC_AuthAttr:
										switch( value )
										{
											case kPGPike_AA_HMAC_MD5:
											case kPGPike_AA_HMAC_SHA:
												trx->u.ipsec.ah.authAttr =
													(PGPipsecAuthAttribute)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_Encapsulation:
										switch( value )
										{
											case kPGPike_PM_Tunnel:
											case kPGPike_PM_Transport:
												trx->u.ipsec.ah.mode =
													(PGPipsecEncapsulation)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_GroupDesc:
										switch( value )
										{
											case kPGPike_GR_MODPOne:
											case kPGPike_GR_MODPTwo:
											case kPGPike_GR_MODPFive:
												trx->u.ipsec.groupID =
													(PGPikeGroupID)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_LifeType:
									{
										PGPikeLifeType	lifeType =
															(PGPikeLifeType) value;
										if( ( mp != tep ) &&
											( ( lifeType == kPGPike_LT_Seconds ) ||
											( lifeType == kPGPike_LT_Kilobytes ) ) )
										{
											pgpIKEGetAttribute( &mp, &attribute,
																&value );
											if( attribute == kPGPike_AT_IPSEC_Duration )
											{
												if( lifeType == kPGPike_LT_Seconds )
													prop->secLifeTime	= value;
												else
													prop->kbLifeTime	= value;
											}
											else
											{
												*alert = kPGPike_AL_InvalidTransform;
												goto done;
											}
										}
										else
										{
											*alert = kPGPike_AL_InvalidTransform;
											goto done;
										}
										break;
									}
									default:
										*alert = kPGPike_AL_InvalidAttribute;
										goto done;
								}
							}
							if( ( trx->u.ipsec.ah.mode == kPGPike_PM_None ) ||
								( trx->u.ipsec.ah.authAttr == kPGPike_AA_None ) )
							{
								*alert = kPGPike_AL_InvalidTransform;
								goto done;
							}
							prop->numTransforms++;
							break;
						}
						case kPGPike_PR_ESP:
						{
							PGPipsecESPTransformID	espTID;
							
							switch( espTID = (PGPipsecESPTransformID)transformID )
							{
								case kPGPike_ET_NULL:
								case kPGPike_ET_3DES:
								case kPGPike_ET_CAST:
								case kPGPike_ET_DES_IV64:
								case kPGPike_ET_DES:
									trx->u.ipsec.esp.cipher = espTID;
									break;
								default:
									goto unsupportedTransform;
							}
							trx->u.ipsec.esp.mode = kPGPike_PM_Transport;
							while( mp < tep )
							{
								pgpIKEGetAttribute( &mp, &attribute, &value );
								switch( attribute )
								{
									case kPGPike_AT_IPSEC_AuthAttr:
										switch( value )
										{
											case kPGPike_AA_HMAC_MD5:
											case kPGPike_AA_HMAC_SHA:
												trx->u.ipsec.esp.authAttr =
													(PGPipsecAuthAttribute)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_Encapsulation:
										switch( value )
										{
											case kPGPike_PM_Tunnel:
											case kPGPike_PM_Transport:
												trx->u.ipsec.esp.mode =
													(PGPipsecEncapsulation)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_GroupDesc:
										switch( value )
										{
											case kPGPike_GR_MODPOne:
											case kPGPike_GR_MODPTwo:
											case kPGPike_GR_MODPFive:
												trx->u.ipsec.groupID =
													(PGPikeGroupID)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_LifeType:
									{
										PGPikeLifeType	lifeType =
															(PGPikeLifeType) value;
										if( ( mp != tep ) &&
											( ( lifeType == kPGPike_LT_Seconds ) ||
											( lifeType == kPGPike_LT_Kilobytes ) ) )
										{
											pgpIKEGetAttribute( &mp, &attribute,
																&value );
											if( attribute == kPGPike_AT_IPSEC_Duration )
											{
												if( lifeType == kPGPike_LT_Seconds )
													prop->secLifeTime	= value;
												else
													prop->kbLifeTime	= value;
											}
											else
											{
												*alert = kPGPike_AL_InvalidTransform;
												goto done;
											}
										}
										else
										{
											*alert = kPGPike_AL_InvalidTransform;
											goto done;
										}
										break;
									}
									default:
										*alert = kPGPike_AL_InvalidAttribute;
										goto done;
								}
							}
							if( ( trx->u.ipsec.esp.mode == kPGPike_PM_None ) )
							{
								*alert = kPGPike_AL_InvalidTransform;
								goto done;
							}
							prop->numTransforms++;
							break;
						}
						case kPGPike_PR_IPCOMP:
						{
							PGPipsecIPCOMPTransformID	ipcompTID;
							
							switch( ipcompTID = (PGPipsecIPCOMPTransformID)transformID )
							{
								case kPGPike_IC_Deflate:
								case kPGPike_IC_LZS:
									trx->u.ipsec.ipcomp.compAlg = ipcompTID;
									break;
								default:
									goto unsupportedTransform;
							}
							while( mp < tep )
							{
								pgpIKEGetAttribute( &mp, &attribute, &value );
								switch( attribute )
								{
									case kPGPike_AT_IPSEC_GroupDesc:
										switch( value )
										{
											case kPGPike_GR_MODPOne:
											case kPGPike_GR_MODPTwo:
											case kPGPike_GR_MODPFive:
												trx->u.ipsec.groupID =
													(PGPikeGroupID)value;
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_Encapsulation:
										switch( value )
										{
											case kPGPike_PM_Tunnel:
											case kPGPike_PM_Transport:
												break;
											default:
												goto unsupportedTransform;
										}
										break;
									case kPGPike_AT_IPSEC_LifeType:
									{
										PGPikeLifeType	lifeType =
															(PGPikeLifeType) value;
										if( ( mp != tep ) &&
											( ( lifeType == kPGPike_LT_Seconds ) ||
											( lifeType == kPGPike_LT_Kilobytes ) ) )
										{
											pgpIKEGetAttribute( &mp, &attribute,
																&value );
											if( attribute == kPGPike_AT_IPSEC_Duration )
											{
												if( lifeType == kPGPike_LT_Seconds )
													prop->secLifeTime	= value;
												else
													prop->kbLifeTime	= value;
											}
											else
											{
												*alert = kPGPike_AL_InvalidTransform;
												goto done;
											}
										}
										else
										{
											*alert = kPGPike_AL_InvalidTransform;
											goto done;
										}
										break;
									}
									default:
										*alert = kPGPike_AL_InvalidAttribute;
										goto done;
								}
							}
							prop->numTransforms++;
							break;
						}
					}
					if( mp != tep )
					{
						*alert = kPGPike_AL_UnequalPayloadLengths;
						goto done;
					}
				}
				else
				{
					*alert = kPGPike_AL_InvalidTransform;
					goto done;
				}
			}
		unsupportedTransform:
			mp = tep;
		}
		if( ( ( numTransforms > 0 ) && ( prop->numTransforms > 0 ) ) ||
			!numTransforms )
		{
			*proposal = prop;
			prop = NULL;
		}
	}
	else
		err = kPGPError_OutOfMemory;
done:
	if( *alert != kPGPike_AL_None )
		err = pgpIKEAbortExchange( exchangeP, *alert );
	if( IsntNull( prop ) )
		(void)PGPFreeData( prop );
	return err;
}

	void
pgpIKEGetAttribute(
	PGPByte **				mpp,
	PGPikeAttributeType *	attribute,
	PGPUInt32 *				value )
{
	PGPByte *				mp = *mpp;
	PGPBoolean				extend;
	PGPUInt16				aVal;
	
	extend			= ( *mp & 0x80 ) == 0;
	*attribute		= (PGPikeAttributeType)
						( PGPEndianToUInt16( kPGPBigEndian, mp ) & 0x7FFF );
	mp += sizeof( PGPUInt16 );
	aVal	 		= PGPEndianToUInt16( kPGPBigEndian, mp );
	mp += sizeof( PGPUInt16 );
	if( extend )
	{
		if( aVal != sizeof(PGPUInt32) )
			*attribute = kPGPike_AT_None;	/* we don't handle these */
		else
			*value = PGPEndianToUInt32( kPGPBigEndian, mp );
		mp += sizeof(PGPUInt32);
	}
	else
		*value = aVal;
	
	*mpp = mp;
}

	PGPError
pgpIKEHeaderSniffer(
	PGPikeContextPriv *		ike,
	PGPikeMTPacket *		packet,
	PGPBoolean *			isValid )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp;
	PGPSize					mLen,
							pLen;
	PGPikePayload			payload;
	
	*isValid = FALSE;
	mp		= packet->packet;
	mLen	= packet->packetSize;
	if( mLen < kPGPike_ISAKMPHeaderSize + kPGPike_ISAKMPPayloadHeaderSize )
	{
		err = pgpIKELocalAlert( ike, packet->ipAddress,
				kPGPike_AL_UnequalPayloadLengths, kPGPike_IA_None, FALSE );
		goto done;
	}
	mp += kPGPike_CookieSize * 2;
	/* Payload */
	/* ***** The following could be stricter */
	payload = (PGPikePayload)*mp++;
	switch( payload )
	{
		case kPGPike_PY_SA:
		case kPGPike_PY_KeyExchange:
		case kPGPike_PY_Identification:
		case kPGPike_PY_Certificate:
		case kPGPike_PY_CertRequest:
		case kPGPike_PY_Hash:
		case kPGPike_PY_Signature:
		case kPGPike_PY_Nonce:
		case kPGPike_PY_Notification:
		case kPGPike_PY_Delete:
		case kPGPike_PY_VendorID:
			break;
		default:
			/* inappropriate first payload */
			err = pgpIKELocalAlert( ike, packet->ipAddress,
					kPGPike_AL_InvalidPayload, kPGPike_IA_None, FALSE );
			goto done;
	}
	/* Version */
	if( ( *mp >> 4 ) != kPGPike_Version >> 4 )
	{
		err = pgpIKELocalAlert( ike, packet->ipAddress,
				kPGPike_AL_InvalidMajorVersion, kPGPike_IA_None, FALSE );
		goto done;
	}
	if( ( *mp++ & 0x0F ) > ( kPGPike_Version & 0x0F ) )
	{
		err = pgpIKELocalAlert( ike, packet->ipAddress,
				kPGPike_AL_InvalidMinorVersion, kPGPike_IA_None, FALSE );
		goto done;
	}
	/* Exchange */
	switch( *mp++ )
	{
		case kPGPike_EX_Identity:
		/*case kPGPike_EX_Aggressive:*/
		case kPGPike_EX_Informational:
		case kPGPike_EX_IPSEC_Quick:
			break;
		default:
			err = pgpIKELocalAlert( ike, packet->ipAddress,
					kPGPike_AL_InvalidExchange, kPGPike_IA_None, FALSE );
			goto done;
	}
	*mp++;	/* Flags */
	mp += sizeof( PGPikeMessageID );
	pLen = PGPEndianToUInt32( kPGPBigEndian, mp );
	if( ( pLen > mLen ) || ( pLen < mLen - 8 ) )
	{
		/*	The first case is corrupt data, the
			second makes an allowance for padding */
		
		err = pgpIKELocalAlert( ike, packet->ipAddress,
				kPGPike_AL_UnequalPayloadLengths, kPGPike_IA_None, FALSE );
		goto done;
	}
	mp += sizeof( PGPUInt32 );
	
	*isValid = TRUE;
done:
	return err;
}

	PGPError
pgpIKEPayloadLengthCheck(
	PGPikePartner *			partner,
	PGPikePayload			payload,
	PGPByte *				mp,
	PGPByte *				ep,
	PGPBoolean *			isValid )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePayload			nextPayload;
	PGPUInt16				payLength;
	PGPUInt16				minLen = 0;

	partner;
	/* Now walk each payload to make sure lengths are all good */
	*isValid = FALSE;
	
#if PGPIKE_DEBUG
	pgpIKEDebugLog( partner->ike, "\tPayloads:" );
#endif
	while( payload != kPGPike_PY_None )
	{
		if( ep - mp < kPGPike_ISAKMPPayloadHeaderSize )
			goto done;
		nextPayload = (PGPikePayload)*mp++;
		mp++;	/* reserved */
		payLength = PGPEndianToUInt16( kPGPBigEndian, mp );
		mp += sizeof(PGPUInt16);
		payLength -= kPGPike_ISAKMPPayloadHeaderSize;
		if( ep - mp < payLength )
			goto done;
		switch( payload )
		{
			case kPGPike_PY_SA:
				minLen = 20;
				break;
			case kPGPike_PY_KeyExchange:
			case kPGPike_PY_Identification:
			case kPGPike_PY_Nonce:
			case kPGPike_PY_VendorID:
				minLen = 4;
				break;
			case kPGPike_PY_Certificate:
				minLen = 2;
				break;
			case kPGPike_PY_CertRequest:
				minLen = 1;
				break;
			case kPGPike_PY_Hash:
			case kPGPike_PY_Signature:
				minLen = kPGPike_MD5HashSize;
				break;
			case kPGPike_PY_Notification:
				minLen = 8;
				break;
			case kPGPike_PY_Delete:
				minLen = 12;
				break;
			default:
				pgpAssert( 0 );
				goto done;
		}
#if PGPIKE_DEBUG
		pgpIKEDebugLog( partner->ike, "%s/", pgpIKEPayloadTypeString( payload ) );
#endif
		if( ep - mp < minLen )
			goto done;
		payload = nextPayload;
		mp += payLength;
	}
#if PGPIKE_DEBUG
	pgpIKEDebugLog( partner->ike, "\n" );
#endif

	*isValid = TRUE;
done:
	return err;
}

	PGPError
pgpIKEInitialContact(
	PGPikeExchange *		exchange )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner	*		partner,
					*		nextPartner;
	PGPikeSA *				sa,
			 *				nextSA;
	
	for( sa = exchange->ike->sa; IsntNull( sa ); )
	{
		nextSA = sa->nextSA;
		if( sa->ipAddress == exchange->partner->ipAddress )
		{
			err = (exchange->ike->msgProc)(
					(PGPikeContextRef)exchange->ike,
					exchange->ike->userData,
					kPGPike_MT_SADied, sa );CKERR;
			err = pgpIKEFreeSA( exchange->ike, sa );	CKERR;
		}
		sa = nextSA;
	}
	for( partner = exchange->ike->partner; IsntNull( partner ); )
	{
		nextPartner = partner->nextPartner;
		if( ( partner->ipAddress == exchange->partner->ipAddress ) &&
			( partner != exchange->partner ) )
		{
			err = pgpIKEFreePartner( partner );	CKERR;
		}
		partner = nextPartner;
	}
#if PGPIKE_DEBUG
	pgpIKEDebugLog( exchange->ike, "\t\tInitial Contact Processed\n" );
#endif
done:
	return err;
}

	PGPError
pgpIKEResponderLifetime(
	PGPikeExchange *		exchange,
	PGPByte *				pp,
	PGPByte *				ep )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeAttributeType		attribute,
							ltType,
							ltDuration;
	PGPUInt32				value;

	if( IsntNull( exchange->proposals ) && exchange->initiator )
	{
		if( exchange->exchangeT == kPGPike_EX_Identity )
		{
			ltType		= kPGPike_AT_LifeType;
			ltDuration	= kPGPike_AT_LifeDuration;
		}
		else
		{
			ltType		= kPGPike_AT_IPSEC_LifeType;
			ltDuration	= kPGPike_AT_IPSEC_Duration;
		}
		while( ( ep - pp ) >= 8 )
		{
			PGPikeLifeType	lifeType;
			
			pgpIKEGetAttribute( &pp, &attribute, &value );
			lifeType = (PGPikeLifeType) value;
			if( ( attribute == ltType ) &&
				( ( ep - pp ) >= 4 ) &&
				( ( lifeType == kPGPike_LT_Seconds ) ||
				  ( lifeType == kPGPike_LT_Kilobytes ) ) )
			{
				pgpIKEGetAttribute( &pp, &attribute, &value );
				if( attribute == ltDuration )
				{
					if( lifeType == kPGPike_LT_Seconds )
					{
						if( ( ( exchange->proposals->secLifeTime > value ) ||
								!exchange->proposals->secLifeTime ) &&
							( value >= kPGPike_SecLifeRekeySlack * 2 ) )
						{
							exchange->proposals->secLifeTime	= value;
		#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike,
								"\t\t\tDuration (seconds) reduced: %u\n",
								value );
		#endif
						}
					}
					else
					{
						if( ( ( exchange->proposals->kbLifeTime > value ) ||
								!exchange->proposals->kbLifeTime ) &&
							( value >= kPGPike_KBLifeMinimum ) )
						{
							exchange->proposals->kbLifeTime		= value;
		#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike,
								"\t\t\tDuration (kb) reduced: %u\n",
								value );
		#endif
						}
					}
				}
			}
		}
	}
	return err;
}

	PGPError
pgpIKEProcessInformational(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPikeExchange **		exchangeP,	/* we might delete this exchange */
	PGPikeNDPayload *		nd )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeExchange *		exchange = NULL;
	PGPikeDOI				doi;
	PGPipsecProtocol		protocol;
	PGPipsecSPI				spi;
	PGPByte					spiSize;
	PGPByte *				pp = nd->pay;
	PGPByte *				ep = nd->pay + nd->payLen;
	
	/* Information exchanges are treated as just that.  Informational.
		Parsing errors in such exchanges are ignored.  For the most part,
		this is due to the fact that informational exchanges are
		largely unprotected except after the initial exchange.  We also
		never reply to an information exchange to avoid potential
		infinite error loops with the remote system.  */
	if( IsntNull( exchangeP ) )
		exchange = *exchangeP;
	if( nd->payLen < 8 )
		goto done;
	doi = (PGPikeDOI)PGPEndianToUInt32( kPGPBigEndian, pp );
	pp += sizeof(PGPikeDOI);
	if( doi != kPGPike_DOI_IPSEC )
		goto done;
	protocol	= (PGPipsecProtocol) *pp++;
	spiSize		= *pp++;
	
	switch( nd->payType )
	{
		case kPGPike_PY_Notification:
		{
			PGPikeAlert			alert;
			
#if PGPIKE_DEBUG
			pgpIKEDebugLog( ike, "\tNotification\n" );
#endif
			alert = (PGPikeAlert)PGPEndianToUInt16( kPGPBigEndian, pp );
			pp += sizeof(PGPUInt16);
			if( alert <= kPGPike_AL_LastErrorType )
			{
				err = pgpIKELocalAlert( ike, ipAddress,
						alert, kPGPike_IA_None, TRUE );	CKERR;
			}
			if( IsNull( exchangeP ) )
				break;
			switch( protocol )
			{
				case kPGPike_PR_IKE:
					if( nd->payLen < (PGPSize) (8 + spiSize) )
						goto done;
					pp += spiSize;
					if( IsntNull( exchangeP ) && ( alert <= kPGPike_AL_LastErrorType ) )
					{
						PGPikeMTSAFailed		saf;
						
						saf.ipAddress		= ipAddress;
						saf.doi				= kPGPike_DOI_IPSEC;
						pgpCopyMemory( &exchange->partner->ipsecOpts, &saf.u.ipsec,
							sizeof(PGPipsecDOIOptions) );
						err = pgpIKESARequestFailed( exchange->ike, &saf );	CKERR;
						err = pgpIKEFreePartner( exchange->partner );	CKERR;
						*exchangeP = NULL;	/* the exchange was killed too */
					}
					else switch( alert )
					{
						case kPGPike_AL_InitialContact:
							err = pgpIKEInitialContact( exchange );	CKERR;
							break;
						case kPGPike_AL_ResponderLifetime:
#if PGPIKE_DEBUG
							pgpIKEDebugLog( ike, "\t\tResponder Lifetime (P1)\n" );
#endif
							if( exchange->exchangeT == kPGPike_EX_Identity )
							{
								err = pgpIKEResponderLifetime( exchange, pp, ep );CKERR;
							}
							break;
						case kPGPike_AL_ReplayStatus:
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike,
								"\t\tReplay Status (P1) - Unsupported\n" );
#endif
							break;
						case kPGPike_AL_Connected:
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike,
								"\t\tConnected (P1) - Unsupported\n" );
#endif
							break;
					}
					break;
				case kPGPike_PR_None:	/* Microsoft sends this, who knows why */
				case kPGPike_PR_AH:
				case kPGPike_PR_ESP:
				case kPGPike_PR_IPCOMP:
					if( spiSize != sizeof(PGPipsecSPI) )
						goto done;
					if( nd->payLen < (PGPSize) (8 + spiSize) )
						goto done;
					pgpCopyMemory( pp, &spi, sizeof(PGPipsecSPI) );
					pp += spiSize;
					if( alert <= kPGPike_AL_LastErrorType )
					{
						/* ******* */
					}
					else switch( alert )
					{
						case kPGPike_AL_ResponderLifetime:
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike,
								"\t\tResponder Lifetime (P2)\n" );
#endif
							/* We check to make sure that Responder Lifetime is
								part of a Quick Mode in which case verifying
								the SPI is not necessary */
							if( exchange->exchangeT == kPGPike_EX_IPSEC_Quick )
							{
								err = pgpIKEResponderLifetime( exchange, pp, ep );	CKERR;
							}
							break;
						case kPGPike_AL_ReplayStatus:
#if PGPIKE_DEBUG
							pgpIKEDebugLog( exchange->ike, "\t\tReplay Status (P2)\n" );
#endif
							break;
						case kPGPike_AL_Connected:
						{
							PGPikeExchange *	p2Exchange;
							
							for( p2Exchange = exchange->partner->exchanges;
									IsntNull( p2Exchange );
									p2Exchange = p2Exchange->nextExchange )
							{
								if( ( p2Exchange != exchange ) &&
									p2Exchange->needsCommit &&
									p2Exchange->complete &&
									pgpMemoryEqual( &p2Exchange->messageID,
													&exchange->messageID,
													sizeof(PGPikeMessageID) ) )
								{
									PGPikeSA *	sa;
									
#if PGPIKE_DEBUG
									pgpIKEDebugLog( exchange->ike,
										"\t\tConnected (P2)\n" );
#endif
									err = pgpIKECompletePhase2( p2Exchange, &sa );CKERR;
									err = pgpIKESAEstablished( p2Exchange->ike, sa );CKERR;
									err = pgpIKEFreeExchange( p2Exchange );CKERR;
									exchange = NULL;
									break;
								}
							}
							break;
						}
					}
					break;
			}
			break;
		}
		case kPGPike_PY_Delete:
		{
			PGPUInt16	numSPI,
						spiIndex;
			
			numSPI = PGPEndianToUInt16( kPGPBigEndian, pp );
			pp += sizeof(PGPUInt16);
			if( nd->payLen < 8 + ( (PGPSize) numSPI * spiSize ) )
				goto done;
#if PGPIKE_DEBUG
			pgpIKEDebugLog( exchange->ike, "\tDelete (prot=%s)\n",
					( protocol == kPGPike_PR_IKE ) ? "IKE" : "IPSEC" );
#endif
			switch( protocol )
			{
				case kPGPike_PR_IKE:
#if PGPIKE_DEBUG
					pgpIKEDebugData( exchange->ike, "iCookie", pp,
											sizeof(PGPikeCookie) );
#endif
					if( ( spiSize != kPGPike_CookieSize * 2 ) || ( numSPI != 1 ) )
						goto done;
					if( !pgpMemoryEqual( exchange->partner->initCookie,
											pp, sizeof(PGPikeCookie) ) )
						goto done;
					pp += sizeof(PGPikeCookie);
#if PGPIKE_DEBUG
					pgpIKEDebugData( exchange->ike, "rCookie", pp,
											sizeof(PGPikeCookie) );
#endif
					if( !pgpMemoryEqual( exchange->partner->respCookie,
											pp, sizeof(PGPikeCookie) ) )
						goto done;
#if PGPIKE_DEBUG
					pgpIKEDebugLog( exchange->ike, "\t\tDeleted IKE SA\n" );
#endif
					err = pgpIKEFreePartner( exchange->partner );	CKERR;
					*exchangeP = NULL;	/* the exchange was killed too */
					break;
				case kPGPike_PR_AH:
				case kPGPike_PR_ESP:
				case kPGPike_PR_IPCOMP:
				{
					PGPikeSA	*	sa,
								*	nextSA;
					
					if( spiSize != sizeof(PGPipsecSPI) )
						goto done;
					for( spiIndex = 0; spiIndex < numSPI; spiIndex++ )
					{
#if PGPIKE_DEBUG
						pgpIKEDebugData( exchange->ike, "SPI", pp,
											sizeof(PGPipsecSPI) );
#endif
						for( sa = exchange->ike->sa; IsntNull( sa ) && IsntNull( exchange );
								sa = nextSA )
						{
							nextSA = sa->nextSA;
							if( sa->ipAddress == exchange->partner->ipAddress )
							{
								PGPUInt16	trInx;
								PGPBoolean	found = FALSE;
								
								for( trInx = 0; trInx < sa->numTransforms; trInx++ )
								{
									if( pgpMemoryEqual( pp,
											sa->transform[trInx].u.ipsec.outSPI,
											sizeof(PGPipsecSPI) ) )
									{
										found = TRUE;
										break;
									}
								}
								if( found )
								{
									PGPikePartner *	partner = exchange->partner;
									
#if PGPIKE_DEBUG
									pgpIKEDebugLog( exchange->ike,
										"\t\tDelete IPSEC SA\n" );
#endif
									sa->activeIn = FALSE;
									err = pgpIKEKillSA( &partner, sa ); CKERR;
									if( IsNull( partner ) )
									{
										*exchangeP = NULL;
										exchange = NULL;
									}
								}
							}
						}
						pp += sizeof(PGPipsecSPI);
					}
					break;
				}
			}
			break;
		}
	}
done:
	return err;
}

	PGPError
pgpIKEKillSA(
	PGPikePartner **		pPartner,
	PGPikeSA *				sa )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeExchange *		ndExchange = NULL;
	PGPikePartner *			partner = *pPartner;
	PGPikeContextPriv *		ike = partner->ike;
	PGPUInt16				tInx;
	PGPBoolean				deleteIt = FALSE;

	/* send delete exchanges for each protocol's SPIs */
	if( !sa->activeOut || !sa->activeIn )
		deleteIt = TRUE;
	if( sa->activeOut )
		for( tInx = 0; tInx < sa->numTransforms; tInx++ )
		{
			err = pgpIKECreateExchange( partner,
								kPGPike_EX_Informational,
								NULL, kPGPike_S_ND_Informational,
								TRUE, &ndExchange );	CKERR;
			ndExchange->outInfoProtocol = sa->transform[tInx].u.ipsec.protocol;
			ndExchange->outInfoSPICount = 1;
			pgpCopyMemory( &sa->transform[tInx].u.ipsec.inSPI,
							&ndExchange->outInfoSPI[0],
							sizeof(PGPipsecSPI) );
			err = pgpIKEDoPacket( &ndExchange,
								kPGPike_PY_Delete,
								kPGPike_PY_None );	CKERR;
			(void)pgpIKEFreeExchange( ndExchange );
			ndExchange = NULL;
		}
	sa->activeOut = FALSE;
	if( deleteIt )
	{
		PGPBoolean	found = FALSE;
		PGPikeSA *	simSA;
		
		/* send SADied message for the SA */
		err = (ike->msgProc)(
				(PGPikeContextRef)ike, ike->userData,
				kPGPike_MT_SADied, sa );CKERR;
		/* make sure there are other SAs that need this partner */
		found = FALSE;
		for( simSA = ike->sa; IsntNull( simSA ); simSA = simSA->nextSA )
		{
			if( ( simSA != sa ) &&
				( simSA->ipAddress == partner->ipAddress ) &&
				( simSA->ipAddrStart == partner->ipsecOpts.ipAddrStart ) &&
				( simSA->ipMaskEnd == partner->ipsecOpts.ipMaskEnd ) &&
				( simSA->destIsRange == partner->ipsecOpts.destIsRange ) )
			{
				found = TRUE;
				break;
			}
		}
		if( !found )
		{
			if( partner->initiator )
			{
				/* no other SAs need this P1 SA, kill that too */
				err = pgpIKECreateExchange( partner,
									kPGPike_EX_Informational,
									NULL, kPGPike_S_ND_Informational,
									TRUE, &ndExchange );	CKERR;
				ndExchange->outInfoProtocol = kPGPike_PR_IKE;
				ndExchange->outInfoSPICount = 0;
#if PGPIKE_DEBUG
				pgpIKEDebugLog( ike, "Sending Phase 1 SA Delete (unneeded)\n" );
#endif
				err = pgpIKEDoPacket( &ndExchange,
									kPGPike_PY_Delete,
									kPGPike_PY_None );	CKERR;
				err = pgpIKEFreePartner( partner );	CKERR;
				*pPartner = NULL;
				ndExchange = NULL;
				/* the exchange is deleted automatically */
			}
			else
			{	
				/* the other side may have gone down so eventually we may
					need to kill it ourselves */
				partner->termSchedule = kPGPike_SADiedSlack;
			}
		}
	}
	else
	{
		sa->termSchedule = PGPGetTime() + kPGPike_SADiedSlack;
		/* let service know to stop outgoing packets for this SA */
		err = (ike->msgProc)(
				(PGPikeContextRef)ike, ike->userData,
				kPGPike_MT_SAUpdate, sa );CKERR;
	}
	
done:
	if( deleteIt )
		(void)pgpIKEFreeSA( ike, sa );
	if( IsntNull( ndExchange ) )
		(void)pgpIKEFreeExchange( ndExchange );
	return err;
}

	PGPError
pgpIKESARequestFailed(
	PGPikeContextPriv *		ike,
	PGPikeMTSAFailed *		saf )
{
	PGPError				err = kPGPError_NoErr;

	err = pgpIKERemovePending( ike, saf->ipAddress, saf->u.ipsec.ipAddrStart,
			saf->u.ipsec.ipMaskEnd, saf->u.ipsec.destIsRange );	CKERR;
#if PGPIKE_DEBUG
	pgpIKEDebugLog( ike, "SAFailed: %u.%u.%u.%u (%u.%u.%u.%u/%u.%u.%u.%u)\n",
		( saf->ipAddress >> 24 ), ( saf->ipAddress >> 16 ) & 0xFF,
		( saf->ipAddress >> 8 ) & 0xFF, ( saf->ipAddress & 0xFF ),
		( saf->u.ipsec.ipAddrStart >> 24 ), ( saf->u.ipsec.ipAddrStart >> 16 ) & 0xFF,
		( saf->u.ipsec.ipAddrStart >> 8 ) & 0xFF, ( saf->u.ipsec.ipAddrStart & 0xFF ),
		( saf->u.ipsec.ipMaskEnd >> 24 ), ( saf->u.ipsec.ipMaskEnd >> 16 ) & 0xFF,
		( saf->u.ipsec.ipMaskEnd >> 8 ) & 0xFF, ( saf->u.ipsec.ipMaskEnd & 0xFF ) );
#endif
	err = (ike->msgProc)( (PGPikeContextRef)ike, ike->userData,
			kPGPike_MT_SARequestFailed, saf );
done:
	return err;
}

	PGPError
pgpIKEAbortExchange(
	PGPikeExchange **		exchangeP,
	PGPikeAlert				alert )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeExchange *		ndExchange = NULL,
					*		exchange = *exchangeP;
	PGPikeMTSAFailed		saf;
	PGPikeInternalAlert		iAlert = kPGPike_IA_None;

	if( alert != kPGPike_AL_None )
	{
		err = pgpIKECreateExchange( exchange->partner,
							kPGPike_EX_Informational,
							NULL, kPGPike_S_ND_Informational,
							TRUE, &ndExchange );	CKERR;
		ndExchange->outAlert			= alert;
		
		if( ( exchange->exchangeT == kPGPike_EX_Identity ) ||
			IsNull( exchange->proposals ) ||
			( (PGPUInt32)exchange->proposals->initSPI == 0 ) ||
			( (PGPUInt32)exchange->proposals->respSPI == 0 ) )
		{
			ndExchange->outInfoProtocol	= kPGPike_PR_IKE;
			ndExchange->outInfoSPICount	= 0;
		}
		else
		{
			ndExchange->outInfoProtocol	= exchange->proposals->protocol;
			ndExchange->outInfoSPICount	= 1;
			if( exchange->initiator )
				pgpCopyMemory( &exchange->proposals->respSPI,
					&ndExchange->outInfoSPI[0], sizeof(PGPipsecSPI) );
			else
				pgpCopyMemory( &exchange->proposals->initSPI,
					&ndExchange->outInfoSPI[0], sizeof(PGPipsecSPI) );
		}
		err = pgpIKEDoPacket( &ndExchange,
							kPGPike_PY_Notification,
							kPGPike_PY_None );	CKERR;
	}
	else
		iAlert = kPGPike_IA_ResponseTimeout;
	err = pgpIKELocalAlert( exchange->ike, exchange->partner->ipAddress,
						alert, iAlert, FALSE );	CKERR;
	saf.ipAddress		= exchange->partner->ipAddress;
	saf.doi				= kPGPike_DOI_IPSEC;
	pgpCopyMemory( &exchange->partner->ipsecOpts, &saf.u.ipsec,
		sizeof(PGPipsecDOIOptions) );
	err = pgpIKESARequestFailed( exchange->ike, &saf );	CKERR;
	if( !exchange->partner->ready )
	{
		PGPUInt32			ipAddress = exchange->partner->ipAddress;
		PGPikePartner	*	oPartner,
						*	nextPartner;
		PGPikeSA		*	sa,
						*	nextSA;
		
		for( sa = exchange->ike->sa; IsntNull( sa ); )
		{
			nextSA = sa->nextSA;
			if( ipAddress == sa->ipAddress )
			{
				err = (exchange->ike->msgProc)(
						(PGPikeContextRef)exchange->ike,
						exchange->ike->userData,
						kPGPike_MT_SADied, sa );CKERR;
				err = pgpIKEFreeSA( exchange->ike, sa );	CKERR;
			}
			sa = nextSA;
		}
		for( oPartner = exchange->ike->partner; IsntNull( oPartner ); )
		{
			nextPartner = oPartner->nextPartner;
			if( ipAddress == oPartner->ipAddress )
			{
				err = pgpIKEFreePartner( oPartner );	CKERR;
			}
			oPartner = nextPartner;
		}
		*exchangeP = NULL;
		ndExchange = NULL;
	}
	else
	{
		err = pgpIKEFreeExchange( exchange );
		*exchangeP = NULL;
	}
done:
	if( IsntNull( ndExchange ) )
		(void)pgpIKEFreeExchange( ndExchange );
	if( IsntPGPError( err ) )	/* a big hack to make sure we dont ref the dead */
		err = kPGPError_UserAbort;
	return err;	
}
	
	PGPError
pgpIKELocalAlert(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPikeAlert				alert,
	PGPikeInternalAlert		value,
	PGPBoolean				remote )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeMTAlert			msg;
	
	msg.ipAddress		= ipAddress;
	msg.alert			= alert;
	msg.value			= value;
	msg.remoteGenerated	= remote;
	err = (ike->msgProc)( (PGPikeContextRef)ike, ike->userData,
							kPGPike_MT_Alert, &msg );
	
	return err;
}

	PGPError
pgpIKEStartIdentityExchange(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPikeMTSASetup *		saSetup,
	PGPBoolean				destIsRange,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeMTSASetup			msg;
	
	if( IsNull( saSetup ) )
	{
		pgpClearMemory( &msg, sizeof(PGPikeMTSASetup) );
		msg.ipAddress = ipAddress;
		err = (ike->msgProc)( (PGPikeContextRef)ike, ike->userData,
					kPGPike_MT_PolicyCheck, &msg );CKERR;
		saSetup = &msg;
	}
	else
		saSetup->approved = TRUE;
	if( saSetup->approved )
	{
		PGPikePartner	*	partner;
		PGPikeExchange	*	exchange;
		
		err = pgpIKECreatePartner( ike, saSetup, TRUE, &partner );CKERR;
		if( ipAddrStart )
		{
			partner->ipsecOpts.destIsRange	= destIsRange;
			partner->ipsecOpts.ipAddrStart	= ipAddrStart & ipMaskEnd;
			partner->ipsecOpts.ipMaskEnd	= ipMaskEnd;
		}
		err = pgpIKECreateExchange( partner, kPGPike_EX_Identity,
							NULL, kPGPike_S_MM_WaitSA, TRUE,
							&exchange );CKERR;
		err = pgpIKECreateProposals( exchange ); CKERR;
		if( IsntNull( exchange->proposals ) &&
			( exchange->proposals->numTransforms == 0 ) )
		{
			PGPikeMTSAFailed	saf;
			
			err = pgpIKELocalAlert( exchange->ike,
						partner->ipAddress, kPGPike_AL_None,
						kPGPike_IA_NoProposals, FALSE );	CKERR;
			saf.ipAddress	= ipAddress;
			saf.doi			= kPGPike_DOI_IPSEC;
			pgpCopyMemory( &partner->ipsecOpts, &saf.u.ipsec,
				sizeof(PGPipsecDOIOptions) );
			err = pgpIKESARequestFailed( ike, &saf );	CKERR;
			err = pgpIKEFreePartner( partner );
			goto done;
		}
#if PGPIKE_DEBUG
		pgpIKEDebugLog( ike, "Initiating Phase 1 Keying\n" );
#endif
		err = pgpIKEDoPacket( &exchange,
					kPGPike_PY_SA,
					kPGPike_PY_VendorID,
					kPGPike_PY_None );	CKERR;
	}
done:
	return err;
}

	PGPError
pgpIKEStartQMExchange(
	PGPikePartner *		partner,
	PGPikeExchange **	exchangeP )
{
	PGPError			err = kPGPError_NoErr;
	PGPikeExchange *	exchange = NULL;
	PGPBoolean			pfs = FALSE,
						idC = FALSE;
	
#if PGPIKE_DEBUG
	pgpIKEDebugLog( partner->ike, "Initiating Phase 2 QM\n" );
#endif
	*exchangeP = NULL;
	err = pgpIKECreateExchange( partner,
					kPGPike_EX_IPSEC_Quick, NULL,
					kPGPike_S_QM_WaitSA,
					TRUE, &exchange );	CKERR;
	err = pgpIKECreateProposals( exchange );	CKERR;
	if( exchange->proposals->t[0].u.ipsec.groupID !=
		kPGPike_GR_None )
	{
		pfs = TRUE;
		err = pgpIKELoadGroup( exchange );	CKERR;
	}
	if( ( partner->ipsecOpts.packetMode == kPGPike_PM_Tunnel ) &&
		partner->ipsecOpts.ipAddrStart )
		idC = TRUE;
	
	err = pgpIKEDoPacket( &exchange,
				kPGPike_PY_SA,
				kPGPike_PY_Nonce,
				pfs ? kPGPike_PY_KeyExchange : kPGPike_PY_Skip,
				idC ? kPGPike_PY_Identification : kPGPike_PY_Skip,
				idC ? kPGPike_PY_Identification : kPGPike_PY_Skip,
				kPGPike_PY_None );	CKERR;
	*exchangeP = exchange;
done:
	return err;
}

	PGPError
pgpIKEIdle(
	PGPikeContextPriv *		ike )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner	*		partner,
					*		nextPartner;
	PGPikeExchange	*		exchange;
	PGPikeSA		*		sa,
					*		nextSA;
	PGPUInt32				curMillisec;
	PGPTime					curTime,
							deathTime;
	PGPBoolean				found;

	curMillisec	= PGPGetMilliseconds();
	curTime		= PGPGetTime();
	for( partner = ike->partner; IsntNull( partner ); partner = nextPartner )
	{
		nextPartner = partner->nextPartner; /* save in case partner dies */
		
		/* Cycle through all exchanges to check for retransmittable packets */
		for( exchange = partner->exchanges; IsntNull( exchange );
				exchange = exchange->nextExchange )
		{
			PGPUInt32		retxTime;
			
			if( !exchange->lastTransmit )
				continue;
			if( exchange->complete )
				retxTime = kPGPike_CommitBitSlack;
			else
				retxTime = partner->rttMillisec + kPGPike_RoundTripSlack;
			if( curMillisec - exchange->lastTransmit > retxTime )
			{
				if( !partner->rttMillisec )
					partner->rttMillisec = kPGPike_RoundTripSlack;
				else
					partner->rttMillisec *= 2;
				err = pgpIKEResendLastPacket( exchange );	CKERR;
			}
		}
		
		if(	partner->ready &&
			( partner->secLifeTime || partner->termSchedule ) &&
			IsNull( partner->exchanges ) )
		{
			/* kPGPike_SecLifeRespSlack makes responders auto-delete P1 SAs
				slightly slower than initiators to avoid unpleasant alerts
				when both sides delete simultaneously */
			
			deathTime = partner->birthTime +
					( ( partner->termSchedule > 0 ) ?
						partner->termSchedule : partner->secLifeTime ) +
					( partner->initiator ? 0 : kPGPike_SecLifeRespSlack );
			if( curTime >= deathTime )
			{
				/* partner's dead Jim */
				
				err = pgpIKECreateExchange( partner,
									kPGPike_EX_Informational,
									NULL, kPGPike_S_ND_Informational,
									TRUE, &exchange );	CKERR;
				exchange->outInfoProtocol = kPGPike_PR_IKE;
				exchange->outInfoSPICount = 0;
#if PGPIKE_DEBUG
				pgpIKEDebugLog( ike, "Sending Phase 1 SA Delete (expired)\n" );
#endif
				err = pgpIKEDoPacket( &exchange,
									kPGPike_PY_Delete,
									kPGPike_PY_None );	CKERR;
				err = pgpIKEFreePartner( partner );	CKERR;
				/* the exchange is deleted automatically */
			}
			else if( partner->initiator &&
					( deathTime - curTime <= kPGPike_SecLifeRekeySlack ) )
			{
				PGPikePartner *		oPartner;
				
				/* try to rekey the IKE SA if there are no others */
				
				found = FALSE;
				for( oPartner = ike->partner; IsntNull( oPartner );
						oPartner = oPartner->nextPartner )
				{
					if( ( oPartner != partner ) &&
						( oPartner->ipAddress == partner->ipAddress ) &&
						( oPartner->ipsecOpts.ipAddrStart ==
							partner->ipsecOpts.ipAddrStart ) &&
						( oPartner->ipsecOpts.ipMaskEnd ==
							partner->ipsecOpts.ipMaskEnd ) &&
						( oPartner->ipsecOpts.destIsRange ==
							partner->ipsecOpts.destIsRange ) )
					{
						found = TRUE;
						break;
					}
				}
				if( !found )
				{
					/* and make sure a P2 SA exists that actually would want us to
						rekey this P1 SA */
					found = FALSE;
					for( sa = ike->sa; IsntNull( sa ); sa = sa->nextSA )
					{
						if( ( sa->ipAddress == partner->ipAddress ) &&
							( sa->ipAddrStart == partner->ipsecOpts.ipAddrStart ) &&
							( sa->ipMaskEnd == partner->ipsecOpts.ipMaskEnd ) &&
							( sa->destIsRange == partner->ipsecOpts.destIsRange ) )
						{
							found = TRUE;
							break;
						}
					}
					if( found )
					{
						err = pgpIKEStartIdentityExchange( ike, partner->ipAddress,
								NULL, partner->ipsecOpts.destIsRange,
								partner->ipsecOpts.ipAddrStart,
								partner->ipsecOpts.ipMaskEnd );	CKERR;
					}
				}
			}
		}
	}
	
	/* Cycle through all SAs to check for dead or dying */
	for( sa = ike->sa; IsntNull( sa ); sa = nextSA )
	{
		nextSA = sa->nextSA;
		
		if( sa->termSchedule )
		{
			if( sa->termSchedule < curTime )
			{
				if( IsntPGPError( pgpIKEFindSAPartner( ike, sa, TRUE, &partner ) ) )
				{
					err = pgpIKEKillSA( &partner, sa );	CKERR;
				}
				else
				{
					err = (ike->msgProc)(
							(PGPikeContextRef)ike,
							ike->userData, kPGPike_MT_SADied, sa );CKERR;
					err = pgpIKEFreeSA( ike, sa );	CKERR;
				}
			}
		}
		else if( sa->secLifeTime )
		{
			deathTime = sa->birthTime + sa->secLifeTime;
			if( curTime >= deathTime )
			{
				/* he's dead Jim */
				
				if( IsntPGPError( pgpIKEFindSAPartner( ike, sa, TRUE, &partner ) ) )
				{
					err = pgpIKEKillSA( &partner, sa );	CKERR;
				}
				else
				{
					/* no partner exists, we'll have to negotiate one */
					if( IsPGPError(
						pgpIKEFindSAPartner( ike, sa, FALSE, &partner ) ) )
					{
#if PGPIKE_DEBUG
						pgpIKEDebugLog( ike, "NO PARTNERS FOR SA DELETE: CREATING\n" );
#endif
						sa->termSchedule = curTime + kPGPike_SADiedSlack;
						err = pgpIKEStartIdentityExchange( ike,
								sa->ipAddress, NULL, sa->destIsRange,
								sa->ipAddrStart, sa->ipMaskEnd );	CKERR;
					}
				}
			}
			else if( deathTime - curTime <= kPGPike_SecLifeRekeySlack )
			{
				PGPikeSA	*	oSA;

				found = FALSE;
				for( oSA = ike->sa; IsntNull( oSA ); oSA = oSA->nextSA )
				{
					if( ( sa != oSA ) && ( oSA->ipAddress == sa->ipAddress ) &&
						( oSA->destIsRange == sa->destIsRange ) &&
						( oSA->ipAddrStart == sa->ipAddrStart ) &&
						( oSA->ipMaskEnd == sa->ipMaskEnd ) )
					{
						found = TRUE;
						break;
					}
				}
				if( !found &&
					IsntPGPError( pgpIKEFindSAPartner( ike, sa, TRUE, &partner ) ) &&
					partner->initiator && IsNull( partner->exchanges ) )
				{
					/* we were the initiator and should rekey this SA */
					
#if PGPIKE_DEBUG
					pgpIKEDebugLog( ike, "Initiating Phase 2 Rekey\n" );
#endif
					err = pgpIKEStartQMExchange( partner, &exchange );	CKERR;
				}
			}
		}
	}
done:
	return err;	
}

/* The Mighty IKE Packet Construction Engine, next 100 exits */
	PGPError
pgpIKEDoPacket(
	PGPikeExchange **		exchangeP,
	PGPikePayload			firstPayload,
	... )
{
	va_list					morePayloads;
	PGPError				err = kPGPError_NoErr;
	PGPikeExchange *		exchange = *exchangeP;
	PGPikePayload			payload;
	PGPByte *				packet = NULL;
	PGPByte *				mp;
	PGPByte *				hp;
	PGPSize					packetSize = kPGPike_ISAKMPHeaderSize,
							allocSize = kPGPike_DefaultPacketAlloc;
	PGPBoolean				commitBit	= FALSE,
							authOnlyBit	= FALSE,
							retry		= TRUE,
							encrypt		= FALSE;
	PGPHMACContextRef		p2hmac = kInvalidPGPHMACContextRef;
	
	va_start( morePayloads, firstPayload );

	pgpAssertAddrValid( exchange, PGPikeExchange * );
	
	mp = packet = PGPNewData( exchange->ike->memMgr, allocSize,
						kPGPMemoryMgrFlags_Clear );
	if( IsNull( packet ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	if( PGPCBCContextRefIsValid( exchange->partner->cbc ) &&
		( ( exchange->exchangeT == kPGPike_EX_Identity ) ||
		  ( exchange->exchangeT == kPGPike_EX_IPSEC_Quick ) ||
		( ( exchange->exchangeT == kPGPike_EX_Informational ) &&
			exchange->partner->ready ) ) )
		encrypt = TRUE;
#if PGPIKE_DEBUG
	pgpIKEDebugLog( exchange->ike, "Send: %s", encrypt ? "(E):" : "" );
#endif
	/* Payloads */
	mp = packet + kPGPike_ISAKMPHeaderSize;
	if( encrypt && ( exchange->exchangeT != kPGPike_EX_Identity ) )
	{
		hp = mp += kPGPike_ISAKMPPayloadHeaderSize + exchange->partner->hashSize;
		err = PGPNewHMACContext( exchange->ike->memMgr,
									exchange->partner->sdkHashAlg,
									exchange->partner->skeyid_a,
									exchange->partner->hashSize,
									&p2hmac );	CKERR;
		if( exchange->initiator &&
			exchange->state == kPGPike_S_QM_WaitHash3 )
		{
			err = PGPContinueHMAC( p2hmac, "\0", 1 );	CKERR;
		}
		err = PGPContinueHMAC( p2hmac, exchange->messageID,
								sizeof(PGPikeMessageID) );	CKERR;
		if( exchange->state == kPGPike_S_QM_WaitHash3 )
		{
			err = PGPContinueHMAC( p2hmac, exchange->initNonce,
											exchange->initNonceLen );CKERR;
			if( exchange->initiator )
			{
				err = PGPContinueHMAC( p2hmac, exchange->respNonce,
											exchange->respNonceLen );CKERR;
			}
		}
#if PGPIKE_DEBUG
		if( exchange->exchangeT == kPGPike_EX_IPSEC_Quick )
			pgpIKEDebugLog( exchange->ike, "%s/",
				pgpIKEPayloadTypeString( kPGPike_PY_Hash ) );
#endif
	}
	for( payload = firstPayload; payload != kPGPike_PY_None; )
	{
		PGPByte *	hdr = mp;
		PGPUInt16	plen;
		
		mp += kPGPike_ISAKMPPayloadHeaderSize;
#if PGPIKE_DEBUG
		pgpIKEDebugLog( exchange->ike, "%s/",
			pgpIKEPayloadTypeString( payload ) );
#endif
		switch( payload )
		{
			case kPGPike_PY_SA:
			{
				err = pgpIKEAddSAPayload( exchange, &mp ); CKERR;
				if( exchange->initiator &&
					( exchange->state == kPGPike_S_MM_WaitSA ) )
				{
					/* save the SA payload body for HASH_X */
					pgpAssert( IsNull( exchange->initSABody ) );
					exchange->initSABodySize =
						mp - hdr - kPGPike_ISAKMPPayloadHeaderSize;
					exchange->initSABody = PGPNewData( exchange->ike->memMgr,
									exchange->initSABodySize,
									kPGPMemoryMgrFlags_Clear );
					if( IsNull( exchange->initSABody ) )
					{
						err = kPGPError_OutOfMemory;
						goto done;
					}
					pgpCopyMemory( hdr + kPGPike_ISAKMPPayloadHeaderSize,
									exchange->initSABody,
									exchange->initSABodySize );
				}
				break;
			}
			case kPGPike_PY_KeyExchange:
				err = pgpIKEAddKEPayload( exchange, &mp ); CKERR;
				break;
			case kPGPike_PY_Identification:
				err = pgpIKEAddIDPayload( exchange, &mp ); CKERR;
				break;
			case kPGPike_PY_Certificate:
				err = pgpIKEAddCertPayload( exchange, &mp ); CKERR;
				break;
			case kPGPike_PY_CertRequest:
				err = pgpIKEAddCertRequestPayload( exchange, &mp ); CKERR;
				break;
			case kPGPike_PY_Signature:
				err = pgpIKEAddSigPayload( exchange, &mp ); CKERR;
				break;
			case kPGPike_PY_Nonce:
				err = pgpIKEAddNoncePayload( exchange, &mp ); CKERR;
				break;
			case kPGPike_PY_Hash:
			{
				/* used only for main mode, QM Hashes are outside this loop */
				err = pgpIKEGetAuthHash( exchange, exchange->initiator,
										mp );
				mp += exchange->partner->hashSize;
				break;
			}
			case kPGPike_PY_Notification:
			{
				PGPUInt32ToEndian( (PGPUInt32) exchange->doi,
						kPGPBigEndian, mp );
				mp += sizeof(PGPUInt32);
				*mp++ = exchange->outInfoProtocol;
				if( exchange->outInfoProtocol == kPGPike_PR_IKE )
				{
					*mp++ = 0;
					PGPUInt16ToEndian( (PGPUInt16) exchange->outAlert,
							kPGPBigEndian, mp );
					mp += sizeof(PGPUInt16);
					if( exchange->outAlert == kPGPike_AL_ResponderLifetime )
					{
						if( exchange->outRLSeconds )
						{
							pgpIKEAddAttribute( kPGPike_AT_LifeType,
								kPGPike_LT_Seconds, &mp );
							pgpIKEAddAttribute( kPGPike_AT_LifeDuration,
								exchange->ike->secLifeTimeIKE, &mp );
							exchange->proposals->secLifeTime = exchange->ike->secLifeTimeIKE;
						}
						if( exchange->outRLKB )
						{
							pgpIKEAddAttribute( kPGPike_AT_LifeType,
								kPGPike_LT_Kilobytes, &mp );
							pgpIKEAddAttribute( kPGPike_AT_LifeDuration,
								exchange->ike->kbLifeTimeIKE, &mp );
							exchange->proposals->kbLifeTime = exchange->ike->kbLifeTimeIKE;
						}
					}
				}
				else
				{
					*mp++ = sizeof(PGPipsecSPI);
					PGPUInt16ToEndian( (PGPUInt16) exchange->outAlert,
							kPGPBigEndian, mp );
					mp += sizeof(PGPUInt16);
					pgpAssert( exchange->outInfoSPICount == 1 );
					pgpCopyMemory( &exchange->outInfoSPI[0], mp,
									sizeof(PGPipsecSPI) );
					mp += sizeof(PGPipsecSPI);
					if( exchange->outAlert == kPGPike_AL_ResponderLifetime )
					{
						if( exchange->outRLSeconds )
						{
							pgpIKEAddAttribute( kPGPike_AT_IPSEC_LifeType,
								kPGPike_LT_Seconds, &mp );
							pgpIKEAddAttribute( kPGPike_AT_IPSEC_Duration,
								exchange->ike->secLifeTimeIPSEC, &mp );
							exchange->proposals->secLifeTime = exchange->ike->secLifeTimeIPSEC;
						}
						if( exchange->outRLKB )
						{
							pgpIKEAddAttribute( kPGPike_AT_IPSEC_LifeType,
								kPGPike_LT_Kilobytes, &mp );
							pgpIKEAddAttribute( kPGPike_AT_IPSEC_Duration,
								exchange->ike->kbLifeTimeIPSEC, &mp );
							exchange->proposals->kbLifeTime = exchange->ike->kbLifeTimeIPSEC;
						}
					}
				}
				break;
			}
			case kPGPike_PY_Delete:
				PGPUInt32ToEndian( (PGPUInt32) exchange->doi,
						kPGPBigEndian, mp );
				mp += sizeof(PGPUInt32);
				*mp++ = exchange->outInfoProtocol;
				if( exchange->outInfoProtocol == kPGPike_PR_IKE )
				{
					*mp++ = sizeof(PGPikeCookie) * 2;
					*mp++ = 0;
					*mp++ = 1;
					pgpCopyMemory( exchange->partner->initCookie, mp,
									sizeof(PGPikeCookie) );
					mp += sizeof(PGPikeCookie);
					pgpCopyMemory( exchange->partner->respCookie, mp,
									sizeof(PGPikeCookie) );
					mp += sizeof(PGPikeCookie);
				}
				else
				{
					PGPUInt16	sInx;
					
					*mp++ = sizeof(PGPipsecSPI);
					PGPUInt16ToEndian( exchange->outInfoSPICount,
							kPGPBigEndian, mp );
					mp += sizeof(PGPUInt16);
					for( sInx = 0; sInx < exchange->outInfoSPICount; sInx++ )
					{
						pgpCopyMemory( &exchange->outInfoSPI[sInx], mp,
										sizeof(PGPipsecSPI) );
						mp += sizeof(PGPipsecSPI);
					}
				}
				break;
			case kPGPike_PY_VendorID:
			{
				PGPUInt16	vLen;
				
				vLen = strlen(kPGPike_PGPVendorString1);
				pgpCopyMemory( kPGPike_PGPVendorString1, mp, vLen );
				mp += vLen;
				vLen = strlen(kPGPike_PGPVendorString2);
				pgpCopyMemory( kPGPike_PGPVendorString2, mp, vLen );
				mp += vLen;
				break;
			}
			case kPGPike_PY_Proposal:
			case kPGPike_PY_Transform:
			default:
				/* inappropriate payload requested */
				pgpAssert( 0 );
				break;
		}
		pgpAssert( packetSize < kPGPike_DefaultPacketAlloc );
		
		do
		{
			payload = va_arg( morePayloads, PGPikePayload );
		} while( payload == kPGPike_PY_Skip );

		plen = mp - hdr;
		*hdr++ = payload;
		*hdr++ = 0; /* reserved */
		PGPUInt16ToEndian( plen, kPGPBigEndian, hdr );
	}
	if( PGPHMACContextRefIsValid( p2hmac ) )
	{
		err = PGPContinueHMAC( p2hmac, hp, mp - hp );	CKERR;
		hp -= exchange->partner->hashSize;
		err = PGPFinalizeHMAC( p2hmac, hp );	CKERR;
		hp -= kPGPike_ISAKMPPayloadHeaderSize;
		*hp++ = firstPayload;
		firstPayload = kPGPike_PY_Hash;
		*hp++ = 0;	/* reserved */
		PGPUInt16ToEndian(
			(PGPUInt16)( exchange->partner->hashSize +
			kPGPike_ISAKMPPayloadHeaderSize ),
			kPGPBigEndian, hp );
	}
	packetSize = mp - packet;
	if( encrypt )
	{
		/* Pad the packet out for the CBC encryption */
		while( ( packetSize - kPGPike_ISAKMPHeaderSize ) %
				kPGPike_MaxCipherBlockSize )
		{
			*mp++ = 0;
			packetSize++;
		}
		/* Encrypt the data */
		err = PGPInitCBC( exchange->partner->cbc,
							exchange->partner->cipherKey,
							exchange->lastCBC );	CKERR;
		err = PGPCBCEncrypt( exchange->partner->cbc,
								packet + kPGPike_ISAKMPHeaderSize,
								packetSize - kPGPike_ISAKMPHeaderSize,
								packet + kPGPike_ISAKMPHeaderSize );	CKERR;
		/* Save the IV for the next packet */
		pgpCopyMemory( packet + packetSize - kPGPike_MaxCipherBlockSize,
						exchange->lastCBC,
						kPGPike_MaxCipherBlockSize );
		if( ( exchange->exchangeT == kPGPike_EX_Identity ) ||
			( exchange->exchangeT == kPGPike_EX_Aggressive ) )
			pgpCopyMemory( exchange->lastCBC, exchange->partner->lastP1CBC,
						kPGPike_MaxCipherBlockSize );
	}
	/* ISAKMP Header */
	mp = packet;
	pgpCopyMemory( exchange->partner->initCookie, mp, kPGPike_CookieSize );
	mp += kPGPike_CookieSize;
	pgpCopyMemory( exchange->partner->respCookie, mp, kPGPike_CookieSize );
	mp += kPGPike_CookieSize;
	*mp++ = firstPayload;
	*mp++ = kPGPike_Version;
	*mp++ = exchange->exchangeT;
	*mp = 0;	/* Flags */
	if( encrypt )
		*mp |= kPGPike_ISAKMPEncryptBit;
	if( commitBit )
		*mp |= kPGPike_ISAKMPCommitBit;
	if( authOnlyBit )
		*mp |= kPGPike_ISAKMPAuthOnlyBit;
	*mp++;
	pgpCopyMemory( exchange->messageID, mp, kPGPike_MessageIDSize );
	mp+= kPGPike_MessageIDSize;
	PGPUInt32ToEndian( (PGPUInt32) packetSize, kPGPBigEndian, mp );
	
	/* resize to its natural size */
	err = PGPReallocData( exchange->ike->memMgr, &packet, packetSize, 0 );CKERR;
	/* done, send the packet */
	if( retry )
	{
		err = pgpIKESendPacket( exchange, packet, packetSize );	CKERR;
	}
	else
	{
		err = pgpIKESendPacket1( exchange->partner, packet, packetSize ); CKERR;
		err = PGPFreeData( packet );
	}
	packet = NULL;
done:
#if PGPIKE_DEBUG
	pgpIKEDebugLog( exchange->ike, "\n" );
#endif
	if( IsPGPError( err ) )
	{
		/* Not clear what error to send out here.  Any PGPError in this
			function indicates a bug in pgpIKE or the calling application.
			We fudge an error here for safety. */
		pgpIKEAbortExchange( exchangeP, kPGPike_AL_UnsupportedExchange );
	}
	if( IsntNull( packet ) )
		PGPFreeData( packet );
	if( PGPHMACContextRefIsValid( p2hmac ) )
		(void)PGPFreeHMACContext( p2hmac );

	va_end( morePayloads );
	return err;
}

	PGPError
pgpIKEAddIDPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPByte **				pBody;
	PGPSize *				pSize;
	
	pBody = &exchange->idBody;
	pSize = &exchange->idBodySize;
	if( exchange->exchangeT == kPGPike_EX_IPSEC_Quick )
	{
		PGPBoolean	idCi = TRUE;
		
		if( exchange->initiator )
		{
			if( IsntNull( exchange->idBody ) )
			{
				pBody = &exchange->idRBody;
				pSize = &exchange->idRBodySize;
				idCi = FALSE;
			}
		}
		else
		{
			if( IsNull( exchange->idBody ) )
				idCi = FALSE;
			else
			{
				pBody = &exchange->idRBody;
				pSize = &exchange->idRBodySize;
			}
		}
		if( idCi )
		{
			*mp++ = kPGPike_ID_IPV4_Addr;		/* ID type */
			*mp++ = 0;							/* protocol ID */
			*mp++ = 0;*mp++ = 0;				/* port */
			pgpCopyMemory( &exchange->partner->localIPAddress, mp, sizeof(PGPUInt32) );
			mp += sizeof(PGPUInt32);
		}
		else
		{
			PGPipsecIdentity	idType;
			
			if( exchange->partner->ipsecOpts.destIsRange )
				idType	= kPGPike_ID_IPV4_Addr_Range;
			else if( exchange->partner->ipsecOpts.ipMaskEnd != 0xFFFFFFFF )
				idType	= kPGPike_ID_IPV4_Addr_Subnet;
			else
				idType	= kPGPike_ID_IPV4_Addr;
			*mp++ = idType;						/* ID type */
			*mp++ = 0;							/* protocol ID */
			*mp++ = 0;*mp++ = 0;				/* port */
			switch( idType )
			{
				case kPGPike_ID_IPV4_Addr:
					pgpCopyMemory( &exchange->partner->ipsecOpts.ipAddrStart,
						mp, sizeof(PGPUInt32) );
					break;
				case kPGPike_ID_IPV4_Addr_Subnet:
				case kPGPike_ID_IPV4_Addr_Range:
					pgpCopyMemory( &exchange->partner->ipsecOpts.ipAddrStart,
						mp, sizeof(PGPUInt32) );
					mp += sizeof(PGPUInt32);
					pgpCopyMemory( &exchange->partner->ipsecOpts.ipMaskEnd,
						mp, sizeof(PGPUInt32) );
					break;
			}
			mp += sizeof(PGPUInt32);
		}
	}
	else
	{
		switch( exchange->proposals->t[0].u.ike.authMethod )
		{
			case kPGPike_AM_DSS_Sig:
			case kPGPike_AM_RSA_Sig:
			{
				PGPKeyRef		pgpKey;
				PGPSigRef		x509Cert;
				
				pgpKey = exchange->partner->pgpAuthKey.authKey;
				x509Cert = exchange->partner->x509AuthKey.authCert;
				if( exchange->partner->remotePGPVersion && PGPKeyRefIsValid( pgpKey ) )
				{
					PGPByte		fingerprint[32];
					PGPSize		fingerLen;
					
					err = PGPGetKeyPropertyBuffer(
							exchange->partner->pgpAuthKey.authKey,
							kPGPKeyPropFingerprint, sizeof( fingerprint ),
							fingerprint, &fingerLen); CKERR;
					*mp++ = kPGPike_ID_Key_ID;			/* ID type */
					*mp++ = 0;							/* protocol ID */
					*mp++ = 0;*mp++ = 0;				/* port */
					pgpCopyMemory( fingerprint, mp, fingerLen );
					mp += fingerLen;
				}
				else if( PGPSigRefIsValid( x509Cert ) )
				{
					char	cstr[384];
					PGPSize	stringLen;
					
					err = PGPGetSigPropertyBuffer( x509Cert,
								kPGPSigPropX509DERDName,
								sizeof(cstr), &cstr[0], &stringLen );
					if( IsntPGPError( err ) )
					{
						*mp++ = kPGPike_ID_DER_ASN1_DN;		/* ID type */
						*mp++ = 0;							/* protocol ID */
						*mp++ = 0;*mp++ = 0;				/* port */
						pgpCopyMemory( cstr, mp, stringLen );
						mp += stringLen;
					}
				}
				else
				{
					err = kPGPError_PublicKeyNotFound;
					goto done;
				}
				break;
			}
			case kPGPike_AM_PreSharedKey:
				if( IsntNull( exchange->partner->ipsecOpts.idData ) )
				{
					*mp++ = exchange->partner->ipsecOpts.idType;	/* ID type */
					*mp++ = 0;										/* protocol ID */
					*mp++ = 0;*mp++ = 0;							/* port */
					pgpCopyMemory( exchange->partner->ipsecOpts.idData, mp,
									exchange->partner->ipsecOpts.idDataSize );
					mp += exchange->partner->ipsecOpts.idDataSize;
				}
				else
				{
					err = kPGPError_ItemNotFound;
					goto done;
				}
				break;
			case kPGPike_AM_RSA_Encrypt:
			case kPGPike_AM_RSA_Encrypt_R:
				pgpAssert( 0 );
				break;
		}
		if( IsntNull( exchange->idBody ) )
		{
			(void)PGPFreeData( exchange->idBody );
			exchange->idBody = NULL;
		}
	}
	*pSize = mp - *mpp;
	if( *pSize )
	{
		*pBody = PGPNewData( exchange->ike->memMgr,
					*pSize, kPGPMemoryMgrFlags_Clear );
		if( IsNull( *pBody ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		pgpCopyMemory( *mpp, *pBody, *pSize );
	}
done:
	*mpp = mp;
	return err;
}

	PGPError
pgpIKEAddCertPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPKeyRef				pgpKey;
	PGPSigRef				x509Cert;
	PGPByte *				certBuffer = NULL;
	PGPSize					certBufferLen = 0;
	PGPContextRef			con = exchange->ike->pgpContext;
	
	pgpKey = exchange->partner->pgpAuthKey.authKey;
	x509Cert = exchange->partner->x509AuthKey.authCert;
	if( exchange->partner->remotePGPVersion && PGPKeyRefIsValid( pgpKey ) )
	{
		*mp++ = kPGPike_CE_PGP;
		err = PGPExport( con,
				PGPOExportKey( con, pgpKey ),
				PGPOAllocatedOutputBuffer( con, (void **) &certBuffer,
					MAX_PGPUInt32, &certBufferLen ),
				PGPOArmorOutput( con, FALSE ),
				PGPOLastOption( con ) );	CKERR;
	}
	else if( PGPSigRefIsValid( x509Cert ) )
	{
		*mp++ = kPGPike_CE_X509_Sig;
		err = PGPExport( con,
				PGPOExportSig( con, x509Cert ),
				PGPOExportFormat( con, kPGPExportFormat_X509Cert ),
				PGPOAllocatedOutputBuffer( con, (void **) &certBuffer,
					MAX_PGPUInt32, &certBufferLen ),
				PGPOLastOption( con ) );	CKERR;
	}
	else
	{
		err = kPGPError_PublicKeyNotFound;
		goto done;
	}
	if( ( certBufferLen > 0 ) &&
		( certBufferLen < MAX_PGPInt16 ) )	/* no DDT style keys!! */
	{
		pgpCopyMemory( certBuffer, mp, certBufferLen );
		mp += certBufferLen;
	}
	else
	{
		err = kPGPError_KeyUnusableForSignature;
		goto done;
	}
done:
	*mpp = mp;
	if( IsntNull( certBuffer ) )
		(void)PGPFreeData( certBuffer );
	return err;
}

	PGPError
pgpIKEAddCertRequestPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPKeyRef				pgpKey;
	PGPSigRef				x509Cert;
	
	pgpKey = exchange->partner->pgpAuthKey.authKey;
	x509Cert = exchange->partner->x509AuthKey.authCert;
	if( exchange->partner->remotePGPVersion && PGPKeyRefIsValid( pgpKey ) )
	{
		*mp++ = kPGPike_CE_PGP;
	}
	else if( PGPSigRefIsValid( x509Cert ) )
	{
		*mp++ = kPGPike_CE_X509_Sig;
	}
	else
	{
		err = kPGPError_PublicKeyNotFound;
		goto done;
	}
	if( exchange->partner->remotePGPVersion == 1 )
		*mp++ = 0;	/* ***** HACK to be compatible with 6.5.0, remove eventually */
done:
	*mpp = mp;
	return err;
}

	PGPError
pgpIKEAddSigPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPByte					authHash[kPGPike_MaxHashSize];
	PGPikeMTCert *			certInfo;
	PGPPrivateKeyContextRef	privKeyCon = kInvalidPGPPrivateKeyContextRef;
	PGPSize					sigSize,
							actSigSize;
	
	if( exchange->partner->remotePGPVersion &&
		PGPKeyRefIsValid( exchange->partner->pgpAuthKey.authKey ) )
		certInfo = &exchange->partner->pgpAuthKey;
	else
		certInfo = &exchange->partner->x509AuthKey;
	/* There's only one SIG payload in IKE, it signs HASH_I or HASH_R */
	err = pgpIKEGetAuthHash( exchange, exchange->initiator, authHash );CKERR;

	err = PGPNewPrivateKeyContext( certInfo->authKey,
		kPGPPublicKeyMessageFormat_IKE,
		&privKeyCon, certInfo->isPassKey ?
		PGPOPasskeyBuffer( exchange->ike->pgpContext,
							certInfo->pass, certInfo->passLength ) :
		PGPOPassphrase( exchange->ike->pgpContext, certInfo->pass ),
		PGPOLastOption( exchange->ike->pgpContext ) );	CKERR;
	err = PGPGetPrivateKeyOperationSizes( privKeyCon, NULL, NULL, &sigSize );
											CKERR;
	err = PGPPrivateKeySignRaw( privKeyCon, authHash,
								exchange->partner->hashSize,
								mp, &actSigSize );
#if PGPIKE_DEBUG
	if( IsPGPError( err ) )
		pgpIKEDebugLog( exchange->ike, "PGPPrivateKeySignRaw PGPError: %ld\n", err );
#endif
	CKERR;
	pgpAssert( actSigSize == sigSize );
	mp += sigSize;
	
done:
	*mpp = mp;
	if( PGPPrivateKeyContextRefIsValid( privKeyCon ) )
		(void)PGPFreePrivateKeyContext( privKeyCon );
	return err;
}

	PGPError
pgpIKEAddKEPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPByte *				secretX = NULL;
	PGPUInt32				modBits,
							modBytes,
							expBytes = 0,
							expBits,
							ySize;
	
	/* Generate the secret random X value */
	modBits	 = PGPBigNumGetSignificantBits( exchange->dhP );
	modBytes = ( modBits + 7 ) / 8;
	(void)PGPDiscreteLogExponentBits( modBits, &expBytes );
	expBytes = ( expBytes * 3 / 2 + 7 ) / 8;
	expBits  = 8 * expBytes;
	secretX  = PGPNewSecureData( exchange->ike->memMgr, expBytes, 0 );
	if( IsNull( secretX ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	err = PGPContextGetRandomBytes( exchange->ike->pgpContext,
									secretX, expBytes ); CKERR;
	secretX[0] |= 0x80;
	err = PGPNewBigNum( exchange->ike->memMgr, TRUE,
						&exchange->dhX ); CKERR;
	err = PGPBigNumInsertBigEndianBytes( exchange->dhX, secretX, 0,
											expBytes ); CKERR;
	if( exchange->initiator )
	{
		/* Generate initiator Y value */
		err = PGPNewBigNum( exchange->ike->memMgr, TRUE,
							&exchange->dhYi );CKERR;
		err = PGPBigNumExpMod( exchange->dhG, exchange->dhX,
								exchange->dhP, exchange->dhYi ); CKERR;
		
		/* Write out just the Y value */
		ySize = ( ( PGPBigNumGetSignificantBits( exchange->dhP ) + 7 ) / 8 );
		err = PGPBigNumExtractBigEndianBytes( exchange->dhYi, mp, 0,
												ySize ); CKERR;
	}
	else
	{
		/* Generate responder Y value */
		err = PGPNewBigNum( exchange->ike->memMgr, TRUE,
								&exchange->dhYr );CKERR;
		err = PGPBigNumExpMod( exchange->dhG, exchange->dhX,
								exchange->dhP, exchange->dhYr ); CKERR;
		
		/* Write out just the Y value */
		ySize = ( ( PGPBigNumGetSignificantBits( exchange->dhP ) + 7 ) / 8 );
		err = PGPBigNumExtractBigEndianBytes( exchange->dhYr, mp, 0,
												ySize ); CKERR;
	}
	mp += ySize;
done:
	*mpp = mp;
	if( IsntNull( secretX ) )
		(void)PGPFreeData( secretX );
	return err;
}

	PGPError
pgpIKEAddNoncePayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	
	err = PGPContextGetRandomBytes( exchange->ike->pgpContext,
								mp, kPGPike_NonceSize ); CKERR;
	if( exchange->initiator )
	{
		pgpCopyMemory( mp, exchange->initNonce, kPGPike_NonceSize );
		exchange->initNonceLen = kPGPike_NonceSize;
	}
	else
	{
		pgpCopyMemory( mp, exchange->respNonce, kPGPike_NonceSize );
		exchange->respNonceLen = kPGPike_NonceSize;
	}
	mp += kPGPike_NonceSize;
done:
	*mpp = mp;
	return err;
}

	PGPError
pgpIKEAddSAPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPByte	*				hdr;
	PGPUInt16				plen;
	PGPikeProposal *		proposal;

	/* doi */
	PGPUInt32ToEndian( (PGPUInt32) exchange->doi,
			kPGPBigEndian, mp );
	mp += sizeof(PGPUInt32);
	
	/* situation */
	switch( exchange->doi )
	{
		case kPGPike_DOI_IPSEC:
			*mp++ = 0;
			*mp++ = 0;
			*mp++ = 0;
			*mp++ = kPGPike_IPSEC_SIT_IdentityOnly;
			break;
		case kPGPike_DOI_IKE:
			/* behavior undefined by IETF */
			pgpAssert( 0 );
			break;
	}
	/* Proposal */
	for( proposal = exchange->proposals; IsntNull( proposal );
			proposal = proposal->nextProposal )
	{
		hdr = mp;
		
		mp += kPGPike_ISAKMPPayloadHeaderSize;
		err = pgpIKEAddProposalPayload( exchange, proposal, &mp ); CKERR;
		plen = mp - hdr;
		if( IsntNull( proposal->nextProposal ) )
			*hdr++ = kPGPike_PY_Proposal;
		else
			*hdr++ = 0;
		*hdr++ = 0; /* reserved */
		PGPUInt16ToEndian( plen, kPGPBigEndian, hdr );
	}
done:
	*mpp = mp;
	return err;
}

	PGPError
pgpIKEAddProposalPayload(
	PGPikeExchange *		exchange,
	PGPikeProposal *		proposal,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPByte *				nt = NULL;
	PGPByte *				hdr;
	PGPByte					spiSize;
	PGPUInt16				tix,
							tcount = 0,
							plen;

	*mp++ = proposal->number;
	*mp++ = proposal->protocol;
	switch( proposal->protocol )
	{
		case kPGPike_PR_IKE:
			*mp++ = 0;	/* no SPI in Main Mode */
			*mp++ = proposal->numTransforms;
			break;
		case kPGPike_PR_IPCOMP:
		case kPGPike_PR_ESP:
		case kPGPike_PR_AH:
			if( proposal->protocol == kPGPike_PR_IPCOMP )
				spiSize = sizeof(PGPUInt16);
			else
				spiSize = sizeof(PGPipsecSPI);
			*mp++ = spiSize;
			*mp++ = proposal->numTransforms;
			/* insert SPI */
			if( proposal->protocol == kPGPike_PR_IPCOMP )
			{
				PGPUInt16	compCPI = (PGPUInt16)proposal->t[0].u.ipsec.ipcomp.compAlg;
				
				PGPUInt16ToEndian( compCPI, kPGPBigEndian, mp );
			}
			else if( exchange->initiator )
				pgpCopyMemory( &proposal->initSPI[0], mp, spiSize );
			else
				pgpCopyMemory( &proposal->respSPI[0], mp, spiSize );
			mp += spiSize;
			break;
		default:
			pgpAssert(0);
			break;
	}
	
	/* add appropriate transform payloads */
	for( tix = 0; tix < proposal->numTransforms; tix++ )
	{
		hdr = mp;
		mp += kPGPike_ISAKMPPayloadHeaderSize;
		*mp++ = tix + 1;
		err = pgpIKEAddTransformPayload( proposal, &proposal->t[tix], &mp ); CKERR;
		plen = mp - hdr;
		if( proposal->numTransforms - tix > 1 )
			*hdr++ = kPGPike_PY_Transform;
		else
			*hdr++ = 0;
		*hdr++ = 0;	/* reserved */
		PGPUInt16ToEndian( plen, kPGPBigEndian, hdr );
	}
	
	*mpp = mp;
done:
	return err;
}

	PGPError
pgpIKEAddTransformPayload(
	PGPikeProposal *		proposal,
	PGPikeGenericTransform *transform,
	PGPByte **				mpp )
{
	PGPError				err = kPGPError_NoErr;
	PGPByte *				mp = *mpp;
	PGPikeAttributeType		lt, ld;
	
	switch( proposal->protocol )
	{
		case kPGPike_PR_IKE:
		{
			PGPikeTransform *t = &transform->u.ike;
			
			*mp++ = kPGPike_Key_IKE_Transform;
			*mp++ = 0;	/* reserved */
			*mp++ = 0;	/* reserved */
			pgpIKEAddAttribute( kPGPike_AT_EncryptAlg, t->cipher, &mp );
			pgpIKEAddAttribute( kPGPike_AT_HashAlg, t->hash, &mp );
			pgpIKEAddAttribute( kPGPike_AT_AuthMethod, t->authMethod, &mp );
			/*pgpIKEAddAttribute( kPGPike_AT_GroupType, kPGPike_MODPGroupType, &mp );*/
			pgpIKEAddAttribute( kPGPike_AT_GroupDesc, t->groupID, &mp );
			lt = kPGPike_AT_LifeType;	ld = kPGPike_AT_LifeDuration;
			break;
		}
		case kPGPike_PR_AH:
		{
			PGPipsecAHTransform *t = &transform->u.ipsec.ah;
			
			*mp++ = t->authAlg;
			*mp++ = 0;	/* reserved */
			*mp++ = 0;	/* reserved */
			pgpIKEAddAttribute( kPGPike_AT_IPSEC_AuthAttr, t->authAttr, &mp );
			pgpIKEAddAttribute( kPGPike_AT_IPSEC_Encapsulation, t->mode, &mp );
			if( proposal->t[0].u.ipsec.groupID != kPGPike_GR_None )
			{
				pgpIKEAddAttribute( kPGPike_AT_IPSEC_GroupDesc,
					proposal->t[0].u.ipsec.groupID, &mp );
			}
			lt = kPGPike_AT_IPSEC_LifeType;	ld = kPGPike_AT_IPSEC_Duration;
			break;
		}
		case kPGPike_PR_ESP:
		{
			PGPipsecESPTransform *t = &transform->u.ipsec.esp;
			
			*mp++ = t->cipher;
			*mp++ = 0;	/* reserved */
			*mp++ = 0;	/* reserved */
			if( t->authAttr != kPGPike_AA_None )
				pgpIKEAddAttribute( kPGPike_AT_IPSEC_AuthAttr, t->authAttr, &mp );
			pgpIKEAddAttribute( kPGPike_AT_IPSEC_Encapsulation, t->mode, &mp );
			if( proposal->t[0].u.ipsec.groupID != kPGPike_GR_None )
			{
				pgpIKEAddAttribute( kPGPike_AT_IPSEC_GroupDesc,
					proposal->t[0].u.ipsec.groupID, &mp );
			}
			lt = kPGPike_AT_IPSEC_LifeType;	ld = kPGPike_AT_IPSEC_Duration;
			break;
		}
		case kPGPike_PR_IPCOMP:
		{
			PGPipsecIPCOMPTransform *t = &transform->u.ipsec.ipcomp;
			
			*mp++ = t->compAlg;
			*mp++ = 0;	/* reserved */
			*mp++ = 0;	/* reserved */
			/* do not send encapsulation here */
			lt = kPGPike_AT_IPSEC_LifeType;	ld = kPGPike_AT_IPSEC_Duration;
			break;
		}
	}
	if( proposal->kbLifeTime )
	{
		pgpIKEAddAttribute( lt, kPGPike_LT_Kilobytes, &mp );
		pgpIKEAddAttribute( ld, proposal->kbLifeTime, &mp );
	}
	if( proposal->secLifeTime )
	{
		pgpIKEAddAttribute( lt, kPGPike_LT_Seconds, &mp );
		pgpIKEAddAttribute( ld, proposal->secLifeTime, &mp );
	}
	
	*mpp = mp;
	return err;
}
	void
pgpIKEAddAttribute(
	PGPikeAttributeType		attribute,
	PGPUInt32				value,
	PGPByte **				mpp )
{
	PGPByte *				mp = *mpp;
	PGPBoolean				variable = FALSE;
	
	/* If we ever output real variable attributes like Group Generator,
		this function will require modification. */
	if( value > 0x0000FFFF )
		variable = TRUE;
	PGPUInt16ToEndian( (PGPUInt16)attribute, kPGPBigEndian, mp );
	if( !variable )
		*mp |= 0x80;
	mp += sizeof(PGPUInt16);
	if( variable )
	{
		PGPUInt16ToEndian( sizeof(PGPUInt32), kPGPBigEndian, mp );
		mp += sizeof(PGPUInt16);
		PGPUInt32ToEndian( value, kPGPBigEndian, mp );
		mp += sizeof(PGPUInt32);
	}
	else
	{
		PGPUInt16ToEndian( (PGPUInt16)value, kPGPBigEndian, mp );
		mp += sizeof(PGPUInt16);
	}
	
	*mpp = mp;
}

	PGPError
pgpIKELoadGroup(
	PGPikeExchange *		exchange )
{
	PGPError				err = kPGPError_NoErr;
	IKEGroup *				group;
	PGPikeGroupID			groupID;
	PGPUInt16				gIndex;
	
	pgpAssert( IsNull( exchange->dhG ) );
	pgpAssert( IsNull( exchange->dhP ) );
	
	if( exchange->exchangeT == kPGPike_EX_Identity )
		groupID = exchange->proposals->t[0].u.ike.groupID;
	else
		groupID = exchange->proposals->t[0].u.ipsec.groupID;

	for( gIndex = 0; gIndex < kPGPikeNumIKEGroups; gIndex++ )
		if( kPGPike_Groups[gIndex].ikeID == groupID )
			break;
	if( gIndex >= kPGPikeNumIKEGroups )
	{
		err = kPGPError_ItemNotFound;
		goto done;
	}
	group = &kPGPike_Groups[gIndex];
	
	/* Load up the prime modulus */
	err = PGPNewBigNum( exchange->ike->memMgr, TRUE, &exchange->dhP );CKERR;
	err =  PGPBigNumInsertBigEndianBytes(	exchange->dhP, group->prime,
											0, group->length ); CKERR;
	/* Load up the generator (always 2) */
	err = PGPNewBigNum( exchange->ike->memMgr, TRUE, &exchange->dhG );CKERR;
	err =  PGPBigNumInsertBigEndianBytes(	exchange->dhG, dhGenerator,
											0, sizeof(dhGenerator) ); CKERR;
done:
	return err;
}

	PGPError
pgpIKEModP(
	PGPikeExchange *		exchange )
{
	PGPError				err = kPGPError_NoErr;
	PGPBigNumRef			dhSecret = kPGPInvalidBigNumRef;
	PGPUInt32				modBytes;

	err = PGPNewBigNum( exchange->ike->memMgr, TRUE, &dhSecret ); CKERR;
	err = PGPBigNumExpMod( exchange->initiator ?
								exchange->dhYr : exchange->dhYi,
							exchange->dhX, exchange->dhP,
							dhSecret ); CKERR;
	
	/* Load gXY with the shared secret */
	modBytes = ( PGPBigNumGetSignificantBits( exchange->dhP ) + 7 ) / 8;
	exchange->gXYLen = modBytes;
	exchange->gXY = PGPNewSecureData( exchange->ike->memMgr,
											exchange->gXYLen, 0 );
	if( IsNull( exchange->gXY ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	err = PGPBigNumExtractBigEndianBytes( dhSecret, exchange->gXY, 0,
											modBytes );	CKERR;
done:
	if( dhSecret != kPGPInvalidBigNumRef )
		(void)PGPFreeBigNum( dhSecret );
	return err;
}

	PGPError
pgpIKEGoSecure(
	PGPikeExchange *		exchange )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner = exchange->partner;
	PGPUInt16				ySize;
	PGPByte *				ivSeed = NULL;
	PGPHMACContextRef		hmac = kInvalidPGPHMACContextRef;
	PGPHMACContextRef		hmacE = kInvalidPGPHMACContextRef;
	PGPHashContextRef		hash = kInvalidPGPHashContextRef;
	PGPByte					hashChar;
	PGPByte					lastHash[kPGPike_MaxHashSize];
	PGPByte					nonceCat[kPGPike_MaxNonceSize * 2];
	PGPSize					nonceCatLen;
			
	PGPSymmetricCipherContextRef symmRef = kInvalidPGPSymmetricCipherContextRef;
	
	if( IsNull( exchange->dhP ) ||
		IsNull( exchange->dhG ) ||
		IsNull( exchange->dhX ) ||
		IsNull( exchange->dhYi ) ||
		IsNull( exchange->dhYr ) ||
		IsntNull( exchange->gXY ) ||
		( exchange->initNonceLen == 0 ) ||
		( exchange->respNonceLen == 0 ) )
	{
		pgpAssert( 0 );		/* these would be a programming error */
		err = kPGPError_ImproperInitialization;
		goto done;
	}
	err = pgpIKEModP( exchange );	CKERR;
#if PGPIKE_DEBUG
	pgpIKEDebugLog( exchange->ike, "\tpgpIKEGoSecure:\n" );
	pgpIKEDebugData( exchange->ike, "gXY", exchange->gXY, exchange->gXYLen );
#endif
	/* everything is in place, shared secret is set, generate SKEYID */
	switch( exchange->proposals->t[0].u.ike.hash )
	{
		case kPGPike_HA_MD5:
			partner->sdkHashAlg		=	kPGPHashAlgorithm_MD5;
			partner->hashSize		=	kPGPike_MD5HashSize;
			break;
		case kPGPike_HA_SHA1:
			partner->sdkHashAlg		=	kPGPHashAlgorithm_SHA;
			partner->hashSize		=	kPGPike_SHAHashSize;
			break;
		case kPGPike_HA_Tiger:
			pgpAssert( 0 );
			break;
	}
	switch( exchange->proposals->t[0].u.ike.cipher )
	{
		case kPGPike_SC_IDEA_CBC:
			partner->sdkCipherAlg	=	kPGPCipherAlgorithm_IDEA;
			partner->cipherSize		=	kPGPike_IDEAKeySize;
			break;
		case kPGPike_SC_DES_CBC:
			partner->singleDESMode	=	TRUE;
		case kPGPike_SC_3DES_CBC:
			partner->sdkCipherAlg	=	kPGPCipherAlgorithm_3DES;
			partner->cipherSize		=	kPGPike_3DESKeySize;
			break;
		case kPGPike_SC_CAST_CBC:
			partner->sdkCipherAlg	=	kPGPCipherAlgorithm_CAST5;
			partner->cipherSize		=	kPGPike_CASTKeySize;
			break;
	}
	partner->cipherBlockSize		=	kPGPike_MaxCipherBlockSize;
#if PGPIKE_DEBUG
	pgpIKEDebugData( exchange->ike, "Ni", exchange->initNonce,
							exchange->initNonceLen );
	pgpIKEDebugData( exchange->ike, "Nr", exchange->respNonce,
							exchange->respNonceLen );
#endif
	pgpCopyMemory( exchange->initNonce, nonceCat,
					exchange->initNonceLen );
	pgpCopyMemory( exchange->respNonce,
					nonceCat + exchange->initNonceLen,
					exchange->respNonceLen );
	nonceCatLen = exchange->initNonceLen + exchange->respNonceLen;
	switch( exchange->proposals->t[0].u.ike.authMethod )
	{
		case kPGPike_AM_DSS_Sig:
		case kPGPike_AM_RSA_Sig:
		{
			err = PGPNewHMACContext( exchange->ike->memMgr,
							partner->sdkHashAlg,
							nonceCat, nonceCatLen, &hmac );	CKERR;
			err = PGPContinueHMAC( hmac, exchange->gXY,
									exchange->gXYLen );	CKERR;
			err = PGPFinalizeHMAC( hmac, partner->skeyid ); CKERR;
			(void)PGPFreeHMACContext( hmac );
			hmac = kInvalidPGPHMACContextRef;
			break;
		}
		case kPGPike_AM_PreSharedKey:
		{
			pgpAssert( IsntNull( partner->sharedKey ) &&
						( partner->sharedKeySize > 0 ) );
			err = PGPNewHMACContext( exchange->ike->memMgr,
							partner->sdkHashAlg,
							partner->sharedKey, partner->sharedKeySize,
							&hmac );	CKERR;
			err = PGPContinueHMAC( hmac, nonceCat, nonceCatLen );	CKERR;
			err = PGPFinalizeHMAC( hmac, partner->skeyid ); CKERR;
			(void)PGPFreeHMACContext( hmac );
			hmac = kInvalidPGPHMACContextRef;
			break;
		}
		case kPGPike_AM_RSA_Encrypt:
		case kPGPike_AM_RSA_Encrypt_R:
			pgpAssert( 0 );
			break;
	}
#if PGPIKE_DEBUG
	pgpIKEDebugData( exchange->ike, "SKEYID", partner->skeyid,
							partner->hashSize );
#endif
	/* SKEYID_d */
	err = PGPNewHMACContext( exchange->ike->memMgr,
					partner->sdkHashAlg,
					partner->skeyid,
					partner->hashSize, &hmac );CKERR;
	err = PGPContinueHMAC( hmac, exchange->gXY, exchange->gXYLen );CKERR;
	err = PGPContinueHMAC( hmac, partner->initCookie,
							kPGPike_CookieSize );CKERR;
	err = PGPContinueHMAC( hmac, partner->respCookie,
							kPGPike_CookieSize );CKERR;
	hashChar = 0x00;
	err = PGPContinueHMAC( hmac, &hashChar, 1 );CKERR;
	err = PGPFinalizeHMAC( hmac, partner->skeyid_d ); CKERR;
	/* SKEYID_a */
	err = PGPResetHMAC( hmac );	CKERR;
	err = PGPContinueHMAC( hmac, partner->skeyid_d,
									partner->hashSize );CKERR;
	err = PGPContinueHMAC( hmac, exchange->gXY, exchange->gXYLen );CKERR;
	err = PGPContinueHMAC( hmac, partner->initCookie,
							kPGPike_CookieSize );CKERR;
	err = PGPContinueHMAC( hmac, partner->respCookie,
							kPGPike_CookieSize );CKERR;
	hashChar = 0x01;
	err = PGPContinueHMAC( hmac, &hashChar, 1 );CKERR;
	err = PGPFinalizeHMAC( hmac, partner->skeyid_a ); CKERR;
	/* SKEYID_e */
	err = PGPResetHMAC( hmac );	CKERR;
	err = PGPContinueHMAC( hmac, partner->skeyid_a,
									partner->hashSize );CKERR;
	err = PGPContinueHMAC( hmac, exchange->gXY, exchange->gXYLen );CKERR;
	err = PGPContinueHMAC( hmac, partner->initCookie,
							kPGPike_CookieSize );CKERR;
	err = PGPContinueHMAC( hmac, partner->respCookie,
							kPGPike_CookieSize );CKERR;
	hashChar = 0x02;
	err = PGPContinueHMAC( hmac, &hashChar, 1 );CKERR;
	err = PGPFinalizeHMAC( hmac, partner->skeyid_e ); CKERR;
#if PGPIKE_DEBUG
	pgpIKEDebugData( exchange->ike, "SKEYID_d", partner->skeyid_d,
							partner->hashSize );
	pgpIKEDebugData( exchange->ike, "SKEYID_a", partner->skeyid_a,
							partner->hashSize );
	pgpIKEDebugData( exchange->ike, "SKEYID_e", partner->skeyid_e,
							partner->hashSize );
#endif
	
	/* This whole protocol, mostly just to get this one key.... */
	if( !partner->singleDESMode && ( partner->cipherSize > partner->hashSize ) )
	{
		PGPUInt16			keySoFar = 0;
		PGPSize				lastHashLen = 1;
		
		err = PGPNewHMACContext( exchange->ike->memMgr,
									partner->sdkHashAlg,
									partner->skeyid_e,
									partner->hashSize, &hmacE );CKERR;
		while( keySoFar < partner->cipherSize )
		{
			if( !keySoFar )
				lastHash[0] = 0x00;
			err = PGPContinueHMAC( hmacE, &lastHash, lastHashLen );CKERR;
			err = PGPFinalizeHMAC( hmacE, lastHash ); CKERR;
			lastHashLen = partner->hashSize;
#if PGPIKE_DEBUG
			pgpIKEDebugData( exchange->ike, "KX", lastHash, lastHashLen );
#endif
			if( keySoFar + lastHashLen > partner->cipherSize )
				lastHashLen = partner->cipherSize - keySoFar;
			pgpCopyMemory( lastHash, partner->cipherKey + keySoFar,
							lastHashLen );
			keySoFar += lastHashLen;
			err = PGPResetHMAC( hmacE );	CKERR;
		}
		err = PGPFreeHMACContext( hmacE );	CKERR;
		hmacE = kInvalidPGPHMACContextRef;
	}
	else
		pgpCopyMemory( partner->skeyid_e, partner->cipherKey,
						partner->cipherSize );
	if( partner->singleDESMode )
	{
		pgpClearMemory( partner->cipherKey + kPGPike_DESKeySize,
						kPGPike_3DESKeySize - kPGPike_DESKeySize );
	}
#if PGPIKE_DEBUG
	pgpIKEDebugData( exchange->ike, "cipherKey", partner->cipherKey,
							partner->cipherSize );
#endif
	if( PGPCBCContextRefIsValid( partner->cbc ) )
	{
		PGPFreeCBCContext( partner->cbc );
		partner->cbc = kInvalidPGPCBCContextRef;
	}
	/* Key is all set, setup the CBC context */
	err = PGPNewSymmetricCipherContext( exchange->ike->memMgr,
							partner->sdkCipherAlg, partner->cipherSize,
							&symmRef );	CKERR;
	err = PGPNewCBCContext( symmRef, &partner->cbc ); CKERR;
	symmRef = kInvalidPGPSymmetricCipherContextRef;
	/* create the initial IV for the CBC context */
	ySize = ( ( PGPBigNumGetSignificantBits( exchange->dhP ) + 7 ) / 8 );
	ivSeed = PGPNewData( exchange->ike->memMgr, ySize * 2,
							kPGPMemoryMgrFlags_Clear );
	if( IsNull( ivSeed ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	err = PGPBigNumExtractBigEndianBytes( exchange->dhYi, ivSeed, 0,
											ySize ); CKERR;
	err = PGPBigNumExtractBigEndianBytes( exchange->dhYr, ivSeed + ySize, 0,
											ySize ); CKERR;
	err = PGPNewHashContext( exchange->ike->memMgr, partner->sdkHashAlg,
								&hash );	CKERR;
	err = PGPContinueHash( hash, ivSeed, ySize * 2 );	CKERR;
	err = PGPFinalizeHash( hash, lastHash );	CKERR;
	pgpCopyMemory( lastHash, exchange->lastCBC, partner->cipherBlockSize );
	pgpCopyMemory( lastHash, partner->lastP1CBC, partner->cipherBlockSize );
#if PGPIKE_DEBUG
	pgpIKEDebugData( exchange->ike, "IV", partner->lastP1CBC,
							partner->cipherBlockSize );
#endif
	
done:
	if( IsntNull( ivSeed ) )
		(void)PGPFreeData( ivSeed );
	if( PGPHashContextRefIsValid( hash ) )
		(void)PGPFreeHashContext( hash );
	if( PGPHMACContextRefIsValid( hmac ) )
		(void)PGPFreeHMACContext( hmac );
	if( PGPHMACContextRefIsValid( hmacE ) )
		(void)PGPFreeHMACContext( hmacE );
	if( PGPSymmetricCipherContextRefIsValid( symmRef ) )
		(void)PGPFreeSymmetricCipherContext( symmRef );
	return err;
}

	PGPError
pgpIKESAEstablished(
	PGPikeContextPriv *		ike,
	PGPikeSA *				sa )
{
	PGPError				err = kPGPError_NoErr;
	
	err = pgpIKERemovePending( ike, sa->ipAddress, sa->ipAddrStart,
			sa->ipMaskEnd, sa->destIsRange );	CKERR;
	err = (ike->msgProc)( (PGPikeContextRef)ike,
				ike->userData, kPGPike_MT_SAEstablished, sa );
	err = pgpIKELocalAlert( ike, sa->ipAddress,
			kPGPike_AL_None, kPGPike_IA_NewPhase2SA, FALSE );
done:
	return err;
}

	PGPError
pgpIKEGetP2Keymat(
	PGPikeExchange *		exchange,
	PGPipsecDOIParams *		ipsec )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner = exchange->partner;
	PGPUInt16				keymatNeeded = 0,
							keymatIters,
							keymatIndex;
	PGPHMACContextRef		hmac = kInvalidPGPHMACContextRef;
	PGPByte					inKeymat[256];
	PGPByte					outKeymat[256];
	PGPByte *				iKeymatP;
	PGPByte *				oKeymatP;
	PGPByte					protocol8;
	PGPSize					keySize;
	
	/* This function could be somewhat cleaner */
	if( IsNull( exchange->gXY ) &&
		( exchange->proposals->t[0].u.ipsec.groupID != kPGPike_GR_None ) )
	{
		if( IsntNull( exchange->dhP ) &&
			IsntNull( exchange->dhG ) &&
			IsntNull( exchange->dhX ) &&
			IsntNull( exchange->dhYi ) &&
			IsntNull( exchange->dhYr ) )
		{
			err = pgpIKEModP( exchange );	CKERR;
#if PGPIKE_DEBUG
			pgpIKEDebugLog( exchange->ike, "\tQM Key Material using PFS.\n" );
#endif
		}
		else
		{
			pgpAssert( 0 );		/* these would be a programming error */
			err = kPGPError_ImproperInitialization;
			goto done;
		}
	}
	switch( ipsec->protocol )
	{
		case kPGPike_PR_AH:
			switch( ipsec->u.ah.t.authAttr )
			{
				case kPGPike_AA_HMAC_MD5:
					keymatNeeded += kPGPike_MD5HashSize;
					break;
				case kPGPike_AA_HMAC_SHA:
					keymatNeeded += kPGPike_SHAHashSize;
					break;
				default:
					pgpAssert( 0 );	/* unsupported AH attr */
					break;
			}
			break;
		case kPGPike_PR_ESP:
			switch( ipsec->u.esp.t.cipher )
			{
				case kPGPike_ET_3DES:
					keymatNeeded += kPGPike_3DESKeySize;
					break;
				case kPGPike_ET_CAST:
					keymatNeeded += kPGPike_CASTKeySize;
					break;
				case kPGPike_ET_DES:
				case kPGPike_ET_DES_IV64:
					keymatNeeded += kPGPike_DESKeySize;
					break;
				case kPGPike_ET_NULL:
					break;
				default:
					pgpAssert( 0 );	/* unsupported cipher */
					break;
			}
			switch( ipsec->u.ah.t.authAttr )
			{
				case kPGPike_AA_HMAC_MD5:
					keymatNeeded += kPGPike_MD5HashSize;
					break;
				case kPGPike_AA_HMAC_SHA:
					keymatNeeded += kPGPike_SHAHashSize;
					break;
				case kPGPike_AA_None:
					break;
				default:
					pgpAssert( 0 );	/* unsupported AH attr */
					break;
			}
			break;
		case kPGPike_PR_IPCOMP:
			goto done;	/* no keymat required */
	}
	pgpAssert( keymatNeeded < 256 );
	keymatIters = keymatNeeded / partner->hashSize;
	if( keymatNeeded % partner->hashSize )
		keymatIters++;
	protocol8 = (PGPByte)ipsec->protocol;
	err = PGPNewHMACContext( exchange->ike->memMgr,
							partner->sdkHashAlg,
							partner->skeyid_d, partner->hashSize, &hmac );	CKERR;
	iKeymatP = inKeymat;
	for( keymatIndex = 0; keymatIndex < keymatIters; keymatIndex++ )
	{
		if( keymatIndex > 0 )
		{
			err = PGPContinueHMAC( hmac, iKeymatP, partner->hashSize );	CKERR;
			iKeymatP += partner->hashSize;
		}
		if( IsntNull( exchange->gXY ) )
		{
			err = PGPContinueHMAC( hmac, exchange->gXY,
								exchange->gXYLen );	CKERR;
		}
		err = PGPContinueHMAC( hmac, &protocol8, 1 );
		err = PGPContinueHMAC( hmac, &ipsec->inSPI, sizeof(PGPipsecSPI) );	CKERR;
		err = PGPContinueHMAC( hmac, &exchange->initNonce,
									exchange->initNonceLen );	CKERR;
		err = PGPContinueHMAC( hmac, &exchange->respNonce,
									exchange->respNonceLen );	CKERR;
		err = PGPFinalizeHMAC( hmac, iKeymatP );	CKERR;
		err = PGPResetHMAC( hmac );	CKERR;
	}
	err = PGPResetHMAC( hmac );	CKERR;
	oKeymatP = outKeymat;
	for( keymatIndex = 0; keymatIndex < keymatIters; keymatIndex++ )
	{
		if( keymatIndex > 0 )
		{
			err = PGPContinueHMAC( hmac, oKeymatP, partner->hashSize );	CKERR;
			oKeymatP += partner->hashSize;
		}
		if( IsntNull( exchange->gXY ) )
		{
			err = PGPContinueHMAC( hmac, exchange->gXY,
								exchange->gXYLen );	CKERR;
		}
		err = PGPContinueHMAC( hmac, &protocol8, 1 );
		err = PGPContinueHMAC( hmac, &ipsec->outSPI, sizeof(PGPipsecSPI) );	CKERR;
		err = PGPContinueHMAC( hmac, &exchange->initNonce,
									exchange->initNonceLen );	CKERR;
		err = PGPContinueHMAC( hmac, &exchange->respNonce,
									exchange->respNonceLen );	CKERR;
		err = PGPFinalizeHMAC( hmac, oKeymatP );	CKERR;
		err = PGPResetHMAC( hmac );	CKERR;
	}
	iKeymatP = inKeymat;
	oKeymatP = outKeymat;
	switch( ipsec->protocol )
	{
		case kPGPike_PR_AH:
			switch( ipsec->u.ah.t.authAttr )
			{
				case kPGPike_AA_HMAC_MD5:
					keySize = kPGPike_MD5HashSize;
					break;
				case kPGPike_AA_HMAC_SHA:
					keySize = kPGPike_SHAHashSize;
					break;
				default:
					pgpAssert( 0 );
			}
			pgpCopyMemory( iKeymatP, ipsec->u.ah.inAuthKey, keySize );
			pgpCopyMemory( oKeymatP, ipsec->u.ah.outAuthKey, keySize );
			iKeymatP += keySize;
			oKeymatP += keySize;
			break;
		case kPGPike_PR_ESP:
			err = PGPContextGetRandomBytes( exchange->ike->pgpContext,
					&ipsec->u.esp.explicitIV,
					kPGPike_MaxExplicitIVSize ); CKERR;
			switch( ipsec->u.esp.t.cipher )
			{
				case kPGPike_ET_3DES:
					keySize = kPGPike_3DESKeySize;
					break;
				case kPGPike_ET_CAST:
					keySize = kPGPike_CASTKeySize;
					break;
				case kPGPike_ET_DES_IV64:
				case kPGPike_ET_DES:
					keySize = kPGPike_DESKeySize;
					break;
				case kPGPike_ET_NULL:
					keySize = 0;
					break;
				default:
					pgpAssert( 0 );
			}
			pgpCopyMemory( iKeymatP, ipsec->u.esp.inESPKey, keySize );
			pgpCopyMemory( oKeymatP, ipsec->u.esp.outESPKey, keySize );
			iKeymatP += keySize;
			oKeymatP += keySize;
			switch( ipsec->u.ah.t.authAttr )
			{
				case kPGPike_AA_HMAC_MD5:
					keySize = kPGPike_MD5HashSize;
					break;
				case kPGPike_AA_HMAC_SHA:
					keySize = kPGPike_SHAHashSize;
					break;
				case kPGPike_AA_None:
					keySize = 0;
					break;
				default:
					pgpAssert( 0 );
			}
			pgpCopyMemory( iKeymatP, ipsec->u.esp.inAuthKey, keySize );
			pgpCopyMemory( oKeymatP, ipsec->u.esp.outAuthKey, keySize );
			iKeymatP += keySize;
			oKeymatP += keySize;
			break;
	}
done:
	if( PGPHMACContextRefIsValid( hmac ) )
		(void)PGPFreeHMACContext( hmac );
	return err;
}

	PGPError
pgpIKECompletePhase2(
	PGPikeExchange *		exchange,
	PGPikeSA **				saP )
{
	PGPError				err = kPGPError_NoErr;
	PGPikePartner *			partner = exchange->partner;
	PGPikeSA *				sa = NULL;
	PGPikeProposal *		proposal;
	PGPUInt16				propNum = 0;
	
	*saP = NULL;
	sa = PGPNewData( exchange->ike->memMgr, sizeof(PGPikeSA),
						kPGPMemoryMgrFlags_Clear );
	if( IsNull( sa ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	sa->ipAddress		= partner->ipAddress;
	sa->activeIn		= TRUE;
	sa->activeOut		= TRUE;
	sa->kbLifeTime		= exchange->proposals->kbLifeTime;
	sa->secLifeTime		= exchange->proposals->secLifeTime;
	sa->birthTime		= PGPGetTime();
	sa->doi				= exchange->doi;
	sa->destIsRange		= partner->ipsecOpts.destIsRange;
	sa->ipAddrStart		= partner->ipsecOpts.ipAddrStart;
	sa->ipMaskEnd		= partner->ipsecOpts.ipMaskEnd;
	for( proposal = exchange->proposals; IsntNull( proposal );
			proposal = proposal->nextProposal )
	{
		sa->transform[propNum].u.ipsec.protocol	= proposal->protocol;
		if( exchange->initiator )
		{
			pgpCopyMemory( proposal->respSPI, sa->transform[propNum].u.ipsec.outSPI,
							sizeof(PGPipsecSPI) );
			pgpCopyMemory( proposal->initSPI, sa->transform[propNum].u.ipsec.inSPI,
							sizeof(PGPipsecSPI) );
		}
		else
		{
			pgpCopyMemory( proposal->respSPI, sa->transform[propNum].u.ipsec.inSPI,
							sizeof(PGPipsecSPI) );
			pgpCopyMemory( proposal->initSPI, sa->transform[propNum].u.ipsec.outSPI,
							sizeof(PGPipsecSPI) );
		}
		switch( proposal->protocol )
		{
			case kPGPike_PR_AH:
				pgpCopyMemory( &proposal->t[0].u.ipsec.ah,
								&sa->transform[propNum].u.ipsec.u.ah.t,
								sizeof(PGPipsecAHTransform) );
				break;
			case kPGPike_PR_ESP:
				
				pgpCopyMemory( &proposal->t[0].u.ipsec.esp,
								&sa->transform[propNum].u.ipsec.u.esp.t,
								sizeof(PGPipsecESPTransform) );
				break;
			case kPGPike_PR_IPCOMP:
				pgpCopyMemory( &proposal->t[0].u.ipsec.ipcomp,
								&sa->transform[propNum].u.ipsec.u.ipcomp.t,
								sizeof(PGPipsecIPCOMPTransform) );
				break;
		}
		err = pgpIKEGetP2Keymat( exchange, &sa->transform[propNum].u.ipsec );CKERR;
		propNum++;
	}
	sa->numTransforms = propNum;
	
	sa->nextSA = exchange->ike->sa;
	if( IsntNull( sa->nextSA ) )
		sa->nextSA->prevSA = sa;
	exchange->ike->sa = sa;
	exchange->partner->termSchedule = 0;
	*saP = sa;
done:
	return err;
}

	PGPError
pgpIKEGetAuthHash(
	PGPikeExchange *		exchange,
	PGPBoolean				initiator,
	PGPByte *				outHash )
{
	PGPError				err = kPGPError_NoErr;
	PGPHMACContextRef		hmac;
	PGPUInt32				ySize;
	PGPByte					yData[512];
	
	/* This function could be more compact, but it works */
	err = PGPNewHMACContext( exchange->ike->memMgr,
							exchange->partner->sdkHashAlg,
							&exchange->partner->skeyid[0],
							exchange->partner->hashSize, &hmac );	CKERR;
	ySize = ( ( PGPBigNumGetSignificantBits( exchange->dhP ) + 7 ) / 8 );
	if( initiator )
	{
		err = PGPBigNumExtractBigEndianBytes( exchange->dhYi, yData, 0,
												ySize ); CKERR;
		(void)PGPContinueHMAC( hmac, yData, ySize );
		err = PGPBigNumExtractBigEndianBytes( exchange->dhYr, yData, 0,
												ySize ); CKERR;
		(void)PGPContinueHMAC( hmac, yData, ySize );
		(void)PGPContinueHMAC( hmac, exchange->partner->initCookie,
								kPGPike_CookieSize );
		(void)PGPContinueHMAC( hmac, exchange->partner->respCookie,
								kPGPike_CookieSize );
		(void)PGPContinueHMAC( hmac, exchange->initSABody,
								exchange->initSABodySize );
		(void)PGPContinueHMAC( hmac, exchange->idBody,
								exchange->idBodySize );
	}
	else
	{
		err = PGPBigNumExtractBigEndianBytes( exchange->dhYr, yData, 0,
												ySize ); CKERR;
		(void)PGPContinueHMAC( hmac, yData, ySize );
		err = PGPBigNumExtractBigEndianBytes( exchange->dhYi, yData, 0,
												ySize ); CKERR;
		(void)PGPContinueHMAC( hmac, yData, ySize );
		(void)PGPContinueHMAC( hmac, exchange->partner->respCookie,
								kPGPike_CookieSize );
		(void)PGPContinueHMAC( hmac, exchange->partner->initCookie,
								kPGPike_CookieSize );
		(void)PGPContinueHMAC( hmac, exchange->initSABody,
								exchange->initSABodySize );
		(void)PGPContinueHMAC( hmac, exchange->idBody,
								exchange->idBodySize );
	}
	
	err = PGPFinalizeHMAC( hmac, outHash );	CKERR;
done:
	if( PGPHMACContextRefIsValid( hmac ) )
		(void)PGPFreeHMACContext( hmac );
	return err;
}

	PGPBoolean
pgpIKEIsTransformEqual(
	PGPipsecProtocol				protocol,
	const PGPikeGenericTransform *	transform1,
	const PGPikeGenericTransform *	transform2 )
{
	PGPBoolean						same = FALSE;
	
	switch( protocol )
	{
		case kPGPike_PR_IKE:
		{	
			PGPikeTransform *		t = (PGPikeTransform *)transform1;
			PGPikeTransform *		t2 = (PGPikeTransform *)transform2;
			
			if( ( t->authMethod == t2->authMethod ) &&
				( t->hash == t2->hash ) &&
				( t->cipher == t2->cipher ) &&
				( t->modpGroup == t2->modpGroup ) &&
				( t->groupID == t2->groupID ) )
				same = TRUE;
			break;
		}
		case kPGPike_PR_ESP:
		{	
			PGPipsecESPTransform *	t = (PGPipsecESPTransform *)transform1;
			PGPipsecESPTransform *	t2 = (PGPipsecESPTransform *)transform2;
			
			if( ( t->cipher == t2->cipher ) &&
				( t->authAttr == t2->authAttr ) &&
				( t->mode == t2->mode ) )
				same = TRUE;
			break;
		}
		case kPGPike_PR_AH:
		{
			PGPipsecAHTransform *	t = (PGPipsecAHTransform *)transform1;
			PGPipsecAHTransform *	t2 = (PGPipsecAHTransform *)transform2;
			
			if( ( t->authAlg == t2->authAlg ) &&
				( t->authAttr == t2->authAttr ) &&
				( t->mode == t2->mode ) )
				same = TRUE;
			break;
		}
		case kPGPike_PR_IPCOMP:
		{	
			PGPipsecIPCOMPTransform *	t = (PGPipsecIPCOMPTransform *)transform1;
			PGPipsecIPCOMPTransform *	t2 = (PGPipsecIPCOMPTransform *)transform2;
			
			if( t->compAlg == t2->compAlg )	/* ***** more attributes? */
				same = TRUE;
			break;
		}
	}
	return same;
}

	PGPBoolean
pgpIKEIsTransformValid(
	PGPikePartner *					partner,
	PGPipsecProtocol				protocol,
	const PGPikeGenericTransform *	transform )
{
	PGPBoolean						valid = FALSE,
									prop;
	PGPInt32						alg;
	PGPikeAllowedAlgorithms *		allow = &partner->ike->allowedAlgorithms;
	
	switch( protocol )
	{
		case kPGPike_PR_IKE:
		{
			switch( transform->u.ike.authMethod )
			{
				case kPGPike_AM_PreSharedKey:
					if( IsNull( partner->sharedKey ) ||
						( partner->sharedKeySize == 0 ) )
						goto done;
					break;
				case kPGPike_AM_DSS_Sig:
					if( PGPKeyRefIsValid( partner->pgpAuthKey.authKey ) &&
						IsntPGPError( PGPGetKeyNumber( partner->pgpAuthKey.authKey,
										kPGPKeyPropAlgID, &alg ) ) &&
						( alg == kPGPPublicKeyAlgorithm_DSA ) &&
						IsntPGPError( PGPGetKeyBoolean( partner->pgpAuthKey.authKey,
										kPGPKeyPropCanSign, &prop ) ) &&
						prop )
						break;
					if( PGPKeyRefIsValid( partner->x509AuthKey.authKey ) &&
						IsntPGPError( PGPGetKeyNumber( partner->x509AuthKey.authKey,
										kPGPKeyPropAlgID, &alg ) ) &&
						( alg == kPGPPublicKeyAlgorithm_DSA ) &&
						IsntPGPError( PGPGetKeyBoolean( partner->x509AuthKey.authKey,
										kPGPKeyPropCanSign, &prop ) ) &&
						prop )
						break;
					goto done;
				case kPGPike_AM_RSA_Sig:
					if( PGPKeyRefIsValid( partner->pgpAuthKey.authKey ) &&
						IsntPGPError( PGPGetKeyNumber( partner->pgpAuthKey.authKey,
										kPGPKeyPropAlgID, &alg ) ) &&
						( alg == kPGPPublicKeyAlgorithm_RSA ) &&
						IsntPGPError( PGPGetKeyBoolean( partner->pgpAuthKey.authKey,
										kPGPKeyPropCanSign, &prop ) ) &&
						prop )
						break;
					if( PGPKeyRefIsValid( partner->x509AuthKey.authKey ) &&
						IsntPGPError( PGPGetKeyNumber( partner->x509AuthKey.authKey,
										kPGPKeyPropAlgID, &alg ) ) &&
						( alg == kPGPPublicKeyAlgorithm_RSA ) &&
						IsntPGPError( PGPGetKeyBoolean( partner->x509AuthKey.authKey,
										kPGPKeyPropCanSign, &prop ) ) &&
						prop )
						break;
				case kPGPike_AM_RSA_Encrypt:
				case kPGPike_AM_RSA_Encrypt_R:
				default:
					goto done;
			}
			switch( transform->u.ike.hash )
			{
				case kPGPike_HA_SHA1:
					if( !allow->sha1 )
						goto done;
					break;
				case kPGPike_HA_MD5:
					if( !allow->md5 )
						goto done;
					break;
				default:
					goto done;
			}
			switch( transform->u.ike.cipher )
			{
				case kPGPike_SC_CAST_CBC:
					if( !allow->cast5 )
						goto done;
					break;
				case kPGPike_SC_3DES_CBC:
					if( !allow->tripleDES )
						goto done;
					break;
				case kPGPike_SC_DES_CBC:
					if( !allow->singleDES )
						goto done;
					break;
				default:
					goto done;
			}
			if( !transform->u.ike.modpGroup )
				goto done;
			switch( transform->u.ike.groupID )
			{
				case kPGPike_GR_MODPOne:
					if( !allow->modpOne768 )
						goto done;
					break;
				case kPGPike_GR_MODPTwo:
					if( !allow->modpTwo1024 )
						goto done;
					break;
				case kPGPike_GR_MODPFive:
					if( !allow->modpFive1536 )
						goto done;
					break;
				default:
					goto done;
			}
			/* This transform looks good */
			valid = TRUE;
			break;
		}
		case kPGPike_PR_AH:
			switch( transform->u.ipsec.ah.authAlg )
			{
				case kPGPike_AH_MD5:
					if( !allow->md5 )
						goto done;
					break;
				case kPGPike_AH_SHA:
					if( !allow->sha1 )
						goto done;
					break;
				default:
					goto done;
			}
			switch( transform->u.ipsec.ah.authAttr )
			{
				case kPGPike_AA_HMAC_MD5:
					if( transform->u.ipsec.ah.authAlg != kPGPike_AH_MD5 )
						goto done;
					break;
				case kPGPike_AA_HMAC_SHA:
					if( transform->u.ipsec.ah.authAlg != kPGPike_AH_SHA )
						goto done;
					break;
				default:
					goto done;
			}
			switch( transform->u.ipsec.ah.mode )
			{
				case kPGPike_PM_Tunnel:
				case kPGPike_PM_Transport:
					break;
				default:
					goto done;
			}
			switch( transform->u.ipsec.groupID )
			{
				case kPGPike_GR_None:
					break;
				case kPGPike_GR_MODPOne:
					if( !allow->modpOne768 )
						goto done;
					break;
				case kPGPike_GR_MODPTwo:
					if( !allow->modpTwo1024 )
						goto done;
					break;
				case kPGPike_GR_MODPFive:
					if( !allow->modpFive1536 )
						goto done;
					break;
				default:
					goto done;
			}
			/* This transform looks good */
			valid = TRUE;
			break;
		case kPGPike_PR_ESP:
			switch( transform->u.ipsec.esp.cipher )
			{
				case kPGPike_ET_3DES:
					if( !allow->tripleDES )
						goto done;
					break;
				case kPGPike_ET_CAST:
					if( !allow->cast5 )
						goto done;
					break;
				case kPGPike_ET_DES:
				case kPGPike_ET_DES_IV64:
					if( !allow->singleDES )
						goto done;
					break;
				case kPGPike_ET_NULL:
					if( !allow->espNULL )
						goto done;
					break;
				default:
					goto done;
			}
			switch( transform->u.ipsec.esp.authAttr )
			{
				case kPGPike_AA_HMAC_MD5:
					if( !allow->md5 )
						goto done;
					break;
				case kPGPike_AA_HMAC_SHA:
					if( !allow->sha1 )
						goto done;
					break;
				case kPGPike_AA_None:
					if( !allow->noAuth )
						goto done;
					break;
				default:
					goto done;
			}
			if( ( transform->u.ipsec.esp.authAttr == kPGPike_AA_None ) &&
				( transform->u.ipsec.esp.cipher == kPGPike_ET_NULL ) )
				goto done;
			switch( transform->u.ipsec.esp.mode )
			{
				case kPGPike_PM_Tunnel:
				case kPGPike_PM_Transport:
					break;
				default:
					goto done;
			}
			switch( transform->u.ipsec.groupID )
			{
				case kPGPike_GR_None:
					break;
				case kPGPike_GR_MODPOne:
					if( !allow->modpOne768 )
						goto done;
					break;
				case kPGPike_GR_MODPTwo:
					if( !allow->modpTwo1024 )
						goto done;
					break;
				case kPGPike_GR_MODPFive:
					if( !allow->modpFive1536 )
						goto done;
					break;
				default:
					goto done;
			}
			/* This transform looks good */
			valid = TRUE;
			break;
		case kPGPike_PR_IPCOMP:
			switch( transform->u.ipsec.ipcomp.compAlg )
			{
				case kPGPike_IC_Deflate:
					if( !allow->deflate )
						goto done;
					break;
				case kPGPike_IC_LZS:
					if( !allow->lzs )
						goto done;
					break;
				default:
					goto done;
			}
			/* This transform looks good */
			valid = TRUE;
			break;
		default:
			goto done;
	}
done:
	return valid;
}

	PGPError
pgpIKEResendLastPacket(
	PGPikeExchange *		exchange )
{
	PGPError				err = kPGPError_NoErr;

#if PGPIKE_DEBUG
	pgpIKEDebugLog( exchange->ike, "pgpIKEResendLastPacket (%d)\n", exchange->retry );
#endif
	--exchange->retry;
	if( !exchange->retry )
	{
		err = pgpIKEAbortExchange( &exchange, kPGPike_AL_None );
		goto done;
	}
	exchange->lastTransmit	= PGPGetMilliseconds();
	err = pgpIKESendPacket1( exchange->partner, exchange->lastPkt,
								exchange->lastPktSize );
done:
	return err;
}

/* send packet and place it in retry position */
	PGPError
pgpIKESendPacket(
	PGPikeExchange *		exchange,
	PGPByte *				packet,
	PGPSize					packetSize )
{
	PGPError				err = kPGPError_NoErr;
	
	pgpAssertAddrValid( exchange, PGPikeExchange * );

#if PGPIKE_DEBUG
	pgpIKEDebugLog( exchange->ike, "SENT" );
#endif
	err = pgpIKESendPacket1( exchange->partner, packet, packetSize );
	
	/* set packet up for future retry */
	if( IsntNull( exchange->lastPkt ) )
		(void)PGPFreeData( exchange->lastPkt );
	
	exchange->retry			= kPGPike_MaxRetries;
	exchange->lastTransmit	= PGPGetMilliseconds();
	exchange->lastPkt		= packet;
	exchange->lastPktSize	= packetSize;
	
	return err;
}

/* Send the header-equipped packet to the caller */
	PGPError
pgpIKESendPacket1(
	PGPikePartner *			partner,
	PGPByte *				packet,
	PGPSize					packetSize )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeMTPacket			s;
	
	pgpAssertAddrValid( partner, PGPikePartner * );
	if( IsNull( packet ) || !packetSize )
		return err;
	
	s.ipAddress			= partner->ipAddress;
	s.packetSize		= packetSize;
	s.packet			= packet;
	
	err = (partner->ike->msgProc)( (PGPikeContextRef)partner->ike,
									partner->ike->userData,
									kPGPike_MT_Packet, &s );
	
	return err;
}

	PGPError
pgpIKEGetCert(
	PGPikePartner *			partner,
	PGPikeMessageType		msg )
{
	PGPError				err = kPGPError_NoErr;
	PGPikeMTCert			cert;
	
	pgpAssertAddrValid( partner, PGPikePartner * );
	pgpClearMemory( &cert, sizeof(PGPikeMTCert) );
	cert.ipAddress = partner->ipAddress;
	err = (partner->ike->msgProc)( (PGPikeContextRef)partner->ike,
									partner->ike->userData,
									msg, &cert );
	
	if( IsntPGPError( err ) && PGPKeyRefIsValid( cert.authKey ) &&
		( ( msg != kPGPike_MT_LocalX509Cert ) ||
			PGPSigRefIsValid( cert.authCert ) ) )
	{
		void *	callerPass = cert.pass;
		
		if( PGPKeySetRefIsValid( cert.baseKeySet ) )
			PGPIncKeySetRefCount( cert.baseKeySet );
		if( ( cert.passLength == 0 ) && !cert.isPassKey )
		{
			cert.passLength = 1;
			callerPass = "\0";
		}
		cert.pass = PGPNewSecureData( partner->ike->memMgr, cert.passLength,
						kPGPMemoryMgrFlags_Clear );
		if( IsntNull( cert.pass ) )
			pgpCopyMemory( callerPass, cert.pass, cert.passLength );
		else
			err = kPGPError_OutOfMemory;
		switch( msg )
		{
			case kPGPike_MT_LocalPGPCert:
				pgpCopyMemory( &cert, &partner->pgpAuthKey,
								sizeof( PGPikeMTCert ) );
				break;
			case kPGPike_MT_LocalX509Cert:
				pgpCopyMemory( &cert, &partner->x509AuthKey,
								sizeof( PGPikeMTCert ) );
				break;
			default:
				pgpAssert( 0 );
				break;
		}
	}
	
	return err;
}

	void
pgpIKEDebugData(
	PGPikeContextPriv *		ike,
	char *					name,
	PGPByte *				data,
	PGPSize					length )
{
	char					buffer[1024];
	PGPUInt16				bIndex,
							bufLen;
	
	if( length > sizeof(buffer) / 5 )
		length = sizeof(buffer) / 5;
	for( bufLen = bIndex = 0; bIndex < length; bIndex++, bufLen += 4 )
	{
		sprintf( buffer + bufLen, "%02X%s", data[bIndex],
					( ( bIndex + 1 ) % 16 ) ? "  " : "\n\t" );
	}
	pgpIKEDebugLog( ike, "\t%s (%d):\n\t%s\n", name, length, buffer );
}

	void
pgpIKEDebugLog(
	PGPikeContextPriv *		ike,
	char *					fmt,
	... )
{
	va_list					ap;
	char					buf[1024];
	
	va_start( ap, fmt );
	vsprintf( buf, fmt, ap );
	va_end( ap );

	(void)(ike->msgProc)( (PGPikeContextRef)ike,
							ike->userData,
							kPGPike_MT_DebugLog, buf );
}

	char *
pgpIKECipherString(
	PGPikeCipher			cipher )
{
	char	*				ciStr = "unknown";
	
	switch( cipher )
	{
		case kPGPike_SC_DES_CBC:	ciStr = "SingleDES";	break;
		case kPGPike_SC_3DES_CBC:	ciStr = "TripleDES";	break;
		case kPGPike_SC_CAST_CBC:	ciStr = "CAST5";		break;
	}
	return ciStr;
}

	char *
pgpIKEAuthMethodString(
	PGPikeAuthMethod		auth )
{
	char	*				amStr = "unknown";
	
	switch( auth )
	{
		case kPGPike_AM_PreSharedKey:	amStr = "PreSharedKey";		break;
		case kPGPike_AM_DSS_Sig:		amStr = "DSS Sig";			break;
		case kPGPike_AM_RSA_Sig:		amStr = "RSA Sig";			break;
		case kPGPike_AM_RSA_Encrypt:	amStr = "RSA Encrypt";		break;
		case kPGPike_AM_RSA_Encrypt_R:	amStr = "RSA Encrypt R";	break;
	}
	return amStr;
}

	char *
pgpIKEPayloadTypeString(
	PGPikePayload			payload )
{
	char	*				ptStr = "unknown";
	
	switch( payload )
	{
		case kPGPike_PY_SA:				ptStr = "SA";				break;
		case kPGPike_PY_Proposal:		ptStr = "Prop";				break;
		case kPGPike_PY_Transform:		ptStr = "Trans";			break;
		case kPGPike_PY_KeyExchange:	ptStr = "KE";				break;
		case kPGPike_PY_Identification:	ptStr = "Ident";			break;
		case kPGPike_PY_Certificate:	ptStr = "Cert";				break;
		case kPGPike_PY_CertRequest:	ptStr = "CertReq";			break;
		case kPGPike_PY_Hash:			ptStr = "Hash";				break;
		case kPGPike_PY_Signature:		ptStr = "Sig";				break;
		case kPGPike_PY_Nonce:			ptStr = "Nonce";			break;
		case kPGPike_PY_Notification:	ptStr = "Notify";			break;
		case kPGPike_PY_Delete:			ptStr = "Delete";			break;
		case kPGPike_PY_VendorID:		ptStr = "Vendor";			break;
	}
	return ptStr;
}

	char *
pgpIKEExchangeTypeString(
	PGPikeExchangeT			exchange )
{
	char	*				etStr = "unknown";
	
	switch( exchange )
	{
		case kPGPike_EX_Base:			etStr = "Base";				break;
		case kPGPike_EX_Identity:		etStr = "Identity";			break;
		case kPGPike_EX_Authentication:	etStr = "Auth";				break;
		case kPGPike_EX_Aggressive:		etStr = "Aggressive";		break;
		case kPGPike_EX_Informational:	etStr = "Informational";	break;
		case kPGPike_EX_IPSEC_Quick:	etStr = "Quick";			break;
		case kPGPike_EX_IPSEC_NewGroup:	etStr = "NewGroup";			break;
	}
	return etStr;
}

	char *
pgpIKEIDTypeString(
	PGPipsecIdentity		id )
{
	char *					idStr = "unknown";
	
	switch( id )
	{
		case kPGPike_ID_IPV4_Addr:			idStr = "IPv4 Addr";	break;
		case kPGPike_ID_FQDN:				idStr = "FQDN";			break;
		case kPGPike_ID_UserFDQN:			idStr = "UserFQDN";		break;
		case kPGPike_ID_IPV4_Addr_Subnet:	idStr = "IPv4 Subnet";	break;
		case kPGPike_ID_IPV6_Addr:			idStr = "IPv6 Addr";	break;
		case kPGPike_ID_IPV6_Addr_Subnet:	idStr = "IPv6 Subnet";	break;
		case kPGPike_ID_IPV4_Addr_Range:	idStr = "IPv4 Range";	break;
		case kPGPike_ID_IPV6_Addr_Range:	idStr = "IPv6 Range";	break;
		case kPGPike_ID_DER_ASN1_DN:		idStr = "DER DN";		break;
		case kPGPike_ID_DER_ASN1_GN:		idStr = "DER GN";		break;
		case kPGPike_ID_Key_ID:				idStr = "KeyID";		break;
	}
	return idStr;
}

	char *
pgpIKEipsecCipherTypeString(
	PGPipsecESPTransformID	cipher )
{
	char *					cipherStr = "unknown";
	
	switch( cipher )
	{
		case kPGPike_ET_DES_IV64:			cipherStr = "DES-IV64";		break;
		case kPGPike_ET_DES:				cipherStr = "DES";			break;
		case kPGPike_ET_3DES:				cipherStr = "3DES";			break;
		case kPGPike_ET_RC5:				cipherStr = "RC5";			break;
		case kPGPike_ET_IDEA:				cipherStr = "IDEA";			break;
		case kPGPike_ET_CAST:				cipherStr = "CAST";			break;
		case kPGPike_ET_Blowfish:			cipherStr = "Blowfish";		break;
		case kPGPike_ET_3IDEA:				cipherStr = "3IDEA";		break;
		case kPGPike_ET_DES_IV32:			cipherStr = "DES-IV32";		break;
		case kPGPike_ET_RC4:				cipherStr = "RC4";			break;
		case kPGPike_ET_NULL:				cipherStr = "NULL";			break;
	}
	return cipherStr;
}

	char *
pgpIKEipsecAuthTypeString(
	PGPipsecAuthAttribute	auth )
{
	char *					authStr = "unknown";
	
	switch( auth )
	{
		case kPGPike_AA_None:				authStr = "None";			break;
		case kPGPike_AA_HMAC_MD5:			authStr = "MD5-HMAC";		break;
		case kPGPike_AA_HMAC_SHA:			authStr = "SHA-HMAC";		break;
		case kPGPike_AA_DES_MAC:			authStr = "DES-MAC";		break;
		case kPGPike_AA_KPDK:				authStr = "KPDK";			break;
	}
	return authStr;
}

	char *
pgpIKEHashTypeString(
	PGPikeHash				hash )
{
	char *					hashStr = "unknown";
	
	switch( hash )
	{
		case kPGPike_HA_MD5:				hashStr	= "MD5";		break;
		case kPGPike_HA_SHA1:				hashStr	= "SHA";		break;
		case kPGPike_HA_Tiger:				hashStr	= "Tiger";		break;
	}
	return hashStr;
}

	char *
pgpIKEGroupTypeString(
	PGPikeGroupID			group )
{
	char *					groupStr = "unknown";
	
	switch( group )
	{
		case kPGPike_GR_MODPOne:			groupStr = "modP-768";	break;
		case kPGPike_GR_MODPTwo:			groupStr = "modP-1024";	break;
		case kPGPike_GR_MODPFive:			groupStr = "modP-1536";	break;
	}
	return groupStr;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/