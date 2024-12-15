#!/usr/local/bin/bash
# I tried to make it /bin/sh but this failed - some machines have level 7 sh.

#
# Script to make a Javadoc release.
#
# script version number (not release version number)
# $Release: $
#
# Version number of the *distribution* not this script.
#
version=A-B-C

verbose=false
verbose_arg=""
v=false
vv=false
vvv=false
vvvv=false
set_on=false
# limited=true
date=$(date)

build_it=true
top_dir=${PWD}
doc_dir=${top_dir}/doc
src_dir=${top_dir}/src
outfile=""
readme_out=""

while [ $# -gt 0 ]
do
    opt="$1"
    case "${opt}" in
        "-version")    version="$2"; shift ;;
        "-d")          doc_dir="$2"; shift ;;
        "-src")        src_dir="$2"; shift ;;
        "-out")        outfile="$2"; shift ;;
        "-readme")     readme_out="$2"; shift ;;

        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true;  verbose_arg="-v" ;;
        "-vvv")        vvv=true; verbose_arg="-vv";  set_on=true ;;
        "-vvvv")       vvv=true; verbose_arg="-vvv"; set_on=true; set -x ;;
        "-build")      build_it=false ; ;;

        "-?"|"-help")
            echo "Usage: util/dist_javadoc.sh [opts] -version 6-6-6"
            cat <<-!
		Make a Javadoc Distribution.
!
exit 0
;;
        *)             echo "unknown arg: ${opt}"; exit 2 ;;
    esac
    shift
done


${vvv} && vv=true
${vv} && verbose=true

type=doc
name=Cryptix_${type}
archive=${name}_${version}
big_name="Javadoc Distribution"

loc=${0%/*}
echo $loc
source ${loc}/dist_common.sh


case "${doc_dir}" in
  */doc) ;;
  doc)   ;;
  "")    doc_dir=doc ;;
  *)     doc_dir=${doc_dir}/doc; echo "adding doc component: ${doc_dir}" ;;
esac





[ ! -d ${doc_dir} ] && mkdir ${doc_dir}
[ ! -d ${doc_dir} ] && echo "Error: mkdir ${doc_dir} failed: $?" && exit 1

readme=README_${type}.html
doc_readme=${doc_dir}/../${readme}     # note, zip expects ${readme} there

#
#  HACK!  Say something extra if the Limited license is there.
#
if [ -f ${doc_dir}/license/CryptixLimited.html ]
then
    extra_license="except for the components identifed within
and covered by
<a href=\"doc/${ltd_license}\">Cryptix Limited License</a>"
fi

        
cat > ${doc_readme} <<README_HERE_DOC || exit 1

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
      <b>Readme Doc</b><br>
    <a href="FAQ.html"><font size=-1>Frequently Asked Questions</font></a><br>
    <a href="doc/packages.html"><font size=-1>Javadoc index</font></a> or
      <a href="doc/index.html"><font size=-1>Help</font></a><br>
    <a href="guide/index.html"><font size=-1>Documentation</font></a><br>
    <a href="guide/security/JCESpec.html"><font size=-1>Java Cryptography Extens
ion Specification</font></a><br>
    <a href="util/index.html"><font size=-1>Utility scripts</f></a><br>
    <a href="license/CryptixGeneral.html"><font size=-1>Copyright and License</f
ont></a><br>


  <td align=right valign=top>
    <b><i>Cryptix<br><br>Development<br><br>Team</i></b>
    </td>

</tr> </table>

<hr size=3 noshade>

<center><b><font size=+4>
    Cryptix <sup><font size=1>TM</font></sup><br>
    ${big_name} ${version_dot}<br>
    README
</font></b></center>

<head> <title>Cryptix ${version_dot} ${type} README</title> </head> <body>

<H1><a name="intro">Introduction</a></H1>

This is
Cryptix ${big_name} ${version_dot}
made on ${date}.


<H1><a name="install">Installation</a></H1>
To get the documentation installed and tested,
download and unzip the ${src_zip}
from the top Cryptix directory (i.e.,
in the directory that contains <i>src</i> and <i>util</i>).  The files
will all go into the <i>doc</i> directory.  Also see the
<a href="doc/index.html">doc index</a> for other notes.

<P>
The indexes at the top of major documentation files
will already point at that <i>doc</i> directory.
Point your browser any one of the entry points in those
files, or listed below.


<H1><a name="not">What's *not* here</a></H1>

These things are missing:

<UL>
  <LI>
    images
</UL>

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





#
#   Javadoc It!
#
pdoc="build_javadoc.sh"
jdoc="build_javadoc.sh"
ex=0
while ${build_it}
do
    #
    #  Run the javadoc program.
    #
    ${verbose} && echo "  build the javadoc ${date}"
   
    ${jdoc} ${verbose_arg} \
        -d ${doc_dir} \
        -src ${src_dir}
    jret=$?
    case $jret in
      0)    ${verbose} && echo "  javadoc built $(date)";;
      1)    echo "$0: Javadoc is bad.";  exit 1 ;;
      2)    echo "Arguments error?"; exit 1 ;;
      4)                                        # soft errors are carried on
            ${verbose} && echo \
                "  javadoc built $(date) with SOFT errors, carry on"
            ex=4
            ;;
      126|127|-1)                                   # try again with fixed name
            if [ ${jdoc} = ${pdoc} ]
	    then
		jdoc=util/${jdoc}
		echo "      (reinvoke as ${jdoc})"
		continue
            fi
            echo "Invocation error, PATH=$PATH"
            exit 1
            ;;
      *)    echo "Unkown error: $ret"; exit 1 ;;
    esac
    build_it=false
done



[ -z "${outfile}" ] && outfile=${archive}.zip
case ${outfile} in                         # make absolute
  /*)  ;;
  *)   outfile=${PWD}/${outfile} ;;
esac

rm -f ${outfile}

#
#  Need to Zip this up with a real Zipper.  Jar doesn't do it.
#  Some Unix machines don't have real Zips.  Life's a bitch and then...
#
cd ${doc_dir}/..
basedir=doc
# basedir=${PWD##*/}
# cd ..
zip -rq9 ${outfile} ${readme} ${basedir}
${vv} && echo "    javadoc archive ${outfile}"

[ -n "${readme_out}" ] && cp ${doc_readme} ${readme_out}   # old, unused?

exit $ex

