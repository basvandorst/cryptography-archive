# Microsoft Developer Studio Project File - Name="PGPdiskResident" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PGPdiskResident - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskResident.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskResident.mak" CFG="PGPdiskResident - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPdiskResident - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPdiskResident - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPdiskResident - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPdiskResident/Release"
# PROP Intermediate_Dir "PGPdiskResident/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GX /O2 /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\pgp\shared" /I "..\..\pgp\win32\PGPcl" /I "Shared" /I "Encryption" /I "PGPdiskResident\Source" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /D PGP_DEBUG=0 /D "PGPDISK_MFC" /D "PGP_INTEL" /D "_AFXDLL" /YX"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 mpr.lib ..\..\..\libs\pfl\win32\pflCommon\Release\pflCommon.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGP_SDK.lib ..\..\pgp\win32\PGPcl\Release\PGPcl.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /nodefaultlib:"msvcrtd.lib"

!ELSEIF  "$(CFG)" == "PGPdiskResident - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPdiskResident/Debug"
# PROP Intermediate_Dir "PGPdiskResident/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GX /ZI /Od /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\pgp\shared" /I "..\..\pgp\win32\PGPcl" /I "Shared" /I "Encryption" /I "PGPdiskResident\Source" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /D PGP_DEBUG=1 /D "PGPDISK_MFC" /D "PGP_INTEL" /D "PGP_WIN32" /D "UNFINISHED_CODE_ALLOWED" /D "_AFXDLL" /YX"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mpr.lib ..\..\..\libs\pfl\win32\pflCommon\Debug\pflCommon.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGP_SDK.lib ..\..\pgp\win32\PGPcl\Debug\PGPcl.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "PGPdiskResident - Win32 Release"
# Name "PGPdiskResident - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Shared Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shared\CommonStrings.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\DriverComm.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\DualErr.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\Errors.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\GlobalPGPContext.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\LinkResolution.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPfl.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPrefs.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\SharedMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\StringAssociation.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\Win32Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\WindowsVersion.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskResident\Source\CHiddenWindow.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\PGPdiskResident\Source\CPGPdiskResidentApp.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskResident\Source\StdAfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Shared Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shared\CommonStrings.h
# End Source File
# Begin Source File

SOURCE=.\Shared\DriverComm.h
# End Source File
# Begin Source File

SOURCE=.\Shared\DualErr.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Errors.h
# End Source File
# Begin Source File

SOURCE=.\Shared\GlobalPGPContext.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Packets.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskDefaults.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPfl.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPrefs.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskRegistry.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskResidentDefines.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskVersion.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Required.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SharedMemory.h
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

SOURCE=.\Shared\Win32Utils.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskResident\Source\CHiddenWindow.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskResident\Source\CPGPdiskResidentApp.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskResident\Source\Globals.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskResident\Source\Resource.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskResident\Source\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPdiskResident\res\PGPdiskResident.ico
# End Source File
# Begin Source File

SOURCE=.\res\PGPdiskResident.ico
# End Source File
# Begin Source File

SOURCE=.\res\PGPdiskResident.rc2
# End Source File
# Begin Source File

SOURCE=.\PGPdiskResident\res\PGPdiskResidentDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\PGPdiskResidentDoc.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskResident\PGPdiskResident.rc
# End Source File
# End Target
# End Project
