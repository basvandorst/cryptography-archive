#
# Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

######################################################################
# put in your architecture here
#
# tested so far are 
#   solaris

ARCH = solaris
BUILD = UNIX

######################################################################
# need sh as shell

SHELL = /bin/sh

######################################################################
# modify your java root here if java lives somewhere else for you

# JAVA_ROOT=/usr/tmp/java

JAVA_ROOT=/home/apps/java


######################################################################
# compiler, linker options

CC = gcc
CC_INCLUDE = -I$(JAVA_ROOT)/include -I$(JAVA_ROOT)/include/$(ARCH)
LINK = gcc
LINK_OPTIONS = -shared
AR = ar

######################################################################
# end of user configuration
######################################################################

PASS = "JAVA_ROOT=$(JAVA_ROOT)" "CC=$(CC)" "CC_INCLUDE=$(CC_INCLUDE)" \
"LINK=$(LINK)" "LINK_OPTIONS=$(LINK_OPTIONS)" "AR=$(AR)" \
"BUILD=$(BUILD)" "ARCH=$(ARCH)"

DIRECTORIES = crypt math

all: $(DIRECTORIES)
	for x in $(DIRECTORIES); do (cd $$x; $(MAKE) -f makefile.$(BUILD) $(PASS)); done

clean:
	for x in $(DIRECTORIES); do (cd $$x; $(MAKE) -f makefile.$(BUILD) $(PASS) clean); done
	
