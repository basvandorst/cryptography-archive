#!/usr/local/bin/bash
# I tried to make it /bin/sh but this failed - some machines have level 7 sh.

#
# Script for distribution of everything.
#
# script version number (not release version number)
# $Release: $
#
# Version number of the *distribution* not this script.
version=3-1-0


export=false
update=true
verbose=false
clean=false
restricted_arg="-restricted_off" #  off to turn off RPK, on to add it in
v=false
vv=false
vvv=false
vvvv=false
set_on=false

source_it=true
binary_it=true
javadoc_it=true

args="$0 $*"                                # for re-invoking

skip_arg=""

while [ $# -gt 0 ]
do
    opt="$1"
    case "${opt}" in
        "-update")     update=false ;;
        "-javadoc")    javadoc_it=false ;;
        "-source")     source_it=false ;;
        "-binary")     binary_it=false ;;
        "-export")     export=true ;;
#        "-clean")      clean=true ;;
        "-skip")       skip_arg="-skip $2"; shift ;;
        "-restricted_on")    restricted_arg="${opt}" ;;
        "-restricted_off")   restricted_arg="${opt}" ;;

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
            echo
            echo "options:"
            echo "      -update       skip the 'cvs update'"
            echo "      -javadoc      skip the javadoc phase"
            echo "      -source       skip the source phase"
            echo "      -binary       skip the source phase"
            echo "      -export       export to server"
            echo "      -skip X       skip archive X"
            echo "      -v            verbose, add more v's to make it more"
            exit 0
            ;;
        *)             echo "unknown arg: ${opt}"; echo  ${usage}; exit 2 ;;
    esac
    shift
done

${vvvv} && vvv=true
${vvv}  && vv=true
${vv}   && verbose=true

name=Cryptix
src_archive=${name}_src_${version}
doc_archive=${name}_doc_${version}
bin_archive=${name}_bin_${version}
src_zip=${src_archive}.zip
doc_zip=${doc_archive}.zip
bin_zip=${bin_archive}.zip
download_index=index.html


loc=${0%/*}
source ${loc}/dist_common.sh          # for the version number


## #
## #  Has the Perversion struck?  Check type of OS for (in)compatibility features.
## #
## perversion=`uname -s`                  # the original Perversion was GNU...
## case "${perversion}" in
##   Linux)    fullname="-path" ;;
##   FreeBSD)  fullname="-path" ;;
##   SunOS)    fullname="-name" ;;
## esac

dist=/tmp/Cj$$
BUILD_TMP=${dist}/; export BUILD_TMP      # slash permits append tmp names
mkdir ${dist}
rm -f /tmp/Cj
ln -s ${dist} /tmp/Cj                     # quick way into latest for browser


## if ${clean}
## then
##     #
##     #  If starting from scratch, make a fresh directory for working in.
##     #
##     dist=/tmp/Cjs$$
##     BUILD_TMP=${dist}; export BUILD_TMP
##     mkdir ${dist}
## else
##     #
##     #  For normal working, assume that there are bugs down the line.
##     #  This means implementing make-like tests to see what has changed.
##     #
##     dist=/tmp/Cjs$$
##     BUILD_TMP=${dist}; export BUILD_TMP
##     mkdir ${dist}
## fi

#
#  Set up some names.
#
# cvs_top=${PWD}
# PATH="${cvs_top}/util:$PATH"; export PATH

# need to be here for cvs.
src_top=${PWD}

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
    arg0=${0##*/}
    
    update_file=${dist}/update
    ${verbose} && echo "Doing CVS update.  This must be clean (-update to skip)"
    cvs update ${static_list} > ${update_file} 2>&1
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
            case "${line}" in                 # Self Update?
                [UP]" "*/${arg0})
                    Im_changed=true
                    echo "The script has been updated as well."
		    chmod 755 ${line#[UP] }
                    ;;
		[UP]" "*.sh)                  # shell script from CVS needs x
		    cmd=${line#[UP] }
		    chmod 755 ${cmd}
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
    done < ${update_file}
    
    if ${dead}
    then
        echo "Update failed.  Clean up.  Full cvs output in ${update_file}"
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



scp_list=""
src_make=${dist}/src_make
src_test=${dist}/src_test
bin_test=${dist}/bin_test
bin_make=${dist}/bin_make
mkdir ${src_test} ${bin_test} ${bin_make}

other_files_loc=${src_test}
other_files="README.html doc/index.html license/CryptixGeneral.html FAQ.html"

##########################################################
#
#   Make the Source
#
if ${source_it}
then
    ${verbose} && echo "Make a Source Distribution. $(date)"

    #
    #  Side-effects.  Guess the names of the files we need.
    #
    src_zip_file=${src_make}/${src_zip}
    src_test_doc=${src_make}/doc
    src_readme_name=README_src.html
    src_files="${src_make}/README_src.html"
    
    msrc="${SH} util/dist_source.sh"       # in ${src_top}
    ${msrc} ${verbose_arg} \
	${restricted_arg} \
    	-dist ${src_make} \
    	-src ${src_top} \
	-version ${version} \
	util/man/source.man
    ret=$?
    case $ret in
      0)    ${verbose} && echo "Source Complete $(date)";;
      1)    echo "Source Archive is bad.";  exit 1 ;;
      2)    echo "Arguments error?"; exit 1 ;;
      4)    echo "warning: Some failed files, carrying on" ;;
      126|127|-1)  echo "Invocation error, PATH=$PATH"; exit 1 ;;
      *)    echo "Unkown error: $ret"; exit 1 ;;
    esac

    ${vvv} && ls -l ${src_make}
    if [ ! -f "${src_zip_file}" ]
    then
        echo "Error: failed to create Source distribution"
        exit 1
    fi
    scp_list="
              ${scp_list}
              ${src_zip_file}
              ${src_files}
             "
