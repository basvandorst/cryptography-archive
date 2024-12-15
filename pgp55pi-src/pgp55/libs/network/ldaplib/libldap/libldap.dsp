# Microsoft Developer Studio Project File - Name="libldap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libldap - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libldap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libldap.mak" CFG="libldap - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libldap - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libldap - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "libldap - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libldap_"
# PROP BASE Intermediate_Dir "libldap_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\include" /I "..\msdos" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\..\pfl\win32" /I "..\..\..\..\pfl\common" /D "NEEDPROTOS" /D "THREAD_NT_PTHREADS" /D "LDAP_LDBM" /D "LDBM_USE_DBBTREE21" /D "PGPKEYSERVER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libldap - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libldap0"
# PROP BASE Intermediate_Dir "libldap0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Od /I "..\..\include" /I "..\msdos" /I "..\..\..\libs\pfl\common" /I "..\..\..\libs\pfl\win32" /I "..\..\..\..\pfl\win32" /I "..\..\..\..\pfl\common" /D "LDAP_DEBUG" /D "NEEDPROTOS" /D "THREAD_NT_PTHREADS" /D "LDAP_LDBM" /D "LDBM_USE_DBBTREE21" /D "PGPKEYSERVER" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PGP_WIN32=1 /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libldap - Win32 Release"
# Name "libldap - Win32 Debug"
# Begin Source File

SOURCE=.\abandon.c
# End Source File
# Begin Source File

SOURCE=.\add.c
# End Source File
# Begin Source File

SOURCE=.\addentry.c
# End Source File
# Begin Source File

SOURCE=.\bind.c
# End Source File
# Begin Source File

SOURCE=.\cache.c
# End Source File
# Begin Source File

SOURCE=.\charset.c
# End Source File
# Begin Source File

SOURCE=.\cldap.c
# End Source File
# Begin Source File

SOURCE=.\compare.c
# End Source File
# Begin Source File

SOURCE=.\delete.c
# End Source File
# Begin Source File

SOURCE=.\disptmpl.c
# End Source File
# Begin Source File

SOURCE=.\dsparse.c
# End Source File
# Begin Source File

SOURCE=.\error.c
# End Source File
# Begin Source File

SOURCE=.\free.c
# End Source File
# Begin Source File

SOURCE=.\friendly.c
# End Source File
# Begin Source File

SOURCE=.\getattr.c
# End Source File
# Begin Source File

SOURCE=.\getdn.c
# End Source File
# Begin Source File

SOURCE=.\getdxbyname.c
# End Source File
# Begin Source File

SOURCE=.\getentry.c
# End Source File
# Begin Source File

SOURCE=.\getfilter.c
# End Source File
# Begin Source File

SOURCE=.\getopt.c
# End Source File
# Begin Source File

SOURCE=.\getvalues.c
# End Source File
# Begin Source File

SOURCE=.\kbind.c
# End Source File
# Begin Source File

SOURCE=.\libldap.rc
# End Source File
# Begin Source File

SOURCE=.\modify.c
# End Source File
# Begin Source File

SOURCE=.\modrdn.c
# End Source File
# Begin Source File

SOURCE=.\open.c
# End Source File
# Begin Source File

SOURCE=".\os-ip.c"
# End Source File
# Begin Source File

SOURCE=.\regex.c
# End Source File
# Begin Source File

SOURCE=.\request.c
# End Source File
# Begin Source File

SOURCE=.\result.c
# End Source File
# Begin Source File

SOURCE=.\sbind.c
# End Source File
# Begin Source File

SOURCE=.\search.c
# End Source File
# Begin Source File

SOURCE=.\sort.c
# End Source File
# Begin Source File

SOURCE=.\srchpref.c
# End Source File
# Begin Source File

SOURCE=.\tmplout.c
# End Source File
# Begin Source File

SOURCE=.\tmpltest.c
# End Source File
# Begin Source File

SOURCE=.\ufn.c
# End Source File
# Begin Source File

SOURCE=.\unbind.c
# End Source File
# Begin Source File

SOURCE=.\url.c
# End Source File
# Begin Source File

SOURCE=.\Version.c
# End Source File
# End Target
# End Project
