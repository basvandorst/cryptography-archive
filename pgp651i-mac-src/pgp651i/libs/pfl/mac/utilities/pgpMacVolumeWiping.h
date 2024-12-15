/*____________________________________________________________________________	Copyright (C) 1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: pgpMacVolumeWiping.h,v 1.7 1999/03/10 02:53:02 heller Exp $____________________________________________________________________________*/#ifndef Included_pgpMacVolumeWiping_h	/* [ */#define Included_pgpMacVolumeWiping_h#include "pgpMacWiping.h"enum{	kWipingError_InvalidVolumeDataStructure	= 13456789,	kWipingError_UnsupportedVolumeFormat};enum VolumeWipeEventType_{	kVolumeWipeEvent_Begin	= 1,	kVolumeWipeEvent_End,	kVolumeWipeEvent_BeginPass,	kVolumeWipeEvent_EndPass,	kVolumeWipeEvent_GatheringInfo,	kVolumeWipeEvent_WipeProgress,		kVolumeWipeEvent_VolumeUnmountFailure,	kVolumeWipeEvent_VolumeMountFailure,	PGP_ENUM_FORCE( VolumeWipeEventType_ )};PGPENUM_TYPEDEF( VolumeWipeEventType_, VolumeWipeEventType );typedef struct VolumeWipeEvent{	VolumeWipeEventType	type;		union	{		struct		{			OSStatus	wipeError;					} end;		struct		{			short		vRefNum;			OSStatus	unmountError;					} unmountFailure;				struct		{			short		driveNumber;			short		driverRefNum;			OSStatus	mountError;				} mountFailure;				struct		{			PGPUInt32	passIndex;			PGPUInt32	totalPasses;			PGPUInt32	totalSectorsToWipe;				} beginPass;				struct		{			PGPUInt32	passIndex;			PGPUInt32	totalPasses;			OSStatus	passError;					} endPass;				struct		{			PGPUInt32	passIndex;			PGPUInt32	numSectorsWiped;			PGPUInt32	totalSectorsToWipe;				} progress;	};} VolumeWipeEvent;typedef OSStatus	(*PGPVolumeWipingEventHandler)(							const VolumeWipeEvent *event,							PGPUserValue userValue);PGP_BEGIN_C_DECLARATIONS#if PRAGMA_IMPORT_SUPPORTED#pragma import on#endifOSStatus	pgpCanWipeVolumeFreeSpace(short volRefNum);OSStatus	pgpWipeVolumeFreeSpace(short volRefNum, ConstStr255Param volName,					PGPUInt32 numPasses, PGPUInt32 numPatterns,					const PGPWipingPattern patternList[],					PGPVolumeWipingEventHandler	eventHandler,					PGPUserValue eventUserValue );#if PRAGMA_IMPORT_SUPPORTED#pragma import reset#endifPGP_END_C_DECLARATIONS#endif /* ] Included_pgpMacVolumeWiping_h *//*__Editor_settings____	Local Variables:	tab-width: 4	End:	vi: ts=4 sw=4	vim: si_____________________*/