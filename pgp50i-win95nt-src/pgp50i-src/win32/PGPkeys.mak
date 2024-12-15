# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=PGPkeys - Win32 Eval_Debug
!MESSAGE No configuration specified.  Defaulting to PGPkeys - Win32 Eval_Debug.
!ENDIF 

!IF "$(CFG)" != "PGPkeys - Win32 Release" && "$(CFG)" !=\
 "PGPkeys - Win32 Debug" && "$(CFG)" != "PGPkeys - Win32 Freeware_Debug" &&\
 "$(CFG)" != "PGPkeys - Win32 Freeware_Release" && "$(CFG)" !=\
 "PGPkeys - Win32 Eval_Release" && "$(CFG)" != "PGPkeys - Win32 Eval_Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPkeys.mak" CFG="PGPkeys - Win32 Eval_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPkeys - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPkeys - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "PGPkeys - Win32 Freeware_Debug" (based on "Win32 (x86) Application")
!MESSAGE "PGPkeys - Win32 Freeware_Release" (based on\
 "Win32 (x86) Application")
!MESSAGE "PGPkeys - Win32 Eval_Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPkeys - Win32 Eval_Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "PGPkeys - Win32 Eval_Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPkeys\Release"
# PROP Intermediate_Dir "PGPkeys\Release"
# PROP Target_Dir ""
OUTDIR=.\PGPkeys\Release
INTDIR=.\PGPkeys\Release

ALL : "$(OUTDIR)\PGPkeys.exe"

CLEAN : 
	-@erase "$(INTDIR)\KMAddUser.obj"
	-@erase "$(INTDIR)\KMChange.obj"
	-@erase "$(INTDIR)\KMConvert.obj"
	-@erase "$(INTDIR)\KMFocus.obj"
	-@erase "$(INTDIR)\KMIDataObject.obj"
	-@erase "$(INTDIR)\KMIDropSource.obj"
	-@erase "$(INTDIR)\KMIDropTarget.obj"
	-@erase "$(INTDIR)\KMIEnumFormatEtc.obj"
	-@erase "$(INTDIR)\KMKeyIO.obj"
	-@erase "$(INTDIR)\KMKeyOps.obj"
	-@erase "$(INTDIR)\KMMenu.obj"
	-@erase "$(INTDIR)\KMMisc.obj"
	-@erase "$(INTDIR)\KMMsgProc.obj"
	-@erase "$(INTDIR)\KMProps.obj"
	-@erase "$(INTDIR)\KMTree.obj"
	-@erase "$(INTDIR)\KMUser.obj"
	-@erase "$(INTDIR)\PGPkeygenwiz.obj"
	-@erase "$(INTDIR)\PGPkeys.obj"
	-@erase "$(INTDIR)\PGPkeys.res"
	-@erase "$(OUTDIR)\PGPkeys.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPkeys.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPkeys\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPkeys.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeys.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib\
 keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib\
 pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/PGPkeys.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPkeys.exe" 
LINK32_OBJS= \
	"$(INTDIR)\KMAddUser.obj" \
	"$(INTDIR)\KMChange.obj" \
	"$(INTDIR)\KMConvert.obj" \
	"$(INTDIR)\KMFocus.obj" \
	"$(INTDIR)\KMIDataObject.obj" \
	"$(INTDIR)\KMIDropSource.obj" \
	"$(INTDIR)\KMIDropTarget.obj" \
	"$(INTDIR)\KMIEnumFormatEtc.obj" \
	"$(INTDIR)\KMKeyIO.obj" \
	"$(INTDIR)\KMKeyOps.obj" \
	"$(INTDIR)\KMMenu.obj" \
	"$(INTDIR)\KMMisc.obj" \
	"$(INTDIR)\KMMsgProc.obj" \
	"$(INTDIR)\KMProps.obj" \
	"$(INTDIR)\KMTree.obj" \
	"$(INTDIR)\KMUser.obj" \
	"$(INTDIR)\PGPkeygenwiz.obj" \
	"$(INTDIR)\PGPkeys.obj" \
	"$(INTDIR)\PGPkeys.res"

"$(OUTDIR)\PGPkeys.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPkeys\Debug"
# PROP Intermediate_Dir "PGPkeys\Debug"
# PROP Target_Dir ""
OUTDIR=.\PGPkeys\Debug
INTDIR=.\PGPkeys\Debug

ALL : "$(OUTDIR)\PGPkeys.exe"

CLEAN : 
	-@erase "$(INTDIR)\KMAddUser.obj"
	-@erase "$(INTDIR)\KMChange.obj"
	-@erase "$(INTDIR)\KMConvert.obj"
	-@erase "$(INTDIR)\KMFocus.obj"
	-@erase "$(INTDIR)\KMIDataObject.obj"
	-@erase "$(INTDIR)\KMIDropSource.obj"
	-@erase "$(INTDIR)\KMIDropTarget.obj"
	-@erase "$(INTDIR)\KMIEnumFormatEtc.obj"
	-@erase "$(INTDIR)\KMKeyIO.obj"
	-@erase "$(INTDIR)\KMKeyOps.obj"
	-@erase "$(INTDIR)\KMMenu.obj"
	-@erase "$(INTDIR)\KMMisc.obj"
	-@erase "$(INTDIR)\KMMsgProc.obj"
	-@erase "$(INTDIR)\KMProps.obj"
	-@erase "$(INTDIR)\KMTree.obj"
	-@erase "$(INTDIR)\KMUser.obj"
	-@erase "$(INTDIR)\PGPkeygenwiz.obj"
	-@erase "$(INTDIR)\PGPkeys.obj"
	-@erase "$(INTDIR)\PGPkeys.res"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PGPkeys.exe"
	-@erase "$(OUTDIR)\PGPkeys.ilk"
	-@erase "$(OUTDIR)\PGPkeys.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPkeys.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPkeys\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPkeys.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeys.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib\
 keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib\
 pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/PGPkeys.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/PGPkeys.exe" 
LINK32_OBJS= \
	"$(INTDIR)\KMAddUser.obj" \
	"$(INTDIR)\KMChange.obj" \
	"$(INTDIR)\KMConvert.obj" \
	"$(INTDIR)\KMFocus.obj" \
	"$(INTDIR)\KMIDataObject.obj" \
	"$(INTDIR)\KMIDropSource.obj" \
	"$(INTDIR)\KMIDropTarget.obj" \
	"$(INTDIR)\KMIEnumFormatEtc.obj" \
	"$(INTDIR)\KMKeyIO.obj" \
	"$(INTDIR)\KMKeyOps.obj" \
	"$(INTDIR)\KMMenu.obj" \
	"$(INTDIR)\KMMisc.obj" \
	"$(INTDIR)\KMMsgProc.obj" \
	"$(INTDIR)\KMProps.obj" \
	"$(INTDIR)\KMTree.obj" \
	"$(INTDIR)\KMUser.obj" \
	"$(INTDIR)\PGPkeygenwiz.obj" \
	"$(INTDIR)\PGPkeys.obj" \
	"$(INTDIR)\PGPkeys.res"

