# Microsoft Developer Studio Project File - Name="jpeglib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=jpeglib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jpeglib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jpeglib.mak" CFG="jpeglib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jpeglib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "jpeglib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jpeglib - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PGP_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "jpeglib - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "PGP_WIN32" /YX /FD /c
# SUBTRACT CPP /X
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

# Name "jpeglib - Win32 Release"
# Name "jpeglib - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=..\common\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\common\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\common\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\common\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\common\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\common\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\common\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\common\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\common\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\common\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\common\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\common\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\common\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\common\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\common\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\common\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\common\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\common\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\common\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\common\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\common\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\common\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\common\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\common\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\common\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\common\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\common\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\common\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\common\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\common\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\common\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\common\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\common\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\common\jerror.c
# End Source File
# Begin Source File

SOURCE=..\common\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\common\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\common\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\common\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\common\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\common\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\common\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\common\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\common\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\common\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\common\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\common\jutils.c
# End Source File
# End Group
# End Target
# End Project
