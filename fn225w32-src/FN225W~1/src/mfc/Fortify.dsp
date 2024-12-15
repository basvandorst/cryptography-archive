# Microsoft Developer Studio Project File - Name="Fortify" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Fortify - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Fortify.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fortify.mak" CFG="Fortify - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fortify - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Fortify - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Fortify - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "L_ENDIAN" /D "_AFXDLL" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /incremental:yes /map /nodefaultlib

!ELSEIF  "$(CFG)" == "Fortify - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "L_ENDIAN" /D "_AFXDLL" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Fortify - Win32 Release"
# Name "Fortify - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap7.bmp
# End Source File
# Begin Source File

SOURCE=.\res\dial32x32.bmp
# End Source File
# Begin Source File

SOURCE=.\res\f32x32.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Fortify.ico
# End Source File
# Begin Source File

SOURCE=.\res\Fortify.rc2
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\splash2.bmp
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\callback.h
# End Source File
# Begin Source File

SOURCE=..\common\Cp.c
# End Source File
# Begin Source File

SOURCE=..\common\Cp.h
# End Source File
# Begin Source File

SOURCE=..\common\Gram.c
# End Source File
# Begin Source File

SOURCE=..\common\Gram.y

!IF  "$(CFG)" == "Fortify - Win32 Release"

# Begin Custom Build
ProjDir=.
InputPath=..\common\Gram.y
InputName=Gram

"$(ProjDir)\..\common\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd ..\common 
	byacc -d $(InputName).y 
	byacc -o $(InputName).c $(InputName).y 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Fortify - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\Includes.h
# End Source File
# Begin Source File

SOURCE=..\common\Index.c
# End Source File
# Begin Source File

SOURCE=..\common\Index.h
# End Source File
# Begin Source File

SOURCE=..\common\Lex.c
# End Source File
# Begin Source File

SOURCE=..\common\Lex.l

!IF  "$(CFG)" == "Fortify - Win32 Release"

# Begin Custom Build
ProjDir=.
InputPath=..\common\Lex.l
InputName=Lex

"$(ProjDir)\..\common\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd ..\common 
	flex -o$(InputName).c $(InputName).l 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Fortify - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\Log.c
# End Source File
# Begin Source File

SOURCE=..\common\Log.h
# End Source File
# Begin Source File

SOURCE=..\common\Md5.h
# End Source File
# Begin Source File

SOURCE=..\common\Md5_calc.c
# End Source File
# Begin Source File

SOURCE=..\common\Md5_calc.h
# End Source File
# Begin Source File

SOURCE=..\common\Md5_dgst.c
# End Source File
# Begin Source File

SOURCE=..\common\Md5_locl.h
# End Source File
# Begin Source File

SOURCE=..\common\Misc.c
# End Source File
# Begin Source File

SOURCE=..\common\Misc.h
# End Source File
# Begin Source File

SOURCE=..\common\Morpher.c
# End Source File
# Begin Source File

SOURCE=..\common\Morpher.h
# End Source File
# Begin Source File

SOURCE=..\common\Msgs.c
# End Source File
# Begin Source File

SOURCE=..\common\Msgs.h
# End Source File
# Begin Source File

SOURCE=..\common\Options.c
# End Source File
# Begin Source File

SOURCE=..\common\Options.h
# End Source File
# Begin Source File

SOURCE=..\common\Os2lx.c
# End Source File
# Begin Source File

SOURCE=..\common\Os2lx.h
# End Source File
# Begin Source File

SOURCE=..\common\Os2obj.c
# End Source File
# Begin Source File

SOURCE=..\common\Os2obj.h
# End Source File
# Begin Source File

SOURCE=..\common\Os2page.c
# End Source File
# Begin Source File

SOURCE=..\common\Os2page.h
# End Source File
# Begin Source File

SOURCE=..\Common\Pef.c
# End Source File
# Begin Source File

SOURCE=..\Common\Pef.h
# End Source File
# Begin Source File

SOURCE=..\Common\Pidata.c
# End Source File
# Begin Source File

SOURCE=..\Common\Pidata.h
# End Source File
# Begin Source File

SOURCE=..\common\Spans.c
# End Source File
# Begin Source File

SOURCE=..\common\Spans.h
# End Source File
# End Group
# Begin Group "cmdline"

# PROP Default_Filter ""
# End Group
# Begin Group "mfc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\backupdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\backupdlg.h
# End Source File
# Begin Source File

SOURCE=.\callback.cpp
# End Source File
# Begin Source File

SOURCE=.\Fortify.cpp
# End Source File
# Begin Source File

SOURCE=.\Fortify.h
# End Source File
# Begin Source File

SOURCE=.\Fortify.rc
# End Source File
# Begin Source File

SOURCE=.\fortifydlg.cpp
# End Source File
# Begin Source File

SOURCE=.\fortifydlg.h
# End Source File
# Begin Source File

SOURCE=.\optionsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\optionsdlg.h
# End Source File
# Begin Source File

SOURCE=.\progressdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\progressdlg.h
# End Source File
# Begin Source File

SOURCE=.\Psapi.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\splashdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\splashdlg.h
# End Source File
# Begin Source File

SOURCE=.\Stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.h
# End Source File
# Begin Source File

SOURCE=.\sysobj.cpp
# End Source File
# Begin Source File

SOURCE=.\sysobj.h
# End Source File
# End Group
# End Target
# End Project
