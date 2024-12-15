#!/usr/local/bin/bash

#
#  testjar.sh - run the tests over a JAR.  Each JAR of this format
#  has a manifest file that should list the tests that can be run.
#  Current issue is that it assumes the environment: if there is
#  anything in dot, it will effect the success or otherwise of the test.
#
failed=false

opts="-verbose|-v[vv]"
usage="Usage: testjar [$opts] [-stdout root] [-auto file [-finish]] JAR_name"
#
#  Arguments.
#
compile=true
verbose=false
v=false
vv=false                                    # very verbose
vvv=false                                   # spill my guts
set_on=false
save_file=""
save_test_script=false
stdout_root=""
save_test_results=false
finish=false

while [ -n "$1" ]
do
    opt="$1"
    case "${opt}" in
        "-auto")       save_file=$2; save_test_script=true; shift ;;
        "-stdout")     stdout_root=$2; save_test_results=true; shift ;;

        "-finish")     finish=true ;;

        "-verbose")    verbose=true ;;
        "-v")          verbose=true ;;
        "-vv")         vv=true ;;
        "-vvv")        vvv=true; set_on=true; set -x ;;

        "-?"|"-help")  echo $usage; exit 2 ;;
        -*)       echo "unknown arg: ${opt}"; echo ${usage}; exit 2; break ;;
        *)        break ;;
    esac
    shift
done

# echo testjar is 3 >&3
# echo testjar is 2 >&3
# echo testjar is 1 >&1

exec 3>&1 1>&2

# echo testjar is 3 again >&3
# echo testjar is 2 again >&3
# echo testjar is 1 again >&1

${vvv} && vv=true
${vv} && verbose=true

#
#   These functions write portions of each script.
#   Call with output redirected.
#
start_unix_sh()
{
        echo "#!/bin/sh"
        echo "# ${save_unix_sh##*/} written automagically by ${0##*/}"
        echo '# ${jopt:=}     # local java options'  # note comment!
        echo ': ${java:=java} # local JVM'
        echo
        echo "good=false"
        echo "bad=false"
        #
        #   Depends on this matrix of choices:
        #
        #                     no failures     failures
        #
        #    no successes     no tests = 3    bad = 1
        #
        #    successes        good = 0        carry on = 4
        #
        echo "# exit with one of 4 codes, depending on results"
        echo "trap 'if \${good}; then if \${bad}; then exit 4; else exit 0; fi; else if \${bad}; then exit 1; else exit 3; fi; fi' 0"

        echo
        pwd_call='PWD=`pwd`       # some shells have no PWD'    # note comment!
        echo "${pwd_call}"
}

start_wintel_sh()
{
        #
        #   DOS.
        #
        echo "@echo off"
        echo "REM ${save_wintel_sh##*/} written automagically by ${0##*/}"
###        echo "call ..\util\setpwd ." >> ${save_wintel_sh}
        echo "REM local java options (uncomment if needed)"
        echo "REM if \"%jopt%\"==\"\" set jopt="
        echo "REM local JVM"
        echo "if \"%java%\"==\"\" set java=java"
        echo
        echo "REM the PWD setting does not work with OS/2"
        echo "set PWD=%~f1"
        echo "set OLDCP=%CLASSPATH%"

}

new_unix_sh()
{
    echo ""
    echo "# good tests for ${jar} - bad tests commented out"
    echo "CLASSPATH=${CLASSP_SH}; export CLASSPATH"
}

new_wintel_sh()
{
    echo ""
    echo "REM good tests for ${jar} - bad tests commented out"
    echo "set CLASSPATH=${CLASSP_DOS}"
}

end_unix_sh()
{
    echo "# end ${jar}"
}

end_wintel_sh()
{
    echo "REM end ${jar}"
}

finish_unix_sh()
{
    echo
    echo "# end ${save_unix_sh##*/}"
}

finish_wintel_sh()
{
    echo
    echo ":exit"
    echo "set CLASSPATH=%OLDCP%"
    echo "REM end ${save_wintel_sh##*/}"
}

#
#   Initialise the test repeatability output files.  This part might
#   terminate the scripts only, and won't need the jar name.
#
if ${save_test_script}
then
    save_unix_sh=${save_file}.sh
    save_wintel_sh=${save_file}.bat
    if ${finish}
    then
        finish_unix_sh >> ${save_unix_sh}
        finish_wintel_sh >> ${save_wintel_sh}
        exit 0
    fi
fi

