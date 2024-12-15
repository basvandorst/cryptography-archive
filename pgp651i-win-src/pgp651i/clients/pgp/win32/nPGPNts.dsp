# Microsoft Developer Studio Project File - Name="nPGPNts" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nPGPNts - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nPGPNts.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nPGPNts.mak" CFG="nPGPNts - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nPGPNts - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nPGPNts - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

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
# ADD CPP /nologo /Zp1 /MT /W3 /GX /O2 /I "include" /I "shared" /I "..\shared" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "W32" /D "PGP_WIN32" /FD /c
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
# ADD LINK32 user32.lib notes.lib /nologo /version:0.0 /subsystem:windows /dll /pdb:none /machine:I386

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

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
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /Gi /GX /Zi /Od /Gf /Gy /I "include" /I "shared" /I "..\shared" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "W32" /D "PGP_WIN32" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"NotesPlugin\debug\nPGPNts.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib notes.lib /nologo /version:0.0 /stack:0x400,0x100 /subsystem:windows /dll /pdb:"NotesPlugin\debug\nPGPNts.pdb" /debug /machine:I386 /out:"f:\Notes\main server\nPGPNts.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no /map
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=put dll with both Domino servers
PostBuild_Cmds=copy "f:\Notes\main server\nPGPNts.dll"\
   "f:\Notes\4.5 server\*.*"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "nPGPNts - Win32 Release"
# Name "nPGPNts - Win32 Debug"
# Begin Group "code"

# PROP Default_Filter ""
# Begin Group "shared PGP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shared\DecryptVerify.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

# ADD CPP /Zp8

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Zp8

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\DecryptVerify.h
# End Source File
# Begin Source File

SOURCE=.\shared\EncryptSign.c
# ADD CPP /Zp8
# End Source File
# Begin Source File

SOURCE=.\shared\EncryptSign.h
# End Source File
# Begin Source File

SOURCE=..\shared\pgpVersionHeader.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /Zp8

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\Prefs.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

# ADD CPP /Zp8

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Zp8

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\Prefs.h
# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

# ADD CPP /Zp8

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Zp8

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\SigEvent.h
# End Source File
# Begin Source File

SOURCE=.\shared\VerificationBlock.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

# ADD CPP /Zp8

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Zp8

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\VerificationBlock.h
# End Source File
# Begin Source File

SOURCE=.\shared\Working.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

# ADD CPP /Zp8

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Zp8

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\Working.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\NotesPlugin\ItemHandling.c
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\ItemHandling.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\LibItem.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\LibNab.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\LibObject.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\LibRichText.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\LibString.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\NabHandling.c
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\NabHandling.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\NotesApiGlobals.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\nPGPNts.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Fr

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\nPGPNts.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\ObjectHandling.c
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\ObjectHandling.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\PgpHandling.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

# ADD CPP /Zp8

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Zp8

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\PgpHandling.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\PstGlobals.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\RevealNoteHandle.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\RevealNoteHandle.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\RichTextHandling.c

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

# ADD CPP /G4 /Zp1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\RichTextHandling.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\StringHandling.c
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\StringHandling.h
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\VbCallable.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\NotesPlugin\nPGPNts.def
# End Source File
# Begin Source File

SOURCE=.\NotesPlugin\nPGPNts.rc

!IF  "$(CFG)" == "nPGPNts - Win32 Release"

!ELSEIF  "$(CFG)" == "nPGPNts - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