"$(OUTDIR)\PGPkeys.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPkeys_"
# PROP BASE Intermediate_Dir "PGPkeys_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPkeys\FreewareDebug"
# PROP Intermediate_Dir "PGPkeys\FreewareDebug"
# PROP Target_Dir ""
OUTDIR=.\PGPkeys\FreewareDebug
INTDIR=.\PGPkeys\FreewareDebug

ALL : "$(OUTDIR)\PGPkeys.exe"

CLEAN : 
	-@erase "$(INTDIR)\KMAddUser.obj"
	-@erase "$(INTDIR)\KMChange.obj"
	-@erase "$(INTDIR)\KMConvert.obj"
	-@erase "$(INTDIR)\KMFocus.obj"
	-@erase "$(INTDIR)\KMIDataObject.obj"
	-@erase "$(INTDIR)\KMIDropSource.obj"
	-@erase "$(INTDIR)\KMIDropTarget.obj"
	-@erase "$(INTDIR)\KMIEnumFormatEtc.obj"
	-@erase "$(INTDIR)\KMKeyIO.obj"
	-@erase "$(INTDIR)\KMKeyOps.obj"
	-@erase "$(INTDIR)\KMMenu.obj"
	-@erase "$(INTDIR)\KMMisc.obj"
	-@erase "$(INTDIR)\KMMsgProc.obj"
	-@erase "$(INTDIR)\KMProps.obj"
	-@erase "$(INTDIR)\KMTree.obj"
	-@erase "$(INTDIR)\KMUser.obj"
	-@erase "$(INTDIR)\PGPkeygenwiz.obj"
	-@erase "$(INTDIR)\PGPkeys.obj"
	-@erase "$(INTDIR)\PGPkeys.res"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PGPkeys.exe"
	-@erase "$(OUTDIR)\PGPkeys.ilk"
	-@erase "$(OUTDIR)\PGPkeys.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "PGP_FREEWARE" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "PGP_FREEWARE" /Fp"$(INTDIR)/PGPkeys.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPkeys\FreewareDebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPkeys.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeys.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib\
 keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib\
 pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/PGPkeys.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/PGPkeys.exe" 
LINK32_OBJS= \
	"$(INTDIR)\KMAddUser.obj" \
	"$(INTDIR)\KMChange.obj" \
	"$(INTDIR)\KMConvert.obj" \
	"$(INTDIR)\KMFocus.obj" \
	"$(INTDIR)\KMIDataObject.obj" \
	"$(INTDIR)\KMIDropSource.obj" \
	"$(INTDIR)\KMIDropTarget.obj" \
	"$(INTDIR)\KMIEnumFormatEtc.obj" \
	"$(INTDIR)\KMKeyIO.obj" \
	"$(INTDIR)\KMKeyOps.obj" \
	"$(INTDIR)\KMMenu.obj" \
	"$(INTDIR)\KMMisc.obj" \
	"$(INTDIR)\KMMsgProc.obj" \
	"$(INTDIR)\KMProps.obj" \
	"$(INTDIR)\KMTree.obj" \
	"$(INTDIR)\KMUser.obj" \
	"$(INTDIR)\PGPkeygenwiz.obj" \
	"$(INTDIR)\PGPkeys.obj" \
	"$(INTDIR)\PGPkeys.res"

"$(OUTDIR)\PGPkeys.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPkeys0"
# PROP BASE Intermediate_Dir "PGPkeys0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPkeys\FreewareRelease"
# PROP Intermediate_Dir "PGPkeys\FreewareRelease"
# PROP Target_Dir ""
OUTDIR=.\PGPkeys\FreewareRelease
INTDIR=.\PGPkeys\FreewareRelease

ALL : "$(OUTDIR)\PGPkeys.exe"

CLEAN : 
	-@erase "$(INTDIR)\KMAddUser.obj"
	-@erase "$(INTDIR)\KMChange.obj"
	-@erase "$(INTDIR)\KMConvert.obj"
	-@erase "$(INTDIR)\KMFocus.obj"
	-@erase "$(INTDIR)\KMIDataObject.obj"
	-@erase "$(INTDIR)\KMIDropSource.obj"
	-@erase "$(INTDIR)\KMIDropTarget.obj"
	-@erase "$(INTDIR)\KMIEnumFormatEtc.obj"
	-@erase "$(INTDIR)\KMKeyIO.obj"
	-@erase "$(INTDIR)\KMKeyOps.obj"
	-@erase "$(INTDIR)\KMMenu.obj"
	-@erase "$(INTDIR)\KMMisc.obj"
	-@erase "$(INTDIR)\KMMsgProc.obj"
	-@erase "$(INTDIR)\KMProps.obj"
	-@erase "$(INTDIR)\KMTree.obj"
	-@erase "$(INTDIR)\KMUser.obj"
	-@erase "$(INTDIR)\PGPkeygenwiz.obj"
	-@erase "$(INTDIR)\PGPkeys.obj"
	-@erase "$(INTDIR)\PGPkeys.res"
	-@erase "$(OUTDIR)\PGPkeys.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PGP_FREEWARE" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "PGP_FREEWARE" /Fp"$(INTDIR)/PGPkeys.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPkeys\FreewareRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPkeys.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeys.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib\
 keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib\
 pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/PGPkeys.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPkeys.exe" 
LINK32_OBJS= \
	"$(INTDIR)\KMAddUser.obj" \
	"$(INTDIR)\KMChange.obj" \
	"$(INTDIR)\KMConvert.obj" \
	"$(INTDIR)\KMFocus.obj" \
	"$(INTDIR)\KMIDataObject.obj" \
	"$(INTDIR)\KMIDropSource.obj" \
	"$(INTDIR)\KMIDropTarget.obj" \
	"$(INTDIR)\KMIEnumFormatEtc.obj" \
	"$(INTDIR)\KMKeyIO.obj" \
	"$(INTDIR)\KMKeyOps.obj" \
	"$(INTDIR)\KMMenu.obj" \
	"$(INTDIR)\KMMisc.obj" \
	"$(INTDIR)\KMMsgProc.obj" \
	"$(INTDIR)\KMProps.obj" \
	"$(INTDIR)\KMTree.obj" \
	"$(INTDIR)\KMUser.obj" \
	"$(INTDIR)\PGPkeygenwiz.obj" \
	"$(INTDIR)\PGPkeys.obj" \
	"$(INTDIR)\PGPkeys.res"

