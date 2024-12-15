#!/usr/local/bin/bash
# I tried to make it /bin/sh but this failed - some machines have level 7 sh.

#
# Script for source release.  Note that this is CVS-centric.
#
# script version number (not release version number)
# $Release: $
#
# Version number of the *distribution* not this script.
#    2.4.0 was a test
#
version=A-B-C
# version_dot=A.B.C

verbose=false
verbose_arg=""
v=false
vv=false
vvv=false
vvvv=false
set_on=false

out=""
readme_out=""

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


        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true;  verbose_arg="-v" ;;
        "-vvv")        vvv=true; verbose_arg="-vv";  set_on=true ;;
        "-vvvv")       vvv=true; verbose_arg="-vvv"; set_on=true; set -x ;;

        "-?"|"-help")
            echo "Usage: util/binary.sh [opts] -version 6-6-6   manifest"
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



###########################################################
#
#   Get the files list of what makes up this distribution.
#
${verbose} && echo "Read Manifest."

failed=false
#
#  Read the manifest - passive, no actions.
#
while read line
do
    ${vvvv} && echo "     . . . . . . . . . . . . . . . . . ${line}"
    case "${line}" in
    *=*)                                  # variable=value
        var=${line%%=*}                   # break it manually to avoid quotes
        val=${line#*=}
        eval ${var}='"'${val}'"'
        ${vv} && eval echo '"   "' \$${val} \= \" \$${var} \"
        ;;

    -[fd]" "*)                            # test existance of a file or dir
        type=${line% *}
        nam=${line#-? }
        if eval "test ! ${type} ${nam}"
        then
            echo "not present: ${nam}"
            failed=true
        else
            ${vv} && echo "    found ${nam}"
        fi
        ;;

    "")                                   # ignore empty lines
        ;;
    "#"*)                                 # ignore comments
        ;;

    *)                                    # hopefully a filename
        case "${line}" in
        *.man)                            # in this case, a manifest to build
            jar_mans="${jar_mans} ${line}"
            ${vv} && echo "    man ${line}"
            ;;
        *)
            top_files="${top_files} ${line}"
            ${vv} && echo "    others ${line}"
            ;;
        esac
        ;;
    esac
    set +x
done < ${man}

if ${failed}
then
    echo "This source directory is not complete."
    exit 1
fi

#
#  Set up the support file names.
#
type=bin
readme="README_${type}_${version}.html"
if [ -n "${outfile}" ]
then
    archive_zip=${outfile}
else
    archive=Cryptix_${type}_${version}
    archive_zip=${archive}.zip
fi
base_archive=${archive_zip##*/}            # name for the dox


#
#   Turn 1-2-3 into 1.2.3 ... my kingdom for a string processing feature.
#
maj=${version%%-*}
min=${version##*-}
mid=${version#*-}
mid=${mid%-*}
version_dot=${maj}.${mid}.${min}
${verbose} && echo "  version ${version_dot}"

# pwd=${PWD}
results="test_results"
classes=${dist}/classes
mkdir ${dist} ${classes} ${dist}/util ${dist}/${results} 2>&-
[ ! -d ${classes} ] && echo "Error: mkdir ${classes} failed: $?" && exit 1

dist_test_script=util/auto_testjar          # a relative name for docs
run_test_script=../${dist_test_script}.sh   # the name to run from ${classes}
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
utilpath=""                             # absolute, trailing /
# buildjar="${SH} ../util/buildjar.sh"
buildjar="${utilpath}buildjar.sh"

${verbose} && echo "  build JARs ${jar_mans}"
cd src
failed=""
class_files=""
return_val=0                            # 4 is soft, else some other error
for man in ${jar_mans}
do
    bj_args="-d ${classes} ${dir_name}/${man}"
    ${vv} && echo && echo && echo "========== ${man} ========================"
    ${buildjar} \
	-test -output ${dist}/${results}/ \
	-save ${test_script} \
	${verbose_arg} ${bj_args}
    ret=$?
    case ${ret} in
    [04])
        if [ 0 -ne ${ret} ]           # 4 is a soft warning
        then
            mmm=${man%.man}
            warning="${warning} <LI> ${mmm}"
            return_val=4
            ${vv} && echo && echo
            echo "Error: ${mmm} failed it's tests - - - - - - - - - -"
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
        ;;
    1)
        failed="${failed} ${man#*/}"
        ;;
    *)  # 2 is arguments errors, but do the lot here
        echo "Error: invocation errors ${ret} with ${buildjar}"
        echo "check PATH=$PATH"
        failed="buildjar not happy"
        break
        ;;
    esac
    ${set_on} || set +x
