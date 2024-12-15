# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=pgpExch - Win32 Eval_Debug
!MESSAGE No configuration specified.  Defaulting to pgpExch - Win32 Eval_Debug.
!ENDIF 

!IF "$(CFG)" != "pgpExch - Win32 Release" && "$(CFG)" !=\
 "pgpExch - Win32 Debug" && "$(CFG)" != "pgpExch - Win32 Eval_Debug" && "$(CFG)"\
 != "pgpExch - Win32 Eval_Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "pgpExch.mak" CFG="pgpExch - Win32 Eval_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pgpExch - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpExch - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpExch - Win32 Eval_Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpExch - Win32 Eval_Release" (based on\
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
# PROP Target_Last_Scanned "pgpExch - Win32 Eval_Release"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "pgpExch - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\pgpExch\Release"
# PROP Intermediate_Dir ".\pgpExch\Release"
# PROP Target_Dir ""
OUTDIR=.\pgpExch\Release
INTDIR=.\pgpExch\Release

ALL : "$(OUTDIR)\pgpExch.dll"

CLEAN : 
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\DEBUG.OBJ"
	-@erase "$(INTDIR)\ExchAddKey.obj"
	-@erase "$(INTDIR)\ExchDecryptVerify.obj"
	-@erase "$(INTDIR)\ExchEncryptSign.obj"
	-@erase "$(INTDIR)\ExchKeyDB.obj"
	-@erase "$(INTDIR)\EXT.OBJ"
	-@erase "$(INTDIR)\GUIDS.OBJ"
	-@erase "$(INTDIR)\pgpExch.pch"
	-@erase "$(INTDIR)\pgpExch.res"
	-@erase "$(INTDIR)\PRSHT.OBJ"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\STDINC.OBJ"
	-@erase "$(INTDIR)\STDNOTE.OBJ"
	-@erase "$(INTDIR)\UI.OBJ"
	-@erase "$(OUTDIR)\pgpExch.dll"
	-@erase "$(OUTDIR)\pgpExch.exp"
	-@erase "$(OUTDIR)\pgpExch.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /Yu"stdinc.h" /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)/pgpExch.pch" /Yu"stdinc.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\.\pgpExch\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpExch.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgpExch.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\Release\pgpcmdlg.lib .\PGPRecip\Release\pgprecip.lib .\PGPPhrase\Release\pgpphrase.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib
LINK32_FLAGS=msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib\
 user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib\
 .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\Release\pgpcmdlg.lib\
 .\PGPRecip\Release\pgprecip.lib .\PGPPhrase\Release\pgpphrase.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/pgpExch.pdb"\
 /machine:I386 /nodefaultlib /def:".\pgpExch\pgpExch.DEF"\
 /out:"$(OUTDIR)/pgpExch.dll" /implib:"$(OUTDIR)/pgpExch.lib" 
DEF_FILE= \
	".\pgpExch\pgpExch.DEF"
LINK32_OBJS= \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\DEBUG.OBJ" \
	"$(INTDIR)\ExchAddKey.obj" \
	"$(INTDIR)\ExchDecryptVerify.obj" \
	"$(INTDIR)\ExchEncryptSign.obj" \
	"$(INTDIR)\ExchKeyDB.obj" \
	"$(INTDIR)\EXT.OBJ" \
	"$(INTDIR)\GUIDS.OBJ" \
	"$(INTDIR)\pgpExch.res" \
	"$(INTDIR)\PRSHT.OBJ" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\STDINC.OBJ" \
	"$(INTDIR)\STDNOTE.OBJ" \
	"$(INTDIR)\UI.OBJ"

"$(OUTDIR)\pgpExch.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\pgpExch\Debug"
# PROP Intermediate_Dir ".\pgpExch\Debug"
# PROP Target_Dir ""
OUTDIR=.\pgpExch\Debug
INTDIR=.\pgpExch\Debug

ALL : "$(OUTDIR)\pgpExch.dll"

