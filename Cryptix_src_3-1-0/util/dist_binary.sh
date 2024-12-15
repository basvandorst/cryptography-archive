#!/usr/local/bin/bash
# I tried to make it /bin/sh but this failed - some machines have level 7 sh.

#
# Script for Binary Distribution.
#
# script version number (not release version number)
# $Release: $
#
# Version number of the *distribution* not this script, probably overwritten.
version=A-B-C

verbose=false
verbose_arg=""
v=false
vv=false
vvv=false
vvvv=false
set_on=false

out=""
readme_out=""
skip=0                                      # skip this many tests

args="$0 $*"                                # for re-invoking

dist=${BUILD_TMP:-/tmp/Cjb$$}

while [ $# -gt 0 ]
do
    opt="$1"
    case "${opt}" in
        "-version")    version="$2"; shift ;;
        "-dist")       dist="$2"; shift ;;
        "-out")        outfile="$2"; shift ;;
        "-readme")     readme_out="$2"; shift ;;
        "-skip")       skip="$2"; shift ;;

        "-restricted_o"*)     echo "${opt} is a no-op" ;;

        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true;  verbose_arg="-v" ;;
        "-vvv")        vvv=true; verbose_arg="-vv";  set_on=true ;;
        "-vvvv")       vvv=true; verbose_arg="-vvv"; set_on=true; set -x ;;

        "-?"|"-help")
            echo "Usage: util/dist_binary.sh [opts] -version 6-6-6   manifest"
            cat <<-!
		Make a Binary Distribution as specified by the manifest.
		The manifest is similar to the JAR type, without compilation,
                and with manifests being passed for compiling and JARing.
!
exit 0
;;
        *)             break ;;
    esac
    shift
done

if [ $# -eq 1 ] && [ -f "$1" ]
then
    man=$1
    case ${man} in                         # make it absolute
        /*) ;;
        *) man=$PWD/${man} ;;
    esac
    base_name=${man#*/}
    dir_name=${man%/*}

else
    echo "need a readable manifest: $1"
    echo $usage
    exit 2
fi

${vvv} && vv=true
${vv} && verbose=true

type=bin
big_name="Compiled Classes"
loc=${0%/*}
# echo $loc
source ${loc}/dist_common.sh

read_manifest ${man} || exit 1          # sets variables $m_*

top_files="${m_javas} ${m_others}"
jar_mans="${m_mans}"


#
#  Set up the support file names.
#
readme_name="README_${type}.html"
if [ -n "${outfile}" ]
then
    archive_zip=${outfile}
else
    archive=Cryptix_${type}_${version}
    archive_zip=${archive}.zip
fi
base_archive=${archive_zip##*/}            # name for the dox


# pwd=${PWD}
results="test_results"
classes=${dist}/classes
auto=${dist}/auto
mkdir ${dist} ${classes} ${dist}/util ${dist}/${results} ${auto} 2>&-
[ ! -d ${classes} ] && echo "Error: mkdir ${classes} failed: $?" && exit 1

dist_test_script=auto/testjar          # a relative name for docs
run_test_script=../${dist_test_script}.sh   # the name to run from ${classes}
run_test_batch=..\\auto\\testjar            # the name to run from ${classes}
test_script=${dist}/${dist_test_script}

#
#  Some finds cannot match dirnames.  Some greps can't do extended.  Some...
#
${verbose} && echo "  copy others ${top_files}"
find ${top_files} ! -type d -print |
	grep -v '/CVS/' |
	grep -v  '/.*\.o/' \
	> ${dist}/find 2>/dev/null

[ $? -ne 0 ] && echo "Error: find failed" && exit 1
cpio -pdm ${dist} < ${dist}/find 2> /dev/null



#######################################################
#
#   Make the JARs and collect the filenames.
#   Everything gets made into the ${classes} dir.
#

PATH="${dist}/util:$PATH"               # use the supplied scripts (testjar)
export PATH
utilpath=""                             # absolute, trailing /, for debugging
# buildjar="${SH} ../util/buildjar.sh"
buildjar="${utilpath}buildjar.sh"
testjar="${utilpath}testjar.sh"

