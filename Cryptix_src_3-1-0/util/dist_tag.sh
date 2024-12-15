#!/usr/local/bin/bash

#
# Script for tagging files for distribution.  Note that this is CVS-centric.
#
# script version number (not release version number)
# $Release: $
#

verbose=false

v=false
vv=false
vvv=false
vvvv=false
set_on=false

args="$0 $*"                                # for re-invoking

src_top=.

while [ $# -gt 0 ]
do
    opt="$1"
    case "${opt}" in
        "-tag")        tag="$2"; shift ;;
        "-old")        old="$2"; shift ;;
        "-src")        src_top="$2"; shift ;;

        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true;   verbose_arg="${opt}" ;;
        "-vvv")        vvv=true;  verbose_arg="${opt}" ;;
        "-vvvv")       vvvv=true; verbose_arg="${opt}"; set_on=true; set -x ;;
        "-?"|"-help")
            echo "To tag a tree from a manifest list, run"
            echo "      util/dist_tag.sh manifest"
	    echo
            echo "The manifest should include:"
	    echo "      tag_version=Your_Tag
	    echo "      old_tag=Old_Tag        # optional, for -r flag"
	    echo "      files+dirs to tag..."
            exit 0
            ;;
        *)             break ;;
    esac
    shift
done

usage="util/dist_tag [-tag T] [-old T] [-src D] [-vvvv] util/man/cvs.man"

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
    [ -n "$1" ] && echo "need a readable manifest: $1"
    echo $usage
    exit 
fi

${vvvv} && vvv=true
${vvv}  && vv=true
${vv}   && verbose=true

loc=${0%/*}
source ${loc}/dist_common.sh

read_manifest ${man} || exit 1          # sets variables

[ -n "${old}" ] && old_tag=${old}        # overrides manifest
[ -n "${tag}" ] && tag_version=${tag}    # overrides manifest

if [ -n "${old_tag}" ]
then
    r_old_tag="-r ${old_tag}"
    ${verbose} && echo "Using old tag ${old_tag}"
fi

#
#  Tag the tree.  Ignore directories, CVS contents, *.o directories.
#  Some finds cannot match dirnames.  Some greps can't do extended.  Some...
#
#  There is some bug in CVS where it recursively builds a dud pathname
#  and goes about tagging it.
#
${verbose} && echo "Tag the source tree. $(date)"
cd ${src_top}

files_list=""
dirs_list=""

for path in ${all}                # try and avoid this CVS bug
do
    if [ -f ${path} ]
    then
	files_list="${files_list} ${path}"
    elif [ -d ${path} ]
    then
	dirs_list="${dirs_list} ${path}"
    else
	echo "${path}: not a file or directory?"
	exit 1
    fi
done

# list=$( find ${file_list} ! -type d -print |
# 	grep -v '/CVS/' |
# 	grep -v  '/.*\.o/' )
# [ $? -ne 0 ] && echo "Error: find failed" && exit 1

cvs tag -F ${r_old_tag} ${tag_version} ${files_list}
[ $? -ne 0 ] && echo "Error: cvs tag files failed" && exit 1

#
#  Bug in CVS tag cannot handle some combination of directories.
#
for d in ${dirs_list}
do
   cvs tag -F ${r_old_tag} ${tag_version} ${d}
   [ $? -ne 0 ] && echo "Error: cvs tag failed: ${d}" && exit 1
done

if [ -n "${deletes}" ]
then
    cvs tag -d ${tag_version} ${deletes}
    [ $? -ne 0 ] && echo "Error: cvs tag DELETE failed" && exit 1
    set +x
    echo 'Check these files are not tagged!!!'
    echo ${deletes}
fi

exit 0
