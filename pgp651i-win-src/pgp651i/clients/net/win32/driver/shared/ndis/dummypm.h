#ifndef _H_DUMMYPM_
#define _H_DUMMYPM_

#include <ndis.h>

typedef struct tagDummySA
{
	ULONG			ipAddress;
	UCHAR			outgoingKey;
	UCHAR			incomingKey;
	BOOLEAN			validSA;
	BOOLEAN			pendingSA;
} DummySA, *PDummySA;

typedef enum tagDUMMY_EVENT_TYPE {
	NewSARequest = 0
} DUMMY_EVENT_TYPE;

typedef struct tagDummyNewSARequest {
	DUMMY_EVENT_TYPE eventType;
	ULONG			 ipAddress;
} DummyNewSARequest, *PDummyNewSARequest;

void PMAddDummySA(struct _PGPnetPMContext *pContext,
			 PDummySA sa);

void PMAddDummyHost(struct _PGPnetPMContext *pContext,
			 ULONG *ipAddress);

#endif