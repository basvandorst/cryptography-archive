#
# help - makefile for calc help files
#
# Copyright (c) 1992 David I. Bell and Landon Curt Noll
# Permission is granted to use, distribute, or modify this source,
# provided that this copyright notice remains intact.
#
# Arbitrary precision calculator.
#
# calculator by David I. Bell
# makefile by Landon Curt Noll

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# where to install things
HELPDIR= /usr/local/lib/calc/help
# how to build a directory
MKDIR=mkdir -p
#MKDIR=mkdir

# The header file, along with these files form the full help file.
#
FULL_HELP_FILES=intro command expression define variable statement \
	operator types obj mat list file builtin config interrupt \
	history usage credit

# We install these help files
#
HELP_FILES= ${FULL_HELP_FILES} full overview stdlib environment todo credit help

SHELL= /bin/sh

all: ${HELP_FILES}

full: header ${FULL_HELP_FILES}
	rm -f full
	cp header full
	chmod +w full
	@for i in ${FULL_HELP_FILES}; do \
		echo '' >> full; \
		echo '' >> full; \
		echo "cat $$i >> full"; \
		cat $$i >> full; \
	done
	chmod 0444 full

stdlib: ../lib/README
	rm -f stdlib
	cp ../lib/README stdlib
	chmod 0444 stdlib

clean:

clobber:
	rm -f full stdlib

install: all
	-@if [ ! -d ${HELPDIR} ]; then \
		echo "	${MKDIR} ${HELPDIR}"; \
		${MKDIR} ${HELPDIR}; \
	fi
	@for i in ${HELP_FILES}; do \
		echo "	rm -f ${HELPDIR}/$$i"; \
		rm -f ${HELPDIR}/$$i; \
		echo "	chmod 0444 $$i"; \
		chmod 0444 $$i; \
		echo "	cp $$i ${HELPDIR}"; \
		cp $$i ${HELPDIR}; \
	done
