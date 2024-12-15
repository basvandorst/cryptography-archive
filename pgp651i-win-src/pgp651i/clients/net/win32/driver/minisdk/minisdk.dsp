# Microsoft Developer Studio Project File - Name="minisdk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=minisdk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "minisdk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "minisdk.mak" CFG="minisdk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "minisdk - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "minisdk - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "minisdk - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "..\..\..\shared\minisdk\include" /D PGP_DEBUG=0 /D "NDEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "WIN32" /D "_WINDOWS" /D USERLAND_TEST=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "minisdk - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "." /I "..\..\..\shared\minisdk\include" /D PGP_DEBUG=1 /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "WIN32" /D "_WINDOWS" /D USERLAND_TEST=1 /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "minisdk - Win32 Release"
# Name "minisdk - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpCAST5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpCBC.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpCFB.c
# End Source File
# Begin Source File

SOURCE=.\pgpDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpDES3.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpHash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpHMAC.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpIDEA.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpMD5.c
# End Source File
# Begin Source File

SOURCE=.\pgpMem.c
# End Source File
# Begin Source File

SOURCE=.\pgpMemoryMgr.c
# End Source File
# Begin Source File

SOURCE=.\pgpMemoryMgrWin32.c
# End Source File
# Begin Source File

SOURCE=.\pgpMutex.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpRIPEMD160.c
# End Source File
# Begin Source File

SOURCE=.\pgpRMWOLock.c
# End Source File
# Begin Source File

SOURCE=.\pgpSemaphore.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpSHA.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpSymmetricCipher.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpCAST5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpCASTBox5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpCBC.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpCBCPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpCFB.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpCFBPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpDES3.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpHash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpHashPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpHMAC.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpIDEA.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpMD5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpPubTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpRIPEMD160.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\hash\pgpSHA.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpSymmetricCipher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\cipher\pgpSymmetricCipherPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\minisdk\include\pgpUsuals.h
# End Source File
# End Group
# End Target
# End Project
