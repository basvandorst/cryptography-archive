dnldnl Local autoconf configuration macros for PGP 3dnldnl $Id: aclocal.m4,v 1.1 1997/05/04 10:29:59 mhw Exp $dnldnldnl The big finish.dnl Produce config.status, config.h, and links, and configure subdirs.dnl PGP_AC_OUTPUT([FILE...] [, PRE.IN] [, POST.IN] [, EXTRA-CMDS] dnl 	[, INIT-CMDS])define(PGP_AC_OUTPUT,[trap '' 1 2 15AC_CACHE_SAVEtrap 'rm -fr conftest* confdefs* core core.* *.core $ac_clean_files; exit 1' \	1 2 15test "x$prefix" = xNONE && prefix=$ac_default_prefix# Let make expand exec_prefix.test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'# Any assignment to VPATH causes Sun make to only execute# the first set of double-colon rules, so remove it if not needed.# If there is a colon in the path, we need to keep it.if test "x$srcdir" = x.; thenchangequote(, )dnl  ac_vpsub='/^[ 	]*VPATH[ 	]*=[^:]*$/d'changequote([, ])dnlfitrap 'rm -f $CONFIG_STATUS conftest*; exit 1' 1 2 15ifdef([AC_LIST_HEADER], [DEFS=-DHAVE_CONFIG_H], [AC_OUTPUT_MAKE_DEFS()])# Without the "./", some shells look in PATH for config.status.: ${CONFIG_STATUS=./config.status}echo creating $CONFIG_STATUSrm -f $CONFIG_STATUScat > $CONFIG_STATUS <<EOF#! /bin/sh# Generated automatically by configure.# Run this file to recreate the current configuration.# This directory was configured as follows,dnl hostname on some systems (SVR3.2, Linux) returns a bogus exit status,dnl so uname gets run too.# on host `(hostname || uname -n) 2>/dev/null | sed 1q`:#[#] [$]0 [$]ac_configure_args## Compiler output produced by configure, useful for debugging# configure, is in ./config.log if it exists.changequote(, )dnlac_cs_usage="Usage: $CONFIG_STATUS [--recheck] [--version] [--help]"changequote([, ])dnlfor ac_optiondo  case "[\$]ac_option" in  -recheck | --recheck | --rechec | --reche | --rech | --rec | --re | --r)    echo "running [\$]{CONFIG_SHELL-/bin/sh} [$]0 [$]ac_configure_args" \		"--no-create --no-recursion"    exec [\$]{CONFIG_SHELL-/bin/sh} [$]0 [$]ac_configure_args --no-create \		--no-recursion ;;  -version | --version | --versio | --versi | --vers | --ver | --ve | --v)    echo "$CONFIG_STATUS generated by autoconf version AC_ACVERSION"    exit 0 ;;  -help | --help | --hel | --he | --h)    echo "[\$]ac_cs_usage"; exit 0 ;;  *) echo "[\$]ac_cs_usage"; exit 1 ;;  esacdoneac_given_srcdir=$srcdirifdef([AC_PROVIDE_AC_PROG_INSTALL], [ac_given_INSTALL="$INSTALL"])dnlchangequote(<<, >>)dnlifdef(<<AC_LIST_HEADER>>,<<trap 'rm -fr `echo "$1 AC_LIST_HEADER" | sed "s/:[^ ]*//g"` conftest*; \	exit 1' 1 2 15>>,<<trap 'rm -fr `echo "$1" | sed "s/:[^ ]*//g"` conftest*; exit 1' 1 2 15>>)changequote([, ])dnlPGP_AC_OUTPUT_FILES($1, $2, $3)ifdef([AC_LIST_HEADER], [AC_OUTPUT_HEADER(AC_LIST_HEADER)])dnlifdef([AC_LIST_LINKS], [AC_OUTPUT_LINKS(AC_LIST_FILES, AC_LIST_LINKS)])dnlifelse([$5], , , [EOFcat >> $CONFIG_STATUS <<EOF$5EOFcat >> $CONFIG_STATUS <<\EOF])$4exit 0EOFchmod +x $CONFIG_STATUSrm -fr confdefs* $ac_clean_filestest "$no_create" = yes || ${CONFIG_SHELL-/bin/sh} $CONFIG_STATUS || exit 1dnl config.status should not do recursion.ifdef([AC_LIST_SUBDIRS], [AC_OUTPUT_SUBDIRS(AC_LIST_SUBDIRS)])dnl])dnldnl This is a subroutine of PGP_AC_OUTPUT.  It is called inside an unquoteddnl here document whose contents are going into config.status.dnl PGP_AC_OUTPUT_FILES(FILE..., PRE.IN, POST.IN)define(PGP_AC_OUTPUT_FILES,[# Protect against being on the right side of a sed subst in config.status. changequote(, )dnlsed 's/%@/@@/; s/@%/@@/; s/%g$/@g/; /@g$/s/[\\\\&%]/\\\\&/g;  s/@@/%@/; s/@@/@%/; s/@g$/%g/' > conftest.subs <<\CEOFchangequote([, ])dnldnl These here document variables are unquoted when configure runsdnl but quoted when config.status runs, so variables are expanded once.$ac_vpsubdnl Shell code in configure.in might set extrasub.$extrasubdnl Insert the sed substitutions of variables.undivert(AC_DIVERSION_SED)CEOFEOFcat >> $CONFIG_STATUS <<EOFCONFIG_FILES=\${CONFIG_FILES-"$1"}CONFIG_PRE=\${CONFIG_PRE-"$2"}CONFIG_POST=\${CONFIG_POST-"$3"}EOFcat >> $CONFIG_STATUS <<\EOFdnl Check for the existence of the pre and post files _FIRST_# test for pre and post files, to make sure they existif test -r "$ac_given_srcdir/$CONFIG_PRE"; then  ac_config_pre="$ac_given_srcdir/$CONFIG_PRE"  echo "Using $ac_config_pre for pre-makefile"else  ac_config_pre=""  echo "No file $CONFIG_PRE"fiif test -r "$ac_given_srcdir/$CONFIG_POST"; then  ac_config_post="$ac_given_srcdir/$CONFIG_POST"  echo "Using $ac_config_post for post-makefile"else  ac_config_post=""  echo "No file $CONFIG_POST"fifor ac_file in .. $CONFIG_FILES; do if test "x$ac_file" != x..; thendnl Specifying an input file breaks the trap to clean up on interrupt,dnl but that's not a huge problem.  # Support "outfile[:infile]", defaulting infile="outfile.in".  case "$ac_file" in  *:*) ac_file_in=`echo "$ac_file"|sed 's%.*:%%'`       ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;  *) ac_file_in="${ac_file}.in" ;;  esac  # Adjust relative srcdir, etc. for subdirectories.  # Remove last slash and all that follows it.  Not all systems have dirname.changequote(, )dnl  ac_dir=`echo $ac_file|sed 's%/[^/][^/]*$%%'`changequote([, ])dnl  if test "$ac_dir" != "$ac_file" && test "$ac_dir" != .; then    # The file is in a subdirectory.    test ! -d "$ac_dir" && mkdir "$ac_dir"    ac_dir_suffix="/`echo $ac_dir|sed 's%^\./%%'`"    # A "../" for each directory in $ac_dir_suffix.changequote(, )dnl    ac_dots=`echo $ac_dir_suffix|sed 's%/[^/]*%../%g'`changequote([, ])dnl  else    ac_dir_suffix= ac_dots=  fi  case "$ac_given_srcdir" in  .)  srcdir=.      if test -z "$ac_dots"; then top_srcdir=.      else top_srcdir=`echo $ac_dots|sed 's%/$%%'`; fi ;;  /*) srcdir="$ac_given_srcdir$ac_dir_suffix"      top_srcdir="$ac_given_srcdir" ;;  *) # Relative path.    srcdir="$ac_dots$ac_given_srcdir$ac_dir_suffix"    top_srcdir="$ac_dots$ac_given_srcdir" ;;  esacifdef([AC_PROVIDE_AC_PROG_INSTALL],[  case "$ac_given_INSTALL" inchangequote(, )dnl  [/$]*) INSTALL="$ac_given_INSTALL" ;;changequote([, ])dnl  *) INSTALL="$ac_dots$ac_given_INSTALL" ;;  esac])dnl  echo creating "$ac_file"  rm -f "$ac_file"  conf_in_file="`echo $ac_file_in|sed 's%.*/%%'`"  configure_input="Generated automatically from $conf_in_file by configure."  case "$ac_file" in  *Makefile*) ac_comsub="1i\\# $configure_input" ;;  *) ac_comsub= ;;  esac  sed -e "$ac_comsubs%@configure_input@%$configure_input%gs%@srcdir@%$srcdir%gs%@top_srcdir@%$top_srcdir%gs%@top_builddir@%$ac_dots.%gs%@dir_suffix@%$ac_dir_suffix%gifdef([AC_PROVIDE_AC_PROG_INSTALL], [s%@INSTALL@%$INSTALL%g])dnl" -f conftest.subs $ac_config_pre $ac_given_srcdir/$ac_file_in \		$ac_config_post > $ac_filefi; donerm -f conftest.subs])