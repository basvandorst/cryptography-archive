# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=EudoraPlugin - Win32 Eval_Debug
!MESSAGE No configuration specified.  Defaulting to EudoraPlugin - Win32\
 Eval_Debug.
!ENDIF 

!IF "$(CFG)" != "EudoraPlugin - Win32 Release" && "$(CFG)" !=\
 "EudoraPlugin - Win32 Debug" && "$(CFG)" != "EudoraPlugin - Win32 Eval_Debug"\
 && "$(CFG)" != "EudoraPlugin - Win32 Eval_Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "EudoraPlugin.mak" CFG="EudoraPlugin - Win32 Eval_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EudoraPlugin - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EudoraPlugin - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EudoraPlugin - Win32 Eval_Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EudoraPlugin - Win32 Eval_Release" (based on\
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
# PROP Target_Last_Scanned "EudoraPlugin - Win32 Eval_Release"
CPP=cl.exe
MTL=mktyplib.exe
RSC=rc.exe

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "EudoraPlugin\Release"
# PROP Intermediate_Dir "EudoraPlugin\Release"
# PROP Target_Dir ""
OUTDIR=.\EudoraPlugin\Release
INTDIR=.\EudoraPlugin\Release

ALL : "$(OUTDIR)\pgpplugin.dll"

CLEAN : 
	-@erase "$(INTDIR)\AddKey.obj"
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\CBTProc.obj"
	-@erase "$(INTDIR)\CreateMimeMessage.obj"
	-@erase "$(INTDIR)\CreateToolbar.obj"
	-@erase "$(INTDIR)\DecryptVerifyBuffer.obj"
	-@erase "$(INTDIR)\DecryptVerifyFile.obj"
	-@erase "$(INTDIR)\DisplayMessage.obj"
	-@erase "$(INTDIR)\ENCODING.OBJ"
	-@erase "$(INTDIR)\EncryptSignBuffer.obj"
	-@erase "$(INTDIR)\EncryptSignFile.obj"
	-@erase "$(INTDIR)\EudoraINI.obj"
	-@erase "$(INTDIR)\EudoraMailHeaders.obj"
	-@erase "$(INTDIR)\EudoraMainWndProc.obj"
	-@erase "$(INTDIR)\EudoraReadMailWndProc.obj"
	-@erase "$(INTDIR)\EudoraSendMailWndProc.obj"
	-@erase "$(INTDIR)\MapFile.obj"
	-@erase "$(INTDIR)\MimeSign.obj"
	-@erase "$(INTDIR)\Mimetype.obj"
	-@erase "$(INTDIR)\MimeVerify.obj"
	-@erase "$(INTDIR)\MyKeyDB.obj"
	-@erase "$(INTDIR)\MyMIMEUtils.obj"
	-@erase "$(INTDIR)\PGPPlug.obj"
	-@erase "$(INTDIR)\PGPplugin.res"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\ReadMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\Rfc822.obj"
	-@erase "$(INTDIR)\SendMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\SigResult.obj"
	-@erase "$(INTDIR)\strstri.obj"
	-@erase "$(INTDIR)\transbmp.obj"
	-@erase "$(INTDIR)\Translators.obj"
	-@erase "$(INTDIR)\TranslatorUtils.obj"
	-@erase "$(INTDIR)\WordWrap.obj"
	-@erase "$(INTDIR)\Working.obj"
	-@erase "$(OUTDIR)\pgpplugin.dll"
	-@erase "$(OUTDIR)\pgpplugin.exp"
	-@erase "$(OUTDIR)\pgpplugin.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/EudoraPlugin.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\EudoraPlugin\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPplugin.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/EudoraPlugin.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBC" /out:"EudoraPlugin\Release/pgpplugin.dll"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib\
 pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib\
 pgpcmdlg\release\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/pgpplugin.pdb" /machine:I386\
 /nodefaultlib:"LIBC" /def:".\EudoraPlugin\PGPplugin.def"\
 /out:"$(OUTDIR)/pgpplugin.dll" /implib:"$(OUTDIR)/pgpplugin.lib" 
DEF_FILE= \
	".\EudoraPlugin\PGPplugin.def"
