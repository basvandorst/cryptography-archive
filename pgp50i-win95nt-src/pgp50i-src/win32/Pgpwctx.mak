# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=pgpwctx - Win32 Eval_Debug
!MESSAGE No configuration specified.  Defaulting to pgpwctx - Win32 Eval_Debug.
!ENDIF 

!IF "$(CFG)" != "pgpwctx - Win32 Release" && "$(CFG)" !=\
 "pgpwctx - Win32 Debug" && "$(CFG)" != "pgpwctx - Win32 Eval_Debug" && "$(CFG)"\
 != "pgpwctx - Win32 Eval_Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Pgpwctx.mak" CFG="pgpwctx - Win32 Eval_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pgpwctx - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpwctx - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpwctx - Win32 Eval_Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpwctx - Win32 Eval_Release" (based on\
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
# PROP Target_Last_Scanned "pgpwctx - Win32 Eval_Release"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pgpwctx\Release"
# PROP Intermediate_Dir "pgpwctx\Release"
# PROP Target_Dir ""
OUTDIR=.\pgpwctx\Release
INTDIR=.\pgpwctx\Release

ALL : "$(OUTDIR)\Pgpwctx.dll"

CLEAN : 
	-@erase "$(INTDIR)\Ctxmenu.obj"
	-@erase "$(INTDIR)\pgpwctx.obj"
	-@erase "$(INTDIR)\pgpwctx.res"
	-@erase "$(INTDIR)\ShellAddKey.obj"
	-@erase "$(INTDIR)\ShellDecryptVerify.obj"
	-@erase "$(INTDIR)\ShellEncryptSend.obj"
	-@erase "$(INTDIR)\ShellEncryptSign.obj"
	-@erase "$(INTDIR)\ShellVerify.obj"
	-@erase "$(INTDIR)\shexinit.obj"
	-@erase "$(INTDIR)\working.obj"
	-@erase "$(OUTDIR)\Pgpwctx.dll"
	-@erase "$(OUTDIR)\Pgpwctx.exp"
	-@erase "$(OUTDIR)\Pgpwctx.lib"
	-@erase "$(OUTDIR)\Pgpwctx.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Pgpwctx.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\pgpwctx\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpwctx.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Pgpwctx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib pgpcmdlg\release\pgpcmdlg.lib pgpphrase\release\pgpphrase.lib pgprecip\release\pgprecip.lib /nologo /subsystem:windows /dll /map /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib\
 pgpcmdlg\release\pgpcmdlg.lib pgpphrase\release\pgpphrase.lib\
 pgprecip\release\pgprecip.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Pgpwctx.pdb" /map:"$(INTDIR)/Pgpwctx.map" /machine:I386\
 /def:".\pgpwctx\ctxmenu.def" /out:"$(OUTDIR)/Pgpwctx.dll"\
 /implib:"$(OUTDIR)/Pgpwctx.lib" 
DEF_FILE= \
	".\pgpwctx\ctxmenu.def"
LINK32_OBJS= \
	"$(INTDIR)\Ctxmenu.obj" \
	"$(INTDIR)\pgpwctx.obj" \
	"$(INTDIR)\pgpwctx.res" \
	"$(INTDIR)\ShellAddKey.obj" \
	"$(INTDIR)\ShellDecryptVerify.obj" \
	"$(INTDIR)\ShellEncryptSend.obj" \
	"$(INTDIR)\ShellEncryptSign.obj" \
	"$(INTDIR)\ShellVerify.obj" \
	"$(INTDIR)\shexinit.obj" \
	"$(INTDIR)\working.obj"

"$(OUTDIR)\Pgpwctx.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pgpwctx\Debug"
# PROP Intermediate_Dir "pgpwctx\Debug"
# PROP Target_Dir ""
OUTDIR=.\pgpwctx\Debug
INTDIR=.\pgpwctx\Debug

ALL : "$(OUTDIR)\Pgpwctx.dll"

CLEAN : 
	-@erase "$(INTDIR)\Ctxmenu.obj"
	-@erase "$(INTDIR)\pgpwctx.obj"
	-@erase "$(INTDIR)\pgpwctx.res"
	-@erase "$(INTDIR)\ShellAddKey.obj"
	-@erase "$(INTDIR)\ShellDecryptVerify.obj"
	-@erase "$(INTDIR)\ShellEncryptSend.obj"
	-@erase "$(INTDIR)\ShellEncryptSign.obj"
	-@erase "$(INTDIR)\ShellVerify.obj"
	-@erase "$(INTDIR)\shexinit.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\working.obj"
	-@erase "$(OUTDIR)\Pgpwctx.dll"
	-@erase "$(OUTDIR)\Pgpwctx.exp"
	-@erase "$(OUTDIR)\Pgpwctx.ilk"
	-@erase "$(OUTDIR)\Pgpwctx.lib"
	-@erase "$(OUTDIR)\Pgpwctx.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Pgpwctx.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\pgpwctx\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpwctx.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Pgpwctx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib pgpcmdlg\debug\pgpcmdlg.lib pgpphrase\debug\pgpphrase.lib pgprecip\debug\pgprecip.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib\
 pgpcmdlg\debug\pgpcmdlg.lib pgpphrase\debug\pgpphrase.lib\
 pgprecip\debug\pgprecip.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Pgpwctx.pdb" /debug /machine:I386 /def:".\pgpwctx\ctxmenu.def"\
 /out:"$(OUTDIR)/Pgpwctx.dll" /implib:"$(OUTDIR)/Pgpwctx.lib" 
DEF_FILE= \
	".\pgpwctx\ctxmenu.def"
LINK32_OBJS= \
	"$(INTDIR)\Ctxmenu.obj" \
	"$(INTDIR)\pgpwctx.obj" \
	"$(INTDIR)\pgpwctx.res" \
	"$(INTDIR)\ShellAddKey.obj" \
	"$(INTDIR)\ShellDecryptVerify.obj" \
	"$(INTDIR)\ShellEncryptSend.obj" \
	"$(INTDIR)\ShellEncryptSign.obj" \
	"$(INTDIR)\ShellVerify.obj" \
	"$(INTDIR)\shexinit.obj" \
	"$(INTDIR)\working.obj"

"$(OUTDIR)\Pgpwctx.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pgpwctx_"
# PROP BASE Intermediate_Dir "pgpwctx_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pgpwctx\EvalDebug"
# PROP Intermediate_Dir "pgpwctx\EvalDebug"
# PROP Target_Dir ""
OUTDIR=.\pgpwctx\EvalDebug
INTDIR=.\pgpwctx\EvalDebug

ALL : "$(OUTDIR)\Pgpwctx.dll"

CLEAN : 
	-@erase "$(INTDIR)\Ctxmenu.obj"
	-@erase "$(INTDIR)\pgpwctx.obj"
	-@erase "$(INTDIR)\pgpwctx.res"
	-@erase "$(INTDIR)\ShellAddKey.obj"
	-@erase "$(INTDIR)\ShellDecryptVerify.obj"
	-@erase "$(INTDIR)\ShellEncryptSend.obj"
	-@erase "$(INTDIR)\ShellEncryptSign.obj"
	-@erase "$(INTDIR)\ShellVerify.obj"
	-@erase "$(INTDIR)\shexinit.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\working.obj"
	-@erase "$(OUTDIR)\Pgpwctx.dll"
	-@erase "$(OUTDIR)\Pgpwctx.exp"
	-@erase "$(OUTDIR)\Pgpwctx.ilk"
	-@erase "$(OUTDIR)\Pgpwctx.lib"
	-@erase "$(OUTDIR)\Pgpwctx.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/Pgpwctx.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\pgpwctx\EvalDebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpwctx.res" /d "_DEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Pgpwctx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib pgpcmdlg\debug\pgpcmdlg.lib pgpphrase\debug\pgpphrase.lib pgprecip\debug\pgprecip.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib pgpcmdlg\evaldebug\pgpcmdlg.lib pgpphrase\debug\pgpphrase.lib pgprecip\debug\pgprecip.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib\
 pgpcmdlg\evaldebug\pgpcmdlg.lib pgpphrase\debug\pgpphrase.lib\
 pgprecip\debug\pgprecip.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Pgpwctx.pdb" /debug /machine:I386 /def:".\pgpwctx\ctxmenu.def"\
 /out:"$(OUTDIR)/Pgpwctx.dll" /implib:"$(OUTDIR)/Pgpwctx.lib" 
DEF_FILE= \
	".\pgpwctx\ctxmenu.def"
LINK32_OBJS= \
	"$(INTDIR)\Ctxmenu.obj" \
	"$(INTDIR)\pgpwctx.obj" \
	"$(INTDIR)\pgpwctx.res" \
	"$(INTDIR)\ShellAddKey.obj" \
	"$(INTDIR)\ShellDecryptVerify.obj" \
	"$(INTDIR)\ShellEncryptSend.obj" \
	"$(INTDIR)\ShellEncryptSign.obj" \
	"$(INTDIR)\ShellVerify.obj" \
	"$(INTDIR)\shexinit.obj" \
	"$(INTDIR)\working.obj"

"$(OUTDIR)\Pgpwctx.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pgpwctx0"
# PROP BASE Intermediate_Dir "pgpwctx0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pgpwctx\EvalRelease"
# PROP Intermediate_Dir "pgpwctx\EvalRelease"
# PROP Target_Dir ""
OUTDIR=.\pgpwctx\EvalRelease
INTDIR=.\pgpwctx\EvalRelease

ALL : "$(OUTDIR)\Pgpwctx.dll"

CLEAN : 
	-@erase "$(INTDIR)\Ctxmenu.obj"
	-@erase "$(INTDIR)\pgpwctx.obj"
	-@erase "$(INTDIR)\pgpwctx.res"
	-@erase "$(INTDIR)\ShellAddKey.obj"
	-@erase "$(INTDIR)\ShellDecryptVerify.obj"
	-@erase "$(INTDIR)\ShellEncryptSend.obj"
	-@erase "$(INTDIR)\ShellEncryptSign.obj"
	-@erase "$(INTDIR)\ShellVerify.obj"
	-@erase "$(INTDIR)\shexinit.obj"
	-@erase "$(INTDIR)\working.obj"
	-@erase "$(OUTDIR)\Pgpwctx.dll"
	-@erase "$(OUTDIR)\Pgpwctx.exp"
	-@erase "$(OUTDIR)\Pgpwctx.lib"
	-@erase "$(OUTDIR)\Pgpwctx.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "EVAL_TIMEOUT" /Fp"$(INTDIR)/Pgpwctx.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\pgpwctx\EvalRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpwctx.res" /d "NDEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Pgpwctx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib pgpcmdlg\release\pgpcmdlg.lib pgpphrase\release\pgpphrase.lib pgprecip\release\pgprecip.lib /nologo /subsystem:windows /dll /map /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib pgpcmdlg\evalrelease\pgpcmdlg.lib pgpphrase\release\pgpphrase.lib pgprecip\release\pgprecip.lib /nologo /subsystem:windows /dll /map /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\keydb.lib lib\pgp.lib lib\simple.lib\
 pgpcmdlg\evalrelease\pgpcmdlg.lib pgpphrase\release\pgpphrase.lib\
 pgprecip\release\pgprecip.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Pgpwctx.pdb" /map:"$(INTDIR)/Pgpwctx.map" /machine:I386\
 /def:".\pgpwctx\ctxmenu.def" /out:"$(OUTDIR)/Pgpwctx.dll"\
 /implib:"$(OUTDIR)/Pgpwctx.lib" 
DEF_FILE= \
	".\pgpwctx\ctxmenu.def"
LINK32_OBJS= \
	"$(INTDIR)\Ctxmenu.obj" \
	"$(INTDIR)\pgpwctx.obj" \
	"$(INTDIR)\pgpwctx.res" \
	"$(INTDIR)\ShellAddKey.obj" \
	"$(INTDIR)\ShellDecryptVerify.obj" \
	"$(INTDIR)\ShellEncryptSend.obj" \
	"$(INTDIR)\ShellEncryptSign.obj" \
	"$(INTDIR)\ShellVerify.obj" \
	"$(INTDIR)\shexinit.obj" \
	"$(INTDIR)\working.obj"

"$(OUTDIR)\Pgpwctx.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "pgpwctx - Win32 Release"
# Name "pgpwctx - Win32 Debug"
# Name "pgpwctx - Win32 Eval_Debug"
# Name "pgpwctx - Win32 Eval_Release"

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\pgpwctx\shexinit.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_SHEXI=\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	

"$(INTDIR)\shexinit.obj" : $(SOURCE) $(DEP_CPP_SHEXI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_SHEXI=\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	

"$(INTDIR)\shexinit.obj" : $(SOURCE) $(DEP_CPP_SHEXI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_SHEXI=\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	

"$(INTDIR)\shexinit.obj" : $(SOURCE) $(DEP_CPP_SHEXI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_SHEXI=\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	

"$(INTDIR)\shexinit.obj" : $(SOURCE) $(DEP_CPP_SHEXI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpwctx\pgpwctx.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_PGPWC=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\pgpwctx.obj" : $(SOURCE) $(DEP_CPP_PGPWC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_PGPWC=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\pgpwctx.obj" : $(SOURCE) $(DEP_CPP_PGPWC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_PGPWC=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\pgpwctx.obj" : $(SOURCE) $(DEP_CPP_PGPWC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_PGPWC=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\pgpwctx.obj" : $(SOURCE) $(DEP_CPP_PGPWC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpwctx\Ctxmenu.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_CTXME=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\pgpCodes.hpp"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\pgpwctx\ShellDecryptVerify.h"\
	".\pgpwctx\ShellEncryptSend.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\pgpwctx\ShellVerify.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Ctxmenu.obj" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_CTXME=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\pgpCodes.hpp"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\pgpwctx\ShellDecryptVerify.h"\
	".\pgpwctx\ShellEncryptSend.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\pgpwctx\ShellVerify.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Ctxmenu.obj" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_CTXME=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\pgpCodes.hpp"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\pgpwctx\ShellDecryptVerify.h"\
	".\pgpwctx\ShellEncryptSend.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\pgpwctx\ShellVerify.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Ctxmenu.obj" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_CTXME=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\pgpCodes.hpp"\
	".\pgpwctx\pgpwctx.hpp"\
	".\pgpwctx\priv.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\pgpwctx\ShellDecryptVerify.h"\
	".\pgpwctx\ShellEncryptSend.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\pgpwctx\ShellVerify.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\Ctxmenu.obj" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Pgpwctx\pgpwctx.rc

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_RSC_PGPWCT=\
	".\Pgpwctx\..\include\pgpversion.h"\
	".\Pgpwctx\PGPwctxVer.rc"\
	".\Pgpwctx\Res\blah.aexpk.ico"\
	".\Pgpwctx\Res\blah.asc.ico"\
	".\Pgpwctx\Res\blah.pgp.ico"\
	".\Pgpwctx\Res\blah.prvkr.ico"\
	".\Pgpwctx\Res\blah.pubkr.ico"\
	".\Pgpwctx\Res\blah.sig.ico"\
	

"$(INTDIR)\pgpwctx.res" : $(SOURCE) $(DEP_RSC_PGPWCT) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpwctx.res" /i "Pgpwctx" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_RSC_PGPWCT=\
	".\Pgpwctx\..\include\pgpversion.h"\
	".\Pgpwctx\PGPwctxVer.rc"\
	".\Pgpwctx\Res\blah.aexpk.ico"\
	".\Pgpwctx\Res\blah.asc.ico"\
	".\Pgpwctx\Res\blah.pgp.ico"\
	".\Pgpwctx\Res\blah.prvkr.ico"\
	".\Pgpwctx\Res\blah.pubkr.ico"\
	".\Pgpwctx\Res\blah.sig.ico"\
	

"$(INTDIR)\pgpwctx.res" : $(SOURCE) $(DEP_RSC_PGPWCT) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpwctx.res" /i "Pgpwctx" /d "_DEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_RSC_PGPWCT=\
	".\Pgpwctx\..\include\pgpversion.h"\
	".\Pgpwctx\PGPwctxVer.rc"\
	".\Pgpwctx\Res\blah.aexpk.ico"\
	".\Pgpwctx\Res\blah.asc.ico"\
	".\Pgpwctx\Res\blah.pgp.ico"\
	".\Pgpwctx\Res\blah.prvkr.ico"\
	".\Pgpwctx\Res\blah.pubkr.ico"\
	".\Pgpwctx\Res\blah.sig.ico"\
	

"$(INTDIR)\pgpwctx.res" : $(SOURCE) $(DEP_RSC_PGPWCT) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpwctx.res" /i "Pgpwctx" /d "_DEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_RSC_PGPWCT=\
	".\Pgpwctx\..\include\pgpversion.h"\
	".\Pgpwctx\PGPwctxVer.rc"\
	".\Pgpwctx\Res\blah.aexpk.ico"\
	".\Pgpwctx\Res\blah.asc.ico"\
	".\Pgpwctx\Res\blah.pgp.ico"\
	".\Pgpwctx\Res\blah.prvkr.ico"\
	".\Pgpwctx\Res\blah.pubkr.ico"\
	".\Pgpwctx\Res\blah.sig.ico"\
	

"$(INTDIR)\pgpwctx.res" : $(SOURCE) $(DEP_RSC_PGPWCT) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpwctx.res" /i "Pgpwctx" /d "NDEBUG" /d\
 "EVAL_TIMEOUT" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpwctx\ctxmenu.def

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpwctx\ShellAddKey.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_SHELL=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellAddKey.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellAddKey.obj" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_SHELL=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellAddKey.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellAddKey.obj" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_SHELL=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellAddKey.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellAddKey.obj" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_SHELL=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellAddKey.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellAddKey.obj" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpwctx\ShellEncryptSign.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_SHELLE=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSign.obj" : $(SOURCE) $(DEP_CPP_SHELLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_SHELLE=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSign.obj" : $(SOURCE) $(DEP_CPP_SHELLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_SHELLE=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSign.obj" : $(SOURCE) $(DEP_CPP_SHELLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_SHELLE=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSign.obj" : $(SOURCE) $(DEP_CPP_SHELLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpwctx\ShellDecryptVerify.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_SHELLD=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_SHELLD=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_SHELLD=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_SHELLD=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\pgpwctx\ShellAddKey.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellDecryptVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Pgpwctx\ShellEncryptSend.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_SHELLEN=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSend.obj" : $(SOURCE) $(DEP_CPP_SHELLEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_SHELLEN=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSend.obj" : $(SOURCE) $(DEP_CPP_SHELLEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_SHELLEN=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSend.obj" : $(SOURCE) $(DEP_CPP_SHELLEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_SHELLEN=\
	".\include\pgpDebug.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpLeaks.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\PGPWCTX\..\include\errs.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\Pgpwctx\..\include\pgpphrase.h"\
	".\Pgpwctx\..\include\PGPRecip.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\ShellEncryptSign.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellEncryptSend.obj" : $(SOURCE) $(DEP_CPP_SHELLEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Pgpwctx\working.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_WORKI=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpLeaks.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_WORKI=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpLeaks.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_WORKI=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpLeaks.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_WORKI=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpLeaks.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpmem.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Pgpwctx\ShellVerify.cpp

!IF  "$(CFG)" == "pgpwctx - Win32 Release"

DEP_CPP_SHELLV=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Debug"

DEP_CPP_SHELLV=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Debug"

DEP_CPP_SHELLV=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgpwctx - Win32 Eval_Release"

DEP_CPP_SHELLV=\
	".\include\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\include\pgpErr.h"\
	".\include\pgpFile.h"\
	".\include\pgpFileMod.h"\
	".\include\pgpHash.h"\
	".\include\pgpPubKey.h"\
	".\include\pgpRndom.h"\
	".\include\pgpRndPool.h"\
	".\include\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\include\pgpTimeDate.h"\
	".\include\pgpTrstPkt.h"\
	".\include\pgpTrust.h"\
	".\include\pgpUsuals.h"\
	".\pgpwctx\..\include\config.h"\
	".\pgpwctx\..\include\PGPcomdlg.h"\
	".\pgpwctx\..\include\pgpFileRef.h"\
	".\pgpwctx\..\include\pgpkeydb.h"\
	".\pgpwctx\..\include\pgpTypes.h"\
	".\pgpwctx\..\include\spgp.h"\
	".\pgpwctx\define.h"\
	".\PGPWCTX\ResultsDlg.h"\
	".\Pgpwctx\working.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\ShellVerify.obj" : $(SOURCE) $(DEP_CPP_SHELLV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
