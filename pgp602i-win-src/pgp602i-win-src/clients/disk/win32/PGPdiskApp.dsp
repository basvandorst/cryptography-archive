# Microsoft Developer Studio Project File - Name="PGPdiskApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PGPdiskApp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskApp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPdiskApp.mak" CFG="PGPdiskApp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPdiskApp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPdiskApp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPdiskApp - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPdiskApp/Release"
# PROP Intermediate_Dir "PGPdiskApp/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /WX /GX /O2 /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\win32" /I "..\..\pgp\shared" /I "..\..\pgp\win32\PGPcl" /I "Shared" /I "Encryption" /I "PGPdiskApp\Source" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D PGP_DEBUG=0 /D "PGPDISK_MFC" /D "PGP_INTEL" /D "PGP_WIN32" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ole32.lib mpr.lib ..\..\..\libs\pfl\win32\pflCommon\Release\pflCommon.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGP_SDK.lib ..\..\..\libs\pgpcdk\win32\PGPsdkUI\Release\PGPsdkUI.lib ..\..\pgp\win32\PGPcl\Release\PGP60cl.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /nodefaultlib:"msvcrtd.lib" /out:"PGPdiskApp/Release/PGPdisk.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PGPdiskApp - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPdiskApp/Debug"
# PROP Intermediate_Dir "PGPdiskApp/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /WX /GX /Z7 /Od /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\pgp\shared" /I "..\..\pgp\win32\PGPcl" /I "Shared" /I "Encryption" /I "PGPdiskApp\Source" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /D PGP_DEBUG=1 /D "PGPDISK_MFC" /D "PGP_INTEL" /D "PGP_WIN32" /D "UNFINISHED_CODE_ALLOWED" /D "_AFXDLL" /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ole32.lib mpr.lib ..\..\..\libs\pfl\win32\pflCommon\Debug\pflCommon.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGP_SDK.lib ..\..\..\libs\pgpcdk\win32\PGPsdkUI\Debug\PGPsdkUI.lib ..\..\pgp\win32\PGPcl\Debug\PGP60cl.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /out:"PGPdiskApp/Debug/PGPddisk.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PGPdiskApp - Win32 Release"
# Name "PGPdiskApp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Custom Classes Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskCmdLine.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\File.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\PGPdisk.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\VolFile.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\Volume.cpp
# End Source File
# End Group
# Begin Group "App Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskApp.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppACI.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppPassphrase.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppVolumes.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppWinutils.cpp
# End Source File
# End Group
# Begin Group "Dialog Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CAboutBoxDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConfirmPassphraseDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGetUserInfoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CMainDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CNagBuyDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\COpenPGPdiskDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPublicKeyDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSavePGPdiskDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSinglePassphraseDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSplashScreenDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CUnmountPGPdiskDialog.cpp
# End Source File
# End Group
# Begin Group "Thread Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertPGPdiskThread.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CCreatePGPdiskThread.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCSearchPGPdiskThread.cpp
# End Source File
# End Group
# Begin Group "Control Class Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDriveLetterCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSecureEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSuperHotKeyControl.cpp
# End Source File
# End Group
# Begin Group "PropSheet Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPreferencesSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPrefsAutoUnmountPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPrefsUnmountHotKeyPage.cpp
# End Source File
# End Group
# Begin Group "Wizard Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizardSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizConvertPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizDonePage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizIntroPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizRandomDataPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizADKPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizardSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizCreationPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizDonePage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizIntroPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizPassphrasePage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizRandomDataPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizVolumeInfoPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizardSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizDonePage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizIntroPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizSearchPage.cpp
# End Source File
# End Group
# Begin Group "Shared Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shared\CommonStrings.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\DriverComm.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\DualErr.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\Errors.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\FatUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\LinkResolution.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskHighLevelUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskLowLevelUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPfl.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPublicKeyUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureString.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\SharedMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\StringAssociation.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\WaitObjectClasses.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\Win32Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\Shared\WindowsVersion.cpp
# End Source File
# End Group
# Begin Group "Encryption Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Encryption\Cast5.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\Cast5Box.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherContext.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\PGPdiskRandomPool.cpp
# End Source File
# Begin Source File