CLEAN : 
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\DEBUG.OBJ"
	-@erase "$(INTDIR)\ExchAddKey.obj"
	-@erase "$(INTDIR)\ExchDecryptVerify.obj"
	-@erase "$(INTDIR)\ExchEncryptSign.obj"
	-@erase "$(INTDIR)\ExchKeyDB.obj"
	-@erase "$(INTDIR)\EXT.OBJ"
	-@erase "$(INTDIR)\GUIDS.OBJ"
	-@erase "$(INTDIR)\pgpExch.pch"
	-@erase "$(INTDIR)\pgpExch.res"
	-@erase "$(INTDIR)\PRSHT.OBJ"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\STDINC.OBJ"
	-@erase "$(INTDIR)\STDNOTE.OBJ"
	-@erase "$(INTDIR)\UI.OBJ"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\pgpExch.dll"
	-@erase "$(OUTDIR)\pgpExch.exp"
	-@erase "$(OUTDIR)\pgpExch.ilk"
	-@erase "$(OUTDIR)\pgpExch.lib"
	-@erase "$(OUTDIR)\pgpExch.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /Yu"stdinc.h" /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)/pgpExch.pch" /Yu"stdinc.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\.\pgpExch\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpExch.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgpExch.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\Debug\pgpcmdlg.lib .\PGPRecip\Debug\pgprecip.lib .\PGPPhrase\Debug\pgpphrase.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib
LINK32_FLAGS=msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib\
 user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib\
 .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\Debug\pgpcmdlg.lib\
 .\PGPRecip\Debug\pgprecip.lib .\PGPPhrase\Debug\pgpphrase.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/pgpExch.pdb" /debug\
 /machine:I386 /nodefaultlib /def:".\pgpExch\pgpExch.DEF"\
 /out:"$(OUTDIR)/pgpExch.dll" /implib:"$(OUTDIR)/pgpExch.lib" 
DEF_FILE= \
	".\pgpExch\pgpExch.DEF"
LINK32_OBJS= \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\DEBUG.OBJ" \
	"$(INTDIR)\ExchAddKey.obj" \
	"$(INTDIR)\ExchDecryptVerify.obj" \
	"$(INTDIR)\ExchEncryptSign.obj" \
	"$(INTDIR)\ExchKeyDB.obj" \
	"$(INTDIR)\EXT.OBJ" \
	"$(INTDIR)\GUIDS.OBJ" \
	"$(INTDIR)\pgpExch.res" \
	"$(INTDIR)\PRSHT.OBJ" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\STDINC.OBJ" \
	"$(INTDIR)\STDNOTE.OBJ" \
	"$(INTDIR)\UI.OBJ"

"$(OUTDIR)\pgpExch.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pgpExch_"
# PROP BASE Intermediate_Dir "pgpExch_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\pgpExch\EvalDebug"
# PROP Intermediate_Dir ".\pgpExch\EvalDebug"
# PROP Target_Dir ""
OUTDIR=.\pgpExch\EvalDebug
INTDIR=.\pgpExch\EvalDebug

ALL : "$(OUTDIR)\pgpExch.dll"

CLEAN : 
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\DEBUG.OBJ"
	-@erase "$(INTDIR)\ExchAddKey.obj"
	-@erase "$(INTDIR)\ExchDecryptVerify.obj"
	-@erase "$(INTDIR)\ExchEncryptSign.obj"
	-@erase "$(INTDIR)\ExchKeyDB.obj"
	-@erase "$(INTDIR)\EXT.OBJ"
	-@erase "$(INTDIR)\GUIDS.OBJ"
	-@erase "$(INTDIR)\pgpExch.pch"
	-@erase "$(INTDIR)\pgpExch.res"
	-@erase "$(INTDIR)\PRSHT.OBJ"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\STDINC.OBJ"
	-@erase "$(INTDIR)\STDNOTE.OBJ"
	-@erase "$(INTDIR)\UI.OBJ"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\pgpExch.dll"
	-@erase "$(OUTDIR)\pgpExch.exp"
	-@erase "$(OUTDIR)\pgpExch.ilk"
	-@erase "$(OUTDIR)\pgpExch.lib"
	-@erase "$(OUTDIR)\pgpExch.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /Yu"stdinc.h" /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Yu"stdinc.h" /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/pgpExch.pch"\
 /Yu"stdinc.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\.\pgpExch\EvalDebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpExch.res" /d "_DEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgpExch.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\Debug\pgpcmdlg.lib .\PGPRecip\Debug\pgprecip.lib .\PGPPhrase\Debug\pgpphrase.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib
