# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=PGPTray - Win32 Eval_Debug
!MESSAGE No configuration specified.  Defaulting to PGPTray - Win32 Eval_Debug.
!ENDIF 

!IF "$(CFG)" != "PGPTray - Win32 Release" && "$(CFG)" !=\
 "PGPTray - Win32 Debug" && "$(CFG)" != "PGPTray - Win32 Eval_Debug" && "$(CFG)"\
 != "PGPTray - Win32 Eval_Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPtray.mak" CFG="PGPTray - Win32 Eval_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPTray - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPTray - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "PGPTray - Win32 Eval_Debug" (based on "Win32 (x86) Application")
!MESSAGE "PGPTray - Win32 Eval_Release" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "PGPTray - Win32 Eval_Release"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "PGPTray - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pgptray\Release"
# PROP Intermediate_Dir "pgptray\Release"
# PROP Target_Dir ""
OUTDIR=.\pgptray\Release
INTDIR=.\pgptray\Release

ALL : "$(OUTDIR)\PGPtray.exe"

CLEAN : 
	-@erase "$(INTDIR)\Actions.obj"
	-@erase "$(INTDIR)\ClipBoard.obj"
	-@erase "$(INTDIR)\EnclyptorAddKey.obj"
	-@erase "$(INTDIR)\EnclyptorDecryptVerify.obj"
	-@erase "$(INTDIR)\EnclyptorEncryptSign.obj"
	-@erase "$(INTDIR)\EnclyptorKeyDB.obj"
	-@erase "$(INTDIR)\EnclyptorProc.obj"
	-@erase "$(INTDIR)\EnclyptorViewClipboard.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\PGPtray.res"
	-@erase "$(INTDIR)\ResultsDlg.obj"
	-@erase "$(INTDIR)\Taskbar.obj"
	-@erase "$(INTDIR)\viewers.obj"
	-@erase "$(INTDIR)\Wordwrap.obj"
	-@erase "$(OUTDIR)\PGPtray.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPtray.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\pgptray\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPtray.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPtray.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib winmm.lib pgpcmdlg\release\pgpcmdlg.lib pgpwctx\release\pgpwctx.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib\
 pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib winmm.lib\
 pgpcmdlg\release\pgpcmdlg.lib pgpwctx\release\pgpwctx.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/PGPtray.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPtray.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Actions.obj" \
	"$(INTDIR)\ClipBoard.obj" \
	"$(INTDIR)\EnclyptorAddKey.obj" \
	"$(INTDIR)\EnclyptorDecryptVerify.obj" \
	"$(INTDIR)\EnclyptorEncryptSign.obj" \
	"$(INTDIR)\EnclyptorKeyDB.obj" \
	"$(INTDIR)\EnclyptorProc.obj" \
	"$(INTDIR)\EnclyptorViewClipboard.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\PGPtray.res" \
	"$(INTDIR)\ResultsDlg.obj" \
	"$(INTDIR)\Taskbar.obj" \
	"$(INTDIR)\viewers.obj" \
	"$(INTDIR)\Wordwrap.obj"

"$(OUTDIR)\PGPtray.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPTray_"
# PROP BASE Intermediate_Dir "PGPTray_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPTray\debug"
# PROP Intermediate_Dir "PGPTray\debug"
# PROP Target_Dir ""
OUTDIR=.\PGPTray\debug
INTDIR=.\PGPTray\debug

ALL : "$(OUTDIR)\PGPtray.exe"

