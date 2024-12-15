# Microsoft Developer Studio Project File - Name="PGPmemlock" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=PGPmemlock - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPmemlock.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPmemlock.mak" CFG="PGPmemlock - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPmemlock - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "PGPmemlock - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "PGPmemlock - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f PGPmemlock.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "PGPmemlock.vxd"
# PROP BASE Bsc_Name "PGPmemlock.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "NMAKE /f PGPmemlock.mak CFG="PGPmemlock - Win32 Release""
# PROP Rebuild_Opt "/a"
# PROP Target_File "PGPmemlock.vxd"
# PROP Bsc_Name "PGPmemlock.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "PGPmemlock - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f PGPmemlock.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "PGPmemlock.vxd"
# PROP BASE Bsc_Name "PGPmemlock.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "NMAKE /f PGPmemlock.mak CFG="PGPmemlock - Win32 Debug""
# PROP Rebuild_Opt "/a"
# PROP Target_File "PGPmemlock.vxd"
# PROP Bsc_Name "PGPmemlock.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "PGPmemlock - Win32 Release"
# Name "PGPmemlock - Win32 Debug"

!IF  "$(CFG)" == "PGPmemlock - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPmemlock - Win32 Debug"

!ENDIF 

# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\pgpMemLock.c
# End Source File
# Begin Source File

SOURCE=.\Source\pgpMemLock.def
# End Source File
# Begin Source File

SOURCE=.\Source\pgpMLdevdcl.asm
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPmemlock.mak
# End Source File
# End Target
# End Project
