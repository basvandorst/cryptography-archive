# Microsoft Developer Studio Project File - Name="PGPtools" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PGPtools - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPtools.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPtools.mak" CFG="PGPtools - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPtools - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPtools - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPtools - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGPTools\Release"
# PROP Intermediate_Dir ".\PGPTools\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /O2 /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib PGPsc\Release\PGP55sc.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PGPtools - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGPtools\Debug"
# PROP Intermediate_Dir ".\PGPtools\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib PGPsc\Debug\PGP55sc.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386

!ENDIF 

# Begin Target

# Name "PGPtools - Win32 Release"
# Name "PGPtools - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\PGPtools\main.c
# End Source File
# Begin Source File

SOURCE=.\PGPtools\openfile.c
# End Source File
# Begin Source File

SOURCE=.\PGPtools\PGPtools.rc
# ADD BASE RSC /l 0x409 /i "PGPtools"
# ADD RSC /l 0x409 /i "PGPtools" /i ".\PGPtools"
# End Source File
# Begin Source File

SOURCE=.\PGPtools\proc.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\PGPtools\openfile.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPcmdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPRecip.h
# End Source File
# Begin Source File

SOURCE=.\include\PGPsc.h
# End Source File
# Begin Source File

SOURCE=.\PGPtools\proc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPtools\res\DECRYPT.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\ENCRYPT.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\ENCSIGN.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\KEYS.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l4decver.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l4enc.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l4encsig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l4keys.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l4sig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l4wipe.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l8decver.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l8enc.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l8encsig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l8keys.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l8sig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\l8wipe.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s4decver.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s4enc.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s4encsig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s4keys.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s4sig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s4wipe.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s8decver.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s8enc.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s8encsig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s8keys.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s8sig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\s8wipe.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\SIGN.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smdec.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smenc.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smencsig.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smkeys.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smkeys2.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smsign.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smwipe.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\smwipe2.ico
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\TOOLSICO.ICO
# End Source File
# Begin Source File

SOURCE=.\PGPtools\res\WIPE.ICO
# End Source File
# End Group
# End Target
# End Project
