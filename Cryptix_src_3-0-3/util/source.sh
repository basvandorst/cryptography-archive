#!/usr/local/bin/bash
# I tried to make it /bin/sh but this failed - some machines have level 7 sh.

#
# Script for source release.  Note that this is CVS-centric.
#
# script version number (not release version number)
# $Release: $
#
# Version number of the *distribution* not this script.
version=2-4-4

include_rpk=true

export=false
update=true
verbose=false
clean=false
v=false
vv=false
vvv=false
vvvv=false
set_on=false

javadoc_it=true
doc_dir=doc

args="$0 $*"                                # for re-invoking


while [ $# -gt 0 ]
do
    opt="$1"
    case "${opt}" in
        "-update")     update=false ;;
        "-javadoc")    javadoc_it=true ;;
        "-export")     export=true ;;
        "-clean")       clean=true ;;

        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true;   verbose_arg="${opt}" ;;
        "-vvv")        vvv=true;  verbose_arg="${opt}" ;;
        "-vvvv")       vvvv=true; verbose_arg="${opt}"; set_on=true; set -x ;;
        "-?"|"-help")
            echo "To make a source distribution, create a fresh CVS tree with"
            echo "tag that you select:"
            echo "      cvs get -r Cryptix_1-2-3"
            echo
            echo "then run this program:"
            echo "      cd current"
            echo "      sh util/source.sh"
            exit 0
            ;;
        *)             echo "unknown arg: ${opt}"; echo  ${usage}; exit 2 ;;
    esac
    shift
done

${vvvv} && vvv=true
${vvv}  && vv=true
${vv}   && verbose=true

type=src
name=Cryptix
src_archive=${name}_${type}_${version}
doc_archive=${name}_doc_${version}
bin_archive=${name}_bin_${version}
src_zip=${src_archive}.zip
doc_zip=${doc_archive}.zip
bin_zip=${bin_archive}.zip
download_index=index_${version}.html

#
#   Turn 1-2-3 into 1.2.3 ... my kingdom for a string processing feature.
#
maj=${version%%-*}
min=${version##*-}
mid=${version#*-}
mid=${mid%-*}
version_dot=${maj}.${mid}.${min}
${verbose} && echo "Version ${version_dot} $(date)"

## #
## #  Has the Perversion struck?  Check type of OS for (in)compatibility features.
## #
## perversion=`uname -s`                  # the original Perversion was GNU...
## case "${perversion}" in
##   Linux)    fullname="-path" ;;
##   FreeBSD)  fullname="-path" ;;
##   SunOS)    fullname="-name" ;;
## esac

if ${clean}
then
    #
    #  If starting from scratch, make a fresh directory for working in.
    #
    dist=/tmp/Cjs$$
    BUILD_TMP=${dist}; export BUILD_TMP
    mkdir ${dist}
else
    #
    #  For normal working, assume that there are bugs down the line.
    #  This means implementing make-like tests to see what has changed.
    #
    dist=/tmp/Cjs$$
    BUILD_TMP=${dist}; export BUILD_TMP
    mkdir ${dist}
fi

#
#  Set up some names.
#
cvs_top=${PWD}
PATH="${cvs_top}/util:$PATH"; export PATH
src_readme="README-${type}-${version}.html"
index="index.html"
static_list="license src util"
all_names="${src_readme} index.html ${static_list}"

#
#  Do a CVS update, to bring the tree into line and make sure
#  that the tree is clean.  It would be easier to do a cvs get,
#  but the repository is so slow that a complete get takes an hour.
#  Also note that this script is probably better off left outside the
#  tree so that it can be worked on whilst making the distribution.
#
if ${update}
then
    updates=0
    dead=false
    Im_changed=false
    
    ${verbose} && echo "Doing CVS update.  This must be clean (-update to skip)"
    cvs update ${static_list} > ${dist}upd 2>&1
    while read line
    do
        case "${line}" in
        "? "*.swp)    echo "Warning: editing: ${line}" ;;
        "M "*)        echo "Warning: locally modified: ${line}" ;;
        "A "*)        echo "Warning: need to commit: ${line}" ;;

        [?C]" "*)
            echo "Error: ${line}"
            dead=true
            ;;
    
        [UP]" "*)
            updates=$(($updates + 1))
            ${vv} && echo "    ${line}"
            case "${line}" in
                [UP]" "*/source.sh)
                    Im_changed=true
                    echo "Source.sh has been updated as well."
                    ;;
            esac
            ;;
    
        *"server: Updating"*) ;;              # server: Updating cryptix/...
    #        dirs="${dirs} ${line%%*Updating }"    # grab dir name
    #        ;;
    
        #
        #  These are warnings from CVS that we don't care about.
        #
        "retrieving rev"*) ;;
        "Merging diff"*) ;;
        "RCS file:"*) ;;
        "Warning: Remote host denied X11"*) ;;
    
        *)
            echo "oops, don't understand: $line"
            dead=true
            ;;
        esac
    done < ${dist}upd
    
    if ${dead}
    then
        echo "Update failed. Please clean up.  Full cvs output in ${dist}upd"
        exit 1
    fi

    if [ ${updates} -ne 0 ]
    then
        clean=true
    fi
    if ${Im_changed}
    then
        echo "Re-invoking without cvs update ..."
        exec $args -update
    fi
