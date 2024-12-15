!include <ntwin32.MAK>

srcdir=		..

DEFS=		-DHAVE_CONFIG_H

INCLDIR=	$(srcdir)\Include
OPT=		$(cvars) $(cdebug) $(cflags) # -O
CFLAGS=		$(OPT) -I$(INCLDIR) -I.. $(DEFS) -Disatty=_isatty


OBJS=		accessobject.obj \
		classobject.obj fileobject.obj floatobject.obj \
		frameobject.obj funcobject.obj intobject.obj listobject.obj \
		longobject.obj mappingobject.obj methodobject.obj \
		moduleobject.obj object.obj rangeobject.obj stringobject.obj \
		tupleobject.obj typeobject.obj

SRCS=		accessobject.c \
		classobject.c fileobject.c floatobject.c \
		frameobject.c funcobject.c intobject.c listobject.c \
		longobject.c mappingobject.c methodobject.c \
		moduleobject.c object.c rangeobject.c stringobject.c \
		tupleobject.c typeobject.c

XLIB=		Objects.lib

# === Rules ===

all:		$(XLIB)

$(XLIB):	$(OBJS)
		$(implib) -out:$(XLIB) $(OBJS)

clean:
		del *.obj *.lib

Makefile:	$(srcdir)/Makefile.in ../config.status
		(cd ..; CONFIG_FILES=Objects/Makefile CONFIG_HEADERS= \
		$(SHELL) config.status)

accessobject.obj: accessobject.c
classobject.obj: classobject.c
fileobject.obj: fileobject.c
floatobject.obj: floatobject.c
frameobject.obj: frameobject.c
funcobject.obj: funcobject.c
intobject.obj: intobject.c
listobject.obj: listobject.c
longobject.obj: longobject.c
mappingobject.obj: mappingobject.c
methodobject.obj: methodobject.c
moduleobject.obj: moduleobject.c
object.obj: object.c
rangeobject.obj: rangeobject.c
stringobject.obj: stringobject.c
tupleobject.obj: tupleobject.c
typeobject.obj: typeobject.c

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.
# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
