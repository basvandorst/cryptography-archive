//////////////////////////////////////////////////////////////////////////////
// CPGPdiskDrv.h
//
// Declaration of class CPGPdiskDrv.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskDrv.h,v 1.18 1998/12/14 18:59:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskDrv_h	// [
#define Included_CPGPdiskDrv_h

#include "DualErr.h"
#include "Packets.h"
#include "PGPdiskContainer.h"
#include "SimpleQueue.h"

#include "CPGPdiskDrvComm.h"
#include "CPGPdiskDrvDebug.h"
#include "CPGPdiskDrvErrors.h"
#include "CPGPdiskDrvHooks.h"
#include "CPGPdiskDrvWinutils.h"
#include "DcbManager.h"
#include "IopProcessor.h"
#include "PGPdisk.h"
#include "Wrappers.h"


////////////
// Constants
////////////

// VToolsD-required device driver constants.

#define	DEVICE_CLASS		CPGPdiskDrv
#define	PGPDISK_DeviceID	UNDEFINED_DEVICE_ID	
#define	PGPDISK_Init_Order	UNDEFINED_INIT_ORDER
#define	PGPDISK_Major		1
#define	PGPDISK_Minor		0
#define	PGPDISK_NAME		"PGPDISK        "
#define	PGPDISK_REV			1
#define	PGPDISK_FEATURE		0
#define	PGPDISK_IFR			0


//////////
// Classes
//////////

// There is only one CPGPdiskDrv object, and it represents the driver itself.

class CPGPdiskDrv : public VDevice
{
public:
	DualErr				mInitErr;

	PDDB				mTheDDB;			// the driver's DDB
	PGPUInt8			mLoadGroupNum;		// current load group number

	DcbManager			mDcbs;				// container for driver's DCBs
	IopProcessor		mIopProcessor;		// processes IOPs
	PGPdiskContainer	mPGPdisks;			// container for PGPdisks

	PGPUInt32			mSecondsInactive;	// # seconds PC inactive

	// From CPGPdiskDrvDebug.cpp

	LPCSTR				GetName(NameAssoc nameTable[], PGPUInt32 n, 
							PGPUInt32 func);
	LPCSTR				GetADPacketName(PGPUInt32 code);
	LPCSTR				GetAEPFunctionName(PGPUInt32 func);
	LPCSTR				GetIORFunctionName(PGPUInt32 func);

	// From CPGPdiskDrvIos.cpp

	static VOID __cdecl	PGPDISK_Aer(PAEP pAep);
	static VOID __cdecl	PGPDISK_RequestHandlerStub(PIOP pIop);

	// From CPGPdiskDrvErrors.cpp

	void				ReportError(PGDMajorError perr, 
							DualErr derr = DualErr::NoError, 
							PGPUInt8 drive = kInvalidDrive);

	// From CPGPdiskDrvVolumes.cpp

	DualErr				MountPGPdisk(LPCSTR path, CipherContext *context, 
							PGPUInt8 drive = kInvalidDrive, 
							PGPBoolean mountReadOnly = FALSE);
	DualErr				UnmountPGPdisk(PGPUInt8 drive, 
							PGPBoolean isThisEmergency = FALSE);
	DualErr				UnmountAllPGPdisks(
							PGPBoolean isThisEmergency = FALSE);
	void				UnmountAllPGPdisksSafely(
							PGPBoolean warnIfUnmountsFail = FALSE);

	// From CPGPdiskDrvWinutils.cpp

	PGPBoolean			DoWinInt(PGPUInt8 interrupt, ALLREGS *pAllRegs);

	LockLevel			GetLockLevel(PGPUInt8 drive);
	DualErr				AcquireLogicalVolLock(PGPUInt8 drive, 
							LockLevel lock, PGPUInt8 permissions = 0x01);
	DualErr				GetFormatLockOnDrive(PGPUInt8 drive);
	DualErr				ReleaseFormatLockOnDrive(PGPUInt8 drive);

	PGPBoolean			IsFileInUseByReader(LPCSTR path);
	PGPBoolean			IsFileInUseByWriter(LPCSTR path);
	PGPBoolean			IsFileInUse(LPCSTR path);
	PGPBoolean			IsFileValid(LPCSTR path);

	DualErr				GetFirstClustFile(LPCSTR path, PGPUInt32 *firstClust);
	DualErr				HasOpenFiles(PGPUInt8 drive, 
							PGPBoolean *hasOpenFiles);

