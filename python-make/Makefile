
# Toplevel Makefile for Python

srcdir=		.

# Compiler options passed to subordinate makes
OPT=		-O

# Subdirectories where to run make recursively
SUBDIRS=	Parser Grammar Objects Python Modules

# Default target
all:		python

# Build the interpreter
python: $(srcdir)\README
	for %%i in ($(SUBDIRS)) do echo %%i & cd %%i & \
		nmake & cd ..

clean:
	for %%i in ($(SUBDIRS)) do cd %%i & nmake clean & cd ..
	del python.exe

