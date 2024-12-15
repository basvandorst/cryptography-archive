# Microsoft Developer Studio Project File - Name="OutlookExpress" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=OutlookExpress - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OutlookExpress.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OutlookExpress.mak" CFG="OutlookExpress - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OutlookExpress - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "OutlookExpress - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OutlookExpress - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\OutlookExpress\Release"
# PROP Intermediate_Dir ".\OutlookExpress\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\shared" /I "shared" /D UNFINISHED_CODE_ALLOWED=0 /D PGP_WIN=1 /D PGP_DEBUG=0 /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /Yu"windows.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i ".\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x32000000" /subsystem:windows /dll /machine:I386 /out:".\OutlookExpress\Release/PGPoe.dll"

!ELSEIF  "$(CFG)" == "OutlookExpress - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\OutlookExpress\Debug"
# PROP Intermediate_Dir ".\OutlookExpress\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\include" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\shared" /I "shared" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN=1 /D PGP_DEBUG=1 /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /Yu"windows.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i ".\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x32000000" /subsystem:windows /dll /debug /machine:I386 /out:".\OutlookExpress\Debug/PGPoe.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "OutlookExpress - Win32 Release"
# Name "OutlookExpress - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.c;*.cpp;*.rc;*.def"
# Begin Source File

SOURCE=.\shared\AddKey.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\shared\BlockUtils.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\CommonWndProc.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\DecryptVerify.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\shared\EncryptSign.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\main.cpp
# ADD CPP /Yc"windows.h"
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\MainWndProc.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\ParseMime.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\PGPoe.rc
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\PluginInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\Prefs.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\ReadMsgWndProc.cpp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Recipients.cpp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\SendMsgWndProc.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\TextIO.cpp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\UIutils.cpp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\UnknownWndProc.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\VerificationBlock.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\shared\Working.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h;*.hpp"
# Begin Source File

SOURCE=.\OutlookExpress\HookProcs.h
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\PluginInfo.h
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Recipients.h
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\TextIO.h
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\UIutils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "*.bmp;*.ico"
# Begin Source File

SOURCE=.\OutlookExpress\Res\decl.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\decl4.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\decoff.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\decoff4.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\decrypt.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\encl.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\encl4.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\encoff.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\encoff4.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\encrypt.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\pgpkeys.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\pgpkeysl.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\pgpkeyso.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\pkl4.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\pko4.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\sign.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\signl.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\signl4.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\signoff.bmp
# End Source File
# Begin Source File

SOURCE=.\OutlookExpress\Res\signoff4.bmp
# End Source File
# End Group
# End Target
# End Project