"$(OUTDIR)\PGPkeys.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPkeys_"
# PROP BASE Intermediate_Dir "PGPkeys_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPkeys\EvalRelease"
# PROP Intermediate_Dir "PGPkeys\EvalRelease"
# PROP Target_Dir ""
OUTDIR=.\PGPkeys\EvalRelease
INTDIR=.\PGPkeys\EvalRelease

ALL : "$(OUTDIR)\PGPkeys.exe"

CLEAN : 
	-@erase "$(INTDIR)\KMAddUser.obj"
	-@erase "$(INTDIR)\KMChange.obj"
	-@erase "$(INTDIR)\KMConvert.obj"
	-@erase "$(INTDIR)\KMFocus.obj"
	-@erase "$(INTDIR)\KMIDataObject.obj"
	-@erase "$(INTDIR)\KMIDropSource.obj"
	-@erase "$(INTDIR)\KMIDropTarget.obj"
	-@erase "$(INTDIR)\KMIEnumFormatEtc.obj"
	-@erase "$(INTDIR)\KMKeyIO.obj"
	-@erase "$(INTDIR)\KMKeyOps.obj"
	-@erase "$(INTDIR)\KMMenu.obj"
	-@erase "$(INTDIR)\KMMisc.obj"
	-@erase "$(INTDIR)\KMMsgProc.obj"
	-@erase "$(INTDIR)\KMProps.obj"
	-@erase "$(INTDIR)\KMTree.obj"
	-@erase "$(INTDIR)\KMUser.obj"
	-@erase "$(INTDIR)\PGPkeygenwiz.obj"
	-@erase "$(INTDIR)\PGPkeys.obj"
	-@erase "$(INTDIR)\PGPkeys.res"
	-@erase "$(OUTDIR)\PGPkeys.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "EVAL_TIMEOUT" /Fp"$(INTDIR)/PGPkeys.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPkeys\EvalRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPkeys.res" /d "NDEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeys.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\evalrelease\pgpcmdlg.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib\
 keyserversupport\release\keyserversupport.lib treelist\release\treelist.lib\
 pgpphrase\release\pgpphrase.lib pgpcmdlg\evalrelease\pgpcmdlg.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/PGPkeys.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPkeys.exe" 
LINK32_OBJS= \
	"$(INTDIR)\KMAddUser.obj" \
	"$(INTDIR)\KMChange.obj" \
	"$(INTDIR)\KMConvert.obj" \
	"$(INTDIR)\KMFocus.obj" \
	"$(INTDIR)\KMIDataObject.obj" \
	"$(INTDIR)\KMIDropSource.obj" \
	"$(INTDIR)\KMIDropTarget.obj" \
	"$(INTDIR)\KMIEnumFormatEtc.obj" \
	"$(INTDIR)\KMKeyIO.obj" \
	"$(INTDIR)\KMKeyOps.obj" \
	"$(INTDIR)\KMMenu.obj" \
	"$(INTDIR)\KMMisc.obj" \
	"$(INTDIR)\KMMsgProc.obj" \
	"$(INTDIR)\KMProps.obj" \
	"$(INTDIR)\KMTree.obj" \
	"$(INTDIR)\KMUser.obj" \
	"$(INTDIR)\PGPkeygenwiz.obj" \
	"$(INTDIR)\PGPkeys.obj" \
	"$(INTDIR)\PGPkeys.res"

"$(OUTDIR)\PGPkeys.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPkeys0"
# PROP BASE Intermediate_Dir "PGPkeys0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPkeys\EvalDebug"
# PROP Intermediate_Dir "PGPkeys\EvalDebug"
# PROP Target_Dir ""
OUTDIR=.\PGPkeys\EvalDebug
INTDIR=.\PGPkeys\EvalDebug

ALL : "$(OUTDIR)\PGPkeys.exe"

CLEAN : 
	-@erase "$(INTDIR)\KMAddUser.obj"
	-@erase "$(INTDIR)\KMChange.obj"
	-@erase "$(INTDIR)\KMConvert.obj"
	-@erase "$(INTDIR)\KMFocus.obj"
	-@erase "$(INTDIR)\KMIDataObject.obj"
	-@erase "$(INTDIR)\KMIDropSource.obj"
	-@erase "$(INTDIR)\KMIDropTarget.obj"
	-@erase "$(INTDIR)\KMIEnumFormatEtc.obj"
	-@erase "$(INTDIR)\KMKeyIO.obj"
	-@erase "$(INTDIR)\KMKeyOps.obj"
	-@erase "$(INTDIR)\KMMenu.obj"
	-@erase "$(INTDIR)\KMMisc.obj"
	-@erase "$(INTDIR)\KMMsgProc.obj"
	-@erase "$(INTDIR)\KMProps.obj"
	-@erase "$(INTDIR)\KMTree.obj"
	-@erase "$(INTDIR)\KMUser.obj"
	-@erase "$(INTDIR)\PGPkeygenwiz.obj"
	-@erase "$(INTDIR)\PGPkeys.obj"
	-@erase "$(INTDIR)\PGPkeys.res"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PGPkeys.exe"
	-@erase "$(OUTDIR)\PGPkeys.ilk"
	-@erase "$(OUTDIR)\PGPkeys.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/PGPkeys.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPkeys\EvalDebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPkeys.res" /d "_DEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeys.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\evaldebug\pgpcmdlg.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib lib\pgp.lib lib\simple.lib lib\bn.lib lib\keydb.lib\
 keyserversupport\debug\keyserversupport.lib treelist\debug\treelist.lib\
 pgpphrase\debug\pgpphrase.lib pgpcmdlg\evaldebug\pgpcmdlg.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/PGPkeys.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/PGPkeys.exe" 
