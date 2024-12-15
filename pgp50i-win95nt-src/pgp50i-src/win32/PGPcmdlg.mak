# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=PGPcmdlg - Win32 Eval_Debug
!MESSAGE No configuration specified.  Defaulting to PGPcmdlg - Win32\
 Eval_Debug.
!ENDIF 

!IF "$(CFG)" != "PGPcmdlg - Win32 Release" && "$(CFG)" !=\
 "PGPcmdlg - Win32 Debug" && "$(CFG)" != "PGPcmdlg - Win32 Freeware_Debug" &&\
 "$(CFG)" != "PGPcmdlg - Win32 Freeware_Release" && "$(CFG)" !=\
 "PGPcmdlg - Win32 Eval_Release" && "$(CFG)" != "PGPcmdlg - Win32 Eval_Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPcmdlg.mak" CFG="PGPcmdlg - Win32 Eval_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPcmdlg - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcmdlg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcmdlg - Win32 Freeware_Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcmdlg - Win32 Freeware_Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcmdlg - Win32 Eval_Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcmdlg - Win32 Eval_Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "PGPcmdlg - Win32 Eval_Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPcmdlg\Release"
# PROP Intermediate_Dir "PGPcmdlg\Release"
# PROP Target_Dir ""
OUTDIR=.\PGPcmdlg\Release
INTDIR=.\PGPcmdlg\Release

ALL : "$(OUTDIR)\PGPcmdlg.dll"

CLEAN : 
	-@erase "$(INTDIR)\PGPads.obj"
	-@erase "$(INTDIR)\PGPcheck.obj"
	-@erase "$(INTDIR)\PGPcomdlg.obj"
	-@erase "$(INTDIR)\PGPcomdlg.res"
	-@erase "$(INTDIR)\PGPerror.obj"
	-@erase "$(INTDIR)\PGPexpire.obj"
	-@erase "$(INTDIR)\PGPhelpabout.obj"
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(INTDIR)\PGPprefs.obj"
	-@erase "$(INTDIR)\PGPrand.obj"
	-@erase "$(INTDIR)\PGPsplash.obj"
	-@erase "$(OUTDIR)\PGPcmdlg.dll"
	-@erase "$(OUTDIR)\PGPcmdlg.exp"
	-@erase "$(OUTDIR)\PGPcmdlg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPcmdlg.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPcmdlg\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPcmdlg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib\
 keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/PGPcmdlg.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPcmdlg.dll" /implib:"$(OUTDIR)/PGPcmdlg.lib" 
LINK32_OBJS= \
	"$(INTDIR)\PGPads.obj" \
	"$(INTDIR)\PGPcheck.obj" \
	"$(INTDIR)\PGPcomdlg.obj" \
	"$(INTDIR)\PGPcomdlg.res" \
	"$(INTDIR)\PGPerror.obj" \
	"$(INTDIR)\PGPexpire.obj" \
	"$(INTDIR)\PGPhelpabout.obj" \
	"$(INTDIR)\PGPphrase.obj" \
	"$(INTDIR)\PGPprefs.obj" \
	"$(INTDIR)\PGPrand.obj" \
	"$(INTDIR)\PGPsplash.obj"

"$(OUTDIR)\PGPcmdlg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPcmdlg\Debug"
# PROP Intermediate_Dir "PGPcmdlg\Debug"
# PROP Target_Dir ""
OUTDIR=.\PGPcmdlg\Debug
INTDIR=.\PGPcmdlg\Debug

ALL : "$(OUTDIR)\PGPcmdlg.dll"

CLEAN : 
	-@erase "$(INTDIR)\PGPads.obj"
	-@erase "$(INTDIR)\PGPcheck.obj"
	-@erase "$(INTDIR)\PGPcomdlg.obj"
	-@erase "$(INTDIR)\PGPcomdlg.res"
	-@erase "$(INTDIR)\PGPerror.obj"
	-@erase "$(INTDIR)\PGPexpire.obj"
	-@erase "$(INTDIR)\PGPhelpabout.obj"
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(INTDIR)\PGPprefs.obj"
	-@erase "$(INTDIR)\PGPrand.obj"
	-@erase "$(INTDIR)\PGPsplash.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PGPcmdlg.dll"
	-@erase "$(OUTDIR)\PGPcmdlg.exp"
	-@erase "$(OUTDIR)\PGPcmdlg.ilk"
	-@erase "$(OUTDIR)\PGPcmdlg.lib"
	-@erase "$(OUTDIR)\PGPcmdlg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPcmdlg.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPcmdlg\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPcmdlg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib\
 keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/PGPcmdlg.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/PGPcmdlg.dll" /implib:"$(OUTDIR)/PGPcmdlg.lib" 
