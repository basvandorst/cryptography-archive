/*
 * Copyright (c) 1998 Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: main.c,v 1.8 1999/01/04 18:44:27 pbj Exp $
 */
#include <stdio.h>
#include <windows.h>

#include "pgpNetPMHost.h"
#include "pgpNetPMSA.h"
#include "pgpNetPMConfig.h"
#include "pgpNetConfig.h"
#include "pgpNetKernel.h"
#include "pgpEndianConversion.h"

PGPContextRef g_Context;

PGPByte originalPacket[] = {
	0x46,0x00,0x00,0x00,0x00,0xFC,0x01,0x00,0x00,0x55,0x13,0xCF,0x87,0xFF,
	0xE1,0x45,0x03,0xFF,0xE1,0x45,0x01,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12
};

int
main(int argc, char *argv)
{
	PGPnetKernelHost *result = 0;
	PGPError err;
	PGPInt32 i = 0;
	PGPikeSA ikeSA[100];
	PGPikeSA *sa = 0, ikeSATemplate;
	PGPnetKernelSA *transportSA = 0;
	PGPnetKernelSA *tunnelSA = 0;
	PGPnetKernelHost *kernelHost = 0;
	PGPnetKernelHost *kernelGateway = 0;
	PGPUInt32 spi = 0;
	PGPByte encryptedPacket[32768];
	PGPByte decryptedPacket[32768];
	PGPnetPacket netPacketSrc, netPacketDest;
	PGPikeDOIParams doiParamsTemplate;
	PNCONFIG pnConfig;
	PGPnetKernelConfig tmpConfig;
	PGPnetPMContext *pContext;

    // Initialize the sdk library
    err = PGPNewContext(kPGPsdkAPIVersion, &g_Context);
    if (IsPGPError(err)) {
        if (err == kPGPError_FeatureNotAvailable) {
			fprintf(stderr, "Fatal Error: This version of the PGP SDK has expired");
            g_Context = 0;
            goto end;
        } else {
            fprintf(stderr, "Fatal Error: Couldn't get a context from the PGP SDK");
            g_Context = 0;
            goto end;
        }
    }

    // read in the PGPnet configuration
    memset(&pnConfig, '\0', sizeof(pnConfig));
    err = PGPnetLoadConfiguration(g_Context, &pnConfig);
    if (IsPGPError(err)) {
        fprintf(stderr, "Fatal Error: Unable to load configuration");
        goto end;
    }

///>>> 04jan99 pbj
	tmpConfig.bPGPnetEnabled = pnConfig.bPGPnetEnabled;
///<<<
	tmpConfig.bAllowUnconfigHost = pnConfig.bAllowUnconfigHost;
	tmpConfig.bAttemptUnconfigHost = pnConfig.bAttemptUnconfigHost;

	pContext = malloc(sizeof(PGPnetPMContext));
	memset(pContext, '\0', sizeof(PGPnetPMContext));

	(void) PMAddConfig(pContext, &tmpConfig);

	for (i = 0; i < pnConfig.uHostCount; i++) {
		result = PMAddHost(pContext, &(pnConfig.pHostList[i]));
	}

	for (i = pnConfig.uHostCount-1; i >= 0; i--) {
		result = PMFindHost(pContext, PGPStorageToUInt32(pnConfig.pHostList[i].ipAddress));
	}

	/*
	typedef struct PGPikeSA
	{
		PGPUInt32				ipAddress;
		PGPUInt32				kbLifeTime;	
		PGPUInt32				secLifeTime;
		PGPTime					birthTime;
		
		PGPikeDOI				doi;		

		PGPUInt16				numTransforms;
		PGPikeDOIParams			transform[kPGPike_MaxTransforms];
	} PGPikeSA;

	typedef struct PGPipsecESPTransform
	{
		PGPipsecESPTransformID		cipher;
		PGPipsecAuthAttribute		authAttr;
		PGPipsecEncapsulation		mode;
	} PGPipsecESPTransform;

	typedef struct PGPipsecDOIParams
	{
		PGPipsecSPI             inSPI;
		PGPipsecSPI             outSPI;
		PGPipsecProtocol        protocol;

		union
		{
	        struct
			{
	            PGPipsecAHTransform     t;
				PGPByte                 inAuthKey[kPGPike_AuthMaximumKeySize];
				PGPByte                 outAuthKey[kPGPike_AuthMaximumKeySize];
			} ah;

			struct
			{
	            PGPipsecESPTransform    t;
				PGPByte                 inESPKey[kPGPike_ESPMaximumKeySize];
				PGPByte                 outESPKey[kPGPike_ESPMaximumKeySize];
				PGPByte                 inAuthKey[kPGPike_AuthMaximumKeySize];
				PGPByte                 outAuthKey[kPGPike_AuthMaximumKeySize];
			} esp;

			struct
			{
	            PGPipsecIPCOMPTransform t;
			} ipcomp;
		} u;
	} PGPipsecDOIParams;

	*/

	memmove(&(doiParamsTemplate.u.ipsec.u.esp.inESPKey),
		"This is an ESP Key", 
		kPGPike_ESPMaximumKeySize);
	memmove(&(doiParamsTemplate.u.ipsec.u.esp.outESPKey),
		"This is an ESP Key", 
		kPGPike_ESPMaximumKeySize);

	memmove(&(doiParamsTemplate.u.ipsec.u.esp.inAuthKey),
		"This is an Auth Key", 
		kPGPike_ESPMaximumKeySize);
	memmove(&(doiParamsTemplate.u.ipsec.u.esp.outAuthKey), 
		"This is an Auth Key", 
		kPGPike_AuthMaximumKeySize);

	doiParamsTemplate.u.ipsec.u.esp.t.cipher = kPGPike_ET_CAST;
	doiParamsTemplate.u.ipsec.u.esp.t.authAttr = kPGPike_AA_HMAC_SHA;
	doiParamsTemplate.u.ipsec.u.esp.t.mode = kPGPike_PM_Transport;
	doiParamsTemplate.u.ipsec.protocol = kPGPike_PR_ESP;

	ikeSATemplate.ipAddress = 0;
	ikeSATemplate.kbLifeTime = 1024;
	ikeSATemplate.secLifeTime = 0;
	ikeSATemplate.birthTime = PGPGetTime();
	ikeSATemplate.doi = kPGPike_DOI_IPSEC;
	ikeSATemplate.numTransforms = 1;

	memmove(&(ikeSATemplate.transform[0]), &doiParamsTemplate, sizeof(PGPikeDOIParams));

	for (i = 0; i < pnConfig.uHostCount; i++) {
		sa = &ikeSA[i];

		memset(sa, '\0', sizeof(sa));
		memmove(sa, &ikeSATemplate, sizeof(PGPikeSA));

		sa->ipAddress = PGPStorageToUInt32(pnConfig.pHostList[i].ipAddress);
		PGPUInt32ToStorage(spi++, sa->transform[0].u.ipsec.inSPI);
		PGPUInt32ToStorage(spi++, sa->transform[0].u.ipsec.outSPI);

		PMAddSA(pContext, (unsigned char*)sa);
	}

	for (i = pnConfig.uHostCount-1; i >= 0; i--) {
		transportSA = NULL;
		tunnelSA = NULL;
		kernelHost = NULL;
		kernelGateway = NULL;

		kernelHost = PMFindHost(pContext, PGPStorageToUInt32(pnConfig.pHostList[i].ipAddress));
		if (kernelHost->childOf >= 0)
			kernelGateway = PMFindHost(pContext, PMParentHostIP(pContext, kernelHost));

		if ((kernelHost->hostType == kPGPnetInsecureHost) ||
			(kernelHost->hostType == kPGPnetSecureGateway))
			kernelHost = NULL;

		if (kernelHost != NULL)
			transportSA = PMFindSA(pContext, kernelHost->ipAddress);

		if (kernelGateway != NULL)
			tunnelSA = PMFindSA(pContext, kernelGateway->ipAddress);

		netPacketSrc.allocatedSize = sizeof(originalPacket);
		netPacketSrc.actualSize = sizeof(originalPacket);
		netPacketSrc.data = originalPacket;

		netPacketDest.allocatedSize = 32768;
		netPacketDest.actualSize = 0;
		netPacketDest.data = encryptedPacket;

		err = PGPnetIPsecApplySA(&netPacketSrc, &netPacketDest, transportSA, tunnelSA, FALSE);
		if (IsPGPError(err)) {
			fprintf(stderr, "Error applying SA to outgoing packet: %d\n", err);
			goto end;
		}

		netPacketSrc.allocatedSize = 32768;
		netPacketSrc.actualSize = netPacketDest.actualSize;
		netPacketSrc.data = encryptedPacket;

		netPacketDest.allocatedSize = 32768;
		netPacketDest.actualSize = 0;
		netPacketDest.data = decryptedPacket;

		err = PGPnetIPsecApplySA(&netPacketSrc, &netPacketDest, transportSA, tunnelSA, TRUE);
		if (IsPGPError(err)) {
			fprintf(stderr, "Error applying SA to incoming packet: %d\n", err);
			goto end;
		}
	}

end:
	free(pContext);
	return 0;
}