LINK32_OBJS= \
	"$(INTDIR)\AddKey.obj" \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\CBTProc.obj" \
	"$(INTDIR)\CreateMimeMessage.obj" \
	"$(INTDIR)\CreateToolbar.obj" \
	"$(INTDIR)\DecryptVerifyBuffer.obj" \
	"$(INTDIR)\DecryptVerifyFile.obj" \
	"$(INTDIR)\DisplayMessage.obj" \
	"$(INTDIR)\ENCODING.OBJ" \
	"$(INTDIR)\EncryptSignBuffer.obj" \
	"$(INTDIR)\EncryptSignFile.obj" \
	"$(INTDIR)\EudoraINI.obj" \
	"$(INTDIR)\EudoraMailHeaders.obj" \
	"$(INTDIR)\EudoraMainWndProc.obj" \
	"$(INTDIR)\EudoraReadMailWndProc.obj" \
	"$(INTDIR)\EudoraSendMailWndProc.obj" \
	"$(INTDIR)\MapFile.obj" \
	"$(INTDIR)\MimeSign.obj" \
	"$(INTDIR)\Mimetype.obj" \
	"$(INTDIR)\MimeVerify.obj" \
	"$(INTDIR)\MyKeyDB.obj" \
	"$(INTDIR)\MyMIMEUtils.obj" \
	"$(INTDIR)\PGPPlug.obj" \
	"$(INTDIR)\PGPplugin.res" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\ReadMailToolbarWndProc.obj" \
	"$(INTDIR)\Rfc822.obj" \
	"$(INTDIR)\SendMailToolbarWndProc.obj" \
	"$(INTDIR)\SigResult.obj" \
	"$(INTDIR)\strstri.obj" \
	"$(INTDIR)\transbmp.obj" \
	"$(INTDIR)\Translators.obj" \
	"$(INTDIR)\TranslatorUtils.obj" \
	"$(INTDIR)\WordWrap.obj" \
	"$(INTDIR)\Working.obj"

"$(OUTDIR)\pgpplugin.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "EudoraPlugin\Debug"
# PROP Intermediate_Dir "EudoraPlugin\Debug"
# PROP Target_Dir ""
OUTDIR=.\EudoraPlugin\Debug
INTDIR=.\EudoraPlugin\Debug

ALL : "$(OUTDIR)\pgpplugin.dll"

CLEAN : 
	-@erase "$(INTDIR)\AddKey.obj"
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\CBTProc.obj"
	-@erase "$(INTDIR)\CreateMimeMessage.obj"
	-@erase "$(INTDIR)\CreateToolbar.obj"
	-@erase "$(INTDIR)\DecryptVerifyBuffer.obj"
	-@erase "$(INTDIR)\DecryptVerifyFile.obj"
	-@erase "$(INTDIR)\DisplayMessage.obj"
	-@erase "$(INTDIR)\ENCODING.OBJ"
	-@erase "$(INTDIR)\EncryptSignBuffer.obj"
	-@erase "$(INTDIR)\EncryptSignFile.obj"
	-@erase "$(INTDIR)\EudoraINI.obj"
	-@erase "$(INTDIR)\EudoraMailHeaders.obj"
	-@erase "$(INTDIR)\EudoraMainWndProc.obj"
	-@erase "$(INTDIR)\EudoraReadMailWndProc.obj"
	-@erase "$(INTDIR)\EudoraSendMailWndProc.obj"
	-@erase "$(INTDIR)\MapFile.obj"
	-@erase "$(INTDIR)\MimeSign.obj"
	-@erase "$(INTDIR)\Mimetype.obj"
	-@erase "$(INTDIR)\MimeVerify.obj"
	-@erase "$(INTDIR)\MyKeyDB.obj"
	-@erase "$(INTDIR)\MyMIMEUtils.obj"
	-@erase "$(INTDIR)\PGPPlug.obj"
	-@erase "$(INTDIR)\PGPplugin.res"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\ReadMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\Rfc822.obj"
	-@erase "$(INTDIR)\SendMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\SigResult.obj"
	-@erase "$(INTDIR)\strstri.obj"
	-@erase "$(INTDIR)\transbmp.obj"
	-@erase "$(INTDIR)\Translators.obj"
	-@erase "$(INTDIR)\TranslatorUtils.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\WordWrap.obj"
	-@erase "$(INTDIR)\Working.obj"
	-@erase "$(OUTDIR)\pgpplugin.dll"
	-@erase "$(OUTDIR)\pgpplugin.exp"
	-@erase "$(OUTDIR)\pgpplugin.ilk"
	-@erase "$(OUTDIR)\pgpplugin.lib"
	-@erase "$(OUTDIR)\pgpplugin.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/EudoraPlugin.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\EudoraPlugin\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPplugin.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/EudoraPlugin.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"EudoraPlugin\Debug/pgpplugin.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib\
 pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib\
 pgpcmdlg\debug\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/pgpplugin.pdb" /debug /machine:I386\
 /def:".\EudoraPlugin\PGPplugin.def" /out:"$(OUTDIR)/pgpplugin.dll"\
 /implib:"$(OUTDIR)/pgpplugin.lib" 
DEF_FILE= \
	".\EudoraPlugin\PGPplugin.def"