LINK32_OBJS= \
	"$(INTDIR)\PGPads.obj" \
	"$(INTDIR)\PGPcheck.obj" \
	"$(INTDIR)\PGPcomdlg.obj" \
	"$(INTDIR)\PGPcomdlg.res" \
	"$(INTDIR)\PGPerror.obj" \
	"$(INTDIR)\PGPexpire.obj" \
	"$(INTDIR)\PGPhelpabout.obj" \
	"$(INTDIR)\PGPphrase.obj" \
	"$(INTDIR)\PGPprefs.obj" \
	"$(INTDIR)\PGPrand.obj" \
	"$(INTDIR)\PGPsplash.obj"

"$(OUTDIR)\PGPcmdlg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPcmdl0"
# PROP BASE Intermediate_Dir "PGPcmdl0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPcmdlg\FreewareDebug"
# PROP Intermediate_Dir "PGPcmdlg\FreewareDebug"
# PROP Target_Dir ""
OUTDIR=.\PGPcmdlg\FreewareDebug
INTDIR=.\PGPcmdlg\FreewareDebug

ALL : "$(OUTDIR)\PGPcmdlg.dll"

CLEAN : 
	-@erase "$(INTDIR)\PGPads.obj"
	-@erase "$(INTDIR)\PGPcheck.obj"
	-@erase "$(INTDIR)\PGPcomdlg.obj"
	-@erase "$(INTDIR)\PGPcomdlg.res"
	-@erase "$(INTDIR)\PGPerror.obj"
	-@erase "$(INTDIR)\PGPexpire.obj"
	-@erase "$(INTDIR)\PGPhelpabout.obj"
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(INTDIR)\PGPprefs.obj"
	-@erase "$(INTDIR)\PGPrand.obj"
	-@erase "$(INTDIR)\PGPsplash.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PGPcmdlg.dll"
	-@erase "$(OUTDIR)\PGPcmdlg.exp"
	-@erase "$(OUTDIR)\PGPcmdlg.ilk"
	-@erase "$(OUTDIR)\PGPcmdlg.lib"
	-@erase "$(OUTDIR)\PGPcmdlg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "PGP_FREEWARE" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "PGP_FREEWARE" /Fp"$(INTDIR)/PGPcmdlg.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPcmdlg\FreewareDebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "PGP_FREEWARE"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /d "_DEBUG" /d "PGP_FREEWARE" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPcmdlg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib\
 keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/PGPcmdlg.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/PGPcmdlg.dll" /implib:"$(OUTDIR)/PGPcmdlg.lib" 
LINK32_OBJS= \
	"$(INTDIR)\PGPads.obj" \
	"$(INTDIR)\PGPcheck.obj" \
	"$(INTDIR)\PGPcomdlg.obj" \
	"$(INTDIR)\PGPcomdlg.res" \
	"$(INTDIR)\PGPerror.obj" \
	"$(INTDIR)\PGPexpire.obj" \
	"$(INTDIR)\PGPhelpabout.obj" \
	"$(INTDIR)\PGPphrase.obj" \
	"$(INTDIR)\PGPprefs.obj" \
	"$(INTDIR)\PGPrand.obj" \
	"$(INTDIR)\PGPsplash.obj"

"$(OUTDIR)\PGPcmdlg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPcmdl1"
# PROP BASE Intermediate_Dir "PGPcmdl1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPcmdlg\FreewareRelease"
# PROP Intermediate_Dir "PGPcmdlg\FreewareRelease"
# PROP Target_Dir ""
OUTDIR=.\PGPcmdlg\FreewareRelease
INTDIR=.\PGPcmdlg\FreewareRelease

ALL : "$(OUTDIR)\PGPcmdlg.dll"