SOURCE=.\Encryption\SHA.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskApp\Source\StdAfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Custom Classes Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskCmdLine.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\File.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\PGPdisk.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\VolFile.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\Volume.h
# End Source File
# End Group
# Begin Group "App Class Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskApp.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppACI.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppPassphrase.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppRegistry.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppVolumes.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPGPdiskAppWinutils.h
# End Source File
# End Group
# Begin Group "Dialog Class Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CAboutBoxDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConfirmPassphraseDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGetUserInfoDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CMainDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CNagBuyDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\COpenPGPdiskDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPublicKeyDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSavePGPdiskDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSinglePassphraseDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSplashScreenDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CUnmountPGPdiskDialog.h
# End Source File
# End Group
# Begin Group "Thread Class Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertPGPdiskThread.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CCreatePGPdiskThread.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCSearchPGPdiskThread.h
# End Source File
# End Group
# Begin Group "Control Class Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDriveLetterCombo.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSecureEdit.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CSuperHotKeyControl.h
# End Source File
# End Group
# Begin Group "PropSheet Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPreferencesSheet.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPrefsAutoUnmountPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CPrefsUnmountHotKeyPage.h
# End Source File
# End Group
# Begin Group "Wizard Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizardSheet.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizConvertPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizDonePage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizIntroPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CConvertWizRandomDataPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizADKPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizardSheet.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizCreationPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizDonePage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizIntroPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizPassphrasePage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizRandomDataPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CDiskWizVolumeInfoPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizardSheet.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizDonePage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizIntroPage.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\CGlobalCWizSearchPage.h
# End Source File
# End Group
# Begin Group "Shared Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Shared\CommonStrings.h
# End Source File
# Begin Source File

SOURCE=.\Shared\DriverComm.h
# End Source File
# Begin Source File

SOURCE=.\Shared\DualErr.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Errors.h
# End Source File
# Begin Source File

SOURCE=.\Shared\FatUtils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\LinkResolution.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Packets.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskContainer.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskDefaults.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskFileFormat.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskHighLevelUtils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskLowLevelUtils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPfl.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskPublicKeyUtils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskRegistry.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskResidentDefines.h
# End Source File
# Begin Source File

SOURCE=.\Shared\PGPdiskVersion.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Required.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureMemory.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SecureString.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SharedMemory.h
# End Source File
# Begin Source File

SOURCE=.\Shared\ShlwapiWrapper.h
# End Source File
# Begin Source File

SOURCE=.\Shared\StringAssociation.h
# End Source File
# Begin Source File

SOURCE=.\Shared\SystemConstants.h
# End Source File
# Begin Source File

SOURCE=.\Shared\UtilityFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Shared\WaitObjectClasses.h
# End Source File
# Begin Source File

SOURCE=.\Shared\Win32Utils.h
# End Source File
# Begin Source File

SOURCE=.\Shared\WindowsVersion.h
# End Source File
# End Group
# Begin Group "Encryption Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Encryption\Cast5.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\Cast5Box.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherContext.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\CipherUtils.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\PGPdiskRandomPool.h
# End Source File
# Begin Source File

SOURCE=.\Encryption\SHA.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPdiskApp\Source\Globals.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\PGPdiskHelpIds.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\Resource.h
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Source\StdAfx.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPdiskApp\Res\App.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Credits1.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Credits4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Credits8.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Document.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconBar.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconCapsLock.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconMain.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconMount.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconMsgBox1.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconMsgBox2.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconNew.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconPGPdisk.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconPrefs.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\iconUnmount.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\PGPdiskApp.rc
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Prefs.ico
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Sidebar1.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Sidebar4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\Sidebar8.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\SplashScreen1.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\SplashScreen4.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPdiskApp\Res\SplashScreen8.bmp
# End Source File
# End Group
# End Target
# End Project
