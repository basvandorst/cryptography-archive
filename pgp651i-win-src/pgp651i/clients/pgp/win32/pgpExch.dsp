# Microsoft Developer Studio Project File - Name="pgpExch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pgpExch - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pgpExch.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pgpExch.mak" CFG="pgpExch - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pgpExch - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpExch - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pgpExch - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\pgpExch\Release"
# PROP Intermediate_Dir ".\pgpExch\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\shared" /I "shared" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN=1 /D PGP_DEBUG=0 /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 libcmt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib shell32.lib /nologo /base:"0x33000000" /subsystem:windows /dll /machine:I386 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cd pgpExch\Release	rename pgpExch.dll PGPexch.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\pgpExch\Debug"
# PROP Intermediate_Dir ".\pgpExch\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\include" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\shared" /I "shared" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN=1 /D PGP_DEBUG=1 /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 libcmtd.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib shell32.lib /nologo /base:"0x33000000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cd pgpExch\Debug	rename pgpExch.dll PGPexch.dll
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "pgpExch - Win32 Release"
# Name "pgpExch - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\shared\AddKey.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Attachments.cpp
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\shared\BlockUtils.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Commands.cpp
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\shared\DecryptVerify.c
# End Source File
# Begin Source File

SOURCE=.\shared\EncryptSign.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\GUIDS.CPP
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Main.cpp
# ADD CPP /Yc"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\pgpExch\MessageEvents.cpp
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\pgpExch\MsgProps.cpp
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\shared\ParseMime.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\pgpExch.DEF
# End Source File
# Begin Source File

SOURCE=.\pgpExch\pgpExch.RC
# ADD BASE RSC /l 0x409 /i "pgpExch"
# ADD RSC /l 0x409 /i "pgpExch" /i ".\pgpExch"
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# End Source File
# Begin Source File

SOURCE=.\shared\Prefs.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\PropertySheets.cpp
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Recipients.cpp
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\shared\RichEdit_IO.c
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\STDNOTE.CPP
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\pgpExch\UIutils.cpp
# ADD CPP /Yu"stdinc.h"
# End Source File
# Begin Source File

SOURCE=.\shared\VerificationBlock.c
# End Source File
# Begin Source File

SOURCE=.\shared\Working.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\pgpExch\Exchange.h
# End Source File
# Begin Source File

SOURCE=.\pgpExch\GUIDS.H
# End Source File
# Begin Source File

SOURCE=.\pgpExch\outlook.h
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Recipients.h
# End Source File
# Begin Source File

SOURCE=.\pgpExch\STDINC.H
# End Source File
# Begin Source File

SOURCE=.\pgpExch\UIutils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\pgpExch\Res\addkey.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\both.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\decrypt.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\encrypt.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\pgpkeys.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\sign.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\Tboth.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\Tdecrypt.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\Tencrypt.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\Tpgpkeys.bmp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Res\Tsign.bmp
# End Source File
# End Group
# End Target
# End Project