done

#
#  These are for informational purposes.
#
# auto_list=$( ls ${dist}/util/auto* )
auto_list_top=$( cd ${dist}; ls util/auto* )
${vv}  && echo "  auto list: ${auto_list_top:-FAILED from here: $(pwd)}"
top_files="${top_files} ${auto_list_top}"

if [ -n "${failed}" ]
then
    echo "Error: JARs failed and ditched: ${failed}"
    echo "       check ${auto_list}* for specific results"
    exit 1
fi
if [ -n "${warning}" ]
then
    build_failures="These JARs failed their tests: <UL> ${warning} </UL>
    See ${dist_test_script}_err for the individual failed tests."
    echo ; echo ; echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
    echo "  Warning: ${build_failures}"
else
    build_failures="No JARs failed their tests."
    ${verbose} && echo "  ${build_failures}"
fi


#
#  Dive in to the JARs and get out the one that has the Install.java
#  for the instructions.
#
spt_jar="${class_files#* SPT}"
# IS THIS NEEDED???
spt_jar="SPT${spt_jar%% *}"
$verbose && echo "  spt is in ${spt_jar}"

#
#  Convert the filenames in class into name relative to the top
#
class_files_top=""
for ff in ${class_files}
do
    class_files_top="${class_files_top} classes/${ff}"
done

# Already there
# ${verbose} && echo "Copy the tree."
# #
# #  Copy the tree.  Ignore (remake) directories, CVS contents, *.o directories.
# #
# find ${files} \
#       ! -type  d ! -path '*/CVS*' ! -path '*.o/*' > ${dist}find
# [ $? -ne 0 ] && echo "Error: find failed" && exit 1
# ${vv} && ( cd $dist/class ;  ls -l ${files} )
# cpio -pdm ${dist}/class < ${dist}find 2> /dev/null

#
#  Sort out modes explicitly.  Slightly conservative - no group writing
#  because it is crypto.
#
find ${dist} -type d -exec chmod 755 {} \;
find ${dist} -type f -exec chmod 644 {} \;
find ${dist} -name '*.sh' -print -exec chmod 755 {} \;



######################################################
#
#   Build the README
#
#   This README file cannot be called README - it will clash
#
index="index.html"

home_made_files="${readme} ${index}"


#
#   For here on, we are inside the directory!!!
#
cd ${dist}
date=$(date)

cat > ${readme} <<README_HERE_DOC || exit 1

<head> <title>Cryptix ${version_dot} ${type} README</title> </head> <body>

<H1>Introduction</H1>

This is an ALPHA pre-release of the Cryptix ${type} distribution

<H2>Inclusions</H2>

This version of Cryptix includes
<UL>
  <LI> the IJCE and providers packaged in JARs
  <LI> test programs within the JARs
  <LI> useful scripts to get it going
  <LI> this readme, etc.
</UL>

<H2>Structure</H2>

<UL>
  <LI> classes - the location of classes and JARs
     <UL>
       <LI>*.jar - the JAR archives
       <LI>cryptix-lib/Cryptix.properties
       <LI>ijce-lib/IJCE.properties
     </UL>

  </LI> <P> <LI> util - scripts to assist

  </LI> <P> <LI> license - the license(s) applicable to Cryptix.

</UL>

<H2>Version</H2>

This release is version ${version_dot} made on ${date}.
Status is ALPHA, about to go BETA.
(What does ALPHA mean?  I call it internal, others say "will change.")
This is a pre-release for internal testing.
(Version ${version_dot} for bug reports only).

<H1>Installation and Test</H1>
How to get the Cryptix binary distributions installed and tested.

<OL> <LI>
  Download and unzip the ${base_archive}
  but do not unJAR the archives (*.jar) within the classes directory.

</LI> <P> <LI>
  Now point your browser into the directory at this
  <a href="${readme}">
  ${readme}
  </a>
  file.

</LI> <P> <LI>
One time only, the Cryptix  provider must be installed as a
valid alternative to the Sun provider.  This change should survive
work for all future versions as well, and can be left in place.