if [ $# -eq 1 ] && [ -f "$1" ]
then
    jar=$1
else
    echo "need a JAR: $1"
    echo $usage
    exit 2
fi



JC="javac -d ${class_dir}"

${verbose} && echo "        get manifest from JAR..."

tmp=${BUILD_TMP:-/tmp/Cj$$}
tmp1=${tmp}testjar1
tmp2=${tmp}testjar2
unzip -l $jar > ${tmp1}
while read line
do
    ${vvv} && echo "       ${line}"
    case "${line}" in
    *.man)                            # manifest file
        mmm="${line##* }"
        man="${mmm#^}"                # Sun unzip has ^filename for some reason
        [ ${man} != ${mmm} ] && ${verbose} && \
            echo  "        (must be a Sun... stripping ^ from unzip output)"
        ${vv} && echo "    manifest = ${man}"
        break
        ;;
    */Test*.class)                    # no manifest, collect Test*.class
        # this part not used, add sun bug above.
        tests="${tests} ${line##* }"
        ;;
    esac
done < ${tmp1}                        # can't use cat in sh, loses ${classes}

tests_mentioned=false
if [ -n "${man}" ]
then
    
    unzip -c $jar ${man} > ${tmp2}
    #
    #  Read the tests= ... lines from the manifest file.
    #
    while read line
    do
        ${vvv} && echo "       ${line}"
        case "${line}" in
        depends=*)                    # variable=value
            val=${line#depends=}
            depends="${val}"

#            if [ -n "${ddd}" ]
#            then
#                ${vv} && echo "    need ${ddd}"
#                depends="${depends} ${ddd}"
    # this part not used, add sun bug above.
#            fi

            ;;
        tests=*)                      # tests=list of test (additive)
            val=${line#tests=}
            tests_mentioned=true
            if [ -n "${val}" ]        # already some in variable
            then
                ${vv} && echo "    try ${val}"
                tests="${tests} ${val}"
            fi
            ;;
        esac
    done < ${tmp2}                    # can't use cat in sh, loses ${classes}
    # rm ${tmp2}
else
    ${verbose} && echo "        no manifest, looking for Tests"
    echo "no test munging code, sorry"; exit 1
fi

if [ -z "${tests}" ]
then
    if ${tests_mentioned}
    then
        echo "no tests in this JAR"
        exit 3
    else
        echo "no mention of tests?"
        exit 1
    fi
fi

#
#  Work out what dependencies are needed to run the tests.
#
case ${jar} in                             # make it dir/base
    */*) ;; /*) ;;
    *) jar=./${jar} ;;                     # local name only
esac
base_jar=${jar##*/}
dir_jar=${jar%/${base_jar}}                 # directory name for other JARs

failed=false
one_ok=false
fatal=false

${verbose} && echo "        check depends ${depends} are there"
classpath=""
classpath_set=""
if [ -n "${depends}" ]
then
    for dep in ${depends}
    do
        dep2=$( ls ${dir_jar}/${dep}*.jar )
        ${vvv} && echo "      ${dep2} ?"
        if [ ! -f ${dep2} ]
        then
            failed=true
            echo "depends on lost JAR: ${dep2}"
        else
            ${vv} && echo "      found ${dep2}"
        fi
        classpath=${classpath}:${dep2}
	classpath_set="${classpath_set}:"'$pwd/'${dep2}
    done
fi

${failed} && exit 1

#
#  For the purpose of documentation and further testing by the
#  users, record our activities here in a file, which will become
#  part of the distribution.  We produce a working script for
#  Unix plus DOS, with errors commented out.
#  (don't be surprised with errors in the DOS, I don't have
#  a Wintel machine to test it on, just mail your corrections).
#
CLASSPATH=${classpath}:${jar}:${PWD}    # depends JARs + test JAR + . (files)
CLASSP_SH=${classpath}:${jar}:\${PWD}   # for embedding in scripts
CLASSP_DOS=$(echo ${classpath}:${jar}:%PWD% |
          sed -e 's,/,\\,g' -e 's,:,;,g' -e 's,\.\\,%PWD%\\,g')
export CLASSPATH
CLASSPATH_REPORT="${classpath_set}:"'$pwd/'${jar}:'${pwd}'


#
#   Organise the test repeatability scripts
#
if ${save_test_script}
then
    if [ ! -f ${save_unix_sh} ]     # first time
    then
        ${verbose} && echo "        starting scripts on root ${save_file}"
        start_unix_sh > ${save_unix_sh}
        chmod 755 ${save_unix_sh}
        start_wintel_sh > ${save_wintel_sh}
    fi
    new_unix_sh >> ${save_unix_sh}
    new_wintel_sh >> ${save_wintel_sh}