CLEAN : 
	-@erase "$(INTDIR)\Actions.obj"
	-@erase "$(INTDIR)\ClipBoard.obj"
	-@erase "$(INTDIR)\EnclyptorAddKey.obj"
	-@erase "$(INTDIR)\EnclyptorDecryptVerify.obj"
	-@erase "$(INTDIR)\EnclyptorEncryptSign.obj"
	-@erase "$(INTDIR)\EnclyptorKeyDB.obj"
	-@erase "$(INTDIR)\EnclyptorProc.obj"
	-@erase "$(INTDIR)\EnclyptorViewClipboard.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\PGPtray.res"
	-@erase "$(INTDIR)\ResultsDlg.obj"
	-@erase "$(INTDIR)\Taskbar.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\viewers.obj"
	-@erase "$(INTDIR)\Wordwrap.obj"
	-@erase "$(OUTDIR)\PGPtray.exe"
	-@erase "$(OUTDIR)\PGPtray.ilk"
	-@erase "$(OUTDIR)\PGPtray.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPtray.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPTray\debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPtray.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPtray.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib winmm.lib pgpcmdlg\debug\pgpcmdlg.lib pgpwctx\debug\pgpwctx.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib\
 pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib winmm.lib\
 pgpcmdlg\debug\pgpcmdlg.lib pgpwctx\debug\pgpwctx.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/PGPtray.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/PGPtray.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Actions.obj" \
	"$(INTDIR)\ClipBoard.obj" \
	"$(INTDIR)\EnclyptorAddKey.obj" \
	"$(INTDIR)\EnclyptorDecryptVerify.obj" \
	"$(INTDIR)\EnclyptorEncryptSign.obj" \
	"$(INTDIR)\EnclyptorKeyDB.obj" \
	"$(INTDIR)\EnclyptorProc.obj" \
	"$(INTDIR)\EnclyptorViewClipboard.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\PGPtray.res" \
	"$(INTDIR)\ResultsDlg.obj" \
	"$(INTDIR)\Taskbar.obj" \
	"$(INTDIR)\viewers.obj" \
	"$(INTDIR)\Wordwrap.obj"

"$(OUTDIR)\PGPtray.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPTray_"
# PROP BASE Intermediate_Dir "PGPTray_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPTray\evaldebug"
# PROP Intermediate_Dir "PGPTray\evaldebug"
# PROP Target_Dir ""
OUTDIR=.\PGPTray\evaldebug
INTDIR=.\PGPTray\evaldebug

ALL : "$(OUTDIR)\PGPtray.exe"

CLEAN : 
	-@erase "$(INTDIR)\Actions.obj"
	-@erase "$(INTDIR)\ClipBoard.obj"
	-@erase "$(INTDIR)\EnclyptorAddKey.obj"
	-@erase "$(INTDIR)\EnclyptorDecryptVerify.obj"
	-@erase "$(INTDIR)\EnclyptorEncryptSign.obj"
	-@erase "$(INTDIR)\EnclyptorKeyDB.obj"
	-@erase "$(INTDIR)\EnclyptorProc.obj"
	-@erase "$(INTDIR)\EnclyptorViewClipboard.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\PGPtray.res"
	-@erase "$(INTDIR)\ResultsDlg.obj"
	-@erase "$(INTDIR)\Taskbar.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\viewers.obj"
	-@erase "$(INTDIR)\Wordwrap.obj"
	-@erase "$(OUTDIR)\PGPtray.exe"
	-@erase "$(OUTDIR)\PGPtray.ilk"
	-@erase "$(OUTDIR)\PGPtray.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MTd /W4 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/PGPtray.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PGPTray\evaldebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPtray.res" /d "_DEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPtray.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib winmm.lib pgpcmdlg\debug\pgpcmdlg.lib pgpwctx\debug\pgpwctx.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib winmm.lib pgpcmdlg\evaldebug\pgpcmdlg.lib pgpwctx\evaldebug\pgpwctx.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib\
 pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib winmm.lib\
 pgpcmdlg\evaldebug\pgpcmdlg.lib pgpwctx\evaldebug\pgpwctx.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/PGPtray.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/PGPtray.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Actions.obj" \
	"$(INTDIR)\ClipBoard.obj" \
	"$(INTDIR)\EnclyptorAddKey.obj" \
	"$(INTDIR)\EnclyptorDecryptVerify.obj" \
	"$(INTDIR)\EnclyptorEncryptSign.obj" \
	"$(INTDIR)\EnclyptorKeyDB.obj" \
	"$(INTDIR)\EnclyptorProc.obj" \
	"$(INTDIR)\EnclyptorViewClipboard.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\PGPtray.res" \
	"$(INTDIR)\ResultsDlg.obj" \
	"$(INTDIR)\Taskbar.obj" \
	"$(INTDIR)\viewers.obj" \
	"$(INTDIR)\Wordwrap.obj"

