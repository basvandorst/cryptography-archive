# Microsoft Developer Studio Project File - Name="pflCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pflCommon - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pflCommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pflCommon.mak" CFG="pflCommon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pflCommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pflCommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "pflCommon - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../" /I "../../common" /I "../../common/file" /I "../../common/prefs" /I "../../common/util" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pflCommon - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "../" /I "../../common" /I "../../common/file" /I "../../common/prefs" /I "../../common/util" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "pflCommon - Win32 Release"
# Name "pflCommon - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp"
# Begin Source File

SOURCE=..\..\common\pflContext.c
# End Source File
# Begin Source File

SOURCE=..\..\common\prefs\pflPrefs.c
# End Source File
# Begin Source File

SOURCE=..\..\common\prefs\pflPrefTypes.c
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileIO.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpec.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecMac.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecStd.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecVTBL.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileUtilities.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpIO.c
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpLeaks.c
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpMem.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpMemoryIO.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpProxyIO.c
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpStdFileIO.c
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpTemplate.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=..\..\common\pflContext.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pflContextPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\prefs\pflPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\common\prefs\pflPrefTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pflTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpBase.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileIO.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileIOPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpec.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecMacPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecStd.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecStdPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileSpecVTBL.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpFileUtilities.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpIO.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpIOPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpLeaks.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpMem.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpMemoryIO.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpMemoryIOPriv.h
# End Source File
# Begin Source File

SOURCE=..\pgpPFLConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpPFLErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpProxyIO.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpProxyIOPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpStdFileIO.h
# End Source File
# Begin Source File

SOURCE=..\..\common\file\pgpStdFileIOPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pgpTypes.h
# End Source File
# End Group
# End Target
# End Project
