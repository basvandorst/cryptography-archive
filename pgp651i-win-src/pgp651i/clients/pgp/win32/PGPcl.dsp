# Microsoft Developer Studio Project File - Name="PGPcl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPcl - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPcl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPcl.mak" CFG="PGPcl - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPcl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPcl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGPcl\Release"
# PROP Intermediate_Dir ".\PGPcl\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\win32\pgpAsyncDNS" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\sorting" /I "..\shared" /I ".\include" /I "..\shared\JPEG\common" /I "..\..\..\libs\pfl\win32" /D PGP_DEBUG=0 /D "PGP_WIN32" /D "_PGPCLDLL" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /D _WIN32_IE=0x0300 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x20000000" /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "PGPcl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGPcl\Debug"
# PROP Intermediate_Dir ".\PGPcl\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32\pgpAsyncDNS" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\sorting" /I "..\shared" /I ".\include" /I "..\shared\JPEG\common" /I "..\..\..\libs\pfl\win32" /D "_DEBUG" /D PGP_DEBUG=1 /D "PGP_WIN32" /D "_PGPCLDLL" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /D _WIN32_IE=0x0300 /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x20000000" /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /verbose /nodefaultlib

!ENDIF 

# Begin Target

# Name "PGPcl - Win32 Release"
# Name "PGPcl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\PGPcl\CLabout.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLavlist.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLcache.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLerror.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLexpire.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLgroups.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLkspref.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLmisc.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLphrase.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLprefs.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLquery.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLrand.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLrecip.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLrecon.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLsdksrv.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLsel509.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLselect.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLsend.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLserver.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLsplash.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLsrvprf.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\CLwarn.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMAddUsr.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMChange.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMColumn.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMConvrt.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMFocus.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMIDataObject.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMIDropSource.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMIDropTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMIEnumFormatEtc.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMKeyIO.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMKeyOps.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMMenu.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMMisc.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMNotPrc.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMPhoto.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMProps.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMRevoke.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMserver.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMShare.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMTree.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\KMUser.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\PGPcl.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\PGPcl.def
# End Source File
# Begin Source File

SOURCE=.\PGPcl\PGPcl.rc
# ADD BASE RSC /l 0x409 /i "PGPcl"
# ADD RSC /l 0x409 /i "PGPcl" /i ".\PGPcl"
# End Source File
# Begin Source File

SOURCE=..\shared\pgpClientErrors.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpHashWords.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpKeyServerPrefs.c
# End Source File
# Begin Source File

SOURCE=.\PGPcl\PGPkm.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpShare.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpShareFile.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpSKEP.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpX509Utils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
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

SOURCE=.\include\pgpbuild.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpBuildFlags.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\Help\PGPcdHlp.h
# End Source File
# Begin Source File

SOURCE=.\PGPcl\PGPcdlgx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpCipherFeedback.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPcl.h
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

SOURCE=..\..\shared\pgpKeyServerPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpKeyServerTypes.h
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

SOURCE=.\PGPphrase\PGPphrsx.h
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

SOURCE=..\..\..\libs\pfl\common\file\pgpStdFileIO.h
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

SOURCE=.\include\PGPversion.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\pgpWerr.h
# End Source File
# Begin Source File

SOURCE=.\PGPcl\TimeDateWndProc.h
# End Source File
# Begin Source File

SOURCE=.\include\treelist.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\shared\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\bizsplash1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\bizsplash4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\bizsplash8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\shared\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\CDimag24.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\CDimag4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\CLimag24.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\CLimag4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\credits1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\credits4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\credits8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\res\eval1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\res\eval4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\res\eval8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\expired.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\freesplash1bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\freesplash4bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\freesplash8bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\id1.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\images24.bmp
# End Source File
# Begin Source File

SOURCE=.\shared\images24.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\images4.bmp
# End Source File
# Begin Source File

SOURCE=.\shared\images4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\Key.ico
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\nagsplash1bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\nagsplash4bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\nagsplash8bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\notavail.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\qmark.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\revoked.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\splash1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\splash4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcl\Res\splash8bit.bmp
# End Source File
# End Group
# End Target
# End Project
