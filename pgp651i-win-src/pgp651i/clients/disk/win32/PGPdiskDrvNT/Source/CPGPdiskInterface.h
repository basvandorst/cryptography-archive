//////////////////////////////////////////////////////////////////////////////
// CPGPdiskInterface.h
//
// Declaration of class CPGPdiskInterface.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskInterface.h,v 1.3 1998/12/14 18:59:29 nryan Exp $

#ifndef Included_CPGPdiskInterface_h	// [
#define Included_CPGPdiskInterface_h

#include "DualErr.h"
#include "Packets.h"
#include "PGPdiskContainer.h"

#include "CPGPdiskDriver.h"
#include "CPGPdiskInterfaceHooks.h"
#include "LookasideList.h"
#include "UserMemoryManager.h"


////////
// Types
////////

class PGPdiskWorkerThread;


//////////////////////////
// Class CPGPdiskInterface
//////////////////////////

class CPGPdiskInterface : public KDevice
{
public:
	DualErr				mInitErr;

	PGPUInt32			mPGPdiskAppVersion;		// application version
	PGPUInt32			mSecondsInactive;		// seconds user inactive

	CPGPdiskDriver		*mPGPdiskDriver;		// pointer to driver
	PGPUInt32			mCallerProcId;			// caller process ID

	PGPdiskContainer	mPGPdisks;				// holds PGPdisk objects

	// From CPGPdiskInterface.cpp

	CPGPdiskInterface(CPGPdiskDriver *pDriver);
	~CPGPdiskInterface();

	DEVMEMBER_DISPATCHERS

	// From CPGPdiskInterfaceErrors.cpp

	void	ReportError(PGDMajorError perr, DualErr derr = DualErr::NoError, 
				PGPUInt8 drive = kInvalidDrive);

	// From CPGPdiskInterfaceVolumes.cpp

	DualErr		MountPGPdisk(LPCSTR path, CipherContext *pContext, 
					PGPUInt8 drive, PGPBoolean mountReadOnly);
	DualErr		UnmountPGPdisk(PGPUInt8 drive, 
					PGPBoolean isThisEmergency = FALSE);
	DualErr		UnmountAllPGPdisks(PGPBoolean isThisEmergency = FALSE);

private:
	PGPBoolean				mTimerStarted;		// unmount timer started?
	PGPBoolean				mUnmountAllMode;	// unmount all per sec

	PGPBoolean				mAutoUnmount;		// auto-unmount enabled?
	PGPUInt32				mUnmountTimeout;	// unmount timeout in mins

	KeyboardFilterDevice	*mKeyboardFilter;	// mouse interceptor
	MouseFilterDevice		*mMouseFilter;		// keyboard interceptor

	PGPBoolean				mCreatedKeyboardFilter;	// made mouse filter?
	PGPBoolean				mCreatedMouseFilter;	// made keyboard filter?

	PGPdiskWorkerThread		mUtilityThread;		// thread for mounts/unmounts
	UserMemoryManager		mUserMemManager;	// manages user memory

	SAFE_DESTRUCTORS

	// From CPGPdiskInterface.cpp

	static VOID	EverySecondCallback(PDEVICE_OBJECT DeviceObject, 
					PVOID Context);
	static void	EverySecondCallbackAux(PGPUInt32 refData);

	// From CPGPdiskInterfaceComm.cpp

	static void	ProcessMountCallback(PGPUInt32 refData);
	DualErr		ProcessMount(PAD_Mount pMNT, 
					PGPUInt32 size);

	static void	ProcessUnmountCallback(PGPUInt32 refData);
	DualErr		ProcessUnmount(PAD_Unmount pUNMNT, 
					PGPUInt32 size);

	DualErr		ProcessQueryVersion(PAD_QueryVersion pQV, 
					PGPUInt32 size);

	DualErr		ProcessQueryMounted(PAD_QueryMounted pQM, 
					PGPUInt32 size);

	static void	ProcessQueryOpenFilesCallback(PGPUInt32 refData);

	DualErr		ProcessQueryOpenFiles(PAD_QueryOpenFiles pQOF, 
					PGPUInt32 size);

	DualErr		ProcessChangePrefs(PAD_ChangePrefs pCP, 
					PGPUInt32 size);

	DualErr		ProcessLockUnlockMem(PAD_LockUnlockMem pLUM, 
					PGPUInt32 size);

	DualErr		ProcessGetPGPdiskInfo(PAD_GetPGPdiskInfo pGPI, 
					PGPUInt32 size);

	static void	ProcessLockUnlockVolCallback(PGPUInt32 refData);
	DualErr		ProcessLockUnlockVol(PAD_LockUnlockVol pLUV, 
					PGPUInt32 size);

	static void	ProcessReadWriteVolCallback(PGPUInt32 refData);
	DualErr		ProcessReadWriteVol(PAD_ReadWriteVol pRWV, 
					PGPUInt32 size);

	static void	ProcessQueryVolInfoCallback(PGPUInt32 refData);
	DualErr		ProcessQueryVolInfo(PAD_QueryVolInfo pQVI, 
					PGPUInt32 size);

	DualErr		ProcessNotifyUserLogoff(PAD_NotifyUserLogoff pNUL, 
					PGPUInt32 size);

	NTSTATUS	ProcessADPacket(PADPacketHeader pPacket, PGPUInt32 size);

	// From CPGPdiskInterfaceErrors.cpp

	PGPBoolean	DoesDriverErrTakeDriveLetter(PGDMajorError perr);
	LPCWSTR		GetErrorString(PGDMajorError perr);

	// From CPGPdiskInterfaceHooks.cpp

	DualErr		SetupSystemHooks();
	void		DeleteSystemHooks();
};

#endif // Included_CPGPdiskInterface_h
