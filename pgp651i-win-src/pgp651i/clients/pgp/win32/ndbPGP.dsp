# Microsoft Developer Studio Project File - Name="ndbPGP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ndbPGP - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ndbPGP.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ndbPGP.mak" CFG="ndbPGP - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ndbPGP - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ndbPGP - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ndbPGP - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NotesPlugin\release"
# PROP BASE Intermediate_Dir "NotesPlugin\release"
# PROP BASE Target_Dir "NotesPlugin"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "NotesPlugin\built"
# PROP Intermediate_Dir "NotesPlugin\release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "W32" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 notes.lib NotesPlugin\built\nPGPNts.lib /nologo /version:0.0 /subsystem:windows /dll /pdb:none /machine:I386

!ELSEIF  "$(CFG)" == "ndbPGP - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "NotesPlugin\debug"
# PROP BASE Intermediate_Dir "NotesPlugin\debug"
# PROP BASE Target_Dir "NotesPlugin"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "NotesPlugin\built"
# PROP Intermediate_Dir "NotesPlugin\debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /Gi /GX /Zi /Od /Gf /Gy /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "W32" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"NotesPlugin\debug\ndbPGP.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 notes.lib NotesPlugin\built\nPGPNts.lib /nologo /version:0.0 /stack:0x400,0x100 /subsystem:windows /dll /pdb:"NotesPlugin\debug\ndbPGP.pdb" /debug /machine:I386 /out:"f:\Notes\main server\ndbPGP.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no /map
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=put dll with both Domino servers
PostBuild_Cmds=copy "f:\Notes\main server\ndbPGP.dll"\
   "f:\Notes\4.5 server\*.*"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ndbPGP - Win32 Release"
# Name "ndbPGP - Win32 Debug"
# Begin Source File

SOURCE=.\NotesPlugin\ndbPGP.c
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\ndbPGP.def
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\ndbPGP.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\ndbPGP.rc
# End Source File
# End Target
# End Project
