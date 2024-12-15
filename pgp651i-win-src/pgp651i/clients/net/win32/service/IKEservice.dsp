# Microsoft Developer Studio Project File - Name="IKEservice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=IKEservice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IKEservice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IKEservice.mak" CFG="IKEservice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IKEservice - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "IKEservice - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IKEservice - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(BASEDIR)\src\network\inc" /I "..\..\shared" /I "..\..\shared\ipsec" /I "..\shared" /I ".\shared" /I "..\..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\..\libs\pgpcdk\win32" /I "..\..\..\..\libs\pfl\common" /I "..\..\..\..\libs\pfl\win32" /I "..\..\..\..\libs\pfl\common\lthread" /I "..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\libs\pfl\common\file" /I "..\..\..\..\libs\pfl\common\util" /I "..\driver\shared\ndis" /I "..\..\..\pgp\win32\include" /I "..\..\..\pgp\shared" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D PGP_WIN32=1 /D PGP_DEBUG=0 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\..\..\pgp\win32\include" /d "NDEBUG"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 PGPsdkUI.lib wsock32.lib PGP_SDK.lib PGPsdkNL.lib pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\..\..\..\libs\pgpcdk\win32\PGPcdk\Release" /libpath:"..\..\..\..\libs\pfl\win32\pflCommon\Release" /libpath:"..\..\..\..\libs\pgpcdk\win32\PGPsdkUI\Release" /libpath:"..\..\..\..\libs\pgpcdk\win32\PGPsdkNetworkLib\Release"

!ELSEIF  "$(CFG)" == "IKEservice - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\src\network\inc" /I "..\..\shared" /I "..\..\shared\ipsec" /I "..\shared" /I ".\shared" /I "..\..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\..\libs\pgpcdk\win32" /I "..\..\..\..\libs\pfl\common" /I "..\..\..\..\libs\pfl\win32" /I "..\..\..\..\libs\pfl\common\lthread" /I "..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\libs\pfl\common\file" /I "..\..\..\..\libs\pfl\common\util" /I "..\driver\shared\ndis" /I "..\..\..\pgp\win32\include" /I "..\..\..\pgp\shared" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D PGP_WIN32=1 /D PGP_DEBUG=1 /FR /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\..\pgp\win32\include" /d "_DEBUG"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 PGPsdkUI.lib wsock32.lib PGP_SDK.lib PGPsdkNL.lib pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\libs\pgpcdk\win32\PGPcdk\Debug" /libpath:"..\..\..\..\libs\pfl\win32\pflCommon\Debug" /libpath:"..\..\..\..\libs\pgpcdk\win32\PGPsdkUI\Debug" /libpath:"..\..\..\..\libs\pgpcdk\win32\PGPsdkNetworkLib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\..\..\..\libs\pgpcdk\win32\pgpcdk\debug\PGP_SDK.dll    .\debug	copy ..\..\..\..\libs\pgpcdk\win32\pgpsdkNetworkLib\debug\PGPsdkNL.dll    .\debug	copy ..\..\..\..\libs\pgpcdk\win32\pgpsdkUI\debug\PGPsdkUI.dll    .\debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "IKEservice - Win32 Release"
# Name "IKEservice - Win32 Debug"
# Begin Group "Source files"

# PROP Default_Filter "c;cpp"
# Begin Source File

SOURCE=.\shared\main.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetAppLog.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetCConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\pgpNetConfig.c
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetDebugLog.cpp

!IF  "$(CFG)" == "IKEservice - Win32 Release"

# ADD CPP /I "..\shared\ipsec"

!ELSEIF  "$(CFG)" == "IKEservice - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetDSAarray.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetGUIWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetIKEmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetIKEservice.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetIKEserviceMsg.mc

!IF  "$(CFG)" == "IKEservice - Win32 Release"

# PROP Intermediate_Dir "."
# Begin Custom Build
InputDir=.\shared
InputPath=.\shared\pgpNetIKEserviceMsg.mc

BuildCmds= \
	mc $(InputPath) -h $(InputDir) -r $(InputDir)

"$(InputDir)\pgpNetIKEserviceMsg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\pgpNetIKEserviceMsg.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\MSG00001.BIN" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "IKEservice - Win32 Debug"

# PROP Intermediate_Dir "."
# Begin Custom Build
InputDir=.\shared
InputPath=.\shared\pgpNetIKEserviceMsg.mc

BuildCmds= \
	mc $(InputPath) -h $(InputDir) -r $(InputDir)

"$(InputDir)\pgpNetIKEserviceMsg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\pgpNetIKEserviceMsg.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\MSG00001.BIN" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetIKEWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetKernelWorker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpNetLog.c
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetLogonUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetLogUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetMainWorker.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\pgpNetPaths.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpNetPrefs.c
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetQueueElement.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetReadWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetWriteWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\pgpService.cpp
# End Source File
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\shared\pgpLockable.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetAppLog.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetCConfig.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetDebugLog.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetDSAarray.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetGUIWorker.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetIKEmanager.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetIKEservice.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetIKEWorker.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetKernelWorker.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpNetLog.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetLogonUtils.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetLogUtils.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetMainWorker.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpNetPrefs.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetProcess.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetQueueElement.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetReadWorker.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetWorker.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpNetWriteWorker.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpRWLockSignal.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpService.h
# End Source File
# Begin Source File

SOURCE=.\shared\pgpSingletonDestroyer.h
# End Source File
# End Group
# Begin Group "Resource files"

# PROP Default_Filter "rc"
# Begin Source File

SOURCE=.\shared\pgpNetIKEservice.rc
# End Source File
# End Group
# End Target
# End Project