${verbose} && echo "  build JARs ${jar_mans}"
cd src
failed=""
failures=0
goods=0
class_files=""
return_val=0                            # 4 is soft, else some other error
for man in ${jar_mans}
do
    bj_args="-d ${classes} ${dir_name}/${man}"
    test_arg="-test"
    if [ ${skip} -gt 0 ]
    then
	test_arg=""
	skip=$((skip - 1))
	${verbose} && echo "  skipping this test (${skip} more to skip)"
    fi
    ${vv} && echo && echo && echo "========== ${man} ========================"
    ${buildjar} \
	${test_arg} \
	-stdout ${dist}/${results}/ \
	-auto ${test_script} \
	${verbose_arg} ${bj_args}
    ret=$?
    mmm=${man%.man}
    case ${ret} in
    [04])
        if [ 0 -ne ${ret} ]           # 4 is a soft warning
        then
            warning="${warning} <LI> ${mmm}"
            return_val=4
            ${vv} && echo && echo
            echo "Error: ${mmm} failed some tests = = = = = = = = = ="
            ${vv} && echo 
        fi
        #
        #  It worked.
        #  Work out what files are provided, for adding in the zip archive.
        #  (with the -d flag, buildjar has already placed the files in $dist).
        #
        new_files="$( ${buildjar} -files ${bj_args} )"
        [ $? -ne 0 ] && echo "huh: ${new_files}" && exit 1
        class_files="${class_files} ${new_files}"
	${vv} && echo "      add: ${new_files}"
        goods=$((goods+1))
        ;;
    1)
        ${vv} && echo && echo
        echo "Error: ${mmm} failed completely ========================"
        ${vv} && echo 
        failed="${failed} ${man#*/}"
        failures=$((failures+1))
        ;;
    *)  # 2 is arguments errors, but do the lot here
        echo "Error: invocation errors ${ret} with ${buildjar}"
        echo "check PATH=$PATH"
        failed="buildjar not happy"
        failures=$((failures+1))
        break
        ;;
    esac
    ${set_on} || set +x

    if [ ${failures} -ge ${goods} ]  # sanity check, are we getting anywhere?
    then                             # especially, IJCE+SPT must be ok
        ${vv} && echo && echo && echo "#########################"
        echo "FAILED: ${failures} failed JARs, only ${goods} succeeded"
        exit 1
    fi
done

#
#   Need to finish the file.  This is a hack really,
#   to avoid adding more weight to the argument calling sequence
#   of buildjar.
#
${testjar} -finish -auto ${test_script} ${verbose_arg}