To do this, run this command (from within the classes directory):
<pre>
    CLASSPATH=${spt_jar} java cryptix.provider.install
</pre>

which will add a line like this:

<pre>
    #
    # Cryptix is the secondary provider.
    #
    security.provider.2=cryptix.provider.Cryptix
</pre>

<P>
at the end of the file <code>lib/security/java.security</code>
in your JDK.  Otherwise Java will not be able to find the
additional provider and complain that algorithms are not available:

<pre>
    java.security.NoSuchProviderException: provider Cryptix is not available.
</pre>

<P>
The added line says the 2nd priority provider will be based on
the class mentioned.  (The 1st priority one can be left as
the Sun one).

<P>
Notes that on Unix systems, you may want to do the above addition by
hand.  Consult the administrator of your JDK setup, as it
may not be a good idea to run Java under root.

</LI> <P> <LI>
Then run the tests on the JARS.  These are listed at distribution
make time into ${dist_test_script}*
JARs.  For example, within the classes directory, run:

<pre>
    ${run_test_script}
</pre>

<P>
If any of the above listed working tests fail, please send a bug report.
Note that the automatic release script produces a list of both working
tests and non-working tests.
</OL>

<H1>Documentation</H1>

Javadoc documentation is found in the
<a href="doc/packages.html">
doc packages
</a>
directory, if supplied.
<P>
A fast index is located at
<a href="index.html">
index.html
</a>
within the main directory.

<H1>Build Failures</H1>

${build_failures}

<H1>Bug Reports</H1>
Send bug reports to
<a href="mailto:cryptix-java@systemics.com?subject=Bug Report ${version} ${type}">
cryptix-java@systemics.com
</a>
<P>
Please include the version number, the JDK version, the platform
and a description of the problem (including exceptions, output).

<P>
Note that if you are not subscribed, you should do so.
<b>But</b>,
bugs will caught by the list manager and sent to an appropriate Cryptix
Development Team member.


<H1>Copyright and License</H1>
All contents of this distribution are
Copyright &#169; 1995, 1996, 1997 Systemics Ltd
on behalf of the Cryptix Development Team.
All rights reserved.

<P>
This release of Cryptix is covered by the
<a href="license/CryptixGeneral.html">
Cryptix General License
</a>
${limited_license}
.

<HR>
</body>
README_HERE_DOC




cat > ${index} <<INDEX_HERE_DOC

<head> <title>Cryptix ${version_dot} ${type} Index</title> </head> <body>

<H1>Cryptix ${version_dot} Index</H1>

This is an ALPHA pre-release of
Cryptix ${version_dot}
made on ${date}.

<UL>

<LI>
  The
  <a href="${readme}">
  ${readme}
  </a>
  covers this product in more depth.

</LI> <P> <LI>

Javadoc documentation is found in the
<a href="doc/packages.html">
doc packages
</a>
directory.

</LI> <P> <LI>

Send bug reports to
<a href="mailto:cryptix-java@systemics.com?subject=Bug Report ${version} ${type}">
cryptix-java@systemics.com
</a>.
Please include the version number, the JDK version, the platform
and a description of the problem (including exceptions, output).

</LI> <P> <LI>

All contents of this distribution are
Copyright &#169; 1995, 1996, 1997 Systemics Ltd
on behalf of the Cryptix Development Team.
All rights reserved.

</LI> <P> <LI>
This release of Cryptix is covered by the
<a href="license/CryptixGeneral.html">
Cryptix General License
</a>
${limited_license}
.

</UL>
<HR>
</body>
INDEX_HERE_DOC



all_files="${readme} ${index} ${auto_list_top} ${top_files} ${class_files_top} ${results}/*"
${vv} && echo "files: ${all_files}"

if ls ${all_files} 1> /dev/null
then
    echo all files found
else
    exit 1
fi

rm -f ${archive_zip}

#
#  Need to Zip this up with a real Zipper.  Jar doesn't do it.
#  Some Unix machines don't have real Zips.  Life's a bitch and then...
#
zip -r9q ${archive_zip} ${all_files}
${verbose} && echo "  Binary Archive ${archive_zip}"

[ -n "${readme_out}" ] && cp ${readme} ${readme_out}

exit ${return_val}