# ADD LINK32 msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\EvalDebug\pgpcmdlg.lib .\PGPRecip\Debug\pgprecip.lib .\PGPPhrase\Debug\pgpphrase.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib
LINK32_FLAGS=msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib\
 user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib\
 .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib\
 .\PGPcmdlg\EvalDebug\pgpcmdlg.lib .\PGPRecip\Debug\pgprecip.lib\
 .\PGPPhrase\Debug\pgpphrase.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/pgpExch.pdb" /debug /machine:I386\
 /nodefaultlib /def:".\pgpExch\pgpExch.DEF" /out:"$(OUTDIR)/pgpExch.dll"\
 /implib:"$(OUTDIR)/pgpExch.lib" 
DEF_FILE= \
	".\pgpExch\pgpExch.DEF"
LINK32_OBJS= \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\DEBUG.OBJ" \
	"$(INTDIR)\ExchAddKey.obj" \
	"$(INTDIR)\ExchDecryptVerify.obj" \
	"$(INTDIR)\ExchEncryptSign.obj" \
	"$(INTDIR)\ExchKeyDB.obj" \
	"$(INTDIR)\EXT.OBJ" \
	"$(INTDIR)\GUIDS.OBJ" \
	"$(INTDIR)\pgpExch.res" \
	"$(INTDIR)\PRSHT.OBJ" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\STDINC.OBJ" \
	"$(INTDIR)\STDNOTE.OBJ" \
	"$(INTDIR)\UI.OBJ"

"$(OUTDIR)\pgpExch.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pgpExch0"
# PROP BASE Intermediate_Dir "pgpExch0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\pgpExch\EvalRelease"
# PROP Intermediate_Dir ".\pgpExch\EvalRelease"
# PROP Target_Dir ""
OUTDIR=.\pgpExch\EvalRelease
INTDIR=.\pgpExch\EvalRelease

ALL : "$(OUTDIR)\pgpExch.dll"

CLEAN : 
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\DEBUG.OBJ"
	-@erase "$(INTDIR)\ExchAddKey.obj"
	-@erase "$(INTDIR)\ExchDecryptVerify.obj"
	-@erase "$(INTDIR)\ExchEncryptSign.obj"
	-@erase "$(INTDIR)\ExchKeyDB.obj"
	-@erase "$(INTDIR)\EXT.OBJ"
	-@erase "$(INTDIR)\GUIDS.OBJ"
	-@erase "$(INTDIR)\pgpExch.pch"
	-@erase "$(INTDIR)\pgpExch.res"
	-@erase "$(INTDIR)\PRSHT.OBJ"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\STDINC.OBJ"
	-@erase "$(INTDIR)\STDNOTE.OBJ"
	-@erase "$(INTDIR)\UI.OBJ"
	-@erase "$(OUTDIR)\pgpExch.dll"
	-@erase "$(OUTDIR)\pgpExch.exp"
	-@erase "$(OUTDIR)\pgpExch.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /Yu"stdinc.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Yu"stdinc.h" /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/pgpExch.pch"\
 /Yu"stdinc.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\.\pgpExch\EvalRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpExch.res" /d "NDEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgpExch.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\Release\pgpcmdlg.lib .\PGPRecip\Release\pgprecip.lib .\PGPPhrase\Release\pgpphrase.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib
# ADD LINK32 msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib .\PGPcmdlg\EvalRelease\pgpcmdlg.lib .\PGPRecip\Release\pgprecip.lib .\PGPPhrase\Release\pgpphrase.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib
LINK32_FLAGS=msvcrt.lib oldnames.lib kernel32.lib wsock32.lib advapi32.lib\
 user32.lib gdi32.lib comdlg32.lib winspool.lib mapi32.lib uuid.lib version.lib\
 .\lib\pgp.lib .\lib\simple.lib .\lib\keydb.lib\
 .\PGPcmdlg\EvalRelease\pgpcmdlg.lib .\PGPRecip\Release\pgprecip.lib\
 .\PGPPhrase\Release\pgpphrase.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/pgpExch.pdb" /machine:I386 /nodefaultlib\
 /def:".\pgpExch\pgpExch.DEF" /out:"$(OUTDIR)/pgpExch.dll"\
 /implib:"$(OUTDIR)/pgpExch.lib" 
DEF_FILE= \
	".\pgpExch\pgpExch.DEF"
LINK32_OBJS= \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\DEBUG.OBJ" \
	"$(INTDIR)\ExchAddKey.obj" \
	"$(INTDIR)\ExchDecryptVerify.obj" \
	"$(INTDIR)\ExchEncryptSign.obj" \
	"$(INTDIR)\ExchKeyDB.obj" \
	"$(INTDIR)\EXT.OBJ" \
	"$(INTDIR)\GUIDS.OBJ" \
	"$(INTDIR)\pgpExch.res" \
	"$(INTDIR)\PRSHT.OBJ" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\STDINC.OBJ" \
	"$(INTDIR)\STDNOTE.OBJ" \
	"$(INTDIR)\UI.OBJ"