LINK32_OBJS= \
	"$(INTDIR)\AddKey.obj" \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\CBTProc.obj" \
	"$(INTDIR)\CreateMimeMessage.obj" \
	"$(INTDIR)\CreateToolbar.obj" \
	"$(INTDIR)\DecryptVerifyBuffer.obj" \
	"$(INTDIR)\DecryptVerifyFile.obj" \
	"$(INTDIR)\DisplayMessage.obj" \
	"$(INTDIR)\ENCODING.OBJ" \
	"$(INTDIR)\EncryptSignBuffer.obj" \
	"$(INTDIR)\EncryptSignFile.obj" \
	"$(INTDIR)\EudoraINI.obj" \
	"$(INTDIR)\EudoraMailHeaders.obj" \
	"$(INTDIR)\EudoraMainWndProc.obj" \
	"$(INTDIR)\EudoraReadMailWndProc.obj" \
	"$(INTDIR)\EudoraSendMailWndProc.obj" \
	"$(INTDIR)\MapFile.obj" \
	"$(INTDIR)\MimeSign.obj" \
	"$(INTDIR)\Mimetype.obj" \
	"$(INTDIR)\MimeVerify.obj" \
	"$(INTDIR)\MyKeyDB.obj" \
	"$(INTDIR)\MyMIMEUtils.obj" \
	"$(INTDIR)\PGPPlug.obj" \
	"$(INTDIR)\PGPplugin.res" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\ReadMailToolbarWndProc.obj" \
	"$(INTDIR)\Rfc822.obj" \
	"$(INTDIR)\SendMailToolbarWndProc.obj" \
	"$(INTDIR)\SigResult.obj" \
	"$(INTDIR)\strstri.obj" \
	"$(INTDIR)\transbmp.obj" \
	"$(INTDIR)\Translators.obj" \
	"$(INTDIR)\TranslatorUtils.obj" \
	"$(INTDIR)\WordWrap.obj" \
	"$(INTDIR)\Working.obj"

"$(OUTDIR)\pgpplugin.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "EudoraPl"
# PROP BASE Intermediate_Dir "EudoraPl"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "EudoraPlugin\EvalDebug"
# PROP Intermediate_Dir "EudoraPlugin\EvalDebug"
# PROP Target_Dir ""
OUTDIR=.\EudoraPlugin\EvalDebug
INTDIR=.\EudoraPlugin\EvalDebug

ALL : "$(OUTDIR)\pgpplugin.dll"

CLEAN : 
	-@erase "$(INTDIR)\AddKey.obj"
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\CBTProc.obj"
	-@erase "$(INTDIR)\CreateMimeMessage.obj"
	-@erase "$(INTDIR)\CreateToolbar.obj"
	-@erase "$(INTDIR)\DecryptVerifyBuffer.obj"
	-@erase "$(INTDIR)\DecryptVerifyFile.obj"
	-@erase "$(INTDIR)\DisplayMessage.obj"
	-@erase "$(INTDIR)\ENCODING.OBJ"
	-@erase "$(INTDIR)\EncryptSignBuffer.obj"
	-@erase "$(INTDIR)\EncryptSignFile.obj"
	-@erase "$(INTDIR)\EudoraINI.obj"
	-@erase "$(INTDIR)\EudoraMailHeaders.obj"
	-@erase "$(INTDIR)\EudoraMainWndProc.obj"
	-@erase "$(INTDIR)\EudoraReadMailWndProc.obj"
	-@erase "$(INTDIR)\EudoraSendMailWndProc.obj"
	-@erase "$(INTDIR)\MapFile.obj"
	-@erase "$(INTDIR)\MimeSign.obj"
	-@erase "$(INTDIR)\Mimetype.obj"
	-@erase "$(INTDIR)\MimeVerify.obj"
	-@erase "$(INTDIR)\MyKeyDB.obj"
	-@erase "$(INTDIR)\MyMIMEUtils.obj"
	-@erase "$(INTDIR)\PGPPlug.obj"
	-@erase "$(INTDIR)\PGPplugin.res"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\ReadMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\Rfc822.obj"
	-@erase "$(INTDIR)\SendMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\SigResult.obj"
	-@erase "$(INTDIR)\strstri.obj"
	-@erase "$(INTDIR)\transbmp.obj"
	-@erase "$(INTDIR)\Translators.obj"
	-@erase "$(INTDIR)\TranslatorUtils.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\WordWrap.obj"
	-@erase "$(INTDIR)\Working.obj"
	-@erase "$(OUTDIR)\pgpplugin.dll"
	-@erase "$(OUTDIR)\pgpplugin.exp"
	-@erase "$(OUTDIR)\pgpplugin.ilk"
	-@erase "$(OUTDIR)\pgpplugin.lib"
	-@erase "$(OUTDIR)\pgpplugin.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "EVAL_TIMEOUT" /Fp"$(INTDIR)/EudoraPlugin.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\EudoraPlugin\EvalDebug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPplugin.res" /d "_DEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/EudoraPlugin.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\debug\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"EudoraPlugin\Debug/pgpplugin.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib pgpcmdlg\evaldebug\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"EudoraPlugin\EvalDebug/pgpplugin.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib\
 pgprecip\debug\pgprecip.lib pgpphrase\debug\pgpphrase.lib\
 pgpcmdlg\evaldebug\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/pgpplugin.pdb" /debug /machine:I386\
 /def:".\EudoraPlugin\PGPplugin.def" /out:"$(OUTDIR)/pgpplugin.dll"\
 /implib:"$(OUTDIR)/pgpplugin.lib" 
