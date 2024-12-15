# Microsoft Developer Studio Project File - Name="PGPdiskDrvNT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=PGPdiskDrvNT - Win32 Checked
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskDrvNT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskDrvNT.mak" CFG="PGPdiskDrvNT - Win32 Checked"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPdiskDrvNT - Win32 Free" (based on "Win32 (x86) External Target")
!MESSAGE "PGPdiskDrvNT - Win32 Checked" (based on\
 "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "PGPdiskDrvNT - Win32 Free"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPdiskD"
# PROP BASE Intermediate_Dir "PGPdiskD"
# PROP BASE Cmd_Line "PGPdiskDrvNT\runbuild checked"
# PROP BASE Rebuild_Opt "-cf"
# PROP BASE Target_File "PGPdiskDrvNT\i386\checked\PGPdisk.sys"
# PROP BASE Bsc_Name "PGPdiskDrvNT\i386\checked\PGPdisk.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPdiskD"
# PROP Intermediate_Dir "PGPdiskD"
# PROP Cmd_Line "PGPdiskDrvNT\RunBuild free"
# PROP Rebuild_Opt "-cf"
# PROP Target_File "PGPdiskDrvNT\i386\free\PGPdisk.sys"
# PROP Bsc_Name "PGPdiskDrvNT\i386\free\PGPdisk.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "PGPdiskDrvNT - Win32 Checked"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPdisk0"
# PROP BASE Intermediate_Dir "PGPdisk0"
# PROP BASE Cmd_Line "PGPdiskDrvNT\runbuild checked"
# PROP BASE Rebuild_Opt "-cf"
# PROP BASE Target_File "PGPdiskDrvNT\i386\checked\PGPdisk.sys"
# PROP BASE Bsc_Name "PGPdiskDrvNT\i386\checked\PGPdisk.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPdisk0"
# PROP Intermediate_Dir "PGPdisk0"
# PROP Cmd_Line "PGPdiskDrvNT\RunBuild checked"
# PROP Rebuild_Opt "-cf"
# PROP Target_File "PGPdiskDrvNT\i386\checked\PGPdisk.sys"
# PROP Bsc_Name "PGPdiskDrvNT\i386\checked\PGPdisk.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "PGPdiskDrvNT - Win32 Free"
# Name "PGPdiskDrvNT - Win32 Checked"

!IF  "$(CFG)" == "PGPdiskDrvNT - Win32 Free"

!ELSEIF  "$(CFG)" == "PGPdiskDrvNT - Win32 Checked"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
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

SOURCE=.\Encryption\PGPdiskRandomPool.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\SHA.cpp
# End Source File
# End Group
# Begin Group "Custom Classes Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\File.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\PGPdisk.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\PGPdiskRequestQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\PGPdiskWorkerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\UserMemoryManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\VolFile.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\Volume.cpp
# End Source File
# End Group
# Begin Group "Driver Classes Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceComm.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceErrors.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceHooks.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceVolumes.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CRunTimeExtras.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\KernelModeUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\NtThreadSecurity.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
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
# Begin Group "FakeSource"

# PROP Default_Filter ""
# Begin Group "sys"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvNT\FakeSource\sys\Types.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskDrvNT\FakeSource\Crtdbg.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\FakeSource\Limits.h
# End Source File
# End Group
# Begin Group "Custom Classes Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\File.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\LockableList.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\LookasideList.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\PGPdisk.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\PGPdiskRequestQueue.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\PGPdiskWorkerThread.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\UserMemoryManager.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\VolFile.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\Volume.h
# End Source File
# End Group
# Begin Group "Driver Classes Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskDriver.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterface.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceComm.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceErrors.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceHooks.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CPGPdiskInterfaceVolumes.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\CRunTimeExtras.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\FSCTLCodes.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\Function.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\Globals.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\KernelModeUtils.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\NtThreadSecurity.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\Source\OperatorNewFixups.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskDrvNT\makefile
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\msglog.mc
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\RunBuild.bat
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\RunBuildAux.bat
# End Source File
# Begin Source File

SOURCE=.\PGPdiskDrvNT\sources
# End Source File
# End Target
# End Project
