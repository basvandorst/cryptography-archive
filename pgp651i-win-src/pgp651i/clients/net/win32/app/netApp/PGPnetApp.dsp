# Microsoft Developer Studio Project File - Name="PGPnetApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PGPnetApp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPnetApp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPnetApp.mak" CFG="PGPnetApp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPnetApp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPnetApp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPnetApp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\shared" /I "..\..\shared" /I "..\..\..\shared" /I "..\..\..\..\..\libs\pfl\win32" /I "..\..\..\..\..\libs\pfl\common" /I "..\..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\..\libs\pgpcdk\win32" /I "..\..\..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\..\..\libs\pfl\common\file" /I "..\..\..\..\..\libs\pfl\common\util" /I "..\..\..\..\pgp\win32\include" /I "..\..\..\..\pgp\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PGP_WIN32" /D PGP_DEBUG=0 /D _WIN32_IE=0x0300 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\..\..\..\pgp\win32\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /machine:I386 /out:".\Release/PGPnet.exe"

!ELSEIF  "$(CFG)" == "PGPnetApp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\shared" /I "..\..\shared" /I "..\..\..\shared" /I "..\..\..\..\..\libs\pfl\win32" /I "..\..\..\..\..\libs\pfl\common" /I "..\..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\..\libs\pgpcdk\win32" /I "..\..\..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\..\..\libs\pfl\common\file" /I "..\..\..\..\..\libs\pfl\common\util" /I "..\..\..\..\pgp\win32\include" /I "..\..\..\..\pgp\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "PGP_WIN32" /D PGP_DEBUG=1 /D _WIN32_IE=0x0300 /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\..\..\pgp\win32\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /out:".\Debug/PGPnet.exe" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                                ..\..\..\..\..\libs\pgpcdk\win32\pgpcdk\debug\PGP_SDK.dll .\debug	copy                                ..\..\..\..\pgp\win32\pgpcl\debug\PGPcl.dll .\debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPnetApp - Win32 Release"
# Name "PGPnetApp - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "*.c;*.rc"
# Begin Source File

SOURCE=.\DurationControl.c
# End Source File
# Begin Source File

SOURCE=.\IPAddressControl.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\pgpIkeAlerts.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecErrors.c
# End Source File
# Begin Source File

SOURCE=.\PGPnetApp.c
# End Source File
# Begin Source File

SOURCE=.\PGPnetApp.rc
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpNetConfig.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\pgpNetLog.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpNetPaths.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\pgpNetPrefs.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpNetServiceErrors.c
# End Source File
# Begin Source File

SOURCE=.\PNaddwiz.c
# End Source File
# Begin Source File

SOURCE=.\PNadvanced.c
# End Source File
# Begin Source File

SOURCE=.\PNedit.c
# End Source File
# Begin Source File

SOURCE=.\PNhosts.c
# End Source File
# Begin Source File

SOURCE=.\PNlog.c
# End Source File
# Begin Source File

SOURCE=.\PNmisc.c
# End Source File
# Begin Source File

SOURCE=.\PNmsgprc.c
# End Source File
# Begin Source File

SOURCE=.\PNoption.c
# End Source File
# Begin Source File

SOURCE=.\PNreport.c
# End Source File
# Begin Source File

SOURCE=.\PNselect.c
# End Source File
# Begin Source File

SOURCE=.\PNstatus.c
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*.ico;*.bmp"
# Begin Source File

SOURCE=.\res\addwiz1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\res\addwiz4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\res\addwiz8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\key.ico
# End Source File
# Begin Source File

SOURCE=.\res\KeyImg24.bmp
# End Source File
# Begin Source File

SOURCE=.\res\KeyImg4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pnicon.ico
# End Source File
# Begin Source File

SOURCE=.\res\PNimag24.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PNimag4.bmp
# End Source File
# End Group
# End Target
# End Project
