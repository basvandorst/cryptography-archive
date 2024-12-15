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
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Gt200 /Od /D "_DEBUG" /I ".\gensock" /FR /GA /Fd"WINVN.PDB"
CFLAGS_R_WEXE = /nologo /G3 /W3 /AL /Gt500 /D "NDEBUG" /I ".\gensock" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /MAP  
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
		WINVN.SBR \
		WVART.SBR \
		WVAUTH.SBR \
		WVBLOCK.SBR \
		WVCNCM.SBR \
		WVFILE.SBR \
		WVGROUP.SBR \
		WVHEADER.SBR \
		WVLIST.SBR \
		WVMAIL.SBR \
		WVMAPI.SBR \
		WVPOST.SBR \
		WVPRINT.SBR \
		WVSCREEN.SBR \
		WVUSENET.SBR \
		WVUTIL.SBR \
		WVCRYPT.SBR \
		WVATTACH.SBR \
		WVCODING.SBR \
		WVTXTBLK.SBR \
		WVCODEWN.SBR \
		WVSCKGEN.SBR


HEADARRY_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


INI_NEWS_DEP = c:\winvn\wvglob.h


WINVN_DEP = c:\winvn\wvglob.h


WINVN_DEP = c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h \
	c:\winvn\defaults.h


WVART_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVAUTH_DEP = 

WVBLOCK_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVCNCM_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVFILE_DEP = c:\winvn\wvglob.h \
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
	c:\winvn\version.h


WVMAIL_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVMAPI_DEP = c:\winvn\mapi.h \
	c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h \
	c:\winvn\wvmapi.h


WVPOST_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVPRINT_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVSCREEN_DEP = c:\winvn\wvglob.h \
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
	c:\winvn\winvn.ico \
	c:\winvn\wvgroup.ico \
	c:\winvn\wvart.ico \
	c:\winvn\wvpost.ico \
	c:\winvn\dish.ico \
	c:\winvn\wvcstat1.ico \
	c:\winvn\wvcstat2.ico \
	c:\winvn\wvdebugc.ico \
	c:\winvn\gensock\gensock.rc \
	c:\winvn\gensock\gensock.h \
	c:\winvn\gensock\gensock.ico


WVCRYPT_DEP = 

WVATTACH_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVCODING_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVTXTBLK_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVCODEWN_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


WVSCKGEN_DEP = c:\winvn\wvglob.h \
	c:\winvn\winvn.h \
	c:\winvn\wvdlg.h \
	c:\winvn\version.h


all:	$(PROJ).EXE $(PROJ).BSC

HEADARRY.OBJ:	HEADARRY.C $(HEADARRY_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c HEADARRY.C

INI_NEWS.OBJ:	INI_NEWS.C $(INI_NEWS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INI_NEWS.C

SHELLSOR.OBJ:	SHELLSOR.C $(SHELLSOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SHELLSOR.C

WINVN.OBJ:	WINVN.C $(WINVN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WINVN.C

WVART.OBJ:	WVART.C $(WVART_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVART.C

WVAUTH.OBJ:	WVAUTH.C $(WVAUTH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVAUTH.C

WVBLOCK.OBJ:	WVBLOCK.C $(WVBLOCK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVBLOCK.C

WVCNCM.OBJ:	WVCNCM.C $(WVCNCM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCNCM.C

WVFILE.OBJ:	WVFILE.C $(WVFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVFILE.C

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

WVPOST.OBJ:	WVPOST.C $(WVPOST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVPOST.C

WVPRINT.OBJ:	WVPRINT.C $(WVPRINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVPRINT.C

WVSCREEN.OBJ:	WVSCREEN.C $(WVSCREEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVSCREEN.C

WVUSENET.OBJ:	WVUSENET.C $(WVUSENET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVUSENET.C

WVUTIL.OBJ:	WVUTIL.C $(WVUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVUTIL.C

WINVN.RES:	WINVN.RC $(WINVN_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r WINVN.RC

WVCRYPT.OBJ:	WVCRYPT.C $(WVCRYPT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCRYPT.C

WVATTACH.OBJ:	WVATTACH.C $(WVATTACH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVATTACH.C

WVCODING.OBJ:	WVCODING.C $(WVCODING_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCODING.C

WVTXTBLK.OBJ:	WVTXTBLK.C $(WVTXTBLK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVTXTBLK.C

WVCODEWN.OBJ:	WVCODEWN.C $(WVCODEWN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVCODEWN.C

WVSCKGEN.OBJ:	WVSCKGEN.C $(WVSCKGEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WVSCKGEN.C


$(PROJ).EXE::	WINVN.RES

$(PROJ).EXE::	HEADARRY.OBJ INI_NEWS.OBJ SHELLSOR.OBJ WINVN.OBJ WVART.OBJ WVAUTH.OBJ \
	WVBLOCK.OBJ WVCNCM.OBJ WVFILE.OBJ WVGROUP.OBJ WVHEADER.OBJ WVLIST.OBJ WVMAIL.OBJ \
	WVMAPI.OBJ WVPOST.OBJ WVPRINT.OBJ WVSCREEN.OBJ WVUSENET.OBJ WVUTIL.OBJ WVCRYPT.OBJ \
	WVATTACH.OBJ WVCODING.OBJ WVTXTBLK.OBJ WVCODEWN.OBJ WVSCKGEN.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
HEADARRY.OBJ +
INI_NEWS.OBJ +
SHELLSOR.OBJ +
WINVN.OBJ +
WVART.OBJ +
WVAUTH.OBJ +
WVBLOCK.OBJ +
WVCNCM.OBJ +
WVFILE.OBJ +
WVGROUP.OBJ +
WVHEADER.OBJ +
WVLIST.OBJ +
WVMAIL.OBJ +
WVMAPI.OBJ +
WVPOST.OBJ +
WVPRINT.OBJ +
WVSCREEN.OBJ +
WVUSENET.OBJ +
WVUTIL.OBJ +
WVCRYPT.OBJ +
WVATTACH.OBJ +
WVCODING.OBJ +
WVTXTBLK.OBJ +
WVCODEWN.OBJ +
WVSCKGEN.OBJ +
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