fi


#
#  Copy the tree.  Ignore (remake) directories, CVS contents, *.o directories.
#
${verbose} && echo "Copy the source tree. $(date)"
${verbose} && echo "  (chmoding util/*.sh)"
chmod 755 util/*.sh                     # try and avoid ${SH}

#
#  Ug.
#  Some finds cannot match dirnames.  Some greps can't do extended.  Some...
#
copy_list="${static_list}"
find ${copy_list} ! -type d -print |
	grep -v '/CVS/' |
	grep -v  '/.*\.o/' \
	> ${dist}/find

[ $? -ne 0 ] && echo "Error: find failed" && exit 1
cpio -pdm ${dist} < ${dist}/find 2> /dev/null



#
#  For here on, we are inside the directory!!!
#
cd ${dist}

${verbose} && echo "Make Support Files."
date=$(date)
rpk_license=""

if ${include_rpk}
then
    rpk_license="except for the RPK provider, which is covered by the
<a href=\"license/CryptixLimited.html\">Cryptix Limited License</a>"
else
    rm -r src/cryptix/provider/rpk
    rm "license/CryptixLimited.html"
fi

cat > ${src_readme} <<README_HERE_DOC || exit 1 

<head> <title>Cryptix ${version_dot} ${type} README</title> </head> <body>

<H1>Introduction</H1>

This is an ALPHA pre-release of
Cryptix ${type} ${version_dot}
made on ${date}.
This version of Cryptix includes the IJCE, providers and tests.


<H1>Version</H1>

This is a pre-release for internal testing.
(Version ${version_dot} for bug reports only).

<H1>Installation and Test</H1>
How to get Cryptix installed and tested.

<OL> <LI>
  Download and unzip the ${src_zip}

</LI> <P> <LI>
  Now point your browser into the directory at this
  <a href="${src_readme}">
  ${src_readme}
  </a>.

</LI> <P> <LI>
  To compile, there is a smorgasboard of scripts in
  <a href="util/index.html">
  the util directory
  </a>.

</LI> <P> <LI>
One time only, the Cryptix  provider must be installed as a
valid alternative to the Sun provider.  This change should survive
work for all future versions as well, and can be left in place.

To do this, run this command (from within the class directory):
<pre> 
    CLASSPATH=. java cryptix.provider.install
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
Then run:

<pre>
  CLASSPATH=. java cryptix.test.TestLOKI91
</pre>

<P>
from within the src directory, or wherever you have compiled
the classes into.

<P>
On some systems, the full path name might be required.

</LI> <P> <LI>
If that works, check out other tests in the cryptix.test.Test* set.

<P>
If any of the tests fail, please send a bug report: in theory, only
working tests are included, and we'd like to keep it that way ;-)
</OL>


<H1>What's here</H1>
<UL>
  <LI>
    A fast index is located at
    <a href="index.html">
    index.html
    </a>
    within the main directory.  Point your browser at it.

  </LI> <P> <LI>
    Some example scripts for managing the distribution are included in
    <a href=file:"util/index.html">
    util
    </a>

  </LI> <P> <LI>
    Javadoc documentation (when compiled) is found in the
    <a href="doc/packages.html">
    doc packages
    </a>
    directory.

</UL>

<H1>What's *not* here</H1>

These things are missing:

<UL>
  <LI>
    IJCE:
      <UL>
        <LI>test devoted to IJCE that can run independant of Cryptix
            (java.security.IJCE_test or some Sun standard name?)
      </UL>

  </LI> <P> <LI>
    ciphers:
      <UL>
        <LI>SPEED
        <LI>DES
        <LI>DES-EDE3
      </UL>

  </LI> <P> <LI>
     message digests:
      <UL>
        <LI>SHA1
        <LI>SHA0
        <LI>MD5
      </UL>

  </LI> <P> <LI>
     PK:
      <UL>
        <LI>ElGamal
      </UL>

</UL>

<H1>Bug Reports</H1>
Send bug reports to
<a href="mailto:cryptix-java@systemics.com">
cryptix-java@systemics.com
</a>
<P>
Please include the version number, the JDK version, the platform
and a description of the problem (including exceptions, output).

<P>
Note that if you are not subscribed, you should.  <b>But</b>, bugs
will caught by the list manager and sent to an appropriate Cryptix
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
${rpk_license}
.

<HR>
</body>
README_HERE_DOC




cat > ${index} <<INDEX_HERE_DOC || exit 1

<head> <title>Cryptix ${version_dot} Classes Index</title> </head> <body>

<H1>Cryptix ${version_dot} Index</H1>

This is an ALPHA pre-release of
Cryptix ${version_dot}
made on ${date}.

<UL>

<LI>
  The
  <a href="${src_readme}">
  README
  </a>
  covers this product in more depth.

</LI> <P> <LI>

  Javadoc documentation is found in the
  <a href="doc/packages.html">
  doc packages
  </a>
  directory (when made).

</LI> <P> <LI>

  <a href="util/index.html">Scripts</a>
  will help with compilation and usage.

</LI> <P> <LI>

  Send bug reports to
  <a href="mailto:cryptix-java@systemics.com">
  cryptix-java@systemics.com
  </a>.
  Please include the version number, the JDK version, the platform
  and a description of the problem (including exceptions, output).

</LI> <P> <LI>
  This release of Cryptix is covered by the
  <a href="license/CryptixGeneral.html">
  Cryptix General License
  </a>
  ${rpk_license}
  .

</LI> <P> <LI>

  All contents of this distribution are
  Copyright &#169; 1995, 1996, 1997 Systemics Ltd
  on behalf of the Cryptix Development Team.
  All rights reserved.

</UL>
</body>
INDEX_HERE_DOC



${verbose} && echo "Make the Archive."
rm -f ${src_zip}

#
#  Need to Zip this up with a real Zipper.  Jar doesn't do it.
#  Some Unix machines don't have real Zips.  Life's a bitch and then...
#  On Iang's Sunlet this is done via an ssh shell script.
#
zip -r9q ${src_zip} ${all_names}
if [ $? -ne 0 ]
then
    echo "Zip failed on archive ${src_zip}"
    echo "${all_names}"
    exit 1
fi

scp_list="${dist}/${src_zip} ${dist}/${src_readme}"


src_test=${dist}/src_test
bin_test=${dist}/bin_test
bin_make=${dist}/bin_make
mkdir ${src_test} ${bin_test} ${bin_make}

#
#  OK, we have a new distribution of the source.
#  Let's make the binary distributions from that, and test them.
#  If that works, we have a consistent release, and we can build
#  a binary release as well.
#
${verbose} && echo "Source Distribution Made.  Extract. $(date)"
cd ${src_test}
qarg=-qq
${vvvv} && qarg=""
unzip ${qarg} ${dist}/${src_zip}

##########################################################
#
#   Javadoc It!
#
limited_arg="-limited CryptixLimited"
if ${javadoc_it}
then
    #
    #  Javadoc capability is distributed with source for
    #  customisation.  It should work, so do it with the src dist.
    #
    ${verbose} && echo "Make a JavaDoc Distribution. $(date)"
    doc_zip_file=${src_test}/${doc_zip}
    src_test_doc=${src_test}/doc
    doc_readme_name=README_doc.html
    doc_readme_file=${tmp}/${doc_readme_name}
    
    jdoc="${SH} dist_javadoc.sh"
    ${jdoc} ${verbose_arg} \
	${limited_arg} \
    	-d ${src_test_doc} \
    	-src ${src_test}/src \
	-out ${doc_zip_file} \
	-readme ${doc_readme_file} \
	-version ${version} 
    ret=$?
    case $ret in
      0)    ${verbose} && echo "JavaDoc Complete $(date)";;
      1)    echo "JavaDoc Archive is bad.";  exit 1 ;;
      2)    echo "Arguments error?"; exit 1 ;;
      4)    echo "warning: Some failed docs, carrying on" ;;
      127|-1)  echo "Invocation error, PATH=$PATH"; exit 1 ;;
      *)    echo "Unkown error: $ret"; exit 1 ;;
    esac

    set -x
    
    ${vv} && ls -l ${src_test_doc} | head
    if [ ! -f "${doc_zip_file}" ]
    then
        echo "Error: failed to create JavaDoc distribution"
        exit 1
    fi
    scp_list="
              ${scp_list}
              ${doc_zip_file}
              ${doc_readme_file}
             "
fi


#
#  Binary make capability is distributed with source for
#  customisation.  It should be able to make the binary perfectly,
#  so let's try it.
#
${verbose} && echo "Make a Binary Distribution. $(date)"

binary="${SH} binary.sh"
${binary} ${verbose_arg} \
	-d ${bin_make} \
	-version ${version} \
	-out ${bin_zip} \
	util/man/binary.man
ret=$?
case $ret in
  0)    ${verbose} && echo "  Binary Complete (and is good). $(date)";;
  1)    echo "$0: Binary is bad.";  exit 1 ;;
  4)    echo "Some failed JARs, carrying on" ;;
  127)  echo "Invocation error, PATH=$PATH"; exit 1 ;;
  -1)   echo "Invocation error, PATH=$PATH"; exit 1 ;;
  2)    echo "Arguments error?"; exit 1 ;;
  *)    echo "Unkown error: $ret"; exit 1 ;;
esac

cd ${dist}

#
#  Look for the binary.
#
bin_name=$( cd ${bin_make} ; ls Cryptix-bin-${version}.zip )
bin_readme_name=$( cd ${bin_make} ; ls README-bin-${version}.html )
bin_zip_file=${bin_make}/${bin_name}
bin_readme_file=${bin_make}/${bin_readme_name}

${vv} && ls -l ${bin_make}
set -x
if [ ! -f "${bin_zip_file}" ]
then
    echo "Error: failed to create binary distribution"
    exit 1
fi
scp_list="
          ${scp_list}
          ${bin_zip_file}
          ${bin_readme_file}
         "

cat > ${download_index} <<-INDEX_HERE_DOC  || exit 1
<head> <title>Cryptix-Java Download</title> </head> <body>
<UL>
  This current distribution is ${version_dot}.
  <LI>
    <a href="${src_zip}">Source</a>
    and
    <a href="${src_readme}">README</a>
  <LI>
    <a href="${bin_zip}">Binary</a>
    and
    <a href="${bin_readme_name}">README</a>
  <LI>
    <a href="${doc_zip}">Javadoc</a>
    and
    <a href="${zip_readme_name}">README</a>
</UL>
<i>Note</i> that the hyperlinks within the READMEs are set up
for the distribution, not this page.
</body>
INDEX_HERE_DOC

remote=sniff:/usr/local/www/data/cryptix
scp_list="
          ${scp_list}
          ${bin_zip_file}
          ${bin_readme_file}
          ${download_index}
         "
ls -l ${scp_list}
if ${export}
then
    echo "Hellooooooo Baby ... It's time to Bop... "
    set -x
    scp ${scp_list} ${remote} &             # background coz tests are slow
fi


#
#  Test it.
#
${verbose} && echo "Test the Binary. $(date)"
cd ${bin_test}
unzip ${qarg} ${bin_zip}

cd class
test_it=../util/auto_testjar.sh
saved=${dist}/bin_test/extract_results
${test_it} > ${saved}
if [ $? -ne 0 ]
then
    ls -l ${test_it}
    echo "$0: Binary Test ${test_it} failed, see ${saved}. $(date)"
    exit 1
fi

${verbose} && echo "Good Binary. $(date)"
# rm -rf ${dist}*

exit 0
