# Microsoft Developer Studio Project File - Name="PGPkeys" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PGPkeys - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPkeys.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPkeys.mak" CFG="PGPkeys - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPkeys - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPkeys - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGPkeys\Release"
# PROP Intermediate_Dir ".\PGPkeys\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\shared" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /I ".\include" /I ".\include\help" /D "PGP_WIN32" /D "UNFINISHED_CODE_ALLOWED" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib treelist\release\treelist.lib pgpcl\release\pgp60cl.lib ../../../libs/pgpcdk/win32/pgpcdk/release/pgp_sdk.lib ../../../libs/pgpcdk/win32/pgpsdknetworklib/release/pgpsdknl.lib WSOCK32.lib ..\..\..\libs\pgpcdk\win32\pgpsdkUI\Release\pgpsdkUI.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGPkeys\Debug"
# PROP Intermediate_Dir ".\PGPkeys\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\shared" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /I ".\include" /I ".\include\help" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib treelist\debug\treelist.lib pgpcl\debug\pgp60cl.lib ../../../libs/pgpcdk/win32/pgpcdk/debug/pgp_sdk.lib ../../../libs/pgpcdk/win32/pgpsdknetworklib/debug/pgpsdknl.lib WSOCK32.lib ..\..\..\libs\pgpcdk\win32\pgpsdkUI\Debug\pgpsdkUI.lib /nologo /subsystem:windows /debug /machine:I386
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Executing Post-Build Commands
PostBuild_Cmds=del /F/Q pgpkeys\debug\*.dll	copy pgpcl\debug\pgp60cl.dll\
                                 pgpkeys\debug	copy  ..\..\..\libs\pgpcdk\win32\pgpcdk\debug\pgp_sdk.dll\
                        pgpkeys\debug	copy\
                 ..\..\..\libs\pgpcdk\win32\pgpsdknetworklib\debug\pgpsdknl.dll\
                                 pgpkeys\debug	copy  ..\..\..\libs\pgpcdk\win32\pgpsdkui\debug\pgpsdkui.dll\
                  pgpkeys\debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPkeys - Win32 Release"
# Name "PGPkeys - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\PGPkeys\Choice.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\ChoiceProc.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\GMIDropTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\GMMenu.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\GMMisc.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\GMNewGrp.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\GMNotPrc.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\GMOps.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\GMTree.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PGPkeys.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PGPkeys.rc
# ADD BASE RSC /l 0x409 /i "PGPkeys"
# ADD RSC /l 0x409 /i "PGPkeys" /i ".\PGPkeys"
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PKgenwiz.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PKMenu.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PKMisc.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PKMsgPrc.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PKTool.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PKUser.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Search.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\SearchCommon.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\SearchFilter.c
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\SearchProc.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\PGPkeys\Choice.h
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\ChoiceProc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pflContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pflContextPriv.h
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

SOURCE=..\..\shared\pgpAdminPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpBase.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpBuildFlags.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpCipherFeedback.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpClientPrefs.h
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

SOURCE=..\..\..\libs\pfl\common\file\pgpFileIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileIOPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecMacPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecStd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecStdPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecVTBL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileUtilities.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpHash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpIOPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpKeys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpKeyServer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpKeyServerTypes.h
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PGPkeysx.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpkm.h
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

SOURCE=.\include\pgpphras.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\Help\PGPpkHlp.h
# End Source File
# Begin Source File

SOURCE=..\libs\pgpcdk\pub\include\pgpPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpPubTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpRandomPool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpSDKPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpStdFileIOPriv.h
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

SOURCE=.\include\pgpversion.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\pgpWerr.h
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\PKVer.h
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Search.h
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\SearchCommon.h
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\SearchFilter.h
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\SearchProc.h
# End Source File
# Begin Source File

SOURCE=.\include\treelist.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPkeys\arrowcop.cur
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\arrowcop.cur
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\bmp00010.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\bmp00011.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\closelock.ico
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\images24.bmp
# End Source File
# Begin Source File

SOURCE=PGPkeys\..\shared\images24.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\images4.bmp
# End Source File
# Begin Source File

SOURCE=PGPkeys\..\shared\images4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\Key.ico
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\kgwiz1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\kgwiz24bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\kgwiz4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\kgwiz8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\openlock.ico
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\pgpkey~21.ico
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\pgpkey~31.ico
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\pgpkey~41.ico
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\push.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\pushedms.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\temp.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\Toolbar24bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPkeys\Res\Toolbar4bit.bmp
# End Source File
# End Group
# End Target
# End Project
