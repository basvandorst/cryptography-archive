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
PostBuild_Cmds=del /q PGP55\Release\*.*	copy\
     EudoraPlugin\Release\pgpEudoraPlugin.dll PGP55\Release\.	copy\
     PGPadmin\Release\PGPadmin.exe PGP55\Release\.	copy\
                      ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGP_SDK.dll PGP55\Release\.	copy\
                      ..\..\..\libs\pgpcdk\win32\PGPsdkNetworkLib\Release\PGPsdkNL.dll\
                      PGP55\Release\.	copy        pgpExch\Release\pgpExch.dll PGP55\Release\.	copy\
            PGPkeys\Release\PGPkeys.exe           PGP55\Release\.	copy\
                   PGPlog\Release\PGPlog.exe PGP55\Release\.	copy PGPhk\Release\PGP60hk.dll\
                      PGP55\Release\.	copy PGPsc\Release\PGP60sc.dll PGP55\Release\.	copy\
                   PGPtools\Release\PGPtools.exe PGP55\Release\.	copy PGPtray\Release\PGPtray.exe\
                      PGP55\Release\.	copy pgpwctx\Release\PGP60mn.dll PGP55\Release\.	copy\
               EudoraPluginV4\Release\pgpEudoraPluginV4.dll        PGP55\Release\.	copy\
              PGPcl\Release\PGP60cl.dll PGP55\Release\.	copy\
                      ..\..\..\libs\pgpcdk\win32\PGPsdkUI\Release\PGPsdkUI.dll PGP55\Release\.	copy\
        OEloader\Release\msimn.exe PGP55\Release\.	copy\
        OutlookExpress\Release\PGP60oe.dll PGP55\Release\.
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
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Copying files into PGP55\Debug
PostBuild_Cmds=del /q PGP55\Debug\*.*	copy\
     EudoraPlugin\Debug\pgpEudoraPlugin.dll               PGP55\Debug\.	copy\
     PGPadmin\Debug\PGPadmin.exe PGP55\Debug\.	copy\
                      ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGP_SDK.dll PGP55\Debug\.	copy\
                      ..\..\..\libs\pgpcdk\win32\PGPsdkNetworkLib\Debug\PGPsdkNL.dll\
                      PGP55\Debug\.	copy        pgpExch\Debug\pgpExch.dll PGP55\Debug\.	copy\
            PGPkeys\Debug\PGPkeys.exe           PGP55\Debug\.	copy\
                   PGPlog\Debug\PGPlog.exe PGP55\Debug\.	copy PGPhk\Debug\PGP60hk.dll\
                      PGP55\Debug\.	copy PGPsc\Debug\PGP60sc.dll PGP55\Debug\.	copy\
                   PGPtools\Debug\PGPtools.exe PGP55\Debug\.	copy PGPtray\Debug\PGPtray.exe\
                      PGP55\Debug\.	copy pgpwctx\Debug\PGP60mn.dll PGP55\Debug\.	copy\
               EudoraPluginV4\Debug\pgpEudoraPluginV4.dll        PGP55\Debug\.	copy\
              PGPcl\Debug\PGP60cl.dll PGP55\Debug\.	copy\
                      ..\..\..\libs\pgpcdk\win32\PGPsdkUI\Debug\PGPsdkUI.dll PGP55\Debug\.	copy\
        OEloader\Debug\msimn.exe PGP55\Debug\.	copy OutlookExpress\Debug\PGP60oe.dll\
        PGP55\Debug\.
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