#
#  These are for repeatability/informational purposes.
#
# auto_list=$( ls ${dist}/util/auto* )
auto_list_top=$( cd ${dist}; echo auto/* )
results_top=$( cd ${dist}; echo test_results/* )
${vv}  && echo "  auto list: ${auto_list_top:-FAILED from: $(pwd) to ${dist}}"
# top_files="${top_files} ${auto_list_top}"

if [ -n "${warning}" ]
then
    x="These JARs failed some tests:"
    build_failures="${x} <UL> ${warning} </UL>
    See ${dist_test_script} for the individual failed tests."
    echo ; echo ; echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"; echo
    echo "  Warning: ${x} ${warning}"
    echo ; echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"; echo; echo
fi

if [ -n "${failed}" ]
then
    echo "Error: JARs failed and ditched: ${failed}"
    echo "       check test_results for specific results"
    exit 1
fi

if [ -z "${warning}" ]
then
    build_failures="No JARs failed their tests."
    ${verbose} && echo "  ${build_failures}"
fi


#
#  Dive in to the JARs and get out the one that has the Install.java
#  for the instructions.
#
spt_jar="${class_files#* SPT}"
# is this needed??? yes, as Install is in SPT
spt_jar="SPT${spt_jar%% *}"
$vv && echo "      Install is in ${spt_jar}"

#
#  Convert the filenames in class into name relative to the top
#
class_files_top=""
for ff in ${class_files}
do
    class_files_top="${class_files_top} classes/${ff}"
done

#
#  Sort out modes explicitly.  Slightly conservative - no group writing
#  because it is crypto.
#
$vvv && echo "      setting modes for Unix"
find ${dist} -type d -exec chmod 755 {} \;
find ${dist} -type f -exec chmod 644 {} \;
find ${dist} -name '*.sh' -print -exec chmod 755 {} \;



######################################################
#
#   Build the README.  We build it after having done all the
#   work, so that it can be modified with results.
#


#
#   For here on, we are inside the directory!!!
#
cd ${dist}
date=$(date)

cat > ${readme_name} <<README_HERE_DOC || exit 1

<!-- Created by: $0 ${date} -->
<head> <title>Cryptix ${version_dot} ${type} README</title> </head>

<body text="#000000" bgcolor="#FFFFFF" link="#0000FF" vlink="#000077" alink="#FF
0000">

<table border="0" width="100%"> <tr>

  <td>
    <img src="images/cryptix.gif" ALT=" Cryptix " border=0 >
    </td>

  <td>
    <a href="README.html"><font size=-1>README</font></a> or
      <a href="README_src.html"><font size=-1>Readme Src</font></a> or
      <b>Readme Bin</b><br>
    <a href="FAQ.html"><font size=-1>Frequently Asked Questions</font></a><br>
    <a href="doc/packages.html"><font size=-1>Javadoc index</font></a> or
      <a href="doc/index.html"><font size=-1>Help</font></a><br>
    <a href="guide/index.html"><font size=-1>Documentation</font></a><br>
    <a href="guide/security/JCESpec.html"><font size=-1>Java Cryptography Extension Specification</font></a><br>
    <a href="util/index.html"><font size=-1>Utility scripts</f></a><br>
    <a href="license/CryptixGeneral.html"><font size=-1>Copyright and License</f
ont></a><br>

    </td>

  <td align=right valign=top>
    <b><i><br>Cryptix<br><br>Development<br><br>Team</i></b>
    </td>

</tr> </table>

<hr size=3 noshade>

<center><b><font size=+4>
    Cryptix <sup><font size=1>TM</font></sup><br>
    ${big_name} ${version_dot}<br>
    README
</font></b></center>



${H1_intro}

<P>
Included in the ${big_name} distribution is:
<UL>
  <LI> the IJCE and providers packaged in JARs
  <LI> test programs within the JARs
  <LI> useful scripts to get it going
  <LI> this release readme, the main readme and the FAQ etc.
  <LI> doco in guide
</UL>

<H1><a name="install">Installation</a></H1>
To get the Cryptix ${big_name} distributions installed and tested,
follow the main <a href="README.html#install">README instructions</a>.

<P>
For this ${big_name} also do the following:

<OL>
  <LI> Do not unJAR the archives (*.jar) within the classes directory.
</OL>


<H1><a name="test">Testing</a></H1>
Once installed, Cryptix should be tested.
See the
<a href="guide/distribution/Testing.html">Binary Distribution Testing Guide</a>
for general ways to test.

<P>
This distribution was tested at make time, and
the actions were recorded into platform scripts
in the <code>auto</code> directory (see ${dist_test_script}*).

<P>
${build_failures}

<P>
In order to repeat the distribution tests, do for Unix:
<pre>
    ${run_test_script}
</pre>
or for NT/95, do:
<pre>
    ; Note, this batch is untested - and written by a non-DOS person :-)
    ${run_test_batch}
</pre>

<P>
If any of the above listed working tests fail, please send a bug report.
Note that the automatic release script produces a list of working
tests;  non-working tests are commented out.
The latter <i>should</i> not be present,
as non-working code is dropped from formal releases.



${H1_new}

${H1_not}

${H1_bugs}


<H1><a name="copyright">Copyright and License</a></H1>
All contents of this distribution are
Copyright &#169; 1995 - 1999 Systemics Ltd
on behalf of the Cryptix Development Team.
All rights reserved.

<P>
This release of Cryptix is covered by the
<a href="license/CryptixGeneral.html">
Cryptix General License
</a>
${limited_license}
.

${full_end_piece}
</body>
README_HERE_DOC

${vvv} && echo "      README is ${dist}/${readme}"



all_files="${auto_list_top} ${readme_name} ${top_files} ${class_files_top} ${results_top}"
${vvvv} && echo "files: ${all_files}"

if ls ${all_files} 1> /dev/null
then
    ${verbose} && echo "  all files found"
else
    ${vvvv} || echo "files: ${all_files}"
    echo "Error: files missing, binary phase terminated."
    exit 1
fi

rm -f ${archive_zip}

#
#  Need to Zip this up with a real Zipper.  Jar doesn't do it.
#  Some Unix machines don't have real Zips.  Life's a bitch and then...
#
zip -r9q ${archive_zip} ${all_files}
${verbose} && echo "  Binary Archive ${archive_zip}"

[ -n "${readme_out}" ] && cp ${readme_name} ${readme_out}

exit ${return_val}

