# Microsoft Developer Studio Project File - Name="deflate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=deflate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "deflate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "deflate.mak" CFG="deflate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "deflate - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "deflate - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "deflate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "deflate_"
# PROP BASE Intermediate_Dir "deflate_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "deflate\Release"
# PROP Intermediate_Dir "deflate\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /D PGP_DEBUG=0 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "deflate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "deflate\Debug"
# PROP Intermediate_Dir "deflate\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /D PGP_DEBUG=1 /YX /FD /c
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

# Name "deflate - Win32 Release"
# Name "deflate - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\infblock.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\infcodes.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\infutil.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\zutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\deflate.h
# End Source File
# Begin Source File

SOURCE=.\shared\DeflateWrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\infblock.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\infcodes.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\infutil.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\pgp\shared\deflate\zutil.h
# End Source File
# End Group
# End Target
# End Project
