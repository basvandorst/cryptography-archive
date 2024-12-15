# Microsoft Developer Studio Project File - Name="pgpExch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
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
!MESSAGE "pgpExch - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpExch - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
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
# ADD LINK32 libcmt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGPsdk.lib .\PGPcmdlg\Release\PGP55cd.lib .\PGPphrase\Release\PGPphras.lib .\PGPRecip\Release\PGP55rd.lib .\PGPkm\Release\PGP55km.lib .\PGPsc\Release\PGP55sc.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\shared" /I "shared" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN=1 /D PGP_DEBUG=1 /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /FD /c
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
# ADD LINK32 libcmtd.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGPsdk.lib .\PGPcmdlg\Debug\pgp55cd.lib .\PGPRecip\Debug\pgp55rd.lib .\PGPPhrase\Debug\pgpphras.lib .\PGPkm\Debug\PGP55km.lib .\PGPsc\Debug\PGP55sc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib

!ENDIF 

# Begin Target

# Name "pgpExch - Win32 Release"
# Name "pgpExch - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\pgpExch\Attachments.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\BlockUtils.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Commands.cpp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\ExchAddKey.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\pgpExch\ExchDecryptVerify.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\pgpExch\ExchEncryptSign.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\pgpExch\ExchPrefs.cpp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\GUIDS.CPP
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\MessageEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\MsgProps.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpDebug.c
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

SOURCE=..\..\..\libs\pfl\common\pgpLeaks.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpMem.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\PropertySheets.cpp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Recipients.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\RichEdit_IO.c
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.c
# End Source File
# Begin Source File

SOURCE=.\pgpExch\STDNOTE.CPP
# End Source File
# Begin Source File

SOURCE=.\pgpExch\UIutils.cpp
# End Source File
# Begin Source File

SOURCE=.\pgpExch\Working.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\pgpExch\Exchange.h
# End Source File
# Begin Source File

SOURCE=.\pgpExch\ExchPrefs.h
# End Source File
# Begin Source File

SOURCE=.\pgpExch\GUIDS.H
# End Source File
# Begin Source File

SOURCE=.\pgpExch\outlook.h
# End Source File
# Begin Source File

SOURCE=.\pgpExch\pgpExch.h
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
# Begin Source File

SOURCE=.\pgpExch\Working.h
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
# End Group
# End Target
# End Project
