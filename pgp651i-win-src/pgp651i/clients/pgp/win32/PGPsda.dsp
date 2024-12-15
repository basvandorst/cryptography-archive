# Microsoft Developer Studio Project File - Name="PGPsda" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PGPsda - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPsda.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPsda.mak" CFG="PGPsda - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPsda - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PGPsda - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPsda - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPsda\Release"
# PROP Intermediate_Dir "PGPsda\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "PGPsda" /I "..\shared" /I "include" /I "shared" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /D _WIN32_IE=0x0300 /D IS_SDA=1 /D "NDEBUG" /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /D PGP_DEBUG=0 /D USERLAND_TEST=1 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"PGPsda\Release/PGPsda.etm"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy       pgpsda\Release\PGPsda.etm PGPsc\Res\.
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPsda - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPsda\Debug"
# PROP Intermediate_Dir "PGPsda\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "PGPsda" /I "..\shared" /I "include" /I "shared" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\common\prefs" /I "..\..\..\libs\pgpcdk\pub\include" /I "..\..\..\libs\pgpcdk\win32" /I "..\..\..\libs\pfl\win32" /I "..\..\..\libs\pfl\common\file" /I "..\..\..\libs\pfl\common\util" /D _WIN32_IE=0x0300 /D IS_SDA=1 /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D PGP_WIN32=1 /D PGP_DEBUG=1 /D USERLAND_TEST=1 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"PGPsda\Debug/PGPsda.etm" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy       pgpsda\Debug\PGPsda.etm PGPsc\Res\.
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPsda - Win32 Release"
# Name "PGPsda - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shared\DecodeStub.c
# End Source File
# Begin Source File

SOURCE=.\shared\DeflateDecompress.c
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpCAST5.c
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpSHA.c
# End Source File
# Begin Source File

SOURCE=.\shared\ProgressDialog.c
# End Source File
# Begin Source File

SOURCE=.\PGPsda\sdacast.c
# End Source File
# Begin Source File

SOURCE=.\PGPsda\sdapass.c
# End Source File
# Begin Source File

SOURCE=.\PGPsda\sdautil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shared\DecodeStub.h
# End Source File
# Begin Source File

SOURCE=.\shared\DeflateWrapper.h
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpCAST5.h
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpCASTBox5.h
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpHashPriv.h
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpSHA.h
# End Source File
# Begin Source File

SOURCE=.\PGPsda\pgpSymmetricCipherPriv.h
# End Source File
# Begin Source File

SOURCE=.\shared\ProgressDialog.h
# End Source File
# Begin Source File

SOURCE=.\PGPsda\sdautil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PGPsda\res\exeicon.ico
# End Source File
# Begin Source File

SOURCE=.\PGPsda\PGPsda.rc
# End Source File
# End Group
# End Target
# End Project
