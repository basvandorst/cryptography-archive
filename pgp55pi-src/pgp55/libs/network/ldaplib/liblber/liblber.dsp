# Microsoft Developer Studio Project File - Name="liblber" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=liblber - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "liblber.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "liblber.mak" CFG="liblber - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "liblber - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "liblber - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "liblber - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\include" /I "..\msdos" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\..\pfl\win32" /I "..\..\..\..\pfl\common" /D "NEEDPROTOS" /D "THREAD_NT_PTHREADS" /D "LDAP_LDBM" /D "LDBM_USE_DBBTREE21" /D "PGPKEYSERVER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "liblber - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "liblber_"
# PROP BASE Intermediate_Dir "liblber_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Od /I "..\..\include" /I "..\msdos" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\..\pfl\win32" /I "..\..\..\..\pfl\common" /D "LDAP_DEBUG" /D "NEEDPROTOS" /D "THREAD_NT_PTHREADS" /D "LDAP_LDBM" /D "LDBM_USE_DBBTREE21" /D "PGPKEYSERVER" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "liblber - Win32 Release"
# Name "liblber - Win32 Debug"
# Begin Source File

SOURCE=.\bprint.c
# End Source File
# Begin Source File

SOURCE=.\decode.c
# End Source File
# Begin Source File

SOURCE=.\encode.c
# End Source File
# Begin Source File

SOURCE=.\io.c
# End Source File
# Begin Source File

SOURCE=.\pgpSrvMem.c
# End Source File
# Begin Source File

SOURCE=.\version.c
# End Source File
# End Target
# End Project
