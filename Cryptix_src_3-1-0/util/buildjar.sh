#!/usr/local/bin/bash

#
#  The JAR version numbers are set in the manifest files.
#  These are some dummy ones just in case the caller doesn't set them.
#
version=X-Y-Z
name=TEST

license="CryptixGeneral.html"               # everybody gets the license
failed=false

opts="-compile|-others|files|-test|-v[v]|-verbose"
usage="Usage: buildjar [$opts] [-name ArchiveName] man_name"
#
#  Arguments.
#
compile=true
list_others=false
list_files=false
test_jar=false
clean=false
fast=true
auto_arg=""
verbose=false 
vv=false                                     # very verbose
vvv=false                                    # very very verbose
vvvv=false                                   # spill my guts
set_on=false
verbose_arg=""                               # pass on one less v

tgt_class_dir=dist.o

while [ -n "$1" ]
do
    opt="$1"
    optarg="$2"
    case "${opt}" in
        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true; verbose_arg=${opt%v} ;;
        "-vvv")        vvv=true; verbose_arg=${opt%v} ;;
        "-vvvv")       vvvv=true; verbose_arg=${opt%v}; set_on=true; set -x ;;

        "-compile")    compile=false ;;
        "-others")     list_others=true ;;
        "-files")      list_files=true ;;
        "-test")       test_jar=true ;;
        "-fast")       fast=false ;;

        "-name")       name="${optarg}" ; shift ;;
        "-version")    version="${optarg}" ; shift ;;
        "-o")          o="${optarg}" ; shift ;;
        "-d")          tgt_class_dir="${optarg}" ; shift ;;
        "-auto")       auto_arg="-auto ${optarg}"; shift ;;
        "-stdout")     stdout_arg="-stdout ${optarg}"; shift ;;

        "-?"|"-help")
            echo $usage
            cat <<-!
            Opts:    -compile   skip the compile phase
                     -files     print out all the files of this JAR
                     -others    print out the other (non JAR) files needed
                     -clean     start from scratch, don't skip if JAR is good
                     -test      invoke testjar on the JAR
                     -auto f    ask testjar to save its actions in a script
                                This is root filename.  Appends always.
                     -stdout f  ask testjar to save its test results in files
                                This is root filename.  Each test gets own file.
                     -d dir     where to compile the binaries into (absolute)
                     -o dir     where to put the JAR (absolute)
                     -name name an alternative name
		     -version v set a version to override the manifest/default

            Makes a JAR from a manifest file.  The Manifest is a text file:
                 - variables set using shell eval format name=value ...
                     + these should be set: name,  version, version_dot
                     + optional: license
                     + no quotes, var is set to line as written
                 - comments started by "*" (no whitespace before)
                 - files to compile/include in zip - we compile as we go
                     + these files can be more than one per line
                     + lines with java are all java (no non-java)
                     + all java files on one line are compiled together
                     + shell wildcard is understood, e.g. cryptix/test/*.java
                     + ${start}list${end} is expanded for all names in list
                       where list is defined as above: list=DES IDEA...
                 - dependencies can exist as files in the distribution dir
                     + directories are a line: -d dirname
                     + files on the format -f filename
                     + one per line, only one space between -f and arg
            See the files in util/man for example manifests
!
;; # annoying these here docs are....
        -*)       echo "unknown arg: ${opt}"; echo ${usage}; exit 2; break ;;
        *)        break ;;
    esac
    shift                                # remove next arg, sometimes not $1
done

##  ${list_files} || echo this is 3 >&3
##  ${list_files} || echo this is 2 >&2
##  ${list_files} || echo this is 1 >&1

#
#  Put normal 1 onto stderr, so echo is a diag.
#  Save 1 first for later use if needed.
#
exec 3>&1 1>&2

##  ${list_files} || echo this is 3 again >&3
##  ${list_files} || echo this is 2 again >&2
##  ${list_files} || echo this is 1 again >&1

if [ $# -eq 1 ] && [ -f "$1" ]
then
    man=$1
    case ${man} in                         # make it absolute
        /*) ;;
        *) man=$PWD/${man} ;;
    esac
    base_man=${man##*/}

else
    echo "need a readable manifest: $1"
    echo $usage
    exit 2
fi

