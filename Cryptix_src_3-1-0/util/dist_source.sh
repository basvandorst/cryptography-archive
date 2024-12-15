#!/usr/local/bin/bash
# I tried to make it /bin/sh but this failed - some machines have level 7 sh.

#
# Script for source distribution.  Note that this is CVS-centric.
#
# script version number (not release version number)
# $Release: $
#
# Version number of the *distribution* not this script.
version=X-Y-Z

restricted=false

update=true
verbose=false

v=false
vv=false
vvv=false
vvvv=false
set_on=false

args="$0 $*"                                # for re-invoking

src_top=.
dist=${BUILD_TMP:-/tmp/Cjs$$}

while [ $# -gt 0 ]
do
    opt="$1"
    case "${opt}" in
        "-version")    version="$2"; shift ;;
        "-dist")       dist="$2"; shift ;;
        "-src")        src_top="$2"; shift ;;

        "-restricted_on")    restricted=true ;;
        "-restricted_off")   restricted=false ;;

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
            echo "      sh util/dist_source.sh"
            exit 0
            ;;
        *)             break ;;
#echo "unknown arg: ${opt}"; echo  ${usage}; exit 2 ;;
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

${vvvv} && vvv=true
${vvv}  && vv=true
${vv}   && verbose=true

type=src
big_name=Source
loc=${0%/*}
# echo $loc
source ${loc}/dist_common.sh

read_manifest ${man} || exit 1          # sets variables $m_*

src_archive=${name}_${type}_${version}
src_zip=${src_archive}.zip

#
#  Set up some names.
#
# readme_name="README_${type}_${version}.html"
readme_name="README_${type}.html"
index="index.html"
index=""
auto_dir="auto"
static_list="${m_all}"                    # from manifest
# all_names="${readme_name} ${index} ${doc_index} ${static_list}"
all_names="${readme_name} ${static_list}"

mkdir ${dist}

#
#  Copy the tree.  Ignore (remake) directories, CVS contents, *.o directories.
#
${verbose} && echo "Copy the source tree. $(date)"

cd ${src_top}

#
#  Ug.
#  Some finds cannot match dirnames.  Some greps can't do extended.  Some...
#
copy_list="${static_list}"
find ${copy_list} ! -type d -print |
	grep -v '/CVS/' |
	grep -v  '/.*\.o/' \
	> ${dist}/find_list

[ $? -ne 0 ] && echo "Error: find failed" && exit 1
cpio -pdm ${dist} < ${dist}/find_list 2> /dev/null



#
#  For here on, we are inside the new directory!!!
#
cd ${dist}

${vv} && echo "    (chmoding util/*.sh)"
chmod 755 $(find . -name '*.sh' -print)     # try and avoid ${SH}
[ ! -d ${doc_dir} ] && mkdir ${doc_dir}     # should be done in manifest

${verbose} && echo "  DELETING:  check it works!"
delete_files ${deletes}

#
#  Go through and strip out internal commentary from doco.
#
for index in $(find . -name index.html -exec grep -l INTERNAL_ONLY {} \; -print)
do

    cp ${index} ${index}.sedin  # do it backwards to preserve modes
    sed -e "/<!-- INTERNAL_ONLY -->/d" ${index}.sedin > ${index}
    rm ${index}.sedin
done

#
#  Fix up the properties file to turn OFF all native calls.
#  We don't want to call native with the standard distribution
#  as it is impossible to distribute native binaries with it.
#  And they make a lot of noise.
#
prop=src/cryptix-lib/Cryptix.properties
cp ${prop} ${prop}.sedin  # do it backwards to preserve modes
sed -e "/^Native.Allow/s/true/false # (distribution setting)/" ${prop}.sedin > ${prop}
rm ${prop}.sedin
${verbose} && echo "  Turned OFF all native calls in ${prop}"

#
#  Hmmm, CryptixProperties has a hardwired Version!!!
#
Prop_java=src/cryptix/CryptixProperties.java
eval $( sed -e '/_VERSION = /!d' -e 's/[a-z ;]//g' ${Prop_java} )
#    private static final int MAJOR_VERSION = 3;

if [ $MAJOR_VERSION-$MINOR_VERSION-$INTER_VERSION != ${version} ]
then
    echo
    echo "WARNING: need to set ===>  ${Prop_java}  <==="
    echo
else
    ${vv} && echo "    (${Prop_java} is ok)"
fi




# util/dist_binary.sh ${verbose_arg} -scripts

###############################################
#
#   Last thing is to make the READMEs which depend on the above actions.
#
${verbose} && echo "Make Support Files."
date=$(date)

### rpk_license=""
### rpk_license_file="license/CryptixLimited.html"
### rpk_src_dir="src/cryptix/provider/rpk"
### 
### if ${restricted}                           # add in the restricted license
### then
###     echo "Restricted License Components Included"
###     limited_license="
###         except for the RPK provider, which is covered by the
###         <a href=\"license/CryptixLimited.html\">Cryptix Limited License</a>
###     "
###     if [ ! -f ${limited_license_file} -o ! -d ${rpk_src_dir} ]
###     then
###         echo "Error: RPK is missing"
###         exit 1
###     fi
### else
###     echo "Removing Restricted License Components  ..."
###     rm -rf ${rpk_license_file} ${rpk_src_dir}
### fi


cat > ${readme_name} <<README_HERE_DOC || exit 1 

<!-- Created by: $0 ${date} -->
<head> <title>Cryptix ${version_dot} ${type} README</title> </head>

<body text="#000000" bgcolor="#FFFFFF" link="#0000FF" vlink="#000077" alink="#FF0000">

<table border="0" width="100%"> <tr>

  <td>
    <img src="images/cryptix.gif" ALT=" Cryptix " border=0 >
    </td>

  <td>
    <a href="README.html"><font size=-1>README</font></a> or
      <b>Readme Src</b> or
      <a href="README_bin.html"><font size=-1>Readme Bin</font></a><br>
    <a href="FAQ.html"><font size=-1>Frequently Asked Questions</font></a><br>
    <a href="doc/packages.html"><font size=-1>Javadoc index</font></a> or
      <a href="doc/index.html"><font size=-1>Help</font></a><br>
    <a href="guide/index.html"><font size=-1>Documentation</font></a><br>
    <a href="guide/security/JCESpec.html"><font size=-1>Java Cryptography Extens
ion Specification</font></a><br>
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



<H1><a name="install">Installation</a></H1>

In this installation,
there is a stable/fixed README.html and this release
readme file.  README files are no longer numbered according
to the release number.

${H1_new} 

${H1_not} 




<H1><a name="copyright">Copyright and License</a></H1>
All contents of this distribution are
Copyright &copy; 1995 - 1999 Systemics Ltd
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










${verbose} && echo "Make the Archive."
rm -f ${src_zip}

#
#  Need to Zip this up with a real Zipper.  Jar doesn't do it.
#  Some Unix machines don't have real Zips.
#  Zips change their options between versions. 
#  Life's a bitch and then...
#  On machines without Zippers this is done via an ssh shell script.
#
zip -r9q ${src_zip} ${all_names}
if [ $? -ne 0 ]
then
    echo "Zip failed on archive ${src_zip}"
    echo "${all_names}"
    exit 1
fi

exit 0