CLEAN : 
	-@erase "$(INTDIR)\PGPads.obj"
	-@erase "$(INTDIR)\PGPcheck.obj"
	-@erase "$(INTDIR)\PGPcomdlg.obj"
	-@erase "$(INTDIR)\PGPcomdlg.res"
	-@erase "$(INTDIR)\PGPerror.obj"
	-@erase "$(INTDIR)\PGPexpire.obj"
	-@erase "$(INTDIR)\PGPhelpabout.obj"
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(INTDIR)\PGPprefs.obj"
	-@erase "$(INTDIR)\PGPrand.obj"
	-@erase "$(INTDIR)\PGPsplash.obj"
	-@erase "$(OUTDIR)\PGPcmdlg.dll"
	-@erase "$(OUTDIR)\PGPcmdlg.exp"
	-@erase "$(OUTDIR)\PGPcmdlg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PGP_FREEWARE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "PGP_FREEWARE" /Fp"$(INTDIR)/PGPcmdlg.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPcmdlg\FreewareRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "PGP_FREEWARE"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /d "NDEBUG" /d "PGP_FREEWARE" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPcmdlg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib\
 keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/PGPcmdlg.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPcmdlg.dll" /implib:"$(OUTDIR)/PGPcmdlg.lib" 
LINK32_OBJS= \
	"$(INTDIR)\PGPads.obj" \
	"$(INTDIR)\PGPcheck.obj" \
	"$(INTDIR)\PGPcomdlg.obj" \
	"$(INTDIR)\PGPcomdlg.res" \
	"$(INTDIR)\PGPerror.obj" \
	"$(INTDIR)\PGPexpire.obj" \
	"$(INTDIR)\PGPhelpabout.obj" \
	"$(INTDIR)\PGPphrase.obj" \
	"$(INTDIR)\PGPprefs.obj" \
	"$(INTDIR)\PGPrand.obj" \
	"$(INTDIR)\PGPsplash.obj"

"$(OUTDIR)\PGPcmdlg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPcmdl0"
# PROP BASE Intermediate_Dir "PGPcmdl0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPcmdlg\EvalRelease"
# PROP Intermediate_Dir "PGPcmdlg\EvalRelease"
# PROP Target_Dir ""
OUTDIR=.\PGPcmdlg\EvalRelease
INTDIR=.\PGPcmdlg\EvalRelease

ALL : "$(OUTDIR)\PGPcmdlg.dll"

CLEAN : 
	-@erase "$(INTDIR)\PGPads.obj"
	-@erase "$(INTDIR)\PGPcheck.obj"
	-@erase "$(INTDIR)\PGPcomdlg.obj"
	-@erase "$(INTDIR)\PGPcomdlg.res"
	-@erase "$(INTDIR)\PGPerror.obj"
	-@erase "$(INTDIR)\PGPexpire.obj"
	-@erase "$(INTDIR)\PGPhelpabout.obj"
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(INTDIR)\PGPprefs.obj"
	-@erase "$(INTDIR)\PGPrand.obj"
	-@erase "$(INTDIR)\PGPsplash.obj"
	-@erase "$(OUTDIR)\PGPcmdlg.dll"
	-@erase "$(OUTDIR)\PGPcmdlg.exp"
	-@erase "$(OUTDIR)\PGPcmdlg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D EVAL_TIMEOUT=30 /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 EVAL_TIMEOUT=30 /Fp"$(INTDIR)/PGPcmdlg.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPcmdlg\EvalRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d EVAL_TIMEOUT=30
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /d "NDEBUG" /d EVAL_TIMEOUT=30 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPcmdlg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib\
 keyserversupport\release\keyserversupport.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/PGPcmdlg.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPcmdlg.dll" /implib:"$(OUTDIR)/PGPcmdlg.lib" 
LINK32_OBJS= \
	"$(INTDIR)\PGPads.obj" \
	"$(INTDIR)\PGPcheck.obj" \
	"$(INTDIR)\PGPcomdlg.obj" \
	"$(INTDIR)\PGPcomdlg.res" \
	"$(INTDIR)\PGPerror.obj" \
	"$(INTDIR)\PGPexpire.obj" \
	"$(INTDIR)\PGPhelpabout.obj" \
	"$(INTDIR)\PGPphrase.obj" \
	"$(INTDIR)\PGPprefs.obj" \
	"$(INTDIR)\PGPrand.obj" \
	"$(INTDIR)\PGPsplash.obj"

