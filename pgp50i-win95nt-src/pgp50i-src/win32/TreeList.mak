# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=TreeList - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to TreeList - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "TreeList - Win32 Release" && "$(CFG)" !=\
 "TreeList - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "TreeList.mak" CFG="TreeList - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TreeList - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TreeList - Win32 Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "TreeList - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "TreeList - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TreeList\Release"
# PROP Intermediate_Dir "TreeList\Release"
# PROP Target_Dir ""
OUTDIR=.\TreeList\Release
INTDIR=.\TreeList\Release

ALL : "$(OUTDIR)\TreeList.lib"

CLEAN : 
	-@erase "$(INTDIR)\TLMisc.obj"
	-@erase "$(INTDIR)\TLMouse.obj"
	-@erase "$(INTDIR)\TLMsgProc.obj"
	-@erase "$(INTDIR)\TLPaint.obj"
	-@erase "$(INTDIR)\TLTree.obj"
	-@erase "$(INTDIR)\TreeList.obj"
	-@erase "$(OUTDIR)\TreeList.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/TreeList.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\TreeList\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/TreeList.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/TreeList.lib" 
LIB32_OBJS= \
	"$(INTDIR)\TLMisc.obj" \
	"$(INTDIR)\TLMouse.obj" \
	"$(INTDIR)\TLMsgProc.obj" \
	"$(INTDIR)\TLPaint.obj" \
	"$(INTDIR)\TLTree.obj" \
	"$(INTDIR)\TreeList.obj"

"$(OUTDIR)\TreeList.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TreeList - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TreeList\Debug"
# PROP Intermediate_Dir "TreeList\Debug"
# PROP Target_Dir ""
OUTDIR=.\TreeList\Debug
INTDIR=.\TreeList\Debug

ALL : "$(OUTDIR)\TreeList.lib"

CLEAN : 
	-@erase "$(INTDIR)\TLMisc.obj"
	-@erase "$(INTDIR)\TLMouse.obj"
	-@erase "$(INTDIR)\TLMsgProc.obj"
	-@erase "$(INTDIR)\TLPaint.obj"
	-@erase "$(INTDIR)\TLTree.obj"
	-@erase "$(INTDIR)\TreeList.obj"
	-@erase "$(OUTDIR)\TreeList.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/TreeList.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\TreeList\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/TreeList.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/TreeList.lib" 
LIB32_OBJS= \
	"$(INTDIR)\TLMisc.obj" \
	"$(INTDIR)\TLMouse.obj" \
	"$(INTDIR)\TLMsgProc.obj" \
	"$(INTDIR)\TLPaint.obj" \
	"$(INTDIR)\TLTree.obj" \
	"$(INTDIR)\TreeList.obj"

"$(OUTDIR)\TreeList.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

# Name "TreeList - Win32 Release"
# Name "TreeList - Win32 Debug"

!IF  "$(CFG)" == "TreeList - Win32 Release"

!ELSEIF  "$(CFG)" == "TreeList - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\TreeList\TreeList.c

!IF  "$(CFG)" == "TreeList - Win32 Release"

DEP_CPP_TREEL=\
	".\TreeList\..\include\TreeList.h"\
	".\TreeList\TLintern.h"\
	".\TreeList\TreeListVer.h"\
	

"$(INTDIR)\TreeList.obj" : $(SOURCE) $(DEP_CPP_TREEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "TreeList - Win32 Debug"

DEP_CPP_TREEL=\
	".\TreeList\..\include\TreeList.h"\
	".\TreeList\TLintern.h"\
	".\TreeList\TreeListVer.h"\
	

"$(INTDIR)\TreeList.obj" : $(SOURCE) $(DEP_CPP_TREEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TreeList\TLMouse.c
DEP_CPP_TLMOU=\
	".\TreeList\..\include\TreeList.h"\
	".\TreeList\TLintern.h"\
	

"$(INTDIR)\TLMouse.obj" : $(SOURCE) $(DEP_CPP_TLMOU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TreeList\TLMsgProc.c
DEP_CPP_TLMSG=\
	".\TreeList\..\include\TreeList.h"\
	".\TreeList\TLintern.h"\
	

"$(INTDIR)\TLMsgProc.obj" : $(SOURCE) $(DEP_CPP_TLMSG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TreeList\TLPaint.c
DEP_CPP_TLPAI=\
	".\TreeList\..\include\TreeList.h"\
	".\TreeList\TLintern.h"\
	

"$(INTDIR)\TLPaint.obj" : $(SOURCE) $(DEP_CPP_TLPAI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TreeList\TLTree.c
DEP_CPP_TLTRE=\
	".\TreeList\..\include\TreeList.h"\
	".\TreeList\TLintern.h"\
	

"$(INTDIR)\TLTree.obj" : $(SOURCE) $(DEP_CPP_TLTRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TreeList\TLMisc.c
DEP_CPP_TLMIS=\
	".\TreeList\..\include\TreeList.h"\
	".\TreeList\TLintern.h"\
	

"$(INTDIR)\TLMisc.obj" : $(SOURCE) $(DEP_CPP_TLMIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TreeList\TreeListVer.h

!IF  "$(CFG)" == "TreeList - Win32 Release"

!ELSEIF  "$(CFG)" == "TreeList - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
