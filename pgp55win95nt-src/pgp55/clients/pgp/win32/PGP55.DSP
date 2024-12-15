# Microsoft Developer Studio Project File - Name="PGP55" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=PGP55 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGP55.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGP55.MAK" CFG="PGP55 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGP55 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PGP55 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "PGP55 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGP55\Release"
# PROP Intermediate_Dir ".\PGP55\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Copying files into PGP55\Release
PostBuild_Cmds=del /q PGP55\Release\*.*	copy EudoraPlugin\Release\pgpplugin.dll\
        PGP55\Release\.	copy PGPadmin\Release\PGPadmin.exe PGP55\Release\.	copy\
        ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGPsdk.dll PGP55\Release\.	copy\
        ..\..\..\libs\pgpcdk\win32\PGPcdkKeyServer\Release\PGPsdkKS.dll\
        PGP55\Release\.	copy PGPcmdlg\Release\PGP55cd.dll PGP55\Release\.	copy\
     pgpExch\Release\pgpExch.dll PGP55\Release\.	copy PGPkeys\Release\PGPkeys.exe\
        PGP55\Release\.	copy PGPkm\Release\PGP55km.dll PGP55\Release\.	copy\
     PGPlog\Release\PGPlog.exe PGP55\Release\.	copy PGPrecip\Release\PGP55rd.dll\
        PGP55\Release\.	copy PGPsc\Release\PGP55sc.dll PGP55\Release\.	copy\
     PGPtools\Release\PGPtools.exe PGP55\Release\.	copy PGPtray\Release\PGPtray.exe\
        PGP55\Release\.	copy pgpwctx\Release\PGP55mn.dll PGP55\Release\.	copy\
 EudoraPluginV4\Release\EudoraPluginV4.dll        PGP55\Release\.
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGP55 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGP55___"
# PROP BASE Intermediate_Dir "PGP55___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGP55\Debug"
# PROP Intermediate_Dir ".\PGP55\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Copying files into PGP55\Debug
PostBuild_Cmds=del /q PGP55\Debug\*.*	copy EudoraPlugin\Debug\pgpplugin.dll\
        PGP55\Debug\.	copy PGPadmin\Debug\PGPadmin.exe PGP55\Debug\.	copy\
        ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGPsdk.dll PGP55\Debug\.	copy\
        ..\..\..\libs\pgpcdk\win32\PGPcdkKeyServer\Debug\PGPsdkKS.dll\
        PGP55\Debug\.	copy PGPcmdlg\Debug\PGP55cd.dll PGP55\Debug\.	copy\
     pgpExch\Debug\pgpExch.dll PGP55\Debug\.	copy PGPkeys\Debug\PGPkeys.exe\
        PGP55\Debug\.	copy PGPkm\Debug\PGP55km.dll PGP55\Debug\.	copy\
     PGPlog\Debug\PGPlog.exe PGP55\Debug\.	copy PGPrecip\Debug\PGP55rd.dll\
        PGP55\Debug\.	copy PGPsc\Debug\PGP55sc.dll PGP55\Debug\.	copy\
     PGPtools\Debug\PGPtools.exe PGP55\Debug\.	copy PGPtray\Debug\PGPtray.exe\
        PGP55\Debug\.	copy pgpwctx\Debug\PGP55mn.dll PGP55\Debug\.	copy\
 EudoraPluginV4\Debug\EudoraPluginV4.dll        PGP55\Debug\.
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGP55 - Win32 Release"
# Name "PGP55 - Win32 Debug"
# Begin Source File

SOURCE=.\PGP55\main.c
# End Source File
# End Target
# End Project