"$(OUTDIR)\pgpExch.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "pgpExch - Win32 Release"
# Name "pgpExch - Win32 Debug"
# Name "pgpExch - Win32 Eval_Debug"
# Name "pgpExch - Win32 Eval_Release"

!IF  "$(CFG)" == "pgpExch - Win32 Release"

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\pgpExch\DEBUG.CPP
DEP_CPP_DEBUG=\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\DEBUG.OBJ" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\DEBUG.OBJ" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\DEBUG.OBJ" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\DEBUG.OBJ" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\ExchAddKey.cpp
DEP_CPP_EXCHA=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\ExchAddKey.obj" : $(SOURCE) $(DEP_CPP_EXCHA) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\ExchAddKey.obj" : $(SOURCE) $(DEP_CPP_EXCHA) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\ExchAddKey.obj" : $(SOURCE) $(DEP_CPP_EXCHA) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\ExchAddKey.obj" : $(SOURCE) $(DEP_CPP_EXCHA) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\ExchDecryptVerify.cpp
DEP_CPP_EXCHD=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\ExchDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_EXCHD) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\ExchDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_EXCHD) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\ExchDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_EXCHD) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\ExchDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_EXCHD) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\ExchEncryptSign.cpp

!IF  "$(CFG)" == "pgpExch - Win32 Release"

DEP_CPP_EXCHE=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ExchEncryptSign.obj" : $(SOURCE) $(DEP_CPP_EXCHE) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

DEP_CPP_EXCHE=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ExchEncryptSign.obj" : $(SOURCE) $(DEP_CPP_EXCHE) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

DEP_CPP_EXCHE=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ExchEncryptSign.obj" : $(SOURCE) $(DEP_CPP_EXCHE) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

DEP_CPP_EXCHE=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ExchEncryptSign.obj" : $(SOURCE) $(DEP_CPP_EXCHE) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\ExchKeyDB.cpp
DEP_CPP_EXCHK=\
	".\include\config.h"\
	".\INCLUDE\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\STDINC.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\ExchKeyDB.obj" : $(SOURCE) $(DEP_CPP_EXCHK) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\ExchKeyDB.obj" : $(SOURCE) $(DEP_CPP_EXCHK) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\ExchKeyDB.obj" : $(SOURCE) $(DEP_CPP_EXCHK) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\ExchKeyDB.obj" : $(SOURCE) $(DEP_CPP_EXCHK) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\EXT.CPP

!IF  "$(CFG)" == "pgpExch - Win32 Release"

DEP_CPP_EXT_C=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\outlook.h"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\re_io.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EXT.OBJ" : $(SOURCE) $(DEP_CPP_EXT_C) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

DEP_CPP_EXT_C=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\outlook.h"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\re_io.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EXT.OBJ" : $(SOURCE) $(DEP_CPP_EXT_C) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

DEP_CPP_EXT_C=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\outlook.h"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\re_io.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EXT.OBJ" : $(SOURCE) $(DEP_CPP_EXT_C) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

DEP_CPP_EXT_C=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\PGPPhrase.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\include\PGPRecip.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\include\spgp.h"\
	".\pgpExch\BlockUtils.h"\
	".\pgpExch\ExchKeyDB.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\outlook.h"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\re_io.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EXT.OBJ" : $(SOURCE) $(DEP_CPP_EXT_C) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\GUIDS.CPP