"$(OUTDIR)\PGPcmdlg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPcmdl1"
# PROP BASE Intermediate_Dir "PGPcmdl1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPcmdlg\EvalDebug"
# PROP Intermediate_Dir "PGPcmdlg\EvalDebug"
# PROP Target_Dir ""
OUTDIR=.\PGPcmdlg\EvalDebug
INTDIR=.\PGPcmdlg\EvalDebug

ALL : "$(OUTDIR)\PGPcmdlg.dll"

CLEAN : 
	-@erase "$(INTDIR)\PGPads.obj"
	-@erase "$(INTDIR)\PGPcheck.obj"
	-@erase "$(INTDIR)\PGPcomdlg.obj"
	-@erase "$(INTDIR)\PGPcomdlg.res"
	-@erase "$(INTDIR)\PGPerror.obj"
	-@erase "$(INTDIR)\PGPexpire.obj"
	-@erase "$(INTDIR)\PGPhelpabout.obj"
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(INTDIR)\PGPprefs.obj"
	-@erase "$(INTDIR)\PGPrand.obj"
	-@erase "$(INTDIR)\PGPsplash.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PGPcmdlg.dll"
	-@erase "$(OUTDIR)\PGPcmdlg.exp"
	-@erase "$(OUTDIR)\PGPcmdlg.ilk"
	-@erase "$(OUTDIR)\PGPcmdlg.lib"
	-@erase "$(OUTDIR)\PGPcmdlg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D EVAL_TIMEOUT=30 /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D EVAL_TIMEOUT=30 /Fp"$(INTDIR)/PGPcmdlg.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPcmdlg\EvalDebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d EVAL_TIMEOUT=30
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /d "_DEBUG" /d EVAL_TIMEOUT=30 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPcmdlg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\bn.lib lib\simple.lib lib\keydb.lib\
 keyserversupport\debug\keyserversupport.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/PGPcmdlg.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/PGPcmdlg.dll" /implib:"$(OUTDIR)/PGPcmdlg.lib" 
LINK32_OBJS= \
	"$(INTDIR)\PGPads.obj" \
	"$(INTDIR)\PGPcheck.obj" \
	"$(INTDIR)\PGPcomdlg.obj" \
	"$(INTDIR)\PGPcomdlg.res" \
	"$(INTDIR)\PGPerror.obj" \
	"$(INTDIR)\PGPexpire.obj" \
	"$(INTDIR)\PGPhelpabout.obj" \
	"$(INTDIR)\PGPphrase.obj" \
	"$(INTDIR)\PGPprefs.obj" \
	"$(INTDIR)\PGPrand.obj" \
	"$(INTDIR)\PGPsplash.obj"

"$(OUTDIR)\PGPcmdlg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "PGPcmdlg - Win32 Release"
# Name "PGPcmdlg - Win32 Debug"
# Name "PGPcmdlg - Win32 Freeware_Debug"
# Name "PGPcmdlg - Win32 Freeware_Release"
# Name "PGPcmdlg - Win32 Eval_Release"
# Name "PGPcmdlg - Win32 Eval_Debug"

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPsplash.c
DEP_CPP_PGPSP=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPsplash.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPsplash.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPsplash.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPsplash.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPsplash.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPsplash.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPcomdlg.c
DEP_CPP_PGPCO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgVer.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPcomdlg.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPcomdlg.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPcomdlg.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPcomdlg.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPcomdlg.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPcomdlg.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPerror.c
DEP_CPP_PGPER=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\..\include\spgp.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPerror.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPerror.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPerror.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPerror.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPerror.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPerror.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPexpire.c

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"

DEP_CPP_PGPEX=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPexpire.obj" : $(SOURCE) $(DEP_CPP_PGPEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"

DEP_CPP_PGPEX=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPexpire.obj" : $(SOURCE) $(DEP_CPP_PGPEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"

DEP_CPP_PGPEX=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPexpire.obj" : $(SOURCE) $(DEP_CPP_PGPEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"

DEP_CPP_PGPEX=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPexpire.obj" : $(SOURCE) $(DEP_CPP_PGPEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"