"$(OUTDIR)\PGPtray.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPTray0"
# PROP BASE Intermediate_Dir "PGPTray0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pgptray\EvalRelease"
# PROP Intermediate_Dir "pgptray\EvalRelease"
# PROP Target_Dir ""
OUTDIR=.\pgptray\EvalRelease
INTDIR=.\pgptray\EvalRelease

ALL : "$(OUTDIR)\PGPtray.exe"

CLEAN : 
	-@erase "$(INTDIR)\Actions.obj"
	-@erase "$(INTDIR)\ClipBoard.obj"
	-@erase "$(INTDIR)\EnclyptorAddKey.obj"
	-@erase "$(INTDIR)\EnclyptorDecryptVerify.obj"
	-@erase "$(INTDIR)\EnclyptorEncryptSign.obj"
	-@erase "$(INTDIR)\EnclyptorKeyDB.obj"
	-@erase "$(INTDIR)\EnclyptorProc.obj"
	-@erase "$(INTDIR)\EnclyptorViewClipboard.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\PGPtray.res"
	-@erase "$(INTDIR)\ResultsDlg.obj"
	-@erase "$(INTDIR)\Taskbar.obj"
	-@erase "$(INTDIR)\viewers.obj"
	-@erase "$(INTDIR)\Wordwrap.obj"
	-@erase "$(OUTDIR)\PGPtray.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W4 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MT /W4 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "EVAL_TIMEOUT" /Fp"$(INTDIR)/PGPtray.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\pgptray\EvalRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPtray.res" /d "NDEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPtray.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib winmm.lib pgpcmdlg\release\pgpcmdlg.lib pgpwctx\release\pgpwctx.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib winmm.lib pgpcmdlg\evalrelease\pgpcmdlg.lib pgpwctx\evalrelease\pgpwctx.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\pgp.lib lib\simple.lib lib\keydb.lib\
 pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib winmm.lib\
 pgpcmdlg\evalrelease\pgpcmdlg.lib pgpwctx\evalrelease\pgpwctx.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/PGPtray.pdb" /machine:I386\
 /out:"$(OUTDIR)/PGPtray.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Actions.obj" \
	"$(INTDIR)\ClipBoard.obj" \
	"$(INTDIR)\EnclyptorAddKey.obj" \
	"$(INTDIR)\EnclyptorDecryptVerify.obj" \
	"$(INTDIR)\EnclyptorEncryptSign.obj" \
	"$(INTDIR)\EnclyptorKeyDB.obj" \
	"$(INTDIR)\EnclyptorProc.obj" \
	"$(INTDIR)\EnclyptorViewClipboard.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\PGPtray.res" \
	"$(INTDIR)\ResultsDlg.obj" \
	"$(INTDIR)\Taskbar.obj" \
	"$(INTDIR)\viewers.obj" \
	"$(INTDIR)\Wordwrap.obj"

"$(OUTDIR)\PGPtray.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "PGPTray - Win32 Release"
# Name "PGPTray - Win32 Debug"
# Name "PGPTray - Win32 Eval_Debug"
# Name "PGPTray - Win32 Eval_Release"