DEF_FILE= \
	".\EudoraPlugin\PGPplugin.def"
LINK32_OBJS= \
	"$(INTDIR)\AddKey.obj" \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\CBTProc.obj" \
	"$(INTDIR)\CreateMimeMessage.obj" \
	"$(INTDIR)\CreateToolbar.obj" \
	"$(INTDIR)\DecryptVerifyBuffer.obj" \
	"$(INTDIR)\DecryptVerifyFile.obj" \
	"$(INTDIR)\DisplayMessage.obj" \
	"$(INTDIR)\ENCODING.OBJ" \
	"$(INTDIR)\EncryptSignBuffer.obj" \
	"$(INTDIR)\EncryptSignFile.obj" \
	"$(INTDIR)\EudoraINI.obj" \
	"$(INTDIR)\EudoraMailHeaders.obj" \
	"$(INTDIR)\EudoraMainWndProc.obj" \
	"$(INTDIR)\EudoraReadMailWndProc.obj" \
	"$(INTDIR)\EudoraSendMailWndProc.obj" \
	"$(INTDIR)\MapFile.obj" \
	"$(INTDIR)\MimeSign.obj" \
	"$(INTDIR)\Mimetype.obj" \
	"$(INTDIR)\MimeVerify.obj" \
	"$(INTDIR)\MyKeyDB.obj" \
	"$(INTDIR)\MyMIMEUtils.obj" \
	"$(INTDIR)\PGPPlug.obj" \
	"$(INTDIR)\PGPplugin.res" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\ReadMailToolbarWndProc.obj" \
	"$(INTDIR)\Rfc822.obj" \
	"$(INTDIR)\SendMailToolbarWndProc.obj" \
	"$(INTDIR)\SigResult.obj" \
	"$(INTDIR)\strstri.obj" \
	"$(INTDIR)\transbmp.obj" \
	"$(INTDIR)\Translators.obj" \
	"$(INTDIR)\TranslatorUtils.obj" \
	"$(INTDIR)\WordWrap.obj" \
	"$(INTDIR)\Working.obj"

"$(OUTDIR)\pgpplugin.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "EudoraP0"
# PROP BASE Intermediate_Dir "EudoraP0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "EudoraPlugin\EvalRelease"
# PROP Intermediate_Dir "EudoraPlugin\EvalRelease"
# PROP Target_Dir ""
OUTDIR=.\EudoraPlugin\EvalRelease
INTDIR=.\EudoraPlugin\EvalRelease

ALL : "$(OUTDIR)\pgpplugin.dll"

CLEAN : 
	-@erase "$(INTDIR)\AddKey.obj"
	-@erase "$(INTDIR)\BlockUtils.obj"
	-@erase "$(INTDIR)\CBTProc.obj"
	-@erase "$(INTDIR)\CreateMimeMessage.obj"
	-@erase "$(INTDIR)\CreateToolbar.obj"
	-@erase "$(INTDIR)\DecryptVerifyBuffer.obj"
	-@erase "$(INTDIR)\DecryptVerifyFile.obj"
	-@erase "$(INTDIR)\DisplayMessage.obj"
	-@erase "$(INTDIR)\ENCODING.OBJ"
	-@erase "$(INTDIR)\EncryptSignBuffer.obj"
	-@erase "$(INTDIR)\EncryptSignFile.obj"
	-@erase "$(INTDIR)\EudoraINI.obj"
	-@erase "$(INTDIR)\EudoraMailHeaders.obj"
	-@erase "$(INTDIR)\EudoraMainWndProc.obj"
	-@erase "$(INTDIR)\EudoraReadMailWndProc.obj"
	-@erase "$(INTDIR)\EudoraSendMailWndProc.obj"
	-@erase "$(INTDIR)\MapFile.obj"
	-@erase "$(INTDIR)\MimeSign.obj"
	-@erase "$(INTDIR)\Mimetype.obj"
	-@erase "$(INTDIR)\MimeVerify.obj"
	-@erase "$(INTDIR)\MyKeyDB.obj"
	-@erase "$(INTDIR)\MyMIMEUtils.obj"
	-@erase "$(INTDIR)\PGPPlug.obj"
	-@erase "$(INTDIR)\PGPplugin.res"
	-@erase "$(INTDIR)\re_io.obj"
	-@erase "$(INTDIR)\ReadMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\Rfc822.obj"
	-@erase "$(INTDIR)\SendMailToolbarWndProc.obj"
	-@erase "$(INTDIR)\SigResult.obj"
	-@erase "$(INTDIR)\strstri.obj"
	-@erase "$(INTDIR)\transbmp.obj"
	-@erase "$(INTDIR)\Translators.obj"
	-@erase "$(INTDIR)\TranslatorUtils.obj"
	-@erase "$(INTDIR)\WordWrap.obj"
	-@erase "$(INTDIR)\Working.obj"
	-@erase "$(OUTDIR)\pgpplugin.dll"
	-@erase "$(OUTDIR)\pgpplugin.exp"
	-@erase "$(OUTDIR)\pgpplugin.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "EVAL_TIMEOUT" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "EVAL_TIMEOUT" /Fp"$(INTDIR)/EudoraPlugin.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\EudoraPlugin\EvalRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "EVAL_TIMEOUT"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPplugin.res" /d "NDEBUG" /d "EVAL_TIMEOUT" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/EudoraPlugin.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\release\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBC" /out:"EudoraPlugin\Release/pgpplugin.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib pgpcmdlg\evalrelease\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBC" /out:"EudoraPlugin\EvalRelease/pgpplugin.dll"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib lib\simple.lib\
 pgprecip\release\pgprecip.lib pgpphrase\release\pgpphrase.lib\
 pgpcmdlg\evalrelease\pgpcmdlg.lib comctl32.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/pgpplugin.pdb" /machine:I386\
 /nodefaultlib:"LIBC" /def:".\EudoraPlugin\PGPplugin.def"\
 /out:"$(OUTDIR)/pgpplugin.dll" /implib:"$(OUTDIR)/pgpplugin.lib" 