DEP_CPP_PGPEX=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPexpire.obj" : $(SOURCE) $(DEP_CPP_PGPEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"

DEP_CPP_PGPEX=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPexpire.obj" : $(SOURCE) $(DEP_CPP_PGPEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPhelpabout.c
DEP_CPP_PGPHE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\..\include\pgpversion.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPhelpabout.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPhelpabout.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPhelpabout.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPhelpabout.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPhelpabout.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPhelpabout.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPphrase.c
DEP_CPP_PGPPH=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpmem.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPprefs.c
DEP_CPP_PGPPR=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpkeyserversupport.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPprefs.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPprefs.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPprefs.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPprefs.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPprefs.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPprefs.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPrand.c
DEP_CPP_PGPRA=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPrand.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPrand.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPrand.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPrand.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPrand.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPrand.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPcheck.c
DEP_CPP_PGPCH=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPcheck.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPcheck.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPcheck.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPcheck.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPcheck.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPcheck.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPcomdlg.rc
DEP_RSC_PGPCOM=\
	".\PGPcmdlg\..\include\pgpversion.h"\
	".\PGPcmdlg\PGPcomdlgVer.rc"\
	".\PGPcmdlg\PGPNag.rc"\
	".\PGPcmdlg\PGPsplash.rc"\
	".\PGPcmdlg\Res\credits1bit.bmp"\
	".\PGPcmdlg\Res\credits4bit.bmp"\
	".\PGPcmdlg\Res\credits8bit.bmp"\
	".\PGPcmdlg\res\eval1bit.bmp"\
	".\PGPcmdlg\res\eval24bit.bmp"\
	".\PGPcmdlg\res\eval4bit.bmp"\
	".\PGPcmdlg\res\eval8bit.bmp"\
	".\PGPcmdlg\Res\freesplash24bit.BMP"\
	".\PGPcmdlg\Res\freesplash4bit.BMP"\
	".\PGPcmdlg\Res\freesplash8bit.BMP"\
	".\PGPcmdlg\Res\splash24bit.bmp"\
	".\PGPcmdlg\Res\splash4bit.bmp"\
	".\PGPcmdlg\Res\splash8bit.bmp"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPcomdlg.res" : $(SOURCE) $(DEP_RSC_PGPCOM) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /i "PGPcmdlg" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPcomdlg.res" : $(SOURCE) $(DEP_RSC_PGPCOM) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /i "PGPcmdlg" /d "_DEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPcomdlg.res" : $(SOURCE) $(DEP_RSC_PGPCOM) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /i "PGPcmdlg" /d "_DEBUG" /d\
 "PGP_FREEWARE" $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPcomdlg.res" : $(SOURCE) $(DEP_RSC_PGPCOM) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /i "PGPcmdlg" /d "NDEBUG" /d\
 "PGP_FREEWARE" $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPcomdlg.res" : $(SOURCE) $(DEP_RSC_PGPCOM) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /i "PGPcmdlg" /d "NDEBUG" /d\
 EVAL_TIMEOUT=30 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPcomdlg.res" : $(SOURCE) $(DEP_RSC_PGPCOM) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPcomdlg.res" /i "PGPcmdlg" /d "_DEBUG" /d\
 EVAL_TIMEOUT=30 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPcomdlgVer.h

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"

!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPcmdlg\PGPads.c
DEP_CPP_PGPAD=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPcmdlg\..\include\config.h"\
	".\PGPcmdlg\..\include\PGPcomdlg.h"\
	".\PGPcmdlg\..\include\pgpErr.h"\
	".\PGPcmdlg\..\include\pgpkeydb.h"\
	".\PGPcmdlg\..\include\pgpRndPool.h"\
	".\PGPcmdlg\PGPcomdlgHelp.h"\
	".\PGPcmdlg\PGPcomdlgx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "PGPcmdlg - Win32 Release"


"$(INTDIR)\PGPads.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Debug"


"$(INTDIR)\PGPads.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Debug"


"$(INTDIR)\PGPads.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Freeware_Release"


"$(INTDIR)\PGPads.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Release"


"$(INTDIR)\PGPads.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPcmdlg - Win32 Eval_Debug"


"$(INTDIR)\PGPads.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