!IF  "$(CFG)" == "PGPTray - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\PGPtray\Wordwrap.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_WORDW=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\errs.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Wordwrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_WORDW=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\errs.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Wordwrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_WORDW=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\errs.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Wordwrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_WORDW=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\errs.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Wordwrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\EnclyptorAddKey.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_ENCLY=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorAddKey.obj" : $(SOURCE) $(DEP_CPP_ENCLY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_ENCLY=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorAddKey.obj" : $(SOURCE) $(DEP_CPP_ENCLY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_ENCLY=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorAddKey.obj" : $(SOURCE) $(DEP_CPP_ENCLY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_ENCLY=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorAddKey.obj" : $(SOURCE) $(DEP_CPP_ENCLY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\EnclyptorDecryptVerify.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_ENCLYP=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\messagestrings.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_ENCLYP)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_ENCLYP=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\messagestrings.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_ENCLYP)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_ENCLYP=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\messagestrings.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_ENCLYP)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_ENCLYP=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\messagestrings.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_ENCLYP)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\EnclyptorEncryptSign.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_ENCLYPT=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorEncryptSign.obj" : $(SOURCE) $(DEP_CPP_ENCLYPT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_ENCLYPT=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorEncryptSign.obj" : $(SOURCE) $(DEP_CPP_ENCLYPT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_ENCLYPT=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorEncryptSign.obj" : $(SOURCE) $(DEP_CPP_ENCLYPT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_ENCLYPT=\
	".\Include\pgpDebug.h"\
	".\include\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\Include\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\include\PGPRecip.h"\
	".\PGPtray\..\include\spgp.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorEncryptSign.obj" : $(SOURCE) $(DEP_CPP_ENCLYPT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\EnclyptorKeyDB.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_ENCLYPTO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorKeyDB.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_ENCLYPTO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorKeyDB.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_ENCLYPTO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorKeyDB.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_ENCLYPTO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorKeyDB.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorKeyDB.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\EnclyptorProc.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_ENCLYPTOR=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\viewers.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorProc.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_ENCLYPTOR=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\viewers.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorProc.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_ENCLYPTOR=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\viewers.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorProc.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_ENCLYPTOR=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\viewers.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorProc.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\EnclyptorViewClipboard.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_ENCLYPTORV=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorViewClipboard.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTORV)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_ENCLYPTORV=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorViewClipboard.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTORV)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_ENCLYPTORV=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorViewClipboard.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTORV)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_ENCLYPTORV=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\DEFINE.H"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\EnclyptorViewClipboard.obj" : $(SOURCE) $(DEP_CPP_ENCLYPTORV)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\Main.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_MAIN_=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\pgpwctx\shelldecryptverify.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorProc.h"\
	".\PGPtray\Taskbar.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_MAIN_=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\pgpwctx\shelldecryptverify.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorProc.h"\
	".\PGPtray\Taskbar.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_MAIN_=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\pgpwctx\shelldecryptverify.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorProc.h"\
	".\PGPtray\Taskbar.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_MAIN_=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\PGPcomdlg.h"\
	".\PGPtray\..\include\PGPkeydb.h"\
	".\PGPtray\..\include\PGPphrase.h"\
	".\PGPtray\..\pgpwctx\shelldecryptverify.h"\
	".\PGPtray\actions.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorProc.h"\
	".\PGPtray\Taskbar.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\PGPtray.rc

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_RSC_PGPTR=\
	".\PGPtray\..\include\pgpversion.h"\
	".\PGPtray\pgptrayver.rc"\
	".\PGPtray\res\icon1.ico"\
	".\PGPtray\Res\Image1.ico"\
	".\PGPtray\res\taskbar.ico"\
	".\PGPtray\Res\ted1.wav"\
	".\PGPtray\Res\ted2.wav"\
	".\PGPtray\Res\ted3.wav"\
	

"$(INTDIR)\PGPtray.res" : $(SOURCE) $(DEP_RSC_PGPTR) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPtray.res" /i "PGPtray" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_RSC_PGPTR=\
	".\PGPtray\..\include\pgpversion.h"\
	".\PGPtray\pgptrayver.rc"\
	".\PGPtray\res\icon1.ico"\
	".\PGPtray\Res\Image1.ico"\
	".\PGPtray\res\taskbar.ico"\
	".\PGPtray\Res\ted1.wav"\
	".\PGPtray\Res\ted2.wav"\
	".\PGPtray\Res\ted3.wav"\
	

"$(INTDIR)\PGPtray.res" : $(SOURCE) $(DEP_RSC_PGPTR) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPtray.res" /i "PGPtray" /d "_DEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_RSC_PGPTR=\
	".\PGPtray\..\include\pgpversion.h"\
	".\PGPtray\pgptrayver.rc"\
	".\PGPtray\res\icon1.ico"\
	".\PGPtray\Res\Image1.ico"\
	".\PGPtray\res\taskbar.ico"\
	".\PGPtray\Res\ted1.wav"\
	".\PGPtray\Res\ted2.wav"\
	".\PGPtray\Res\ted3.wav"\
	

"$(INTDIR)\PGPtray.res" : $(SOURCE) $(DEP_RSC_PGPTR) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPtray.res" /i "PGPtray" /d "_DEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_RSC_PGPTR=\
	".\PGPtray\..\include\pgpversion.h"\
	".\PGPtray\pgptrayver.rc"\
	".\PGPtray\res\icon1.ico"\
	".\PGPtray\Res\Image1.ico"\
	".\PGPtray\res\taskbar.ico"\
	".\PGPtray\Res\ted1.wav"\
	".\PGPtray\Res\ted2.wav"\
	".\PGPtray\Res\ted3.wav"\
	

"$(INTDIR)\PGPtray.res" : $(SOURCE) $(DEP_RSC_PGPTR) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPtray.res" /i "PGPtray" /d "NDEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\Taskbar.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"


"$(INTDIR)\Taskbar.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"


"$(INTDIR)\Taskbar.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"


"$(INTDIR)\Taskbar.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"


"$(INTDIR)\Taskbar.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\ClipBoard.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_CLIPB=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ClipBoard.obj" : $(SOURCE) $(DEP_CPP_CLIPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_CLIPB=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ClipBoard.obj" : $(SOURCE) $(DEP_CPP_CLIPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_CLIPB=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ClipBoard.obj" : $(SOURCE) $(DEP_CPP_CLIPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_CLIPB=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ClipBoard.obj" : $(SOURCE) $(DEP_CPP_CLIPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\ResultsDlg.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_RESUL=\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\viewers.h"\
	

"$(INTDIR)\ResultsDlg.obj" : $(SOURCE) $(DEP_CPP_RESUL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_RESUL=\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\viewers.h"\
	

"$(INTDIR)\ResultsDlg.obj" : $(SOURCE) $(DEP_CPP_RESUL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_RESUL=\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\viewers.h"\
	

"$(INTDIR)\ResultsDlg.obj" : $(SOURCE) $(DEP_CPP_RESUL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_RESUL=\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	".\PGPtray\errs.h"\
	".\PGPtray\viewers.h"\
	

"$(INTDIR)\ResultsDlg.obj" : $(SOURCE) $(DEP_CPP_RESUL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\viewers.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_VIEWE=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\viewers.obj" : $(SOURCE) $(DEP_CPP_VIEWE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_VIEWE=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\viewers.obj" : $(SOURCE) $(DEP_CPP_VIEWE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_VIEWE=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\viewers.obj" : $(SOURCE) $(DEP_CPP_VIEWE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_VIEWE=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorViewClipboard.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\viewers.obj" : $(SOURCE) $(DEP_CPP_VIEWE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPtray\Actions.c

!IF  "$(CFG)" == "PGPTray - Win32 Release"

DEP_CPP_ACTIO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\errs.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Actions.obj" : $(SOURCE) $(DEP_CPP_ACTIO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Debug"

DEP_CPP_ACTIO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\errs.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Actions.obj" : $(SOURCE) $(DEP_CPP_ACTIO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Debug"

DEP_CPP_ACTIO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\errs.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Actions.obj" : $(SOURCE) $(DEP_CPP_ACTIO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPTray - Win32 Eval_Release"

DEP_CPP_ACTIO=\
	".\Include\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\Include\pgpLeaks.h"\
	".\INCLUDE\pgpTypes.h"\
	".\PGPtray\..\include\config.h"\
	".\PGPtray\..\include\errs.h"\
	".\PGPtray\..\include\pgpMem.h"\
	".\PGPtray\ClipBoard.h"\
	".\PGPtray\DEFINE.H"\
	".\PGPtray\EnclyptorAddKey.h"\
	".\PGPtray\EnclyptorDecryptVerify.h"\
	".\PGPtray\EnclyptorEncryptSign.h"\
	".\PGPtray\EnclyptorKeyDB.h"\
	".\PGPtray\Wordwrap.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Actions.obj" : $(SOURCE) $(DEP_CPP_ACTIO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
