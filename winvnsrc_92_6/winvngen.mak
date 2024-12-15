# Microsoft Visual C++ generated build script - Do not modify

PROJ = WINVNGEN
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = c:\news\niven\winvn.ini c:\news\niven\newsrc
DLLS = gensock.dll
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WINVN\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = HEADARRY.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Gt200 /YX /Od /D "_DEBUG" /D "STRICT" /I ".\gensock" /FR /GA /Fd"WINVN.PDB" /Fp"WINVNGEN.PCH"
CFLAGS_R_WEXE = /nologo /G3 /W3 /AL /Gt200 /YX /D "NDEBUG" /D "STRICT" /I ".\gensock" /FR /GA /Fp"c:\winvn\WINVNGEN.PCH"  
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:8192 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:8192 /ALIGN:16 /ONERROR:NOEXE /MAP  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo /i .\gensock 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = WINVNGEN.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = HEADARRY.SBR \
		INI_NEWS.SBR \
		SHELLSOR.SBR \
		VERSION.SBR \
		WINVN.SBR \
		WVART.SBR \
		WVATTACH.SBR \
		WVAUTH.SBR \
		WVBLOCK.SBR \
		WVCNCM.SBR \
		WVCODEWN.SBR \
		WVCODING.SBR \
		WVCRYPT.SBR \
		WVFILE.SBR \
		WVFILER.SBR \
		WVGROUP.SBR \
		WVHEADER.SBR \
		WVLIST.SBR \
		WVMAIL.SBR \
		WVMAPI.SBR \
		WVPATH.SBR \
		WVPOST.SBR \
		WVPRINT.SBR \
		WVSCKGEN.SBR \
		WVSCREEN.SBR \
		WVTXTBLK.SBR \
		WVUSENET.SBR \
		WVUTIL.SBR


HEADARRY_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


INI_NEWS_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


VERSION_DEP = 

WINVN_DEP = c:\winvn\wvglob.h


WINVN_DEP = c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h \
	c:\winvn\defaults.h


WVART_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVATTACH_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVAUTH_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVBLOCK_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVCNCM_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVCODEWN_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVCODING_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVCRYPT_DEP = 

WVFILE_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVFILER_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVGROUP_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVHEADER_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVLIST_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h \
	c:\winvn\primes.h


WVMAIL_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVMAPI_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h \
	c:\winvn\mapi.h \
	c:\winvn\wvmapi.h


WVPATH_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVPOST_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVPRINT_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVSCKGEN_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVSCREEN_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVTXTBLK_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVUSENET_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h \
	c:\winvn\version.c


WVUTIL_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WINVN_RCDEP = c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h \
	c:\msvc\include\winsock.h \
	c:\winvn\gensock/gensock.rc \
	c:\winvn\gensock/\gensock.h \
	c:\winvn\winvn.ico \
	c:\winvn\wvgroup.ico \
	c:\winvn\wvart.ico \
	c:\winvn\wvpost.ico \
	c:\winvn\dish.ico \
	c:\winvn\wvcstat2.ico \
	c:\winvn\wvdebugc.ico \
	c:\winvn\wvcstat1.ico


all:	$(PROJ).EXE $(PROJ).BSC

HEADARRY.OBJ:	HEADARRY.C $(HEADARRY_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c HEADARRY.C

INI_NEWS.OBJ:	INI_NEWS.C $(INI_NEWS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INI_NEWS.C

SHELLSOR.OBJ:	SHELLSOR.C $(SHELLSOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SHELLSOR.C

VERSION.OBJ:	VERSION.C $(VERSION_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VERSION.C

WINVN.OBJ:	WINVN.C $(WINVN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WINVN.C

WVART.OBJ:	WVART.C $(WVART_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVART.C

WVATTACH.OBJ:	WVATTACH.C $(WVATTACH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVATTACH.C

WVAUTH.OBJ:	WVAUTH.C $(WVAUTH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVAUTH.C

WVBLOCK.OBJ:	WVBLOCK.C $(WVBLOCK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVBLOCK.C

WVCNCM.OBJ:	WVCNCM.C $(WVCNCM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCNCM.C

WVCODEWN.OBJ:	WVCODEWN.C $(WVCODEWN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCODEWN.C

WVCODING.OBJ:	WVCODING.C $(WVCODING_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCODING.C

WVCRYPT.OBJ:	WVCRYPT.C $(WVCRYPT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCRYPT.C

WVFILE.OBJ:	WVFILE.C $(WVFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVFILE.C

WVFILER.OBJ:	WVFILER.C $(WVFILER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVFILER.C

WVGROUP.OBJ:	WVGROUP.C $(WVGROUP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVGROUP.C

WVHEADER.OBJ:	WVHEADER.C $(WVHEADER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVHEADER.C

WVLIST.OBJ:	WVLIST.C $(WVLIST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVLIST.C

WVMAIL.OBJ:	WVMAIL.C $(WVMAIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVMAIL.C

WVMAPI.OBJ:	WVMAPI.C $(WVMAPI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVMAPI.C

WVPATH.OBJ:	WVPATH.C $(WVPATH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVPATH.C

WVPOST.OBJ:	WVPOST.C $(WVPOST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVPOST.C

WVPRINT.OBJ:	WVPRINT.C $(WVPRINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVPRINT.C

WVSCKGEN.OBJ:	WVSCKGEN.C $(WVSCKGEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVSCKGEN.C

WVSCREEN.OBJ:	WVSCREEN.C $(WVSCREEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVSCREEN.C

WVTXTBLK.OBJ:	WVTXTBLK.C $(WVTXTBLK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVTXTBLK.C

WVUSENET.OBJ:	WVUSENET.C $(WVUSENET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVUSENET.C

WVUTIL.OBJ:	WVUTIL.C $(WVUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVUTIL.C

WINVN.RES:	WINVN.RC $(WINVN_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r WINVN.RC


$(PROJ).EXE::	WINVN.RES

$(PROJ).EXE::	HEADARRY.OBJ INI_NEWS.OBJ SHELLSOR.OBJ VERSION.OBJ WINVN.OBJ WVART.OBJ \
	WVATTACH.OBJ WVAUTH.OBJ WVBLOCK.OBJ WVCNCM.OBJ WVCODEWN.OBJ WVCODING.OBJ WVCRYPT.OBJ \
	WVFILE.OBJ WVFILER.OBJ WVGROUP.OBJ WVHEADER.OBJ WVLIST.OBJ WVMAIL.OBJ WVMAPI.OBJ \
	WVPATH.OBJ WVPOST.OBJ WVPRINT.OBJ WVSCKGEN.OBJ WVSCREEN.OBJ WVTXTBLK.OBJ WVUSENET.OBJ \
	WVUTIL.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
HEADARRY.OBJ +
INI_NEWS.OBJ +
SHELLSOR.OBJ +
VERSION.OBJ +
WINVN.OBJ +
WVART.OBJ +
WVATTACH.OBJ +
WVAUTH.OBJ +
WVBLOCK.OBJ +
WVCNCM.OBJ +
WVCODEWN.OBJ +
WVCODING.OBJ +
WVCRYPT.OBJ +
WVFILE.OBJ +
WVFILER.OBJ +
WVGROUP.OBJ +
WVHEADER.OBJ +
WVLIST.OBJ +
WVMAIL.OBJ +
WVMAPI.OBJ +
WVPATH.OBJ +
WVPOST.OBJ +
WVPRINT.OBJ +
WVSCKGEN.OBJ +
WVSCREEN.OBJ +
WVTXTBLK.OBJ +
WVUSENET.OBJ +
WVUTIL.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) WINVN.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	WINVN.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) WINVN.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
