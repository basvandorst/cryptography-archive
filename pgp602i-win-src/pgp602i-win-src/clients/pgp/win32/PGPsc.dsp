# Microsoft Developer Studio Project File - Name="PGPsc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPsc - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPsc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPsc.mak" CFG="PGPsc - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPsc - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPsc - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPsc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGPsc\Release"
# PROP Intermediate_Dir ".\PGPsc\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\shared" /I "include" /I "shared" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /D PGP_DEBUG=0 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Release\pgp_sdk.lib PGPcl\Release\PGP60cl.lib ..\..\..\libs\pgpcdk\win32\pgpsdkUI\Release\pgpsdkUI.lib ..\..\..\libs\pfl\win32\pflCommon\Release\pflCommon.lib ..\..\..\libs\pgpcdk\win32\pgpsdknetworklib\Release\pgpsdknl.lib PGPhk\Release\PGP60hk.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\PGPsc\Release/PGP60sc.dll"

!ELSEIF  "$(CFG)" == "PGPsc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGPsc\Debug"
# PROP Intermediate_Dir ".\PGPsc\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\shared" /I "include" /I "shared" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /D PGP_DEBUG=1 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Debug\pgp_sdk.lib PGPcl\Debug\PGP60cl.lib ..\..\..\libs\pgpcdk\win32\pgpsdkUI\Debug\pgpsdkUI.lib ..\..\..\libs\pfl\win32\pflCommon\Debug\pflCommon.lib ..\..\..\libs\pgpcdk\win32\pgpsdknetworklib\Debug\pgpsdknl.lib PGPhk\Debug\PGP60hk.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\PGPsc\Debug/PGP60sc.dll"

!ENDIF 

# Begin Target

# Name "PGPsc - Win32 Release"
# Name "PGPsc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\PGPsc\AddKey.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\Clipbrd.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\ClVwClip.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\DDElog.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\decrypt.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\encrypt.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\events.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\main.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpDiskWiper.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\pgpsc.def
# End Source File
# Begin Source File

SOURCE=.\PGPsc\PGPsc.rc
# ADD BASE RSC /l 0x409 /i "PGPsc"
# ADD RSC /l 0x409 /i "PGPsc" /i ".\PGPsc"
# End Source File
# Begin Source File

SOURCE=..\shared\pgpShare.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpShareFile.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\recpass.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\SaveOpen.c
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\Tempest.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\Utils.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\WFfat.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\WFnt.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\WFwiz.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\wipe.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\working.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\PGPsc\Clipbrd.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\ClVwClip.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\events.h
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

SOURCE=..\shared\pgpAdminPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpBase.h
# End Source File
# Begin Source File

SOURCE=..\shared\pgpBuildFlags.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPcl.h
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

SOURCE=..\shared\pgpDiskWiper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpEncode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpGroups.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpHash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpKeys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpKeyServer.h
# End Source File
# Begin Source File

SOURCE=..\shared\pgpKeyServerPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpMemoryMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpOptionList.h
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

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpPubTypes.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\PGPsc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpSDKPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpSymmetricCipher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpTLS.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpUserInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpUtilities.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpversion.h
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\pgpWerr.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\Precomp.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\ProgVal.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\recpass.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\saveopen.h
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\WFfat.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\WFnt.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\WFwiz.h
# End Source File
# Begin Source File

SOURCE=.\PGPsc\working.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPsc\res\AEXPK.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\ASC.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\fyeo.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\grp.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\ma13x24.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\PGP.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\pgr.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\PRVKR.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\PUBKR.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\rnd.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\shf.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\SIG.ICO
# End Source File
# End Group
# End Target
# End Project