LINK32_OBJS= \
	"$(INTDIR)\KMAddUser.obj" \
	"$(INTDIR)\KMChange.obj" \
	"$(INTDIR)\KMConvert.obj" \
	"$(INTDIR)\KMFocus.obj" \
	"$(INTDIR)\KMIDataObject.obj" \
	"$(INTDIR)\KMIDropSource.obj" \
	"$(INTDIR)\KMIDropTarget.obj" \
	"$(INTDIR)\KMIEnumFormatEtc.obj" \
	"$(INTDIR)\KMKeyIO.obj" \
	"$(INTDIR)\KMKeyOps.obj" \
	"$(INTDIR)\KMMenu.obj" \
	"$(INTDIR)\KMMisc.obj" \
	"$(INTDIR)\KMMsgProc.obj" \
	"$(INTDIR)\KMProps.obj" \
	"$(INTDIR)\KMTree.obj" \
	"$(INTDIR)\KMUser.obj" \
	"$(INTDIR)\PGPkeygenwiz.obj" \
	"$(INTDIR)\PGPkeys.obj" \
	"$(INTDIR)\PGPkeys.res"

"$(OUTDIR)\PGPkeys.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "PGPkeys - Win32 Release"
# Name "PGPkeys - Win32 Debug"
# Name "PGPkeys - Win32 Freeware_Debug"
# Name "PGPkeys - Win32 Freeware_Release"
# Name "PGPkeys - Win32 Eval_Release"
# Name "PGPkeys - Win32 Eval_Debug"

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMIEnumFormatEtc.cpp
DEP_CPP_KMIEN=\
	".\PGPkeys\KMIEnumFormatEtc.h"\
	

!IF  "$(CFG)" == "PGPkeys - Win32 Release"