fi

#########################################################
#
#   OK, we have a new distribution of the source.
#   Let's break that open, and do all our work using this copy.
#
${verbose} && echo "Source Distribution Made.  Extract. $(date)"
cd ${src_test}
qarg=-qq
${vvvv} && qarg=""
unzip ${qarg} ${src_zip_file}



##########################################################
#
#   Javadoc It!
#
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
    doc_files="${src_test}/${doc_readme_name}"
    
    jdoc="${SH} util/dist_javadoc.sh"       # in ${src_test}
    ${jdoc} ${verbose_arg} \
    	-d ${src_test_doc} \
    	-src ${src_test}/src \
	-version ${version} 
    ret=$?

#	-out ${doc_zip_file} \
#	-readme ${doc_readme_file} \
    case $ret in
      0)    ${verbose} && echo "JavaDoc Complete $(date)";;
      1)    echo "JavaDoc Archive is bad.";  exit 1 ;;
      2)    echo "Arguments error?"; exit 1 ;;
      4)    echo "warning: Some failed docs, carrying on" ;;
      126|127|-1)  echo "Invocation error, PATH=$PATH"; exit 1 ;;
      *)    echo "Unkown error: $ret"; exit 1 ;;
    esac

    ${vv} && ls -l ${src_test_doc} | head
    if [ ! -f "${doc_zip_file}" ]
    then
        echo "Error: failed to create JavaDoc distribution"
        exit 1
    fi
    scp_list="
              ${scp_list}
              ${doc_zip_file}
              ${doc_files}
             "
fi


##########################################################
#
#   Now the Big One!
#
#   Binary make capability is distributed with source for
#   customisation.  It should be able to make the binary perfectly,
#   so let's try it.
#
if ${binary_it}
then
    ${verbose} && echo "Make a Binary Distribution. $(date)"

    bin_zip_file=${bin_make}/${bin_zip}
    src_test_bin=${bin_make}/bin
    bin_readme_name=README_bin.html
    bin_readme_file=${bin_make}/${bin_readme_name}
    bin_files="${bin_readme_file}"
    
    PATH="${src_test}/util:${PATH}"; export PATH
    binary="${SH} util/dist_binary.sh"         # in ${src_test}
    ${binary} ${verbose_arg} \
	${restricted_arg} \
    	-dist ${bin_make} \
    	${skip_arg} \
    	-version ${version} \
    	util/man/binary.man
    ret=$?

#    	-out ${bin_zip} \

    case $ret in
      0)    ${verbose} && echo "  Binary Complete (and is good). $(date)";;
      1)    echo "$0: Binary is bad.";  exit 1 ;;
      4)    echo "Some failed JARs, carrying on" ;;
      126|127|-1)  echo "Invocation error, PATH=$PATH"; exit 1 ;;
      2)    echo "Arguments error?"; exit 1 ;;
      *)    echo "Unkown error: $ret"; exit 1 ;;
    esac
    
    cd ${dist}
    
    ${vv} && ls -l ${bin_make}
    if [ ! -f "${bin_zip_file}" ]
    then
        echo "Error: failed to create binary distribution"
        exit 1
    fi
    scp_list="
              ${scp_list}
              ${bin_zip_file}
              ${bin_files}
             "
fi



big_name="Download Page"

