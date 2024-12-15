# Microsoft Developer Studio Project File - Name="PGPdiskDrvWin95" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=PGPdiskDrvWin95 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskDrvWin95.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskDrvWin95.mak" CFG="PGPdiskDrvWin95 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPdiskDrvWin95 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "PGPdiskDrvWin95 - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "PGPdiskDrvWin95 - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f PGPdiskDrvWin95.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "PGPdiskDrvWin95.exe"
# PROP BASE Bsc_Name "PGPdiskDrvWin95.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "PGPdiskDrvWin95\PGPdiskDrvRelease.bat"
# PROP Rebuild_Opt "/a"
# PROP Target_File "PGPdisk.pdr"
# PROP Bsc_Name "PGPdiskDrv.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "PGPdiskDrvWin95 - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f PGPdiskDrvWin95.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "PGPdiskDrvWin95.exe"
# PROP BASE Bsc_Name "PGPdiskDrvWin95.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "PGPdiskDrvWin95\PGPdiskDrvDebug.bat"
# PROP Rebuild_Opt "/a"
# PROP Target_File "PGPdisk.pdr"
# PROP Bsc_Name "PGPdisk.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "PGPdiskDrvWin95 - Win32 Release"
# Name "PGPdiskDrvWin95 - Win32 Debug"

!IF  "$(CFG)" == "PGPdiskDrvWin95 - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPdiskDrvWin95 - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Custom Classes Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\DcbManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\FatParser.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\File.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\FileSystemParser.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\IopProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\PGPdisk.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\VolFile.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\Volume.cpp
# End Source File
# End Group
# Begin Group "Driver Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrv.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvComm.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvErrors.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvHooks.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvIos.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvVolumes.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvWinutils.cpp
# End Source File
# End Group
# Begin Group "Encryption Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Encryption\Cast5.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\Cast5Box.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherContext.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\SHA.cpp
# End Source File
# End Group
# Begin Group "Shared Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shared\CommonStrings.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\DualErr.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\Errors.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\FatUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskHighLevelUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskLowLevelUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPfl.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureString.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\StringAssociation.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\WaitObjectClasses.cpp
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Group "Custom Classes Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\DcbManager.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\FatParser.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\File.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\FileSystemParser.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\IopProcessor.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\PGPdisk.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\VolFile.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\Volume.h
# End Source File
# End Group
# Begin Group "Driver Class Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrv.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvComm.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvDebug.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvErrors.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvHooks.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvIos.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvVolumes.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\CPGPdiskDrvWinutils.h
# End Source File
# End Group
# Begin Group "Fake Headers"

# PROP Default_Filter ""
# Begin Group "sys"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\FakeSource\sys\Types.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\FakeSource\Crtdbg.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\FakeSource\Limits.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\FakeSource\Stdarg.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\FakeSource\Stddef.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\FakeSource\Stdlib.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\FakeSource\String.h
# End Source File
# End Group
# Begin Group "Encryption Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Encryption\Cast5.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\Cast5Box.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherContext.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherUtils.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\SHA.h
# End Source File
# End Group
# Begin Group "Shared Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shared\CommonStrings.h
# End Source File
# Begin Source File

SOURCE=.\Shared\DualErr.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Errors.h
# End Source File
# Begin Source File

SOURCE=.\Shared\FatUtils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\GenericCallback.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Packets.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskContainer.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskDefaults.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskFileFormat.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskHighLevelUtils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskLowLevelUtils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPfl.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskVersion.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Required.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureMemory.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureString.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SimpleQueue.h
# End Source File
# Begin Source File

SOURCE=.\Shared\StringAssociation.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SystemConstants.h
# End Source File
# Begin Source File

SOURCE=.\Shared\UtilityFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Shared\WaitObjectClasses.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\Globals.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\PlacementNew.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\Source\Wrappers.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\PGPdiskDrvDebug.bat
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\PGPdiskDrvDebug.mak
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\PGPdiskDrvDebugAux.bat
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\PGPdiskDrvRelease.bat
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\PGPdiskDrvRelease.mak
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvWin95\PGPdiskDrvReleaseAux.bat
# End Source File
# End Target
# End Project
