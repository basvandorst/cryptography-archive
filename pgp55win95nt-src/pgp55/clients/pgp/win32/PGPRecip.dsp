# Microsoft Developer Studio Project File - Name="PGPrecip" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPrecip - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPRecip.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPRecip.mak" CFG="PGPrecip - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPrecip - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPrecip - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPrecip - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\pgprecip\Release"
# PROP Intermediate_Dir ".\pgprecip\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Release\pgpsdk.lib PGPcmdlg\Release\PGP55cd.lib PGPkm\release\PGP55km.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\pgprecip\Release/PGP55rd.dll"

!ELSEIF  "$(CFG)" == "PGPrecip - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\pgprecip\Debug"
# PROP Intermediate_Dir ".\pgprecip\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\prefs" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Debug\pgpsdk.lib PGPcmdlg\Debug\PGP55cd.lib PGPkm\Debug\PGP55km.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\pgprecip\Debug/PGP55rd.dll"

!ENDIF 

# Begin Target

# Name "PGPrecip - Win32 Release"
# Name "PGPrecip - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\PGPRecip\PGPrecip.rc
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDdrag.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDdraw.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDkeyDB.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDlstmng.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDlstsrt.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDlstsub.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDmain.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDmove.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDproc.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDserver.c
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RDwarn.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\PGPRecip\DragItem.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\KeyServe.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\ListSort.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\ListSub.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\MoveItem.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\OwnDraw.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpBuild.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpcmdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPRecip.h
# End Source File
# Begin Source File

SOURCE=.\include\pgpversion.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RecKeyDB.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\RecProc.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\strstri.h
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\WarnProc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPRecip\Res\bttn.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\buttnorm.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\buttpush.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\dianorm.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\diapush.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\dlgicon.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\dmnd.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\grbt.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\multdsa.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\multdsa.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\multiple.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\multrsa.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\unknown.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\unknown.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\valdsa.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\valdsa.ico
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\valrsa.bmp
# End Source File
# Begin Source File

SOURCE=.\PGPRecip\Res\valrsa.ico
# End Source File
# End Group
# End Target
# End Project
