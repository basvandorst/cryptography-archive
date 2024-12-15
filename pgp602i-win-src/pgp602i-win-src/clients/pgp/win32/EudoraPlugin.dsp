# Microsoft Developer Studio Project File - Name="EudoraPlugin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=EudoraPlugin - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EudoraPlugin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EudoraPlugin.mak" CFG="EudoraPlugin - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EudoraPlugin - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EudoraPlugin - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\EudoraPlugin\Release"
# PROP Intermediate_Dir ".\EudoraPlugin\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\shared" /I "include" /I "shared" /I "..\shared" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /D UNFINISHED_CODE_ALLOWED=0 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /Yu"windows.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ..\..\..\libs\pfl\win32\pflCommon\Release\pflCommon.lib ..\..\..\libs\pgpcdk\win32\pgpsdknetworklib\release\pgpsdknl.lib pgpcl\release\pgp60cl.lib pgpsc\release\pgp60sc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBC" /out:".\EudoraPlugin\Release\pgpEudoraPlugin.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\EudoraPlugin\Debug"
# PROP Intermediate_Dir ".\EudoraPlugin\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\shared" /I "include" /I "shared" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /Yu"windows.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 ..\..\..\libs\pfl\win32\pflCommon\Debug\pflCommon.lib ..\..\..\libs\pgpcdk\win32\pgpsdknetworklib\debug\pgpsdknl.lib pgpcl\debug\pgp60cl.lib pgpsc\debug\pgp60sc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\EudoraPlugin\Debug\pgpEudoraPlugin.dll"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "EudoraPlugin - Win32 Release"
# Name "EudoraPlugin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\shared\AddKey.c
# End Source File
# Begin Source File

SOURCE=.\shared\BlockUtils.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\CBTProc.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\CreateToolbar.c
# End Source File
# Begin Source File

SOURCE=.\shared\DecryptVerify.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\DisplayMessage.c
# End Source File
# Begin Source File

SOURCE=.\shared\EncryptSign.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraMailHeaders.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraMainWndProc.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraReadMailWndProc.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraSendMailWndProc.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\MapFile.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\MIMETYPE.CPP
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\MyMIMEUtils.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\PGPPlug.c
# ADD CPP /Yc"windows.h"
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\PGPplugin.def
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\PGPplugin.rc
# ADD BASE RSC /l 0x409 /i "EudoraPlugin"
# ADD RSC /l 0x409 /i "EudoraPlugin" /i ".\EudoraPlugin"
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\shared\Prefs.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\ReadMailToolbarWndProc.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\RFC822.CPP
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\shared\RichEdit_IO.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\SendMailToolbarWndProc.c
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\strstri.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\transbmp.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Translators.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\TranslatorUtils.c
# End Source File
# Begin Source File

SOURCE=.\shared\VerificationBlock.c
# End Source File
# Begin Source File

SOURCE=.\shared\Working.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\EudoraPlugin\Res\lock.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Res\lock1.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Res\pgp50.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Res\readtool.bmp
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Res\sendtool.bmp
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Res\sign.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Res\verify.ico
# End Source File
# End Group
# Begin Group "Project Header Files"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=.\shared\BlockUtils.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\CBTProc.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\CreateToolbar.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\DisplayMessage.h
# End Source File
# Begin Source File

SOURCE=".\EudoraPlugin\emssdk\ems-win.h"
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\ENCODING.H
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EncryptSign.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraMailHeaders.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraMainWndProc.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraReadMailWndProc.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraSendMailWndProc.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\MapFile.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\MIMETYPE.H
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\MyMIMEUtils.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\PGPDefinedMessages.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\PGPkm.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpphras.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\PGPPlugTypes.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPRecip.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpversion.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\pgpWerr.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\ReadMailToolbarWndProc.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\RFC822.H
# End Source File
# Begin Source File

SOURCE=.\shared\RichEdit_IO.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\SendMailToolbarWndProc.h
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\strstri.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\transbmp.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\TranslatorIDs.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\Translators.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\TranslatorUtils.h
# End Source File
# Begin Source File

SOURCE=.\shared\WrapBuff.h
# End Source File
# End Group
# Begin Group "Library Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pflContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\prefs\pflPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\prefs\pflPrefTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pflTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpCipherFeedback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\win32\pgpConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpEncode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpHash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpKeys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpLeaks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpMem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\win32\pgpPFLConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpPFLErrors.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPlugin\PGPPlug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpPubTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpSDKPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpSymmetricCipher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpUtilities.h
# End Source File
# End Group
# End Target
# End Project
