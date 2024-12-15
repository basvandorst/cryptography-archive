rem Makefile wrapper for PGPdiskDrv.

cd PGPdiskDrvWin95

copy ..\..\..\..\libs\pfl\common\*.c Source
copy ..\Shared\*.cpp Source
copy ..\Encryption\*.cpp Source

mkdir Release

copy PGPdiskDrvRelease.mak Release

cd Release

NMAKE %1 /s /f PGPdiskDrvRelease.mak

cd ..\Source

cl /P /EP PGPdiskVrcSource.cpp

del ..\Release\PGPdisk.vrc
move PGPdiskVrcSource.i ..\Release\PGPdisk.vrc

cd ..\Release

%VTOOLSD%\bin\vxdver PGPdisk.vrc PGPdisk.res
%VTOOLSD%\bin\sethdr -r PGPdisk.res PGPdisk.pdr

cd ..\Source

del pgpBinaryTree.c
del pgpDebug.c
del pgpLeaks.c
del pgpMem.c
del pgpMemoryMgr.c
del pgpMemoryMgrMac.c
del pgpMemoryMgrStd.c
del pgpMemoryMgrWin32.c
del pgpMilliseconds.c
del pgpRMWOLock.c
del pgpTemplate.c

del CommonStrings.cpp
del DriverComm.cpp
del DualErr.cpp
del Errors.cpp
del FatUtils.cpp
del LinkResolution.cpp
del PGPdiskContainer.cpp
del PGPdiskHighLevelUtils.cpp
del PGPdiskLowLevelUtils.cpp
del PGPdiskPfl.cpp
del PGPdiskPrefs.cpp
del PGPdiskPublicKeyUtils.cpp
del PGPdiskRegistry.cpp
del SecureMemory.cpp
del SecureString.cpp
del SharedMemory.cpp
del StringAssociation.cpp
del WaitObjectClasses.cpp
del Win32Utils.cpp
del WindowsVersion.cpp

del Cast5.cpp
del Cast5Box.cpp
del CipherContext.cpp
del CipherUtils.cpp
del PGPdiskRandomPool.cpp
del SHA.cpp
