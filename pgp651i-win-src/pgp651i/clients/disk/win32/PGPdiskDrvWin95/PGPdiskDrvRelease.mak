# PGPdiskDrvRelease.mak - makefile for port driver PGPdisk

DEVICENAME		= PGPDISK
DEVICETYPE		= PORTDRIVER
DYNAMIC			= 1
FRAMEWORK		= CPP
USER_COMMENT	= "PGPdisk Volume Driver"

DEBUG			= 0

_MSC_VER		= 6000
SOURCEPATH		= ../Source

USER_LIB		= $(BASEDIR)\lib\i386\checked\int64.lib

PGPDIRS			= /I "..\..\..\..\..\libs\pfl\common" \
					/I "..\..\..\..\..\libs\pfl\win32" /I "..\FakeSource" \
					/I "..\..\Shared" /I "..\..\Encryption"

COPTFLAGS	= /O2
OBJPATH		= ../Release/

XFLAGS = $(PGPDIRS) /W3 /WX /Fp"PGPdiskDrvNT.pch" /YX"vtoolscp.h" \
			/DUCHAR_MAX=0xFF /DUSHRT_MAX=0xFFFF /DULONG_MAX=0xFFFFFFFF \
			/DINT_MAX=0xFFFFFFFF /D_CRT_ASSERT=0 /D_MSC_VER=5000 \
			/DPGP_DEBUG=0 /DPGPDISK_95DRIVER /DPGP_INTEL /DPGP_WIN32 \
			/DPGPDISK_NOSDK

OBJECTS =	$(OBJPATH)pgpDebug.OBJ \
			$(OBJPATH)pgpLeaks.OBJ \
			$(OBJPATH)pgpMem.OBJ \
			$(OBJPATH)DcbManager.OBJ \
			$(OBJPATH)FatParser.OBJ \
			$(OBJPATH)File.OBJ \
			$(OBJPATH)FileSystemParser.OBJ \
			$(OBJPATH)IopProcessor.OBJ \
			$(OBJPATH)PGPdisk.OBJ \
			$(OBJPATH)VolFile.OBJ \
			$(OBJPATH)Volume.OBJ \
			$(OBJPATH)CPGPdiskDrv.OBJ \
			$(OBJPATH)CPGPdiskDrvComm.OBJ \
			$(OBJPATH)CPGPdiskDrvDebug.OBJ \
			$(OBJPATH)CPGPdiskDrvErrors.OBJ \
			$(OBJPATH)CPGPdiskDrvHooks.OBJ \
			$(OBJPATH)CPGPdiskDrvIos.OBJ \
			$(OBJPATH)CPGPdiskDrvVolumes.OBJ \
			$(OBJPATH)CPGPdiskDrvWinutils.OBJ \
			$(OBJPATH)CommonStrings.OBJ \
			$(OBJPATH)DualErr.OBJ \
			$(OBJPATH)Errors.OBJ \
			$(OBJPATH)FatUtils.OBJ \
			$(OBJPATH)PGPdiskContainer.OBJ \
			$(OBJPATH)PGPdiskHighLevelUtils.OBJ \
			$(OBJPATH)PGPdiskLowLevelUtils.OBJ \
			$(OBJPATH)PGPdiskPfl.OBJ \
			$(OBJPATH)SecureMemory.OBJ \
			$(OBJPATH)SecureString.OBJ \
			$(OBJPATH)StringAssociation.OBJ \
			$(OBJPATH)WaitObjectClasses.OBJ \
			$(OBJPATH)Cast5.OBJ \
			$(OBJPATH)Cast5Box.OBJ \
			$(OBJPATH)CipherContext.OBJ \
			$(OBJPATH)CipherUtils.OBJ \
			$(OBJPATH)SHA.OBJ

!include $(VTOOLSD)\include\vtoolsd.mak
!include $(VTOOLSD)\include\vxdtarg.mak

pgpDebug.OBJ:				pgpDebug.c
pgpLeaks.OBJ:				pgpLeaks.c
pgpMem.OBJ:					pgpMem.c
CommonStrings.OBJ:			CommonStrings.cpp
DcbManager.OBJ:				DcbManager.cpp
DualErr.OBJ:				DualErr.cpp
Errors.OBJ:					Errors.cpp
FatParser.OBJ:				FatParser.cpp
File.OBJ:					File.cpp
FileSystemParser.OBJ:		FileSystemParser.cpp
IopProcessor.OBJ:			IopProcessor.cpp
PGPdisk.OBJ:				PGPdisk.cpp
VolFile.OBJ:				VolFile.cpp
Volume.OBJ:					Volume.cpp
CPGPdiskDrv.OBJ:			CPGPdiskDrv.cpp
CPGPdiskDrvComm.OBJ:		CPGPdiskDrvComm.cpp
CPGPdiskDrvDebug.OBJ:		CPGPdiskDrvDebug.cpp
CPGPdiskDrvErrors.OBJ:		CPGPdiskDrvErrors.cpp
CPGPdiskDrvHooks.OBJ:		CPGPdiskDrvHooks.cpp
CPGPdiskDrvIos.OBJ:			CPGPdiskDrvIos.cpp
CPGPdiskDrvVolumes.OBJ:		CPGPdiskDrvVolumes.cpp
CPGPdiskDrvWinutils.OBJ:	CPGPdiskDrvWinutils.cpp
FatUtils.OBJ:				FatUtils.cpp
PGPdiskContainer.OBJ:		PGPdiskContainer.cpp
PGPdiskHighLevelUtils.OBJ:	PGPdiskHighLevelUtils.cpp
PGPdiskLowLevelUtils.OBJ:	PGPdiskLowLevelUtils.cpp
PGPdiskPfl.OBJ:				PGPdiskPfl.cpp
SecureMemory.OBJ:			SecureMemory.cpp
SecureString.OBJ:			SecureString.cpp
StringAssociation.OBJ:		StringAssociation.cpp
WaitObjectClasses.OBJ:		WaitObjectClasses.cpp
Cast5.OBJ:					Cast5.cpp
Cast5Box.OBJ:				Cast5Box.cpp
CipherContext.OBJ:			CipherContext.cpp
CipherUtils.OBJ:			CipherUtils.cpp
PGPdiskRandomPool.OBJ:		PGPdiskRandomPool.cpp
SHA.OBJ:					SHA.cpp