fi

save_run()
{
    local run="$1"
    local ret=${2:-0}
    local stop=${3:-false}

    if [ $ret -ne 0 ]
    then
        run_me=""                          # make a useful diagnostic
        if ${save_test_script}
        then
            echo "# ${run}" >> ${save_unix_sh}      # commented out
            echo "REM ${run}" >> ${save_wintel_sh}  # commented out
            if ${stop}
            then
                echo "# critical test, stopping here" >> ${save_unix_sh}
                echo "REM critical test, stopping here" >> ${save_wintel_sh}
            fi
        fi

        ${verbose} && sed -e "s/^/  |/" ${out_file}
        echo "FAILED: test $t"                       #  ${run_me}
        echo "        cd $PWD"
        echo "        CLASSPATH=${CLASSPATH}; export CLASSPATH"
        echo "        ${run}"
        echo "============================================"; echo; echo
        failed=true
        if ${stop}
        then
            echo "Error: critical test, stopping here"
            fatal=true
            exit 1  # why bother to unwind?
        fi
    else
        ${verbose} && echo "        test $t ok"
        ${vv} && cat ${out_file}
        one_ok=true

        if ${save_test_script}
        then
            case ${run} in
              java" "*)
                    run2=${run#java }
                    go='${java} ${jopt} '${run2}
                    bat='%java% %jopt% '${run2}
                    ;;

              *) echo "huh?  ${run} not java"; exit 1 ;;
            esac
            #
            #  Attempt to record goodness of test.
            #  This is not perfect, it depends on the test returning
            #  an error code.  See the trap matrix above.
            #  If the test is critical, then bail out.
            #
            bad_action="bad=true"
            ${stop} && bad_action="exit 1"
            echo "if ${go}; then good=true; else ${bad_action}; fi" >> ${save_unix_sh}

            echo "${bat}" >> ${save_wintel_sh}
            ${stop} && echo "if errorlevel 1 goto exit" >> ${save_wintel_sh}
        fi
    fi
}

${verbose} && echo "        testing in pwd=$PWD"
${verbose} && echo "        CLASSPATH=$CLASSPATH_REPORT"
${vvv} && set -x
for t in ${tests}
do
    stop_on_error=false
    ret="nan"                              # not a number

    if ${save_test_results}
    then
        out_file=${stdout_root}${t##*/}    # if sh test, strip off path
        out_arg="> ${out_file}"
    else
        out_arg=""
    fi

    #
    #  Different types of tests.
    #
    case ${t} in
        */*.sh)                            # ye gods shell?  is it worth it?
            if [ ! -f ${t} ]
            then
                echo "Warning: ${t} lost?  Unzipping..."
                unzip -o ${jar} ${t}
            fi
            chmod 755 ${t}
            eval "${t} ${out_arg}"
            ret=$?

            echo "Shell test - not recording run mechanism."
            # is this more trouble than it is worth?
            # YES, because it's a shell test.
            # dos: no test.  sh: return codes of unzip+chmod+... not sensible
            # notice written test is -f  freshen, not -o  overwrite
            # save_run "unzip -f ${jar} ${t}" ${ret}
            # save_run "${chm}" ${ret}
            # save_run "${run}" ${ret}
            ;;

        *.STOP_ON_ERROR)                   # if this fails, the rest are rubbish
            t=${t%.STOP_ON_ERROR}
            ${verbose} && echo "          critical test: ${t}"
            eval "java ${t} ${out_arg}"
            ret=$?
            save_run "java ${t}" ${ret} true  # extra arg: stop on error
            ;;

        *.*)                               # an ordinary java test
            eval "java ${t} ${out_arg}"
            ret=$?
            save_run "java ${t}" ${ret}
            ;;
    esac

    ${fatal} && break
done
#  ${set_on} || set +x



#
#  Cleanup.  Terminate the files, and decide on an error code to return.
#
if ${save_test_script}
then
    end_unix_sh >> ${save_unix_sh}
    end_wintel_sh >> ${save_wintel_sh}
fi

${fatal} && exit 1                         # a critical test and failed
${one_ok} || exit 1                        # nothing worked? serious problems
${failed} && exit 4                        # 4 can be soft - some tests failed

rm -f ${tmp1} ${tmp2}
exit 0