	DualErr				Int21OpenFile(LPCSTR path, PGPUInt16 mode, 
							PGPUInt16 attribs, PGPUInt16 action, 
							PGPUInt16 *pHandle);
	DualErr				Int21CloseFile(PGPUInt16 handle);

	DualErr				Int21GetFileLength(PGPUInt16 handle, 
							PGPUInt32 *pLength);
	DualErr				Int21SetFilePos(PGPUInt16 handle, PGPUInt32 pos);

	DualErr				Int21ReadFileAux(PGPUInt16 handle, PGPUInt8 *buf, 
							PGPUInt16 nBytes);
	DualErr				Int21WriteFileAux(PGPUInt16 handle, PGPUInt8 *buf, 
							PGPUInt16 nBytes);

	DualErr				Int21ReadFile(PGPUInt16 handle, PGPUInt8 *buf, 
							PGPUInt32 pos, PGPUInt32 nBytes);
	DualErr				Int21WriteFile(PGPUInt16 handle, PGPUInt8 *buf, 
							PGPUInt32 pos, PGPUInt32 nBytes);

	DualErr				LockUserBuffer(void *pMem, PGPUInt32 nBytes);
	DualErr				UnlockUserBuffer(void *pMem, PGPUInt32 nBytes);

private:
	PGPUInt32				mPGPdiskAppVersion;		// application version

	DeviceService_THUNK		mKeyboard_Thunk;		// keyboard hook thunk
	DeviceService_THUNK		mMouse_Thunk;			// mouse hook thunk

	PGPBoolean				mIsEjectHooked[kMaxDrives];		// hooked whom?
	SYSBHOOK_HANDLE			mBroadcastHookHandle;	// handle for msg hook

	PGPBoolean				mUnmountAllMode;		// unmount all per sec

	PGPBoolean				mHookedVKD;				// hooked VKD?
	PGPBoolean				mHookedVMD;				// hooked VMD?
	PGPBoolean				mHookedSystemBroadcast;	// hooked sysbroadcast?

	PGPBoolean				mLockInProgress;		// next lock is ours
	PGPUInt8				mDriveBeingLocked;		// drive we're locking

	PGPBoolean				mAutoUnmount;			// auto-unmount enabled?
	PGPUInt32				mUnmountTimeout;		// unmount timeout in mins

	ErrorCell				mErrPackets[kNumErrPackets];	// error packets
	PGPBoolean				mIsErrorCallbackBusy;	// callback busy?
	SimpleQueue				mErrorQueue;			// queue for errors

	// From CPGPdiskDrv.cpp

	void					ClearVariables();

	BOOL					OnSysDynamicDeviceInit();
	BOOL					OnInitComplete(VMHANDLE hVM, PCHAR CommandTail);
	BOOL					OnSysDynamicDeviceExit();
	VOID					OnSystemExit(VMHANDLE hVM);

	// From CPGPdiskDrvComm.cpp

	DualErr					ProcessMount(PAD_Mount pMNT, 
								PGPUInt32 size);
	DualErr					ProcessUnmount(PAD_Unmount pUNMNT, 
								PGPUInt32 size);
	DualErr					ProcessQueryVersion(PAD_QueryVersion pQV, 
								PGPUInt32 size);
	DualErr					ProcessQueryMounted(PAD_QueryMounted pQM, 
								PGPUInt32 size);
	DualErr					ProcessQueryOpenFiles(PAD_QueryOpenFiles pQOF, 
								PGPUInt32 size);
	DualErr					ProcessChangePrefs(PAD_ChangePrefs pCP, 
								PGPUInt32 size);
	DualErr					ProcessLockUnlockMem(PAD_LockUnlockMem pLUM, 
								PGPUInt32 size);
	DualErr					ProcessGetPGPdiskInfo(PAD_GetPGPdiskInfo pGPI, 
								PGPUInt32 size);
	DualErr					ProcessLockUnlockVol(PAD_LockUnlockVol pLUV, 
								PGPUInt32 size);
	DualErr					ProcessReadWriteVol(PAD_ReadWriteVol pRWV, 
								PGPUInt32 size);
	DualErr					ProcessQueryVolInfo(PAD_QueryVolInfo pQVI, 
								PGPUInt32 size);

	DualErr					ProcessNotifyUserLogoff(
								PAD_NotifyUserLogoff pNUL, PGPUInt32 size);

	PGPUInt32				ProcessADPacket(PADPacketHeader pPacket, 
								PGPUInt32 size);
	virtual DWORD			OnW32DeviceIoControl(PIOCTLPARAMS p);

	// From CPGPdiskDrvErrors.cpp

