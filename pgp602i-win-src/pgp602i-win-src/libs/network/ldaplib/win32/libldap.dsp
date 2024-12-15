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
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\pgpcdk\priv\networklib\win32" /I "..\libldap" /I "..\include" /I "..\pgpsockets" /I "..\..\..\pgpcdk\pub\include" /I "..\..\..\pgpcdk\win32" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\win32\pgpAsyncDNS" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\lthread" /I "..\\" /I "..\..\..\pgpcdk\priv\networklib\keyserver" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "LDAP_DEBUG" /D "NEEDPROTOS" /D "THREAD_NT_PTHREADS" /D "LDAP_LDBM" /D "LDBM_USE_DBBTREE21" /D UNFINISHED_CODE_ALLOWED=0 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D PGPSOCKETSLDAP=1 /D UNFINISHED_CODE_ALLOWED=1 /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libldap - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\libldap"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\..\..\pgpcdk\priv\networklib\win32" /I "..\libldap" /I "..\include" /I "..\pgpsockets" /I "..\..\..\pgpcdk\pub\include" /I "..\..\..\pgpcdk\win32" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\win32\pgpAsyncDNS" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\lthread" /I "..\\" /I "..\..\..\pgpcdk\priv\networklib\keyserver" /D "_DEBUG" /D "LDAP_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NEEDPROTOS" /D "THREAD_NT_PTHREADS" /D "LDAP_LDBM" /D "LDBM_USE_DBBTREE21" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D PGPSOCKETSLDAP=1 /YX /FD /c
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

SOURCE=..\libldap\abandon.c
# End Source File
# Begin Source File

SOURCE=..\libldap\add.c
# End Source File
# Begin Source File

SOURCE=..\libldap\addentry.c
# End Source File
# Begin Source File

SOURCE=..\libldap\bind.c
# End Source File
# Begin Source File

SOURCE=..\libldap\cache.c
# End Source File
# Begin Source File

SOURCE=..\libldap\charset.c
# End Source File
# Begin Source File

SOURCE=..\libldap\cldap.c
# End Source File
# Begin Source File

SOURCE=..\libldap\compare.c
# End Source File
# Begin Source File

SOURCE=..\libldap\delete.c
# End Source File
# Begin Source File

SOURCE=..\libldap\disptmpl.c
# End Source File
# Begin Source File

SOURCE=..\libldap\dsparse.c
# End Source File
# Begin Source File

SOURCE=..\libldap\error.c
# End Source File
# Begin Source File

SOURCE=..\libldap\free.c
# End Source File
# Begin Source File

SOURCE=..\libldap\friendly.c
# End Source File
# Begin Source File

SOURCE=..\libldap\getattr.c
# End Source File
# Begin Source File

SOURCE=..\libldap\getdn.c
# End Source File
# Begin Source File

SOURCE=..\libldap\getdxbyname.c
# End Source File
# Begin Source File

SOURCE=..\libldap\getentry.c
# End Source File
# Begin Source File

SOURCE=..\libldap\getfilter.c
# End Source File
# Begin Source File

SOURCE=..\libldap\getopt.c
# End Source File
# Begin Source File

SOURCE=..\libldap\getvalues.c
# End Source File
# Begin Source File

SOURCE=..\libldap\kbind.c
# End Source File
# Begin Source File

SOURCE=..\libldap\modify.c
# End Source File
# Begin Source File

SOURCE=..\libldap\modrdn.c
# End Source File
# Begin Source File

SOURCE=..\libldap\open.c
# End Source File
# Begin Source File

SOURCE="..\pgpsockets\pgpSockets-ip.c"
# End Source File
# Begin Source File

SOURCE=..\libldap\regex.c
# End Source File
# Begin Source File

SOURCE=..\libldap\request.c
# End Source File
# Begin Source File

SOURCE=..\libldap\result.c
# End Source File
# Begin Source File

SOURCE=..\libldap\sbind.c
# End Source File
# Begin Source File

SOURCE=..\libldap\search.c
# End Source File
# Begin Source File

SOURCE=..\libldap\sort.c
# End Source File
# Begin Source File

SOURCE=..\libldap\srchpref.c
# End Source File
# Begin Source File

SOURCE=..\libldap\tmplout.c
# End Source File
# Begin Source File

SOURCE=..\libldap\ufn.c
# End Source File
# Begin Source File

SOURCE=..\libldap\unbind.c
# End Source File
# Begin Source File

SOURCE=..\libldap\url.c
# End Source File
# Begin Source File

SOURCE=..\libldap\Version.c
# End Source File
# End Target
# End Project
