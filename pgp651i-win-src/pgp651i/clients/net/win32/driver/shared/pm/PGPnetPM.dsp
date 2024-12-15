# Microsoft Developer Studio Project File - Name="PGPnetPM" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PGPnetPM - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPnetPM.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPnetPM.mak" CFG="PGPnetPM - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPnetPM - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "PGPnetPM - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPnetPM - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\..\shared" /I "..\..\..\shared" /I "..\..\shared" /I "..\..\..\..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\..\..\..\libs\pgpcdk\win32" /I "..\..\..\..\..\..\libs\pfl\common" /I "..\..\..\..\..\..\libs\pfl\win32" /I "..\..\..\..\..\..\libs\pfl\common\lthread" /I "..\..\..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\..\..\libs\pfl\common\file" /I "..\..\..\..\..\..\libs\pfl\common\util" /I "..\..\..\..\shared\ipsec" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D PGP_WIN32=1 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib PGP_SDK.lib pflCommon.lib ipsec.lib minisdk.lib /nologo /subsystem:console /machine:I386 /libpath:"../../../../../../libs/pgpcdk/win32/PGPcdk/Release" /libpath:"../../../../../../libs/pfl/win32/pflCommon/Release" /libpath:"../../ipsec/Release" /libpath:"../../minisdk/Release"

!ELSEIF  "$(CFG)" == "PGPnetPM - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPnetPM"
# PROP BASE Intermediate_Dir "PGPnetPM"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\..\shared" /I "..\..\..\shared" /I "..\..\shared" /I "..\..\..\..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\..\..\..\libs\pgpcdk\win32" /I "..\..\..\..\..\..\libs\pfl\common" /I "..\..\..\..\..\..\libs\pfl\win32" /I "..\..\..\..\..\..\libs\pfl\common\lthread" /I "..\..\..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\..\..\libs\pfl\common\file" /I "..\..\..\..\..\..\libs\pfl\common\util" /I "..\..\..\..\shared\ipsec" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D PGP_WIN32=1 /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib PGP_SDK.lib pflCommon.lib ipsec.lib minisdk.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../../../../libs/pgpcdk/win32/PGPcdk/Debug" /libpath:"../../../../../../libs/pfl/win32/pflCommon/Debug" /libpath:"../../ipsec/Debug" /libpath:"../../minisdk/Debug"

!ENDIF 

# Begin Target

# Name "PGPnetPM - Win32 Release"
# Name "PGPnetPM - Win32 Debug"
# Begin Group "Source files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\pgpNetConfig.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\shared\pgpNetHostPrefs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\pgpNetPaths.c
# End Source File
# Begin Source File

SOURCE=.\pgpNetPMConfig.c
# End Source File
# Begin Source File

SOURCE=.\pgpNetPMHost.c
# End Source File
# Begin Source File

SOURCE=.\pgpNetPMSA.c
# End Source File
# Begin Source File

SOURCE=.\pgpNetPolicyManager.c
# End Source File
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\..\..\shared\pgpNetConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\shared\pgpNetHostPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\pgpNetIPC.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\pgpNetPaths.h
# End Source File
# Begin Source File

SOURCE=.\pgpNetPMConfig.h
# End Source File
# Begin Source File

SOURCE=.\pgpNetPMHost.h
# End Source File
# Begin Source File

SOURCE=.\pgpNetPMSA.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\shared\pgpNetPrefs.h
# End Source File
# End Group
# End Target
# End Project