DEF_FILE= \
	".\EudoraPlugin\PGPplugin.def"
LINK32_OBJS= \
	"$(INTDIR)\AddKey.obj" \
	"$(INTDIR)\BlockUtils.obj" \
	"$(INTDIR)\CBTProc.obj" \
	"$(INTDIR)\CreateMimeMessage.obj" \
	"$(INTDIR)\CreateToolbar.obj" \
	"$(INTDIR)\DecryptVerifyBuffer.obj" \
	"$(INTDIR)\DecryptVerifyFile.obj" \
	"$(INTDIR)\DisplayMessage.obj" \
	"$(INTDIR)\ENCODING.OBJ" \
	"$(INTDIR)\EncryptSignBuffer.obj" \
	"$(INTDIR)\EncryptSignFile.obj" \
	"$(INTDIR)\EudoraINI.obj" \
	"$(INTDIR)\EudoraMailHeaders.obj" \
	"$(INTDIR)\EudoraMainWndProc.obj" \
	"$(INTDIR)\EudoraReadMailWndProc.obj" \
	"$(INTDIR)\EudoraSendMailWndProc.obj" \
	"$(INTDIR)\MapFile.obj" \
	"$(INTDIR)\MimeSign.obj" \
	"$(INTDIR)\Mimetype.obj" \
	"$(INTDIR)\MimeVerify.obj" \
	"$(INTDIR)\MyKeyDB.obj" \
	"$(INTDIR)\MyMIMEUtils.obj" \
	"$(INTDIR)\PGPPlug.obj" \
	"$(INTDIR)\PGPplugin.res" \
	"$(INTDIR)\re_io.obj" \
	"$(INTDIR)\ReadMailToolbarWndProc.obj" \
	"$(INTDIR)\Rfc822.obj" \
	"$(INTDIR)\SendMailToolbarWndProc.obj" \
	"$(INTDIR)\SigResult.obj" \
	"$(INTDIR)\strstri.obj" \
	"$(INTDIR)\transbmp.obj" \
	"$(INTDIR)\Translators.obj" \
	"$(INTDIR)\TranslatorUtils.obj" \
	"$(INTDIR)\WordWrap.obj" \
	"$(INTDIR)\Working.obj"

