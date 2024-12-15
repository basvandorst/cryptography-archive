# Microsoft Developer Studio Project File - Name="PGPclient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PGPclient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPclient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPclient.mak" CFG="PGPclient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPclient - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "PGPclient - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPclient - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPClient\Release"
# PROP Intermediate_Dir "PGPClient\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=del /q PGPclient\Release\*.*	copy          EudoraPlugin\Release\PGPEudoraPlugin.dll PGPclient\Release\. 	copy          EudoraPluginV4\Release\PGPEudoraPluginV4.dll PGPClient\Release\.	copy          OutlookExpress\Release\PGPoe.dll PGPClient\Release\.	copy          PGPadmin\Release\PGPadmin.exe PGPClient\Release\.	copy          ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGP_SDK.dll PGPclient\Release\.	copy          PGPcl\Release\PGPcl.dll PGPclient\Release\.	copy pgpExch\Release\PGPExch.dll          PGPclient\Release\.	copy PGPhk\Release\PGPhk.dll PGPclient\Release\.	copy          PGPkeys\Release\PGPkeys.exe PGPclient\Release\.	copy PGPlog\Release\PGPlog.exe          PGPclient\Release\.	copy PGPsc\Release\PGPsc.dll PGPclient\Release\.	copy          ..\..\..\libs\pgpcdk\win32\PGPsdkNetworkLib\Release\PGPsdkNL.dll          PGPclient\Release\.	copy         ..\..\..\libs\pgpcdk\win32\PGPsdkUI\Release\PGPsdkUI.dll  PGPClient\Release\.        	copy PGPtools\Release\PGPtools.exe PGPClient\Release\.	copy          PGPtray\Release\PGPtray.exe PGPClient\Release\.	copy         pgpwctx\Release\PGPmn.dll  PGPClient\Release\.	copy         MakeSEA\Release\MakeSEA.exe\
 PGPClient\Release\.
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPclient - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPclient\Debug"
# PROP Intermediate_Dir "PGPclient\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying files
PostBuild_Cmds=del /q PGPclient\Debug\*.*	copy          EudoraPlugin\Debug\PGPEudoraPlugin.dll PGPclient\Debug\. 	copy          EudoraPluginV4\Debug\PGPEudoraPluginV4.dll PGPClient\Debug\.	copy          OutlookExpress\Debug\PGPoe.dll PGPClient\Debug\.	copy          PGPadmin\Debug\PGPadmin.exe PGPClient\Debug\.	copy          ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGP_SDK.dll PGPclient\Debug\.	copy          PGPcl\Debug\PGPcl.dll PGPclient\Debug\.	copy pgpExch\Debug\PGPExch.dll          PGPclient\Debug\.	copy PGPhk\Debug\PGPhk.dll PGPclient\Debug\.	copy          PGPkeys\Debug\PGPkeys.exe PGPclient\Debug\.	copy PGPlog\Debug\PGPlog.exe          PGPclient\Debug\.	copy PGPsc\Debug\PGPsc.dll PGPclient\Debug\.	copy          ..\..\..\libs\pgpcdk\win32\PGPsdkNetworkLib\Debug\PGPsdkNL.dll          PGPclient\Debug\.	copy ..\..\..\libs\pgpcdk\win32\PGPsdkUI\Debug\PGPsdkUI.dll          PGPClient\Debug\.	copy PGPtools\Debug\PGPtools.exe PGPClient\Debug\.	copy          PGPtray\Debug\PGPtray.exe PGPClient\Debug\.	copy pgpwctx\Debug\PGPmn.dll          PGPClient\Debug\.	copy MakeSEA\Debug\MakeSEA.exe    PGPClient\Debug\.
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPclient - Win32 Release"
# Name "PGPclient - Win32 Debug"
# Begin Source File

SOURCE=.\PGPclient\main.c
# End Source File
# End Target
# End Project