DEP_CPP_GUIDS=\
	".\pgpExch\GUIDS.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\GUIDS.OBJ" : $(SOURCE) $(DEP_CPP_GUIDS) "$(INTDIR)"
   $(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\GUIDS.OBJ" : $(SOURCE) $(DEP_CPP_GUIDS) "$(INTDIR)"
   $(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\GUIDS.OBJ" : $(SOURCE) $(DEP_CPP_GUIDS) "$(INTDIR)"
   $(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\GUIDS.OBJ" : $(SOURCE) $(DEP_CPP_GUIDS) "$(INTDIR)"
   $(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\pgpExch.RC
DEP_RSC_PGPEX=\
	".\pgpExch\..\include\pgpversion.h"\
	".\pgpExch\pgpExchVer.rc"\
	".\pgpExch\Res\addkey.bmp"\
	".\pgpExch\Res\both.bmp"\
	".\pgpExch\Res\decrypt.bmp"\
	".\pgpExch\Res\encrypt.bmp"\
	".\pgpExch\Res\pgpkeys.bmp"\
	".\pgpExch\Res\sign.bmp"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\pgpExch.res" : $(SOURCE) $(DEP_RSC_PGPEX) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpExch.res" /i "pgpExch" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\pgpExch.res" : $(SOURCE) $(DEP_RSC_PGPEX) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpExch.res" /i "pgpExch" /d "_DEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\pgpExch.res" : $(SOURCE) $(DEP_RSC_PGPEX) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpExch.res" /i "pgpExch" /d "_DEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\pgpExch.res" : $(SOURCE) $(DEP_RSC_PGPEX) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpExch.res" /i "pgpExch" /d "NDEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\PRSHT.CPP
DEP_CPP_PRSHT=\
	".\include\config.h"\
	".\include\PGPcomdlg.h"\
	".\INCLUDE\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\include\pgpKeyDB.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\include\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\PRSHT.OBJ" : $(SOURCE) $(DEP_CPP_PRSHT) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\PRSHT.OBJ" : $(SOURCE) $(DEP_CPP_PRSHT) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\PRSHT.OBJ" : $(SOURCE) $(DEP_CPP_PRSHT) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\PRSHT.OBJ" : $(SOURCE) $(DEP_CPP_PRSHT) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\re_io.c
DEP_CPP_RE_IO=\
	".\pgpExch\re_io.h"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\STDINC.CPP
DEP_CPP_STDIN=\
	".\pgpExch\STDINC.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"

# ADD CPP /Yc"stdinc.h"

BuildCmds= \
	$(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)/pgpExch.pch" /Yc"stdinc.h"\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDINC.OBJ" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pgpExch.pch" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

# ADD CPP /Yc"stdinc.h"

BuildCmds= \
	$(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)/pgpExch.pch" /Yc"stdinc.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDINC.OBJ" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pgpExch.pch" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

# ADD BASE CPP /Yc"stdinc.h"
# ADD CPP /Yc"stdinc.h"

BuildCmds= \
	$(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/pgpExch.pch"\
 /Yc"stdinc.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDINC.OBJ" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pgpExch.pch" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

# ADD BASE CPP /Yc"stdinc.h"
# ADD CPP /Yc"stdinc.h"

BuildCmds= \
	$(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/pgpExch.pch"\
 /Yc"stdinc.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDINC.OBJ" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pgpExch.pch" : $(SOURCE) $(DEP_CPP_STDIN) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\STDNOTE.CPP
DEP_CPP_STDNO=\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\STDNOTE.OBJ" : $(SOURCE) $(DEP_CPP_STDNO) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\STDNOTE.OBJ" : $(SOURCE) $(DEP_CPP_STDNO) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\STDNOTE.OBJ" : $(SOURCE) $(DEP_CPP_STDNO) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\STDNOTE.OBJ" : $(SOURCE) $(DEP_CPP_STDNO) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\UI.CPP
DEP_CPP_UI_CP=\
	".\pgpExch\GUIDS.H"\
	".\pgpExch\pgpExch.h"\
	".\pgpExch\STDINC.H"\
	".\pgpExch\UI.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"


"$(INTDIR)\UI.OBJ" : $(SOURCE) $(DEP_CPP_UI_CP) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"


"$(INTDIR)\UI.OBJ" : $(SOURCE) $(DEP_CPP_UI_CP) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"


"$(INTDIR)\UI.OBJ" : $(SOURCE) $(DEP_CPP_UI_CP) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"


"$(INTDIR)\UI.OBJ" : $(SOURCE) $(DEP_CPP_UI_CP) "$(INTDIR)"\
 "$(INTDIR)\pgpExch.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\BlockUtils.cpp
DEP_CPP_BLOCK=\
	".\include\config.h"\
	".\INCLUDE\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\include\pgpMem.h"\
	".\include\pgpTypes.h"\
	".\pgpExch\BlockUtils.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "pgpExch - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".\include" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) /nologo /MT /W3 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "HAVE_CONFIG_H" /D "EVAL_TIMEOUT" /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpExch\pgpExch.DEF

!IF  "$(CFG)" == "pgpExch - Win32 Release"

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Debug"

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "pgpExch - Win32 Eval_Release"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
