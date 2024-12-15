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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\win32" /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Release\pgpsdk.lib PGPcmdlg\Release\PGP55cd.lib PGPphrase\Release\PGPphras.lib PGPkm\Release\PGP55km.lib PGPrecip\Release\PGP55rd.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\PGPsc\Release/PGP55sc.dll"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\libs\pgpcdk\win32\pgpcdk\Debug\pgpsdk.lib PGPphrase\Debug\PGPphras.lib PGPcmdlg\Debug\PGP55cd.lib PGPkm\Debug\PGP55km.lib PGPrecip\Debug\PGP55rd.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\PGPsc\Debug/PGP55sc.dll"

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

SOURCE=.\PGPsc\PGPsc.rc
# ADD BASE RSC /l 0x409 /i "PGPsc"
# ADD RSC /l 0x409 /i "PGPsc" /i ".\PGPsc"
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c
# End Source File
# Begin Source File

SOURCE=.\PGPsc\ProgVal.c
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

SOURCE=.\PGPsc\Utils.c
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

SOURCE=.\PGPsc\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\PGP.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\PRVKR.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\PUBKR.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPsc\res\SIG.ICO
# End Source File
# End Group
# End Target
# End Project
