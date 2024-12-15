# Microsoft Developer Studio Project File - Name="PGPadmin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PGPadmin - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPadmin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPadmin.mak" CFG="PGPadmin - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPadmin - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPadmin - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPadmin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGPadmin\Release"
# PROP Intermediate_Dir ".\PGPadmin\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\shared" /I "include" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D PGP_DEBUG=0 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Release\PGPsdk.lib PGPcmdlg\Release\PGP55cd.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "PGPadmin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGPadmin\Debug"
# PROP Intermediate_Dir ".\PGPadmin\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\shared" /I "include" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D PGP_DEBUG=1 /D USE_PGP_LEAKS=1 /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib ..\..\..\libs\pgpcdk\win32\PGPcdk\Debug\PGPsdk.lib PGPcmdlg\Debug\PGP55cd.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "PGPadmin - Win32 Release"
# Name "PGPadmin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp;rc;def"
# Begin Source File

SOURCE=.\PGPadmin\AdditionalDecryption.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\ClientPrefs.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\CopyInstaller.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\CorpKey.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\CreateWiz.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\DefaultKeys.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\FindInstaller.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Finish.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Intro.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\KeyGen.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\License.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Misc.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\PassPhrase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pflContext.c
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\PGPadmin.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\PGPadmin.rc
# ADD BASE RSC /l 0x409 /i "PGPadmin"
# ADD RSC /l 0x409 /i "PGPadmin" /i ".\PGPadmin"
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpDebug.c
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

SOURCE=..\..\..\libs\pfl\common\pgpLeaks.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\pgpMem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\pfl\common\util\pgpStrings.c
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Review.cpp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx"
# Begin Source File

SOURCE=.\PGPadmin\CopyInstaller.h
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\CreateWiz.h
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\FindInstaller.h
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\PGPadmin.h
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "bmp;ico;gif;jpg;dlg"
# Begin Source File

SOURCE=.\PGPadmin\Res\admin.ico
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Res\AdminWiz1bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Res\AdminWiz24bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Res\AdminWiz4bit.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPadmin\Res\AdminWiz8bit.bmp
# End Source File
# Begin Source File

SOURCE=PGPadmin\..\shared\images24.bmp
# End Source File
# Begin Source File

SOURCE=PGPadmin\..\shared\images4.bmp
# End Source File
# End Group
# End Target
# End Project