if ${list_others} || ${list_files}
then
#    verbose=false; v=false; vv=false; vvv=false
    compile=false
fi

${vvvv} && vvv=true
${vvv} &&  vv=true
${vv} &&   verbose=true

${vvv} && echo && echo "= = = = = = = = = = = = = = = = = = = ="
${vvv} && echo "           Build JAR from ${man}"

tmp=${BUILD_TMP:-/tmp/Cj$$}
tmp1=${tmp}bjReadme$$
tmp2=${tmp}bjErr$$

type=JAR
big_name="Java Archive"
loc=${0%/*}  
# echo $loc
source ${loc}/dist_common.sh

# leaves results in $m_*
read_manifest ${man} ${tmp1} || exit 1

#
#  Looks like I need to know how to compile.  Guavac does things
#  sufficiently differently that a script to make it look like
#  javac doesn't fly.
#
#  For now, make the classes in place then copy them,
#  as this is simpler for compilers (default) and some
#  compilers differ in how they handle remote destinations.
#
local_class_dir=.
#
#  Guavac has these problems:
#    * not the same paramater effects as javac: -classpath, -d, CLASSPATH
#    * cannot handle multiple javas
#    * needs correct sequence
#    * core dumps on assert for some special cases
#             (core dump in proc fs crashes)
#    * does core dump if name is not long enough ...
#
guavac()
{
    gc=/usr/local/apps/java/guavac-1.0/compiler/guavac
    for i in $*
    do
	${vv} && echo "        ${gc} -d . ${i}"
        ${gc} -d . $i
    done
}

jc=javac
if [ ${jc} = "javac" ]
then
    export CLASSPATH=.
    JC="javac"  # " -d ${local_class_dir}"

elif [ ${jc} = "guavac" ]
then
    JC="guavac" 
fi

[ -d src/cryptix/provider ] && cd src
[ ! -d cryptix/provider ] && echo "run in the top src directory" && exit 1
[ ! -d ${tgt_class_dir} ] && mkdir ${tgt_class_dir} 2> /dev/null
[ ! -d ${tgt_class_dir} ] && echo "Error: mkdir fail: ${tgt_class_dir}" && exit 1


for jc in ${m_javas}           # convert source name to bin name
do
    base="${jc%.java}"
    class_files="$class_files ${base}.class"
    if [ ! -f "${jc}" ]
    then
      echo "no such file: ${jc}"
      if ${compile}
      then
          do_compile=false
          failed=true
          echo "skipping compilation"
      fi
    fi
done

# Fast hack: add aditional class files
class_files="$class_files $m_class"

if ${do_compile}
then
    ${verbose} && echo "    ${JC} ${m_javas%%.java *}.java ..."
    ${vvv} &&     echo "     ${m_javas#*.java }"
    ${JC} ${m_javas}
fi

#
#  Name and version are set in the manifest, now we can build the file names.
#
archive=${name}_${version}
${vv} && echo "      archive ${archive}"
outfile=${archive}.jar                # this is relative to the class directory

if ${list_others}                     # just print out what files are required
then                                  # used by scripts to copy, etc
    echo ${m_others} 1>&3
    exit 0
elif ${list_files}
then
    echo ${m_others} ${outfile} 1>&3
    exit 0
fi

if ${fast} && [ -f "${outfile}" ] && [ -x "${outfile}" ] # not currently used
then
    echo "        Fast mode: ${archive} is already ok"
fi

rm -f ${outfile}

if [ -z "${class_files}" ]             # may be reasonable, wait and see
then
    echo "no class files found?"
    exit 1
fi

if ${failed}
then
    echo "failed."
    exit 1
fi



${verbose} && echo "    copy classes and others..."
if [ -n "${m_others}" -o -n "${class_files}" ]
then
    #
    #  Copy class files across, and also expand class files into inners.
    #
    if [ -n "$class_files}" ]
    then
	dollar='$'
	# pattern=""
	ff=""
	inners=""
	for cl in ${class_files}
	do
	    base=${cl%.class}
	    pattern="${base}${dollar}*.class"
	    ff=$(echo ${pattern})         # echo is internal, should be fast,
	    case "${ff}" in               # but needs to be checked for null
	      *'*.class')  ;;   # not expanded, files not found
	      *)           inners="${inners} ${ff}" ;;
	    esac
	done

	[ -n "${inners}" ] && echo "  found inners: ${inners}"
    fi
    ${vvv} && echo "       others:  ${m_others}"
    ${vvv} && echo "       classes: ${class_files}"
    ${verbose} && echo "       inners: ${inners}"
    find ${m_others} ${class_files} ${inners} -print | cpio -pdm ${tgt_class_dir} 2>${tmp2}
    ret=$?
    if [ ${ret} -ne 0 ]
    then
	echo "Error: find/cpio failed: ${ret} pwd=$PWD"
	failed=true

	exit 1
    fi
    # rm ${classes} # leave cleanup to others, might effect chained buildjars
fi


#
#  Construct a text message.  Choose a java comment to show it as a comment.
#
date=`date`
${verbose} && echo "    finish support files..."

#
#  Collect the extra files and make the archive.
#
readme="README_${archive}_ARCHIVE"
jar_license="LICENSE_${name}.html"
# license_dump="lynx -dump"
license_dump="cat"
${license_dump} ../license/${license} > ${tgt_class_dir}/${jar_license}


##############################################################
#
#   From here on, we are in the class dir.
#
cd ${tgt_class_dir}
# cp ${tmp1} ${readme}
cp ${man} ${base_man}

cat > ${readme} <<END_HEADER
/**
 * Copyright (C) 1995 - 1999 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 *
 * Cryptix Classes Zip archive built by $0 on ${date}
 *
$(cat ${tmp1})
 *
 * Version ${version_dot}
 * DO NOT unzip or unjar this file, put it in the CLASSPATH instead.
 * Bug reports to bugs@cryptix.org
 */
END_HEADER


${vv} && echo "      check files..."
files="${readme} ${jar_license} ${base_man} ${class_files} ${m_others} "
if ls ${files} 1> /dev/null
then
    ${verbose} && echo "    all files found"
else
    echo "Files are missing.  Terminated."
    exit 1
fi


# ????
if ${failed}
then
    echo "failed."
    exit 1
fi
${vv} && echo "      make archive into ${outfile}"
${vvv} && jarv=v
# hmmm, warning about classpath.  Odd...
# add David's kludge CLASSPATH=dummy - no, that's for zip behaviour
jar -c0${jarv}f ${outfile} ${files}
chmod -x ${outfile}                # cheeky: leave a bit flag saying "not good"
                                   # except: not used currently

rm ${class_files}                  # so as to force proper testing later on
rm ${readme} ${jar_license} ${base_man} # cleanliness, etc
# rm ${m_others}                    # not yet as they are used later?
${vv} && echo "      $(ls -l ${outfile})"

[ ! -z "${tests_exit_code}" ] && expand_with_exit_code="-okexit ${tests_exit_code}"

#
#   Each archive should have some tests in it.  Run the tests here.
#   The tests are duplicable, so no hints given here.
#
if ${test_jar}
then
    #  arg0=$0
    #  testjar=${arg0%buildjar.sh}testjar.sh
    ${verbose} && echo "      testing..."
    ${verbose} && [ ! -z "${tests_exit_code}" ] && echo "      (tests should exit with ${tests_exit_code})"
    testjar=testjar.sh            # let PATH sort it out

    ${testjar} ${stdout_arg} ${auto_arg} ${verbose_arg} ${expand_with_exit_code} ${outfile} 1>&3
    case $? in
    0)    ${vv} && echo "       good test"; chmod u+x ${outfile} ;;   # is good
    3)    ${vv} && echo "       (no tests)"; chmod u+x ${outfile} ;;  # is good

    4)    echo "Error: ${testjar} test failed - debug please"; exit 4 ;; # soft

    1)    echo "Error: ${testjar} ALL tests failed";           exit 1 ;;
    2)    echo "Error: ${testjar} arguments - check version";  exit 1 ;;
    -1)   echo "Error: ${testjar} blew up at end ?????";       exit 1 ;; # why?
    127)  echo "Error: ${testjar} invocation - PATH=$PATH";    exit 1 ;;
    *)    echo "Error: ${testjar} unknown - $?";               exit 1 ;;
    esac

fi

if [ -n "${o}" ]
then
    mv ${outfile} $o
fi

exit 0