##########################################################
#
#   Send it up to the site with it's own download page.
#   Well, maybe, depends on the flag.
#
cat > ${download_index} <<-INDEX_HERE_DOC  || exit 1
  <head><title>Cryptix-Java ${version_dot} Download</title></head>
  <body text="#000000" bgcolor="#FFFFFF" link="#0000FF" vlink="#000077" alink="#FF0000">
  
  <table border="0" width="100%"> <tr>
  
    <td>
      <img src="images/cryptix.gif" ALT=" Cryptix " border=0 >
      </td>
  
    <td>
      <a href="README.html"><font size=-1>README</font></a> or
        <a href="README_src.html"><font size=-1>Readme Src</font></a> or
        <a href="README_bin.html"><font size=-1>Readme Bin</font></a><br>
      <a href="FAQ.html"><font size=-1>Frequently Asked Questions</font></a><br>
      <a href="doc/packages.html"><font size=-1>Javadoc index</font></a> or
        <a href="doc/index.html"><font size=-1>Help</font></a><br>
      <a href="guide/index.html"><font size=-1>Documentation</font></a><br>
      <a href="license/CryptixGeneral.html"><font size=-1>Copyright and License</font></a><br>
  
      </td>
  
    <td align=right valign=top>
      <b><i><br>Cryptix<br><br>Development<br><br>Team</i></b>
      </td>
  
  </tr> </table>
  
  <hr size=3 noshade>
   
  <center><b><font size=+4>
    Cryptix <sup><font size=1>TM</font></sup><br>
    Mirror ${version_dot}<br>
  </font></b></center>
  
  <P>
  The current release: <big>  ${version_dot}  </big><br>
  See the <a href="README.html">README</a> or download:
  <UL>
    <LI>
      <a href="${src_zip}">Source</a>
      includes
      <a href="${src_readme_name}">Src Readme</a>
    </LI> <P> <LI>
      <a href="${bin_zip}">Binary</a>
      includes
      <a href="${bin_readme_name}">Bin Readme</a>
    </LI> <P> <LI>
      <!-- only one on this line ... read by mirror script -->
      <a href="${doc_zip}">Javadoc</a>
      includes
      <a href="${doc_readme_name}">Doc Readme</a>
    </LI> <P> <LI>
      <a href="Cryptix-1.16.tar.gz">Cryptix-perl 1.16</a>
      is strong crypto in C for Perl.
      <a href="PGP-0.09.tar.gz">PGP for Perl 0.09</a>
      adds PGP 2.6 messaging and key management to Cryptix-Perl.
      <br><small>Note: use right-click on browsers that don't understand...</small>
  </UL>
  
  <P>
  <b><i>Note</i></b> that the hyperlinks within the READMEs, FAQ, etc,
  are set up for the <i>distribution</i>, not for this download page.
  So many of them don't work: download the distribution instead.

  <P>
  Downloading problems?  <i>Please</i> let us in the secret:
  <a href="mailto:bugs@cryptix.org?subject=Download ${version_dot}">    bugs@cryptix.org
  </a>
  and include the site name.

  <!-- OLD_FILES Cryptix_src_3-0-1.zip Cryptix_bin_3-0-1.zip Cryptix_doc_3-0-1.zip -->
  <!-- OLD_FILES README_src_3-0-1.html README_bin_3-0-1.html README_doc.html -->

  ${full_end_piece}
  </body>
INDEX_HERE_DOC



######################################################
#
#  Copy it all to the server.
#
scp_list="
    ${scp_list}
    ${download_index}
"
loc=public_html/cryptix
ssh="ssh1 -x"
scp="scp1"
if ${export}				# set remote copy params
then
    if [ -n "${CRYPTIX_NODE}" ]
    then
        remote=${CRYPTIX_NODE}
    else
        echo "Cannot export: set CRYPTIX_NODE (and maybe CRYPTIX_KEY)"
        export=false
    fi
    if [ -n "${CRYPTIX_KEY}" ]
    then
        key="-i ${CRYPTIX_KEY}"
    fi
fi

if ${export}				# now do the copy
then
    echo "Hellooooooo Baby ... It's time to Bop... "
    dirs="doc guide guide/cryptix license images"
    set -x
    # if in background, must put -n on ssh...
    ${ssh} -n ${key} ${remote} mkdir ${loc} ';' cd ${loc} ';' mkdir -p ${dirs}
    # ...unless already got an input file!
    ( cd ${other_files_loc} ; tar cf - ${other_files} ) |
               ${ssh} ${key} ${remote} cd ${loc} ';' tar vxf -
    ${scp} ${key} ${scp_list} ${remote}:${loc} < /dev/null &
    set +x
    # background coz tests are slow
else
    ${verbose} && echo "Not exporting these"
    ${verbose} && ls -l ${scp_list}
fi


######################################################
#
#  Test it.
#
${verbose} && echo "Test the Binary. $(date)"
cd ${bin_test}
unzip ${qarg} ${bin_zip_file}

cd classes
test_it=../auto/testjar.sh
saved=${dist}/bin_test/extract_results
errors=${dist}/bin_test/extract_errors
${test_it} > ${saved} 2> ${errors}
if [ $? -ne 0 ]
then
    ls -l ${test_it}
    echo "$0: Binary Test ${test_it} failed, see ${saved}. $(date)"
    exit 1
fi

${verbose} && echo "Good Binary. $(date)"
# rm -rf ${dist}*

exit 0
