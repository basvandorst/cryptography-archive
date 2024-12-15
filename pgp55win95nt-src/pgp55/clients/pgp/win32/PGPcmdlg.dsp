# Microsoft Developer Studio Project File - Name="PGPcmdlg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPcmdlg - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPcmdlg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPcmdlg.mak" CFG="PGPcmdlg - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPcmdlg - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcmdlg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGPcmdlg\Release"
# PROP Intermediate_Dir ".\PGPcmdlg\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\sorting" /I "..\shared" /I ".\include" /D PGP_DEBUG=0 /D "PGP_WIN32" /D "UNFINISHED_CODE_ALLOWED" /D "_PGPCDDLL" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Release\pgpsdk.lib treelist\release\treelist.lib ..\..\..\libs\pgpcdk\win32\pgpcdkkeyserver\release\pgpsdkks.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\PGPcmdlg\Release/PGP55cd.dll"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGPcmdlg\Debug"
# PROP Intermediate_Dir ".\PGPcmdlg\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\sorting" /I "..\shared" /I ".\include" /D "_DEBUG" /D PGP_DEBUG=1 /D "PGP_WIN32" /D "UNFINISHED_CODE_ALLOWED" /D "_PGPCDDLL" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Debug\pgpsdk.lib treelist\debug\treelist.lib ..\..\..\libs\pgpcdk\win32\pgpcdkkeyserver\debug\pgpsdkks.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\PGPcmdlg\Debug/PGP55cd.dll"

!ENDIF 

# Begin Target

# Name "PGPcmdlg - Win32 Release"
# Name "PGPcmdlg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\PGPcmdlg\CDabout.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDcheck.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDerror.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDexpire.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDgroups.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDkspref.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDmisc.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDphrase.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDprefs.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDrand.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDserver.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\CDsplash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pflContext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\prefs\pflPrefs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\prefs\pflPrefTypes.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpAdminPrefs.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpClientPrefs.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\PGPcmdlg.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\PGPcmdlg.def
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\PGPcmdlg.rc

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"

# ADD BASE RSC /l 0x409 /i "PGPcmdlg"
# ADD RSC /l 0x409 /i "PGPcmdlg" /i ".\PGPcmdlg"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"

# ADD BASE RSC /l 0x409 /i "PGPcmdlg"
# ADD RSC /l 0x409 /i "PGPcmdlg" /i ".\PGPcmdlg"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\util\pgpEndianConversion.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileIO.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecStd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecVTBL.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileSpecWin32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpFileUtilities.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpGroups.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpGroupsUtil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpIO.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpKeyServerPrefs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpLeaks.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpMem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpMemoryIO.c
# End Source File
# Begin Source File

SOURCE=.\PGPphrase\PGPphras.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\file\pgpStdFileIO.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\util\pgpStrings.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\util\pgpWordWrap.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\TimeDate.c
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\TimeDateWndProc.c
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

SOURCE=.\PGPcmdlg\PGPcdlgx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pgpcdk\pub\include\pgpCipherFeedback.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\pgpClientPrefs.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPcmdlg.h
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

SOURCE=.\PGPcmdlg\TimeDateWndProc.h
# End Source File
# Begin Source File

SOURCE=.\include\treelist.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPcmdlg\Res\bizsplash1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\bizsplash4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\bizsplash8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\CDimag24.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\CDimag4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\chek_ci.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\chek_cm.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\chek_ui.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\chek_um.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\credits1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\credits4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\credits8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\domain_i.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\domain_m.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\res\eval1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\res\eval4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\res\eval8bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\freesplash1bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\freesplash4bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\freesplash8bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\http_i.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\http_m.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\images24.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\images4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\ldap_i.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\ldap_m.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\nagsplash1bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\nagsplash4bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\nagsplash8bit.BMP
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\splash1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\splash4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPcmdlg\Res\splash8bit.bmp
# End Source File
# End Group
# End Target
# End Project
