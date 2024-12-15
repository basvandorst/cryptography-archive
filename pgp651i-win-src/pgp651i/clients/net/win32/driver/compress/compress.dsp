# Microsoft Developer Studio Project File - Name="compress" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=compress - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "compress.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "compress.mak" CFG="compress - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "compress - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "compress - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "compress - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\shared\compress" /I "..\..\..\shared\compress\deflate" /I "..\..\..\shared\compress\LZS" /I "..\minisdk" /I "..\..\..\shared\minisdk\include" /D PGP_DEBUG=0 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D USERLAND_TEST=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "compress - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\..\..\shared\compress" /I "..\..\..\shared\compress\deflate" /I "..\..\..\shared\compress\LZS" /I "..\minisdk" /I "..\..\..\shared\minisdk\include" /D PGP_DEBUG=1 /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D USERLAND_TEST=1 /YX /FD /c
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

# Name "compress - Win32 Release"
# Name "compress - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "deflate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\infblock.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\infcodes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\infutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\pgpCompDeflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\zutil.c
# End Source File
# End Group
# Begin Group "LZS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\shared\compress\LZS\LZSC.C
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\LZS\pgpCompLZS.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\shared\compress\pgpCompress.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\infblock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\infcodes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\infutil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\LZS\LZSC.H
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\pgpCompDeflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\LZS\pgpCompLZS.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\pgpCompress.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\pgpCompressPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\compress\deflate\zutil.h
# End Source File
# End Group
# End Target
# End Project