"$(INTDIR)\KMIEnumFormatEtc.obj" : $(SOURCE) $(DEP_CPP_KMIEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"


"$(INTDIR)\KMIEnumFormatEtc.obj" : $(SOURCE) $(DEP_CPP_KMIEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"


"$(INTDIR)\KMIEnumFormatEtc.obj" : $(SOURCE) $(DEP_CPP_KMIEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"


"$(INTDIR)\KMIEnumFormatEtc.obj" : $(SOURCE) $(DEP_CPP_KMIEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"


"$(INTDIR)\KMIEnumFormatEtc.obj" : $(SOURCE) $(DEP_CPP_KMIEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"


"$(INTDIR)\KMIEnumFormatEtc.obj" : $(SOURCE) $(DEP_CPP_KMIEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMChange.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMCHA=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMChange.obj" : $(SOURCE) $(DEP_CPP_KMCHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMCHA=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMChange.obj" : $(SOURCE) $(DEP_CPP_KMCHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMCHA=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMChange.obj" : $(SOURCE) $(DEP_CPP_KMCHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMCHA=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMChange.obj" : $(SOURCE) $(DEP_CPP_KMCHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMCHA=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMChange.obj" : $(SOURCE) $(DEP_CPP_KMCHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMCHA=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMChange.obj" : $(SOURCE) $(DEP_CPP_KMCHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMConvert.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMCON=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMConvert.obj" : $(SOURCE) $(DEP_CPP_KMCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMCON=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMConvert.obj" : $(SOURCE) $(DEP_CPP_KMCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMCON=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMConvert.obj" : $(SOURCE) $(DEP_CPP_KMCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMCON=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMConvert.obj" : $(SOURCE) $(DEP_CPP_KMCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMCON=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMConvert.obj" : $(SOURCE) $(DEP_CPP_KMCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMCON=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMConvert.obj" : $(SOURCE) $(DEP_CPP_KMCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMFocus.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMFOC=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMFocus.obj" : $(SOURCE) $(DEP_CPP_KMFOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMFOC=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMFocus.obj" : $(SOURCE) $(DEP_CPP_KMFOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMFOC=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMFocus.obj" : $(SOURCE) $(DEP_CPP_KMFOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMFOC=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMFocus.obj" : $(SOURCE) $(DEP_CPP_KMFOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMFOC=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMFocus.obj" : $(SOURCE) $(DEP_CPP_KMFOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMFOC=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMFocus.obj" : $(SOURCE) $(DEP_CPP_KMFOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMKeyIO.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMKEY=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyIO.obj" : $(SOURCE) $(DEP_CPP_KMKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMKEY=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyIO.obj" : $(SOURCE) $(DEP_CPP_KMKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMKEY=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyIO.obj" : $(SOURCE) $(DEP_CPP_KMKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMKEY=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyIO.obj" : $(SOURCE) $(DEP_CPP_KMKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMKEY=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyIO.obj" : $(SOURCE) $(DEP_CPP_KMKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMKEY=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyIO.obj" : $(SOURCE) $(DEP_CPP_KMKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMKeyOps.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMKEYO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyOps.obj" : $(SOURCE) $(DEP_CPP_KMKEYO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMKEYO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyOps.obj" : $(SOURCE) $(DEP_CPP_KMKEYO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMKEYO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyOps.obj" : $(SOURCE) $(DEP_CPP_KMKEYO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMKEYO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyOps.obj" : $(SOURCE) $(DEP_CPP_KMKEYO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMKEYO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyOps.obj" : $(SOURCE) $(DEP_CPP_KMKEYO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMKEYO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMKeyOps.obj" : $(SOURCE) $(DEP_CPP_KMKEYO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMMisc.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMMIS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMisc.obj" : $(SOURCE) $(DEP_CPP_KMMIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMMIS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMisc.obj" : $(SOURCE) $(DEP_CPP_KMMIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMMIS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMisc.obj" : $(SOURCE) $(DEP_CPP_KMMIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMMIS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMisc.obj" : $(SOURCE) $(DEP_CPP_KMMIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMMIS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMisc.obj" : $(SOURCE) $(DEP_CPP_KMMIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMMIS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMisc.obj" : $(SOURCE) $(DEP_CPP_KMMIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMMsgProc.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMMSG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMsgProc.obj" : $(SOURCE) $(DEP_CPP_KMMSG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMMSG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMsgProc.obj" : $(SOURCE) $(DEP_CPP_KMMSG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMMSG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMsgProc.obj" : $(SOURCE) $(DEP_CPP_KMMSG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMMSG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMsgProc.obj" : $(SOURCE) $(DEP_CPP_KMMSG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMMSG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMsgProc.obj" : $(SOURCE) $(DEP_CPP_KMMSG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMMSG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMsgProc.obj" : $(SOURCE) $(DEP_CPP_KMMSG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMProps.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMPRO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMProps.obj" : $(SOURCE) $(DEP_CPP_KMPRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMPRO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMProps.obj" : $(SOURCE) $(DEP_CPP_KMPRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMPRO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMProps.obj" : $(SOURCE) $(DEP_CPP_KMPRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMPRO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMProps.obj" : $(SOURCE) $(DEP_CPP_KMPRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMPRO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMProps.obj" : $(SOURCE) $(DEP_CPP_KMPRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMPRO=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMProps.obj" : $(SOURCE) $(DEP_CPP_KMPRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMTree.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMTRE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMTree.obj" : $(SOURCE) $(DEP_CPP_KMTRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMTRE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMTree.obj" : $(SOURCE) $(DEP_CPP_KMTRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMTRE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMTree.obj" : $(SOURCE) $(DEP_CPP_KMTRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMTRE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMTree.obj" : $(SOURCE) $(DEP_CPP_KMTRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMTRE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMTree.obj" : $(SOURCE) $(DEP_CPP_KMTRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMTRE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMTree.obj" : $(SOURCE) $(DEP_CPP_KMTRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMUser.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMUSE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMUser.obj" : $(SOURCE) $(DEP_CPP_KMUSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMUSE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMUser.obj" : $(SOURCE) $(DEP_CPP_KMUSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMUSE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMUser.obj" : $(SOURCE) $(DEP_CPP_KMUSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMUSE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMUser.obj" : $(SOURCE) $(DEP_CPP_KMUSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMUSE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMUser.obj" : $(SOURCE) $(DEP_CPP_KMUSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMUSE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMUser.obj" : $(SOURCE) $(DEP_CPP_KMUSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\PGPkeys.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_PGPKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	".\PGPkeys\PGPkeysVer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeys.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_PGPKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	".\PGPkeys\PGPkeysVer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeys.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_PGPKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	".\PGPkeys\PGPkeysVer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeys.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_PGPKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	".\PGPkeys\PGPkeysVer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeys.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_PGPKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	".\PGPkeys\PGPkeysVer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeys.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_PGPKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	".\PGPkeys\PGPkeysVer.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeys.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMIDataObject.cpp
DEP_CPP_KMIDA=\
	".\PGPkeys\KMIDataObject.h"\
	".\PGPkeys\KMIEnumFormatEtc.h"\
	

!IF  "$(CFG)" == "PGPkeys - Win32 Release"


"$(INTDIR)\KMIDataObject.obj" : $(SOURCE) $(DEP_CPP_KMIDA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"


"$(INTDIR)\KMIDataObject.obj" : $(SOURCE) $(DEP_CPP_KMIDA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"


"$(INTDIR)\KMIDataObject.obj" : $(SOURCE) $(DEP_CPP_KMIDA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"


"$(INTDIR)\KMIDataObject.obj" : $(SOURCE) $(DEP_CPP_KMIDA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"


"$(INTDIR)\KMIDataObject.obj" : $(SOURCE) $(DEP_CPP_KMIDA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"


"$(INTDIR)\KMIDataObject.obj" : $(SOURCE) $(DEP_CPP_KMIDA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMIDropSource.cpp
DEP_CPP_KMIDR=\
	".\PGPkeys\KMIDropSource.h"\
	

!IF  "$(CFG)" == "PGPkeys - Win32 Release"


"$(INTDIR)\KMIDropSource.obj" : $(SOURCE) $(DEP_CPP_KMIDR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"


"$(INTDIR)\KMIDropSource.obj" : $(SOURCE) $(DEP_CPP_KMIDR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"


"$(INTDIR)\KMIDropSource.obj" : $(SOURCE) $(DEP_CPP_KMIDR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"


"$(INTDIR)\KMIDropSource.obj" : $(SOURCE) $(DEP_CPP_KMIDR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"


"$(INTDIR)\KMIDropSource.obj" : $(SOURCE) $(DEP_CPP_KMIDR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"


"$(INTDIR)\KMIDropSource.obj" : $(SOURCE) $(DEP_CPP_KMIDR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMIDropTarget.cpp
DEP_CPP_KMIDRO=\
	".\PGPkeys\KMIDropTarget.h"\
	

!IF  "$(CFG)" == "PGPkeys - Win32 Release"


"$(INTDIR)\KMIDropTarget.obj" : $(SOURCE) $(DEP_CPP_KMIDRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"


"$(INTDIR)\KMIDropTarget.obj" : $(SOURCE) $(DEP_CPP_KMIDRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"


"$(INTDIR)\KMIDropTarget.obj" : $(SOURCE) $(DEP_CPP_KMIDRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"


"$(INTDIR)\KMIDropTarget.obj" : $(SOURCE) $(DEP_CPP_KMIDRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"


"$(INTDIR)\KMIDropTarget.obj" : $(SOURCE) $(DEP_CPP_KMIDRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"


"$(INTDIR)\KMIDropTarget.obj" : $(SOURCE) $(DEP_CPP_KMIDRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMAddUser.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMADD=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMAddUser.obj" : $(SOURCE) $(DEP_CPP_KMADD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMADD=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMAddUser.obj" : $(SOURCE) $(DEP_CPP_KMADD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMADD=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMAddUser.obj" : $(SOURCE) $(DEP_CPP_KMADD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMADD=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMAddUser.obj" : $(SOURCE) $(DEP_CPP_KMADD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMADD=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMAddUser.obj" : $(SOURCE) $(DEP_CPP_KMADD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMADD=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpphrase.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMAddUser.obj" : $(SOURCE) $(DEP_CPP_KMADD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\PGPkeys.rc

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_RSC_PGPKEY=\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\PGPkeysVer.rc"\
	".\PGPkeys\Res\bitmap1.bmp"\
	".\PGPkeys\Res\bmp00001.bmp"\
	".\PGPkeys\Res\bmp00002.bmp"\
	".\PGPkeys\Res\bmp00003.bmp"\
	".\PGPkeys\Res\bmp00004.bmp"\
	".\PGPkeys\Res\bmp00005.bmp"\
	".\PGPkeys\Res\bmp00006.bmp"\
	".\PGPkeys\Res\bmp00007.bmp"\
	".\PGPkeys\Res\bmp00008.bmp"\
	".\PGPkeys\Res\cert.bmp"\
	".\PGPkeys\Res\cert1.bmp"\
	".\PGPkeys\Res\certmsk.bmp"\
	".\PGPkeys\Res\certmsk1.bmp"\
	".\PGPkeys\Res\clock89.avi"\
	".\PGPkeys\Res\dis.bmp"\
	".\PGPkeys\Res\dismsk.bmp"\
	".\PGPkeys\Res\dsadiske.bmp"\
	".\PGPkeys\Res\dsaprivkey.bmp"\
	".\PGPkeys\Res\dsaprivkeymsk.bmp"\
	".\PGPkeys\Res\dsapubex.bmp"\
	".\PGPkeys\Res\dsapubkey.bmp"\
	".\PGPkeys\Res\dsapubkeymsk.bmp"\
	".\PGPkeys\Res\dsarevke.bmp"\
	".\PGPkeys\Res\dsasecdi.bmp"\
	".\PGPkeys\Res\dsasecex.bmp"\
	".\PGPkeys\Res\dsasecre.bmp"\
	".\PGPkeys\Res\dsauid.bmp"\
	".\PGPkeys\Res\exp.bmp"\
	".\PGPkeys\Res\expmsk.bmp"\
	".\PGPkeys\Res\Key.ico"\
	".\PGPkeys\Res\kgwiz1bit.bmp"\
	".\PGPkeys\Res\kgwiz24bit.bmp"\
	".\PGPkeys\Res\kgwiz4bit.bmp"\
	".\PGPkeys\Res\kgwiz8bit.bmp"\
	".\PGPkeys\Res\priv.bmp"\
	".\PGPkeys\Res\privmsk.bmp"\
	".\PGPkeys\Res\pub.bmp"\
	".\PGPkeys\Res\pubmsk.bmp"\
	".\PGPkeys\Res\rev.bmp"\
	".\PGPkeys\Res\revcert1.bmp"\
	".\PGPkeys\Res\revcertm.bmp"\
	".\PGPkeys\Res\revmsk.bmp"\
	".\PGPkeys\Res\rsasecdi.bmp"\
	".\PGPkeys\Res\rsasecex.bmp"\
	".\PGPkeys\Res\rsasecre.bmp"\
	".\PGPkeys\Res\rsauid.bmp"\
	".\PGPkeys\Res\uidmsk.bmp"\
	

"$(INTDIR)\PGPkeys.res" : $(SOURCE) $(DEP_RSC_PGPKEY) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPkeys.res" /i "PGPkeys" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_RSC_PGPKEY=\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\PGPkeysVer.rc"\
	".\PGPkeys\Res\bitmap1.bmp"\
	".\PGPkeys\Res\bmp00001.bmp"\
	".\PGPkeys\Res\bmp00002.bmp"\
	".\PGPkeys\Res\bmp00003.bmp"\
	".\PGPkeys\Res\bmp00004.bmp"\
	".\PGPkeys\Res\bmp00005.bmp"\
	".\PGPkeys\Res\bmp00006.bmp"\
	".\PGPkeys\Res\bmp00007.bmp"\
	".\PGPkeys\Res\bmp00008.bmp"\
	".\PGPkeys\Res\cert.bmp"\
	".\PGPkeys\Res\cert1.bmp"\
	".\PGPkeys\Res\certmsk.bmp"\
	".\PGPkeys\Res\certmsk1.bmp"\
	".\PGPkeys\Res\clock89.avi"\
	".\PGPkeys\Res\dis.bmp"\
	".\PGPkeys\Res\dismsk.bmp"\
	".\PGPkeys\Res\dsadiske.bmp"\
	".\PGPkeys\Res\dsaprivkey.bmp"\
	".\PGPkeys\Res\dsaprivkeymsk.bmp"\
	".\PGPkeys\Res\dsapubex.bmp"\
	".\PGPkeys\Res\dsapubkey.bmp"\
	".\PGPkeys\Res\dsapubkeymsk.bmp"\
	".\PGPkeys\Res\dsarevke.bmp"\
	".\PGPkeys\Res\dsasecdi.bmp"\
	".\PGPkeys\Res\dsasecex.bmp"\
	".\PGPkeys\Res\dsasecre.bmp"\
	".\PGPkeys\Res\dsauid.bmp"\
	".\PGPkeys\Res\exp.bmp"\
	".\PGPkeys\Res\expmsk.bmp"\
	".\PGPkeys\Res\Key.ico"\
	".\PGPkeys\Res\kgwiz1bit.bmp"\
	".\PGPkeys\Res\kgwiz24bit.bmp"\
	".\PGPkeys\Res\kgwiz4bit.bmp"\
	".\PGPkeys\Res\kgwiz8bit.bmp"\
	".\PGPkeys\Res\priv.bmp"\
	".\PGPkeys\Res\privmsk.bmp"\
	".\PGPkeys\Res\pub.bmp"\
	".\PGPkeys\Res\pubmsk.bmp"\
	".\PGPkeys\Res\rev.bmp"\
	".\PGPkeys\Res\revcert1.bmp"\
	".\PGPkeys\Res\revcertm.bmp"\
	".\PGPkeys\Res\revmsk.bmp"\
	".\PGPkeys\Res\rsasecdi.bmp"\
	".\PGPkeys\Res\rsasecex.bmp"\
	".\PGPkeys\Res\rsasecre.bmp"\
	".\PGPkeys\Res\rsauid.bmp"\
	".\PGPkeys\Res\uidmsk.bmp"\
	

"$(INTDIR)\PGPkeys.res" : $(SOURCE) $(DEP_RSC_PGPKEY) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPkeys.res" /i "PGPkeys" /d "_DEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_RSC_PGPKEY=\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\PGPkeysVer.rc"\
	".\PGPkeys\Res\bitmap1.bmp"\
	".\PGPkeys\Res\bmp00001.bmp"\
	".\PGPkeys\Res\bmp00002.bmp"\
	".\PGPkeys\Res\bmp00003.bmp"\
	".\PGPkeys\Res\bmp00004.bmp"\
	".\PGPkeys\Res\bmp00005.bmp"\
	".\PGPkeys\Res\bmp00006.bmp"\
	".\PGPkeys\Res\bmp00007.bmp"\
	".\PGPkeys\Res\bmp00008.bmp"\
	".\PGPkeys\Res\cert.bmp"\
	".\PGPkeys\Res\cert1.bmp"\
	".\PGPkeys\Res\certmsk.bmp"\
	".\PGPkeys\Res\certmsk1.bmp"\
	".\PGPkeys\Res\clock89.avi"\
	".\PGPkeys\Res\dis.bmp"\
	".\PGPkeys\Res\dismsk.bmp"\
	".\PGPkeys\Res\dsadiske.bmp"\
	".\PGPkeys\Res\dsaprivkey.bmp"\
	".\PGPkeys\Res\dsaprivkeymsk.bmp"\
	".\PGPkeys\Res\dsapubex.bmp"\
	".\PGPkeys\Res\dsapubkey.bmp"\
	".\PGPkeys\Res\dsapubkeymsk.bmp"\
	".\PGPkeys\Res\dsarevke.bmp"\
	".\PGPkeys\Res\dsasecdi.bmp"\
	".\PGPkeys\Res\dsasecex.bmp"\
	".\PGPkeys\Res\dsasecre.bmp"\
	".\PGPkeys\Res\dsauid.bmp"\
	".\PGPkeys\Res\exp.bmp"\
	".\PGPkeys\Res\expmsk.bmp"\
	".\PGPkeys\Res\Key.ico"\
	".\PGPkeys\Res\kgwiz1bit.bmp"\
	".\PGPkeys\Res\kgwiz24bit.bmp"\
	".\PGPkeys\Res\kgwiz4bit.bmp"\
	".\PGPkeys\Res\kgwiz8bit.bmp"\
	".\PGPkeys\Res\priv.bmp"\
	".\PGPkeys\Res\privmsk.bmp"\
	".\PGPkeys\Res\pub.bmp"\
	".\PGPkeys\Res\pubmsk.bmp"\
	".\PGPkeys\Res\rev.bmp"\
	".\PGPkeys\Res\revcert1.bmp"\
	".\PGPkeys\Res\revcertm.bmp"\
	".\PGPkeys\Res\revmsk.bmp"\
	".\PGPkeys\Res\rsasecdi.bmp"\
	".\PGPkeys\Res\rsasecex.bmp"\
	".\PGPkeys\Res\rsasecre.bmp"\
	".\PGPkeys\Res\rsauid.bmp"\
	".\PGPkeys\Res\uidmsk.bmp"\
	

"$(INTDIR)\PGPkeys.res" : $(SOURCE) $(DEP_RSC_PGPKEY) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPkeys.res" /i "PGPkeys" /d "_DEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_RSC_PGPKEY=\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\PGPkeysVer.rc"\
	".\PGPkeys\Res\bitmap1.bmp"\
	".\PGPkeys\Res\bmp00001.bmp"\
	".\PGPkeys\Res\bmp00002.bmp"\
	".\PGPkeys\Res\bmp00003.bmp"\
	".\PGPkeys\Res\bmp00004.bmp"\
	".\PGPkeys\Res\bmp00005.bmp"\
	".\PGPkeys\Res\bmp00006.bmp"\
	".\PGPkeys\Res\bmp00007.bmp"\
	".\PGPkeys\Res\bmp00008.bmp"\
	".\PGPkeys\Res\cert.bmp"\
	".\PGPkeys\Res\cert1.bmp"\
	".\PGPkeys\Res\certmsk.bmp"\
	".\PGPkeys\Res\certmsk1.bmp"\
	".\PGPkeys\Res\clock89.avi"\
	".\PGPkeys\Res\dis.bmp"\
	".\PGPkeys\Res\dismsk.bmp"\
	".\PGPkeys\Res\dsadiske.bmp"\
	".\PGPkeys\Res\dsaprivkey.bmp"\
	".\PGPkeys\Res\dsaprivkeymsk.bmp"\
	".\PGPkeys\Res\dsapubex.bmp"\
	".\PGPkeys\Res\dsapubkey.bmp"\
	".\PGPkeys\Res\dsapubkeymsk.bmp"\
	".\PGPkeys\Res\dsarevke.bmp"\
	".\PGPkeys\Res\dsasecdi.bmp"\
	".\PGPkeys\Res\dsasecex.bmp"\
	".\PGPkeys\Res\dsasecre.bmp"\
	".\PGPkeys\Res\dsauid.bmp"\
	".\PGPkeys\Res\exp.bmp"\
	".\PGPkeys\Res\expmsk.bmp"\
	".\PGPkeys\Res\Key.ico"\
	".\PGPkeys\Res\kgwiz1bit.bmp"\
	".\PGPkeys\Res\kgwiz24bit.bmp"\
	".\PGPkeys\Res\kgwiz4bit.bmp"\
	".\PGPkeys\Res\kgwiz8bit.bmp"\
	".\PGPkeys\Res\priv.bmp"\
	".\PGPkeys\Res\privmsk.bmp"\
	".\PGPkeys\Res\pub.bmp"\
	".\PGPkeys\Res\pubmsk.bmp"\
	".\PGPkeys\Res\rev.bmp"\
	".\PGPkeys\Res\revcert1.bmp"\
	".\PGPkeys\Res\revcertm.bmp"\
	".\PGPkeys\Res\revmsk.bmp"\
	".\PGPkeys\Res\rsasecdi.bmp"\
	".\PGPkeys\Res\rsasecex.bmp"\
	".\PGPkeys\Res\rsasecre.bmp"\
	".\PGPkeys\Res\rsauid.bmp"\
	".\PGPkeys\Res\uidmsk.bmp"\
	

"$(INTDIR)\PGPkeys.res" : $(SOURCE) $(DEP_RSC_PGPKEY) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPkeys.res" /i "PGPkeys" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_RSC_PGPKEY=\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\PGPkeysVer.rc"\
	".\PGPkeys\Res\bitmap1.bmp"\
	".\PGPkeys\Res\bmp00001.bmp"\
	".\PGPkeys\Res\bmp00002.bmp"\
	".\PGPkeys\Res\bmp00003.bmp"\
	".\PGPkeys\Res\bmp00004.bmp"\
	".\PGPkeys\Res\bmp00005.bmp"\
	".\PGPkeys\Res\bmp00006.bmp"\
	".\PGPkeys\Res\bmp00007.bmp"\
	".\PGPkeys\Res\bmp00008.bmp"\
	".\PGPkeys\Res\cert.bmp"\
	".\PGPkeys\Res\cert1.bmp"\
	".\PGPkeys\Res\certmsk.bmp"\
	".\PGPkeys\Res\certmsk1.bmp"\
	".\PGPkeys\Res\clock89.avi"\
	".\PGPkeys\Res\dis.bmp"\
	".\PGPkeys\Res\dismsk.bmp"\
	".\PGPkeys\Res\dsadiske.bmp"\
	".\PGPkeys\Res\dsaprivkey.bmp"\
	".\PGPkeys\Res\dsaprivkeymsk.bmp"\
	".\PGPkeys\Res\dsapubex.bmp"\
	".\PGPkeys\Res\dsapubkey.bmp"\
	".\PGPkeys\Res\dsapubkeymsk.bmp"\
	".\PGPkeys\Res\dsarevke.bmp"\
	".\PGPkeys\Res\dsasecdi.bmp"\
	".\PGPkeys\Res\dsasecex.bmp"\
	".\PGPkeys\Res\dsasecre.bmp"\
	".\PGPkeys\Res\dsauid.bmp"\
	".\PGPkeys\Res\exp.bmp"\
	".\PGPkeys\Res\expmsk.bmp"\
	".\PGPkeys\Res\Key.ico"\
	".\PGPkeys\Res\kgwiz1bit.bmp"\
	".\PGPkeys\Res\kgwiz24bit.bmp"\
	".\PGPkeys\Res\kgwiz4bit.bmp"\
	".\PGPkeys\Res\kgwiz8bit.bmp"\
	".\PGPkeys\Res\priv.bmp"\
	".\PGPkeys\Res\privmsk.bmp"\
	".\PGPkeys\Res\pub.bmp"\
	".\PGPkeys\Res\pubmsk.bmp"\
	".\PGPkeys\Res\rev.bmp"\
	".\PGPkeys\Res\revcert1.bmp"\
	".\PGPkeys\Res\revcertm.bmp"\
	".\PGPkeys\Res\revmsk.bmp"\
	".\PGPkeys\Res\rsasecdi.bmp"\
	".\PGPkeys\Res\rsasecex.bmp"\
	".\PGPkeys\Res\rsasecre.bmp"\
	".\PGPkeys\Res\rsauid.bmp"\
	".\PGPkeys\Res\uidmsk.bmp"\
	

"$(INTDIR)\PGPkeys.res" : $(SOURCE) $(DEP_RSC_PGPKEY) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPkeys.res" /i "PGPkeys" /d "NDEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_RSC_PGPKEY=\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\PGPkeysVer.rc"\
	".\PGPkeys\Res\bitmap1.bmp"\
	".\PGPkeys\Res\bmp00001.bmp"\
	".\PGPkeys\Res\bmp00002.bmp"\
	".\PGPkeys\Res\bmp00003.bmp"\
	".\PGPkeys\Res\bmp00004.bmp"\
	".\PGPkeys\Res\bmp00005.bmp"\
	".\PGPkeys\Res\bmp00006.bmp"\
	".\PGPkeys\Res\bmp00007.bmp"\
	".\PGPkeys\Res\bmp00008.bmp"\
	".\PGPkeys\Res\cert.bmp"\
	".\PGPkeys\Res\cert1.bmp"\
	".\PGPkeys\Res\certmsk.bmp"\
	".\PGPkeys\Res\certmsk1.bmp"\
	".\PGPkeys\Res\clock89.avi"\
	".\PGPkeys\Res\dis.bmp"\
	".\PGPkeys\Res\dismsk.bmp"\
	".\PGPkeys\Res\dsadiske.bmp"\
	".\PGPkeys\Res\dsaprivkey.bmp"\
	".\PGPkeys\Res\dsaprivkeymsk.bmp"\
	".\PGPkeys\Res\dsapubex.bmp"\
	".\PGPkeys\Res\dsapubkey.bmp"\
	".\PGPkeys\Res\dsapubkeymsk.bmp"\
	".\PGPkeys\Res\dsarevke.bmp"\
	".\PGPkeys\Res\dsasecdi.bmp"\
	".\PGPkeys\Res\dsasecex.bmp"\
	".\PGPkeys\Res\dsasecre.bmp"\
	".\PGPkeys\Res\dsauid.bmp"\
	".\PGPkeys\Res\exp.bmp"\
	".\PGPkeys\Res\expmsk.bmp"\
	".\PGPkeys\Res\Key.ico"\
	".\PGPkeys\Res\kgwiz1bit.bmp"\
	".\PGPkeys\Res\kgwiz24bit.bmp"\
	".\PGPkeys\Res\kgwiz4bit.bmp"\
	".\PGPkeys\Res\kgwiz8bit.bmp"\
	".\PGPkeys\Res\priv.bmp"\
	".\PGPkeys\Res\privmsk.bmp"\
	".\PGPkeys\Res\pub.bmp"\
	".\PGPkeys\Res\pubmsk.bmp"\
	".\PGPkeys\Res\rev.bmp"\
	".\PGPkeys\Res\revcert1.bmp"\
	".\PGPkeys\Res\revcertm.bmp"\
	".\PGPkeys\Res\revmsk.bmp"\
	".\PGPkeys\Res\rsasecdi.bmp"\
	".\PGPkeys\Res\rsasecex.bmp"\
	".\PGPkeys\Res\rsasecre.bmp"\
	".\PGPkeys\Res\rsauid.bmp"\
	".\PGPkeys\Res\uidmsk.bmp"\
	

"$(INTDIR)\PGPkeys.res" : $(SOURCE) $(DEP_RSC_PGPKEY) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPkeys.res" /i "PGPkeys" /d "_DEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\PGPkeysVer.h

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\KMMenu.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_KMMEN=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMenu.obj" : $(SOURCE) $(DEP_CPP_KMMEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_KMMEN=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMenu.obj" : $(SOURCE) $(DEP_CPP_KMMEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_KMMEN=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMenu.obj" : $(SOURCE) $(DEP_CPP_KMMEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_KMMEN=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMenu.obj" : $(SOURCE) $(DEP_CPP_KMMEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_KMMEN=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMenu.obj" : $(SOURCE) $(DEP_CPP_KMMEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_KMMEN=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KMMenu.obj" : $(SOURCE) $(DEP_CPP_KMMEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPkeys\PGPkeygenwiz.c

!IF  "$(CFG)" == "PGPkeys - Win32 Release"

DEP_CPP_PGPKEYG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\getkeys.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeygenwiz.obj" : $(SOURCE) $(DEP_CPP_PGPKEYG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Debug"

DEP_CPP_PGPKEYG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\getkeys.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeygenwiz.obj" : $(SOURCE) $(DEP_CPP_PGPKEYG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Debug"

DEP_CPP_PGPKEYG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\getkeys.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeygenwiz.obj" : $(SOURCE) $(DEP_CPP_PGPKEYG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Freeware_Release"

DEP_CPP_PGPKEYG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\getkeys.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeygenwiz.obj" : $(SOURCE) $(DEP_CPP_PGPKEYG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Release"

DEP_CPP_PGPKEYG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\getkeys.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeygenwiz.obj" : $(SOURCE) $(DEP_CPP_PGPKEYG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeys - Win32 Eval_Debug"

DEP_CPP_PGPKEYG=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
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
	".\PGPkeys\..\include\config.h"\
	".\PGPkeys\..\include\pgpcomdlg.h"\
	".\PGPkeys\..\include\pgpkeydb.h"\
	".\PGPkeys\..\include\pgpkeyserversupport.h"\
	".\PGPkeys\..\include\pgpmem.h"\
	".\PGPkeys\..\include\pgpRndPool.h"\
	".\PGPkeys\..\include\pgpVersion.h"\
	".\PGPkeys\..\include\treelist.h"\
	".\PGPkeys\..\pgpkeyserver\getkeys.h"\
	".\PGPkeys\..\pgpkeyserver\winids.h"\
	".\PGPkeys\PGPkeys.h"\
	".\PGPkeys\PGPkeysHelp.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPkeygenwiz.obj" : $(SOURCE) $(DEP_CPP_PGPKEYG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
