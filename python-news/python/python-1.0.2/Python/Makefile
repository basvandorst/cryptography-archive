!include <ntwin32.MAK>

srcdir=		..


DEFS=		-DHAVE_CONFIG_H

INCLDIR=	$(srcdir)\Include
OPT=		$(cvars) $(cdebug) $(cflags) # -O
CFLAGS=		$(OPT) -I$(INCLDIR) -I.. $(DEFS)

OBJS=		\
		bltinmodule.obj \
		ceval.obj cgensupport.obj compile.obj \
		errors.obj \
		frozenmain.obj \
		getmtime.obj graminit.obj \
		import.obj \
		marshal.obj modsupport.obj mystrtoul.obj \
		pythonmain.obj pythonrun.obj \
		structmember.obj sysmodule.obj \
		traceback.obj \
		getopt.obj \
		version.obj \
		$(LIBOBJS)

XLIB=		Python.lib

all:		$(XLIB)

$(XLIB):	$(OBJS)
		$(implib) -out:$(XLIB) $(OBJS)

clean:
		del *.obj *.lib *~

bltinmodule.obj: bltinmodule.c
ceval.obj: ceval.c
cgensupport.obj: cgensupport.c
compile.obj: compile.c
dup2.obj: dup2.c
errors.obj: errors.c
fmod.obj: fmod.c
frozenmain.obj: frozenmain.c
getcwd.obj: getcwd.c
getmtime.obj: getmtime.c
graminit.obj: graminit.c
import.obj: import.c
marshal.obj: marshal.c
memmove.obj: memmove.c
modsupport.obj: modsupport.c
mystrtoul.obj: mystrtoul.c
pythonmain.obj: pythonmain.c
pythonrun.obj: pythonrun.c
strerror.obj: strerror.c
strtod.obj: strtod.c
structmember.obj: structmember.c
sysmodule.obj: sysmodule.c
thread.obj: thread.c
traceback.obj: traceback.c
version.obj: version.c

