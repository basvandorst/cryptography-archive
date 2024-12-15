#!/usr/local/bin/bash

top=${PWD}
src=${top}/src
doc=${top}/doc


verbose=false
vv=false
vvv=false
vvvv=false
set_on=false
build_it=true
test=false

top_dir=${PWD}
doc_dir=${top_dir}/doc
src_dir=${top_dir}/src

while [ $# -gt 0 ]
do
    opt="$1"
    case "${opt}" in
        "-d")          doc_dir="$2"; shift ;;
        "-src")        src_dir="$2"; shift ;;  # not the same as sourcepath

        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true;   verbose_arg="-v" ;;
        "-vvv")        vvv=true;  verbose_arg="-vv";  set_on=true ;;
        "-vvvv")       vvvv=true; verbose_arg="-vvv"; set_on=true; set -x ;;
        "-test")       test=true ;;            # go thru the motions
        "-build")      build_it=false ;;

        "-?"|"-help")
            echo "Usage: util/build_javadoc.sh [-d dir -src dir] [javadoc opts]"
            cat <<-!
                Build the Javadoc.

                -d dir     where to build the html files (passwd onto javadoc)
                           default is 'doc'
                -src dir   top of source tree, stuck in -sourcepath
                           default is 'src'
                -verbose   some diags.  more with -vv, -vvv, -vvvv
                -build     don't actually run the javadoc command
                -test      only include the first package found in src
!
exit 0
;;
        *)             break ;;
    esac
    shift
done


${vvv} && vv=true
${vv} && verbose=true


if [ "${javadoc}" = "" ]
then
    javadoc=javadoc
    # No: only useful in Wintel, and blocks javadoc setup
    # javadoc="java -ms16m sun.tools.javadoc.Main"
fi
${verbose} && echo "      javadoc = ${javadoc}"


#
#  We need the Java Lib to stop Javadoc from installing broken links.
#  However, it's hidden, so we need to dig it out.
#
if [ "${JDK_SOURCE}" = "" ] || [ ! -f ${JDK_SOURCE} ]
then
    javac=$(which javac)

    
    # stolen from javac
    # If PRG is a symlink, trace it to the real home directory
    while [ -L "$javac" ]
    do
        symlink=$(ls -l ${javac})
        case ${symlink} in
          *" -> /"*) javac=${symlink##* } ;;  # symlink to other place
          *)         break ;;                 # no, relative link, no use
        esac

#        newprg=`expr "\`/bin/ls -l "$PRG"\`" : ".*$PRG -> \(.*\)"`
#        expr "$newprg" : / >/dev/null || newprg="`dirname $PRG`/$newprg"
#        PRG="$newprg"
    done

    javabin=${javac%/javac}
    javahome=${javabin%/bin}
    classes_zip=${javahome}/lib/classes.zip
    # last ditch effort -- on hayek
    [ ! -f "${classes_zip}" ] && classes_zip=/usr/local/apps/java/jdk/lib/classes.zip
    if [ ! -f "${classes_zip}" ]
    then
        # how the hell does one find this out???
	# on hayek, set by java script
        echo "build_javadoc: cannot determine JDK_SOURCE"
	echo "set JDK_SOURCE to the classes.zip in your JDK to get Java classes"
	exit 1
    else
        JDK_SOURCE=${classes_zip}
    fi
fi
${verbose} && echo "      JDK_SOURCE = ${JDK_SOURCE}"

#
#  Grab all the directories in the source area, and include them
#  if the test positive for java crypto.
#
cd ${src}
for d in $(find * -type d -print)
do
    if [ "$(echo ${d}/*.java)" != ${d}/'*.java' ]
    then
        ${vv} && echo "          ${d}"
	package_dirs="${package_dirs} ${d}"
        ${test} && break
    fi
done

[ -z "${package_dirs}" ] && echo "no packages found in ${src} ???" && exit 1

#
#  Convert to package names.  Javadoc wants package names not directories.
#
packages=$( echo ${package_dirs} | sed -e 's,/,.,g' )

tmp=${BUILD_TMP:-/tmp}

ret=0
if ${build_it}
then
    cd ${doc}                      # only does file.html if in dot?
    case ${src} in 
      /*) ;;
      *)  src=../${src} ;;
    esac

    #
    #  Run the command.
    #  See http://www.javasoft.com/products/jdk/javadoc/index.html
    #  for more info.
    #
    ${verbose} && echo "      javadoc'g now (this will take some time...)"
    ${vv} && echo      "             (a couple of minutes on a P200)"
    ttt=${tmp}/javadoc.o
    ${vvv} && set -x
    javadoc \
        -sourcepath "${src}:${JDK_SOURCE}" \
        -d . \
        -author $* \
        ${packages} \
        > ${ttt}
    ret=$?
    ${vvvv} || set +x

    dead=false
    java_errors=0
    
    #
    #  Try and find the errors in the JavaDoc and return a soft
    #  indication if they are basic coding.
    #
    while read line
    do
        case "${line}" in
#        [?C]" "*)
#            echo "Error: ${line}"
#            dead=true
#            ;;
    
        *.java:[0-9]*)
            java_errors=$((java_errors + 1))
            ;;
    
        #
        #  These are warnings from javadoc that we don't care about.
        #
        'Loading source files '*) ;;
        ' '*) ;;
        '	'*) ;;
        *'^') ;;
        "import "*) ;;
        "Method "*) ;;
        "but there is no corresponding "*) ;;
        *class*) ;;
        '}'*) ;;
        'Generating documentation for class'*) ;;
    
        #
        #  These are messages that merit printing out.
        #
        'Generating '*)     ${verbose} && echo "        ${line}" ;;
        'Sorting '*)        ${verbose} && echo "        ${line}" ;;
        *[0-9]' errors')    ${verbose} && echo "        ${line}" ;;

        *)          echo "? $line" ;;       # can we skip this line too?
        esac
    done < ${ttt}
    
    if ${dead}
    then
        echo "JavaDoc failed. Please check it.  Full output in ${ttt}"
        exit 1
    fi

    if [ ${java_errors} -ne 0 ]
    then
        ret=4
        echo "     found ${java_errors} errors in javadoc output, carry on"
        ${verbose} && echo "        errors in ${ttt}"
    fi
else
    ${verbose} && echo "        skipping build"
fi


exit $ret
