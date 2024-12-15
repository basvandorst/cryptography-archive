# Microsoft Developer Studio Project File - Name="EudoraPluginV4" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=EudoraPluginV4 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EudoraPluginV4.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EudoraPluginV4.mak" CFG="EudoraPluginV4 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EudoraPluginV4 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EudoraPluginV4 - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EudoraPluginV4 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\shared" /I "include" /I "shared" /I "..\shared" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /D UNFINISHED_CODE_ALLOWED=0 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pgprecip\release\pgp55rd.lib pgpphrase\release\pgpphras.lib pgpcmdlg\release\pgp55cd.lib pgpkm\release\pgp55km.lib pgpsc\release\pgp55sc.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"EudoraPluginV4/Release/EudoraPluginV4.dll"

!ELSEIF  "$(CFG)" == "EudoraPluginV4 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "EudoraPluginV4/Debug"
# PROP Intermediate_Dir "EudoraPluginV4/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\shared" /I "include" /I "shared" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pgprecip\debug\pgp55rd.lib pgpphrase\debug\pgpphras.lib pgpcmdlg\debug\pgp55cd.lib pgpkm\debug\pgp55km.lib pgpsc\debug\pgp55sc.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Performing Post-build Steps
PostBuild_Cmds=del  c:\progra~1\Eudora\plugins\EudoraPluginV4.dll	copy\
      EudoraPluginV4\debug\EudoraPluginV4.dll c:\progra~1\Eudora\plugins
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "EudoraPluginV4 - Win32 Release"
# Name "EudoraPluginV4 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp"
# Begin Source File

SOURCE=.\EudoraPluginV4\AddKey.c
# End Source File
# Begin Source File

SOURCE=.\shared\BlockUtils.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\DecryptVerify.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\DisplayMessage.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\emssdk\ENCODING.CPP
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\EncryptSign.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\MapFile.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\emssdk\MIMETYPE.CPP
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\MyMIMEUtils.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\MyPrefs.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\PGPplugin.def
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\PGPplugin.rc

!IF  "$(CFG)" == "EudoraPluginV4 - Win32 Release"

!ELSEIF  "$(CFG)" == "EudoraPluginV4 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\PluginMain.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\emssdk\RFC822.CPP
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\strstri.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Translators.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\TranslatorUtils.c
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Working.c
# End Source File
# End Group
# Begin Group "Project Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\EudoraPluginV4\AddKey.h
# End Source File
# Begin Source File

SOURCE=.\shared\BlockUtils.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\DecryptVerify.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\DisplayMessage.h
# End Source File
# Begin Source File

SOURCE=".\EudoraPluginV4\emssdk\ems-win.h"
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\emssdk\ENCODING.H
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\EncryptSign.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\MapFile.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\emssdk\Mimetype.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\MyMIMEUtils.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\MyPrefs.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPcmdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpkm.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpphras.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPRecip.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\PGPsc.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\PluginMain.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\emssdk\RFC822.H
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\TranslatorIDs.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Translators.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\TranslatorUtils.h
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Working.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;bmp"
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\addkey.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\encryptsign.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\Key.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\lock.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\lock1.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\pgp50.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\pgp_addkey.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\pgp_encrypt_sign.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\readtool.bmp
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\sendtool.bmp
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\sign.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\unlock.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraPluginV4\Res\verify.ico
# End Source File
# End Group
# Begin Group "Library Header Files"

# PROP Default_Filter "h"
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

SOURCE=..\shared\pgpBuildFlags.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpCFB.h
# End Source File
# Begin Source File

SOURCE=..\shared\pgpClientPrefs.h
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

SOURCE=..\shared\pgpGroups.h
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
# Begin Source File

SOURCE=.\include\PGPversion.h
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\pgpWerr.h
# End Source File
# End Group
# End Target
# End Project
