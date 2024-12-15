# Microsoft Developer Studio Project File - Name="PGPdiskShellExt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPdiskShellExt - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskShellExt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskShellExt.mak" CFG="PGPdiskShellExt - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPdiskShellExt - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPdiskShellExt - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPdiskShellExt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPdiskShellExt\Release"
# PROP Intermediate_Dir "PGPdiskShellExt\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /WX /GX /O2 /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\pgp\shared" /I "..\..\pgp\win32\PGPcl" /I "Shared" /I "Encryption" /I "PGPdiskShellExt\Source" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /D PGP_DEBUG=0 /D "PGPDISK_MFC" /D "PGP_INTEL" /D "_USRDLL" /D "_WINDLL" /YX"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 mpr.lib ..\..\..\libs\pfl\win32\pflCommon\Release\pflCommon.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGP_SDK.lib ..\..\pgp\win32\PGPcl\Release\PGPcl.lib /nologo /subsystem:windows /dll /map /machine:I386 /out:"PGPdiskShellExt\Release\PGPdskSE.dll"

!ELSEIF  "$(CFG)" == "PGPdiskShellExt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPdiskShellExt\Debug"
# PROP Intermediate_Dir "PGPdiskShellExt\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GX /ZI /Od /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\pgp\shared" /I "..\..\pgp\win32\PGPcl" /I "Shared" /I "Encryption" /I "PGPdiskShellExt\Source" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /D PGP_DEBUG=1 /D "PGPDISK_MFC" /D "PGP_INTEL" /D "UNFINISHED_CODE_ALLOWED" /D "_USRDLL" /D "_WINDLL" /YX"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 mpr.lib ..\..\..\libs\pfl\win32\pflCommon\Debug\pflCommon.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGP_SDK.lib ..\..\pgp\win32\PGPcl\Debug\PGPcl.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"PGPdiskShellExt\Debug\PGPdskSE.dll"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "PGPdiskShellExt - Win32 Release"
# Name "PGPdiskShellExt - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
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

SOURCE=.\PGPdiskShellExt\Source\CPGPdiskShellExtDll.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\CShellExt.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\CShellExtClassFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\StdAfx.cpp
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

SOURCE=.\Shared\DriverComm.h
# End Source File
# Begin Source File

SOURCE=.\Shared\DualErr.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Errors.h
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

SOURCE=.\Shared\PGPdiskVersion.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Required.h
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

SOURCE=.\PGPdiskShellExt\Source\CPGPdiskShellExtDll.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\CShellExt.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\CShellExtClassFactory.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\Globals.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\Resource.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\Source\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\PGPdiskShellExt\PGPdiskShellExt.def
# End Source File
# Begin Source File

SOURCE=.\PGPdiskShellExt\PGPdiskShellExt.rc
# End Source File
# End Target
# End Project
