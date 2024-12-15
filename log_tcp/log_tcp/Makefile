# @(#) Makefile 1.5 92/06/11 22:21:35

##############################
## Begin configuration options

# By default, logfile entries are written to the same file as used for
# sendmail transaction logs. Change the definition of the following macro
# if you disagree. See `man 3 syslog' for examples. Some syslog versions
# do not provide this flexibility.

FACILITY= LOG_MAIL

# To disable host access control, comment out the following macro definition.
# Note: host access control requires the strtok() and strchr() routines.
# Host access control can also be turned off by providing no access control
# tables.

ACCESS	= -DHOSTS_ACCESS

# Disable the following macro definition if you wish to talk to hosts that
# pretend to have someone elses host name. Doing so is a bad idea if you run
# services whose authentication relies on host names, such as rlogin or rsh.
#
# If the macro is undefined the event is logged anyway (vince@ghost)

PARANOID= # -DPARANOID

# If your system supports NIS or YP-style netgroups, enable the following
# macro definition.

NETGROUP= -DNETGROUP

# Enable the following definition if the front ends should attempt to get
# the remote user name. This works only if the remote host runs a daemon
# that supports the RFC 931 protocol. Remote user name lookups will not
# work for UDP-based connections, and will cause noticeable delays with
# connections from PCs.

AUTH	= -DRFC931

# On many systems, network daemons and other system processes are started
# with a zero umask value, so that world-writable files may be produced.
# It is a good idea to edit your /etc/rc* files so that they begin with
# an explicit umask setting.  On our site we use 022 because it does not
# break anything yet gives adequate protection against tampering.
# 
# The following macro specifies the default umask for daemons run under
# control of the daemon front ends. Comment it out only if you are sure
# that inetd and its children are started with a sane umask value.

UMASK	= -DDAEMON_UMASK=022

# Some versions of Apollo or SYSV.4 UNIX have a bug in the getpeername(2)
# routine.  You have this bug if the front end reports that all UDP
# connections come from address 0.0.0.0. Compile with -DGETPEERNAME_BUG
# for a workaround. The workaround does no harm on other systems. If in
# doubt, leave it in.

BUGS	= -DGETPEERNAME_BUG

# Apollo Domain/OS offers both bsd and sys5 environments, sometimes
# on the same machine.  If your Apollo is primarily sys5.3 and also
# has bsd4.3, enable the following to build under bsd and run under
# either environment.

#SYSTYPE=  -A run,any -A sys,any

# Some C compilers (Ultrix 4.x) insist that ranlib(1) be run on an object
# library; some don't care as long as the modules are in the right order;
# some systems don't even have a ranlib(1) command. Make your choice.

RANLIB	= ranlib	# use ranlib
#RANLIB	= echo		# no ranlib

# If your C library does not have memcmp(3), compile with -Dmemcmp=bcmp.

CFLAGS	= +DA1.0 -O -DFACILITY=$(FACILITY) $(ACCESS) $(PARANOID) $(NETGROUP) \
	$(BUGS) $(SYSTYPE) $(AUTH) $(UMASK)

# Include the file strcasecmp.o if strcasecmp() is not provided by your C
# library.  The strcasecmp.c file provided with this package comes from
# 43BSD UNIX.  You may have to add -Du_char="unsigned char" to the CFLAGS
# macro above.

AUX_OBJ	= strcasecmp.o

# Some System-V versions require that you explicitly specify the networking
# libraries (for example, -lnet or -linet).

LIBS	= # -lnet -lsyslog -ldbm

# Ultrix users may want to use the miscd front end, too. The Ultrix miscd
# implements among others the SYSTAT service which runs the WHO command,
# and thus provides a subset of the finger service. The very first front
# end application was to monitor the use of SYSTAT.

all:	tcpd try # miscd

## End configuration options
############################

LIB_OBJ= hosts_access.o options.o shell_cmd.o rfc931.o hosts_info.o \
	hosts_ctl.o refuse.o percent_x.o clean_exit.o $(AUX_OBJ)

KIT	= README miscd.c tcpd.c fromhost.c hosts_access.c shell_cmd.c \
	log_tcp.h try.c refuse.c Makefile hosts_access.5 strcasecmp.c BLURB \
	rfc931.c tcpd.8 hosts_info.c hosts_access.3 hosts_ctl.c percent_x.c \
	options.c clean_exit.c rfc931_option

LIB	= libwrap.a

$(LIB):	$(LIB_OBJ)
	rm -f $(LIB)
	ar rv $(LIB) $(LIB_OBJ)
	$(RANLIB) $(LIB)

tcpd:	tcpd.o fromhost.o $(LIB)
	$(CC) $(CFLAGS) -o $@ tcpd.o fromhost.o $(LIB)

miscd:	miscd.o fromhost.o $(LIB)
	$(CC) $(CFLAGS) -o $@ miscd.o fromhost.o $(LIB)

try:	try.o $(LIB)
	$(CC) $(CFLAGS) -o $@ try.o $(LIB)

fromhost: fromhost.c log_tcp.h Makefile $(LIB)
	$(CC) $(CFLAGS) -DTEST -o fromhost fromhost.c $(LIB)
	rm -f fromhost.o

shar:	
	@shar $(KIT)

kit:
	@makekit $(KIT)

archive:
	$(ARCHIVE) $(KIT)

clean:
	rm -f tcpd miscd try fromhost *.[oa] core

# Enable all bells and whistles for linting.

lint: tcpd_lint miscd_lint try_lint

tcpd_lint:
	lint -DFACILITY=LOG_MAIL -DHOSTS_ACCESS -DPARANOID -DNETGROUP \
	-DGETPEERNAME_BUG -DRFC931 -DDAEMON_UMASK=022 tcpd.c fromhost.c \
	hosts_access.c shell_cmd.c refuse.c rfc931.c hosts_info.c percent_x.c \
	clean_exit.c

miscd_lint:
	lint -DFACILITY=LOG_MAIL -DHOSTS_ACCESS -DPARANOID -DNETGROUP \
	-DGETPEERNAME_BUG -DRFC931 -DDAEMON_UMASK=022 miscd.c fromhost.c \
	hosts_access.c shell_cmd.c refuse.c rfc931.c hosts_info.c percent_x.c \
	clean_exit.c

try_lint:
	lint -DFACILITY=LOG_MAIL -DHOSTS_ACCESS -DNETGROUP try.c \
	hosts_ctl.c hosts_access.c hosts_info.c percent_x.c

# Compilation dependencies.

clean_exit.o: log_tcp.h Makefile
fromhost.o: log_tcp.h Makefile
hosts_access.o: log_tcp.h Makefile
hosts_ctl.o: log_tcp.h Makefile
hosts_info.o: log_tcp.h Makefile
miscd.o: log_tcp.h Makefile
options.o: log_tcp.h Makefile
percent_x.o: log_tcp.h Makefile
refuse.o: log_tcp.h Makefile
rfc931.o: log_tcp.h Makefile
shell_cmd.o: log_tcp.h Makefile
tcpd.o: log_tcp.h Makefile
try.o: log_tcp.h Makefile