	static VOID __stdcall	ReportErrorEndCallback(DWORD ResponseCode, 
								PVOID Refdata);
	static VOID __stdcall	ReportErrorStartCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);

	void					ScheduleErrorCallback();

	// From CPGPdiskDrvHooks.cpp

	static  int _cdecl		FilesystemHook(pIFSFunc pfn, int func, int drive, 
								int resType, int codePage, pioreq pir);

	static BOOL __cdecl		BroadcastMessageHook(DWORD uMsg, DWORD wParam, 
								DWORD lParam, DWORD dwRef);

	void					ScheduleTryToUnmountCallback(
								PGPBoolean delayExecution = FALSE);

	static VOID __stdcall	KeepTryingToUnmountCallbackStub(VMHANDLE hVM, 
								PCLIENT_STRUCT pcrs, PVOID RefData, 
								DWORD extra);

	static VOID __stdcall	KeepTryingToUnmountCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);

	static VOID __stdcall	KeyboardHook(PDSFRAME regs);
	static VOID __stdcall	MouseHook(PDSFRAME regs);

	DualErr					HookEjectionFilter(PGPUInt8 drive);
	static VOID __cdecl		PGPDISK_EjectHandlerStub(PIOP pIop);
	static VOID				PGPDISK_EjectHandler(PIOP pIop);
	static VOID __stdcall	SetupSystemHooksCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
	void					SetupSystemHooks();
	void					RemoveSystemHooks();

	// From CPGPdiskDrvIos.cpp

	PGPUInt16				AepInitialize(PAEP_bi_init pAep);
	PGPUInt16				AepDeviceInquiry(PAEP_inquiry_device pAEP);
	PGPUInt16				AepIopTimeout(PAEP_iop_timeout_occurred pAep);
	static VOID __stdcall	AepOneSecondCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
	PGPUInt16				AepOneSecond();

	static void				PGPDISK_RequestHandler(PIOP pIop);

	// From CPGPdiskDrvVolumes.cpp

	static VOID __stdcall	UnmountAllPGPdisksCallback(VMHANDLE hVM, 
								THREADHANDLE hThread, PVOID Refdata, 
								PCLIENT_STRUCT pRegs);
};


/////////
// Macros
/////////

// It was necessary to rewrite some of the VToolsD included macros, which
// wouldn't work properly with port drivers.

// Declare_Virtual_Device_Ex is a utility function for Declare_Port_Driver.

#define Declare_Virtual_Device_Ex(devName, RefData) extern "C" VDevice* \
	__cdecl _CreateDevice(); \
	extern "C" VOID __stdcall v86_api_entry();	              \
	extern "C" VOID __stdcall pm_api_entry();		      \
	extern "C" VOID __stdcall localControlDispatcher();	      \
	extern "C" DDB The_DDB;					      \
	extern "C" void (*VXD_SERVICE_TABLE[])(); 		      \
	DDB The_DDB = {		      	      			      \
		0,						      \
		DDK_VERSION,					      \
		devName##_DeviceID,				      \
		devName##_Major,				      \
		devName##_Minor,				      \
		0,						      \
		{' ',' ',' ',' ',' ',' ',' ',' '},		      \
		devName##_Init_Order,				      \
		(PGPUInt32)localControlDispatcher,			      \
		(PGPUInt32)v86_api_entry,	      			      \
		(PGPUInt32)pm_api_entry,				      \
		0,0,RefData,		 	 			      \
		(PGPUInt32)VXD_SERVICE_TABLE,			      \
		0,					      	      \
		0,					      	      \
		__SIG__,				      	      \
	};							      \
	VDevice* __cdecl _CreateDevice()			      \
	{							      \
		return (VDevice*)new DEVICE_CLASS;		      \
	}

// Declare_Port_Driver defines many important system-required fields for the
// driver.

#define Declare_Port_Driver(VName, _lgn, _asc, _rev, _fc, _ifr, _bt, _rd) \
	VOID __cdecl VName##_Aer(AEP*);	\
	ILB VName##_Ilb;	\
	DRP VName##_Drp={   \
		{'X','X','X','X','X','X','X','X'}, \
		_lgn, \
		Driver->VName##_Aer, \
		&VName##_Ilb, \
		_asc, \
		_rev, \
		_fc,  \
		_ifr, \
		_bt,  \
		0,    \
		_rd}; \
	Declare_Virtual_Device_Ex(VName, (PGPUInt32)&VName##_Drp)

#endif	// ] Included_CPGPdiskDrv_h
