# Microsoft Developer Studio Project File - Name="PGPwdNT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPwdNT - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPwdNT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPwdNT.mak" CFG="PGPwdNT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPwdNT - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPwdNT - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPwdNT - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\PGPwdNT\release"
# PROP Intermediate_Dir ".\PGPwdNT\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PGPwdNT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Gz /MTd /W3 /O2 /I "..\.." /I "$(NT4DDKROOT)\inc" /D "WIN32" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X=100 /D WINNT=1 /D DEVL=1 /D FPO=1 /D _DLL=1 /D WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /FD /Zel -cbstring /QIfdiv- /QIf /GF /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /entry:"DriverEntry@8" /dll /pdb:none /machine:I386 /nodefaultlib /out:".\PGPwdNT\release/PGPwdNT.sys" /libpath:"$(NT4DDKROOT)\lib\i386\free" /subsystem:native

!ELSEIF  "$(CFG)" == "PGPwdNT - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\PGPwdNT\debug"
# PROP Intermediate_Dir ".\PGPwdNT\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PGPwdNT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /MTd /W3 /Zi /Od /Oy /Gy /I "..\.." /I "$(NT4DDKROOT)\inc" /D "WIN32" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X=100 /D WINNT=1 /D DEVL=1 /D FPO=1 /D _DLL=1 /D WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D "_DEBUG" /D "DEBUG" /D "PGP_DEBUG" /FD /Zel -cbstring /QIfdiv- /QIf /GF /Gs /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /entry:"DriverEntry@8" /dll /pdb:none /debug /machine:I386 /nodefaultlib /out:".\PGPwdNT\debug/PGPwdNT.sys" /libpath:"$(NT4DDKROOT)\lib\i386\checked" /subsystem:native

!ENDIF 

# Begin Target

# Name "PGPwdNT - Win32 Release"
# Name "PGPwdNT - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PGPwdNT\IRP.C
# End Source File
# Begin Source File

SOURCE=.\PGPwdNT\PGPwdNT.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\PGPwdNT\PGPwdNT.h
# End Source File
# Begin Source File

SOURCE=.\PGPwdNT\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPwdNT\PGPwdNT.rc
# End Source File
# End Group
# End Target
# End Project
