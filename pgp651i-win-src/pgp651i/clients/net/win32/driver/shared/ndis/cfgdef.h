/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	$Id: cfgdef.h,v 1.14 1999/03/30 23:20:30 elowe Exp $
____________________________________________________________________________*/

#ifndef Included_cfgdef_h
#define Included_cfgdef_h

#ifndef NDIS50
#define	OID_GEN_MEDIA_CAPABILITIES				0x00010201
#endif

/*
 Win32 IOCTL OID. Safe at least until next millenium.
 */

#define OID_PGP_OPERATION_MANDATORY				0xFF010100
#define OID_PGP_WIN32_IOCTL						0xFF010101

#define OID_PGP_OPERATION_OPTIONAL				0xFF010200
#define OID_PGP_EVENT_CREATE					0xFF010201
#define OID_PGP_EVENT_DESTROY					0xFF010202
#define OID_PGP_SHARED_MEM_ALLOC				0xFF010203
#define OID_PGP_SHARED_MEM_FREE					0xFF010204
#define OID_PGP_NEWCONFIG						0xFF010205
#define OID_PGP_NEWHOST							0xFF010206
#define OID_PGP_NEWSA							0xFF010207
#define OID_PGP_SAFAILED						0xFF010208
#define OID_PGP_SADIED							0xFF010209
#define OID_PGP_SAUPDATE						0xFF010210
#define OID_PGP_ALLHOSTS						0xFF010211
#define OID_PGP_SHUTDOWN						0xFF010212
#define OID_PGP_LOCALIP					        0xFF010213

#define OID_PGP_QUEUE_INITIALIZE				0xFF010220
#define OID_PGP_QUEUE_RELEASE					0xFF010221

#define OID_PGP_NEW_DUMMY_HOST					0xFF0102E0
#define OID_PGP_NEW_DUMMY_SA					0xFF0102E1

#define OID_PGP_EVENT_SET						0xFF0102FF

#define OID_PGP_STATISTICS_MANDATORY			0xFF020100

#define OID_PGP_STATISTICS_OPTIONAL				0xFF020200
#define OID_PGP_GET_SEND_PACKETS				0xFF020201
#define OID_PGP_GET_RECEIVE_PACKETS				0xFF020202

#pragma pack(push, 8)
typedef struct _PGPEVENT_CONTEXT
{
	UCHAR           EventName[128];     // Event name, the only connection btw ring0/ring3

    HANDLE          Win32EventHandle;	// Ring3 copy of the event handle

    HANDLE          DriverEventHandle;  // Ring0 copy of the event handle

    PVOID           DriverEventObject;  // The event object

} PGPEVENT_CONTEXT, *PPGPEVENT_CONTEXT;
#pragma pack(pop)

#endif	// _Included_cfgdef_h