"$(OUTDIR)\pgpplugin.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "EudoraPlugin - Win32 Release"
# Name "EudoraPlugin - Win32 Debug"
# Name "EudoraPlugin - Win32 Eval_Debug"
# Name "EudoraPlugin - Win32 Eval_Release"

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\TranslatorUtils.cpp
DEP_CPP_TRANS=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\BlockUtils.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\TranslatorUtils.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\TranslatorUtils.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\TranslatorUtils.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\TranslatorUtils.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\DisplayMessage.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\DisplayMessage.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\DisplayMessage.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\DisplayMessage.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\DisplayMessage.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\EncryptSignFile.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_ENCRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignFile.obj" : $(SOURCE) $(DEP_CPP_ENCRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_ENCRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignFile.obj" : $(SOURCE) $(DEP_CPP_ENCRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_ENCRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignFile.obj" : $(SOURCE) $(DEP_CPP_ENCRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_ENCRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignFile.obj" : $(SOURCE) $(DEP_CPP_ENCRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\MapFile.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\MapFile.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\MapFile.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\MapFile.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\MapFile.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\SigResult.c
DEP_CPP_SIGRE=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\SigResult.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\SigResult.obj" : $(SOURCE) $(DEP_CPP_SIGRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\SigResult.obj" : $(SOURCE) $(DEP_CPP_SIGRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\SigResult.obj" : $(SOURCE) $(DEP_CPP_SIGRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\SigResult.obj" : $(SOURCE) $(DEP_CPP_SIGRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\AddKey.cpp
DEP_CPP_ADDKE=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\AddKey.obj" : $(SOURCE) $(DEP_CPP_ADDKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\AddKey.obj" : $(SOURCE) $(DEP_CPP_ADDKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\AddKey.obj" : $(SOURCE) $(DEP_CPP_ADDKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\AddKey.obj" : $(SOURCE) $(DEP_CPP_ADDKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\BlockUtils.cpp
DEP_CPP_BLOCK=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\BlockUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpLeaks.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\BlockUtils.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\MimeSign.cpp

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_MIMES=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\MimeSign.obj" : $(SOURCE) $(DEP_CPP_MIMES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_MIMES=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\MimeSign.obj" : $(SOURCE) $(DEP_CPP_MIMES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_MIMES=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\MimeSign.obj" : $(SOURCE) $(DEP_CPP_MIMES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_MIMES=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\MimeSign.obj" : $(SOURCE) $(DEP_CPP_MIMES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\MimeVerify.cpp
DEP_CPP_MIMEV=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\BlockUtils.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\MimeVerify.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\SigResult.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\MimeVerify.obj" : $(SOURCE) $(DEP_CPP_MIMEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\MimeVerify.obj" : $(SOURCE) $(DEP_CPP_MIMEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\MimeVerify.obj" : $(SOURCE) $(DEP_CPP_MIMEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\MimeVerify.obj" : $(SOURCE) $(DEP_CPP_MIMEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\MyKeyDB.cpp
DEP_CPP_MYKEY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\MyKeyDB.obj" : $(SOURCE) $(DEP_CPP_MYKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\MyKeyDB.obj" : $(SOURCE) $(DEP_CPP_MYKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\MyKeyDB.obj" : $(SOURCE) $(DEP_CPP_MYKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\MyKeyDB.obj" : $(SOURCE) $(DEP_CPP_MYKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\MyMIMEUtils.cpp
DEP_CPP_MYMIM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\ENCODING.H"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpLeaks.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\MyMIMEUtils.obj" : $(SOURCE) $(DEP_CPP_MYMIM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\MyMIMEUtils.obj" : $(SOURCE) $(DEP_CPP_MYMIM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\MyMIMEUtils.obj" : $(SOURCE) $(DEP_CPP_MYMIM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\MyMIMEUtils.obj" : $(SOURCE) $(DEP_CPP_MYMIM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\PGPPlug.cpp

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_PGPPL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\CBTProc.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPPlug.obj" : $(SOURCE) $(DEP_CPP_PGPPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_PGPPL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\CBTProc.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPPlug.obj" : $(SOURCE) $(DEP_CPP_PGPPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_PGPPL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\CBTProc.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPPlug.obj" : $(SOURCE) $(DEP_CPP_PGPPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_PGPPL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\CBTProc.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPPlug.obj" : $(SOURCE) $(DEP_CPP_PGPPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\Translators.cpp

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_TRANSL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MimeVerify.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\Translators.obj" : $(SOURCE) $(DEP_CPP_TRANSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_TRANSL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MimeVerify.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\Translators.obj" : $(SOURCE) $(DEP_CPP_TRANSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_TRANSL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MimeVerify.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\Translators.obj" : $(SOURCE) $(DEP_CPP_TRANSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_TRANSL=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\MimeSign.h"\
	".\EudoraPlugin\MimeVerify.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\Translators.obj" : $(SOURCE) $(DEP_CPP_TRANSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\DecryptVerifyFile.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_DECRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyFile.obj" : $(SOURCE) $(DEP_CPP_DECRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_DECRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyFile.obj" : $(SOURCE) $(DEP_CPP_DECRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_DECRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyFile.obj" : $(SOURCE) $(DEP_CPP_DECRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_DECRY=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyFile.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyFile.obj" : $(SOURCE) $(DEP_CPP_DECRY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\Mimetype.cpp
DEP_CPP_MIMET=\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EUDORAPLUGIN\EMSSDK\rfc822.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\Mimetype.obj" : $(SOURCE) $(DEP_CPP_MIMET) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\Mimetype.obj" : $(SOURCE) $(DEP_CPP_MIMET) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\Mimetype.obj" : $(SOURCE) $(DEP_CPP_MIMET) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\Mimetype.obj" : $(SOURCE) $(DEP_CPP_MIMET) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\PGPplugin.rc
DEP_RSC_PGPPLU=\
	".\EudoraPlugin\..\include\pgpversion.h"\
	".\EudoraPlugin\PGPpluginVer.rc"\
	".\EudoraPlugin\Res\lock.ico"\
	".\EudoraPlugin\Res\lock1.ico"\
	".\EudoraPlugin\Res\pgp50.ico"\
	".\EudoraPlugin\Res\readtool.bmp"\
	".\EudoraPlugin\Res\sendtool.bmp"\
	".\EudoraPlugin\Res\sign.ico"\
	".\EudoraPlugin\Res\verify.ico"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\PGPplugin.res" : $(SOURCE) $(DEP_RSC_PGPPLU) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPplugin.res" /i "EudoraPlugin" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\PGPplugin.res" : $(SOURCE) $(DEP_RSC_PGPPLU) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPplugin.res" /i "EudoraPlugin" /d "_DEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\PGPplugin.res" : $(SOURCE) $(DEP_RSC_PGPPLU) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPplugin.res" /i "EudoraPlugin" /d "_DEBUG"\
 /d "EVAL_TIMEOUT" $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\PGPplugin.res" : $(SOURCE) $(DEP_RSC_PGPPLU) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPplugin.res" /i "EudoraPlugin" /d "NDEBUG"\
 /d "EVAL_TIMEOUT" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\Rfc822.cpp
DEP_CPP_RFC82=\
	".\EUDORAPLUGIN\EMSSDK\rfc822.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\Rfc822.obj" : $(SOURCE) $(DEP_CPP_RFC82) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\Rfc822.obj" : $(SOURCE) $(DEP_CPP_RFC82) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\Rfc822.obj" : $(SOURCE) $(DEP_CPP_RFC82) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\Rfc822.obj" : $(SOURCE) $(DEP_CPP_RFC82) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\PGPplugin.def

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraINI.cpp
DEP_CPP_EUDOR=\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\PGPPlug.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\EudoraINI.obj" : $(SOURCE) $(DEP_CPP_EUDOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\EudoraINI.obj" : $(SOURCE) $(DEP_CPP_EUDOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\EudoraINI.obj" : $(SOURCE) $(DEP_CPP_EUDOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\EudoraINI.obj" : $(SOURCE) $(DEP_CPP_EUDOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\transbmp.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\transbmp.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\transbmp.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\transbmp.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\transbmp.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\CreateToolbar.cpp
DEP_CPP_CREAT=\
	".\EudoraPlugin\CreateToolbar.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\SendMailToolbarWndProc.h"\
	".\EudoraPlugin\transbmp.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\CreateToolbar.obj" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\CreateToolbar.obj" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\CreateToolbar.obj" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\CreateToolbar.obj" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\DecryptVerifyBuffer.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_DECRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyBuffer.obj" : $(SOURCE) $(DEP_CPP_DECRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_DECRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyBuffer.obj" : $(SOURCE) $(DEP_CPP_DECRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_DECRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyBuffer.obj" : $(SOURCE) $(DEP_CPP_DECRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_DECRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\SigResult.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DecryptVerifyBuffer.obj" : $(SOURCE) $(DEP_CPP_DECRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraReadMailWndProc.cpp
DEP_CPP_EUDORA=\
	".\EudoraPlugin\CreateToolbar.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\EudoraReadMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\EudoraReadMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\EudoraReadMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\EudoraReadMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraSendMailWndProc.cpp
DEP_CPP_EUDORAS=\
	".\EudoraPlugin\CreateToolbar.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\EudoraSendMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAS)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\EudoraSendMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAS)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\EudoraSendMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAS)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\EudoraSendMailWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAS)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\re_io.c
DEP_CPP_RE_IO=\
	".\EudoraPlugin\RE_io.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\re_io.obj" : $(SOURCE) $(DEP_CPP_RE_IO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\ReadMailToolbarWndProc.cpp

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_READM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ReadMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_READM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_READM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ReadMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_READM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_READM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ReadMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_READM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_READM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\DecryptVerifyBuffer.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ReadMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_READM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\SendMailToolbarWndProc.cpp

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_SENDM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\EudoraMailHeaders.h"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\SendMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_SENDM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_SENDM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\EudoraMailHeaders.h"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\SendMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_SENDM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_SENDM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\EudoraMailHeaders.h"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\SendMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_SENDM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_SENDM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\AddKey.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\EncryptSignFile.h"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\EudoraMailHeaders.h"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\PGPPlugTypes.h"\
	".\EudoraPlugin\RE_io.h"\
	".\EudoraPlugin\ReadMailToolbarWndProc.h"\
	".\EudoraPlugin\TranslatorIDs.h"\
	".\EudoraPlugin\Translators.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\SendMailToolbarWndProc.obj" : $(SOURCE) $(DEP_CPP_SENDM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\strstri.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\strstri.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\strstri.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\strstri.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\strstri.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\CBTProc.cpp
DEP_CPP_CBTPR=\
	".\EudoraPlugin\EudoraMainWndProc.h"\
	".\EudoraPlugin\EudoraReadMailWndProc.h"\
	".\EudoraPlugin\EudoraSendMailWndProc.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\EudoraPlugin\strstri.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\CBTProc.obj" : $(SOURCE) $(DEP_CPP_CBTPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\CBTProc.obj" : $(SOURCE) $(DEP_CPP_CBTPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\CBTProc.obj" : $(SOURCE) $(DEP_CPP_CBTPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\CBTProc.obj" : $(SOURCE) $(DEP_CPP_CBTPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\PGPDefinedMessages.h

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\EncryptSignBuffer.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_ENCRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignBuffer.obj" : $(SOURCE) $(DEP_CPP_ENCRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_ENCRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignBuffer.obj" : $(SOURCE) $(DEP_CPP_ENCRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_ENCRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignBuffer.obj" : $(SOURCE) $(DEP_CPP_ENCRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_ENCRYP=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\PGPRecip.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\DisplayMessage.h"\
	".\EudoraPlugin\EncryptSignBuffer.h"\
	".\EudoraPlugin\MyKeyDB.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EncryptSignBuffer.obj" : $(SOURCE) $(DEP_CPP_ENCRYP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\emssdk\ENCODING.CPP
DEP_CPP_ENCOD=\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\ENCODING.H"\
	".\EUDORAPLUGIN\EMSSDK\rfc822.h"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\ENCODING.OBJ" : $(SOURCE) $(DEP_CPP_ENCOD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\ENCODING.OBJ" : $(SOURCE) $(DEP_CPP_ENCOD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\ENCODING.OBJ" : $(SOURCE) $(DEP_CPP_ENCOD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\ENCODING.OBJ" : $(SOURCE) $(DEP_CPP_ENCOD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraMainWndProc.cpp

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_EUDORAM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMainWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_EUDORAM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMainWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_EUDORAM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMainWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_EUDORAM=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\PGPDefinedMessages.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMainWndProc.obj" : $(SOURCE) $(DEP_CPP_EUDORAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\CreateMimeMessage.cpp
DEP_CPP_CREATE=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\emssdk\ENCODING.H"\
	".\EudoraPlugin\emssdk\MIMETYPE.H"\
	".\EudoraPlugin\EudoraINI.h"\
	".\EudoraPlugin\MapFile.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\TranslatorUtils.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpLeaks.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\CreateMimeMessage.obj" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\CreateMimeMessage.obj" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\CreateMimeMessage.obj" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\CreateMimeMessage.obj" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\EudoraMailHeaders.cpp

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_EUDORAMA=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\strstri.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMailHeaders.obj" : $(SOURCE) $(DEP_CPP_EUDORAMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_EUDORAMA=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\strstri.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMailHeaders.obj" : $(SOURCE) $(DEP_CPP_EUDORAMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_EUDORAMA=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\strstri.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMailHeaders.obj" : $(SOURCE) $(DEP_CPP_EUDORAMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_EUDORAMA=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\PGPphrase.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\CreateMimeMessage.h"\
	".\EudoraPlugin\emssdk\ems-win.h"\
	".\EudoraPlugin\MyMIMEUtils.h"\
	".\EudoraPlugin\strstri.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\EudoraMailHeaders.obj" : $(SOURCE) $(DEP_CPP_EUDORAMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\WordWrap.c

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"

DEP_CPP_WORDW=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\WordWrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"

DEP_CPP_WORDW=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\WordWrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"

DEP_CPP_WORDW=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\WordWrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"

DEP_CPP_WORDW=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpComDlg.h"\
	".\EudoraPlugin\..\include\pgpkeydb.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\WordWrap.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\WordWrap.obj" : $(SOURCE) $(DEP_CPP_WORDW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EudoraPlugin\Working.c
DEP_CPP_WORKI=\
	".\EudoraPlugin\..\include\config.h"\
	".\EudoraPlugin\..\include\pgpmem.h"\
	".\EudoraPlugin\..\include\pgpTypes.h"\
	".\EudoraPlugin\..\include\spgp.h"\
	".\EudoraPlugin\Working.h"\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpUsuals.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "EudoraPlugin - Win32 Release"


"$(INTDIR)\Working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Debug"


"$(INTDIR)\Working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Debug"


"$(INTDIR)\Working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "EudoraPlugin - Win32 Eval_Release"


"$(INTDIR)\Working.obj" : $(SOURCE) $(DEP_CPP_WORKI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
