
# not a script, but a common group of commands and variables
date=$(date)

dot_version()
{
    local version=$1
    #
    #   Turn 1-2-3 into 1.2.3 ... my kingdom for a string processing feature.
    #
    local maj=${version%%-*}
    local min=${version##*-}
    local mid=${version#*-}
          mid=${mid%-*}
    local version_dot=${maj}.${mid}.${min}
    echo ${version_dot}
}

name=Cryptix
version_dot=$(dot_version ${version})        # might change later
${verbose} && echo "  Version ${version_dot} ${date}"


#
# Standard end piece for html.  Note no quotes used.
#
mirror_site='<a href="http://www.cryptix.org/">mirror site</a>'

copy_by_who="
  Copyright &copy; 1995 - 1999 Systemics Ltd<br>
  on behalf of the Cryptix Development Team.<br>
  All rights reserved.<br>
  Cryptix is a trademark of Systemics Ltd.<br>
"
full_end_piece="
  <hr size=3 noshade>
  <img src=images/cryptix.gif alt=Cryptix align=right>
  <font size=-1>
  All contents of this distribution are<br>
  ${copy_by_who}
  </font>
"
dotdot_end_piece="
  <hr size=3 noshade>
  <img src=../images/cryptix.gif alt=Cryptix align=right>
  <font size=-1>
  All contents of this distribution are<br>
  ${copy_by_who}
  </font>
"

#
#  Standard parts of READMEs
#
H1_intro="
  <H1><a name="intro">Introduction</a></H1>

  Cryptix is a suite of cryptographic classes for Java.
  This distribution is Cryptix
  ${big_name} ${version_dot}
  made on ${date}.  Cryptix 3 is on a maintenance track,
  new effort is going into a later generation based on
  the JCE 1.2 format.

"


doc_dir="doc"
doc_packages="${doc_dir}/packages.html"
doc_pack_url="<a href=\"${doc_packages}\">${doc_packages}</a>"
doc_index="${doc_dir}/index.html"
doc_index_url="<a href=\"${doc_index}\">${doc_index}</a>"

small_bugs="
  Send bug reports to
  <a href=\"mailto:bugs@cryptix.org?subject=Bug ${version} ${type}\">
  bugs@cryptix.org
  </a>
  <P>
  Please include
  <UL>
    <LI>the Cryptix version number: ${version_dot},
    <LI>the type of distribution: <i>${type}</i>,
    <LI>your JDK version / supplier,
    <LI>the platform
  </UL>
  and <i>of course</i>
  a description of the problem (including exceptions, output).
"
H1_bugs="
  <H1><a name=\"bugs\">Bug Reports</a></H1>
  ${small_bugs}
   
  <P>
  Note that if you are not subscribed, you should do so.
  <i>Regardless</i>,
  bugs will caught by the list manager and sent to an appropriate Cryptix
  Development Team member.
"


H1_new="
<H1><a name="new">New in Release ${version_dot} </a></H1>

<P>
This release - ${version_dot} - is mostly a bug fix release
after an extended period of time.  It may very well be the
last release in the Cryptix 3 line, as the next generation
will substantially change distribution patterns.

<p>
Changes include:
<P>
<UL></LI>
    Java 2 (JDK 1.2) compatibility.  No JCE 1.2 as yet, that will
    be released under a different stream.

  </LI><P><LI>
    SHA-1 incompatibility bug is fixed (hopefully).
    MD5, SHA-1, SHA-0 update() bug should be fixed.

  </LI><P><LI>
    RC4 static variable bug is fixed.

  </LI><P><LI>
    Cryptix 2.2 compatibility with RSA is now added.
    It should not be used except by legacy code, like PGP 2.6 below.

  </LI><P><LI>
    PGP 2.6 (not OpenPGP) has been added.  There is only
    a nominal test that suits distribution.

  </LI><P><LI>
    Testing now is more deterministic, as tests return
    10 on success.  This derives from the JVM's
    viewpoint that an uncaught exception is a successful
    result.  New support classes are also added to the test cycle.

  </LI><P><LI>
    Building:  Old scripts in <tt>util</tt> cleaned up,
    and new scripts added.

  </LI><P><LI>
    There is now an example manifest file
    <tt>util/man/webfunds.man</tt>
    which builds a complete single jar,
    taken from the WebFunds project.
</UL>

<P>
These features were included in the previous release:
<P>
<UL>
  <LI> Tools in <code>cryptix/tools</code>: Scar and UnixCrypt.
  <LI> HMACs for all message digests.
  <LI> Cryptix 2.2 compatibility with message digests, modes, ciphers.
  <LI> <a href="guide/cryptix/Support.html">Supported Algorithms</a>.
  <LI> Bugs fixed: RSA ArrayIndexOutOfBoundsException, Triple DES compatibility.
</UL>
<P>
"

H1_not="
<H1><a name="not">What's *not* here</a></H1>

<UL> 
  <LI> compatibility with JCE 1.2

  </LI> <P> <LI> ElGamal encryption is incomplete
                 (the tests are disabled), and
                 whilst ElGamal signing is working and has a
                 test, it is not really audited.  You should
                 use DSA in preference, which is faster and
                 more reliable.

  </LI> <P> <LI> IJCE:
      <UL>
        <LI>test devoted to IJCE that can run independant of Cryptix
            (java.security.IJCE_test or some Sun standard name?)
      </UL>

  </LI> <P> <LI> testing:
      <UL>
        <LI>Some of the Test* programs do not correctly exit,
          so failures are not detected.
        <LI>
          <code>util/testjar.sh</code> produces guess-code W95/NT batch
          scripts in <code>auto/testjar.bat</code>.  This is
          reported to work, but is not reliable.
      </UL>


  </LI> <P> <LI> Native libraries are not currently included.

  </LI> <P> <LI> OpenPGP is <b>not</b> here at all

  </LI> <P> <LI> Cryptix has only been compiled and run with
    Sun-sourced compilers and JVMs.

  </LI> <P> <LI> Javadoc is distributed without images.

</UL>
<P>
<a href=\"mailto:bugs@cryptix.org?subject=my priorities\">Mail us</a> with your priorities.
"




###########################################################
#
#   Get the files list of what makes up this distribution.
#
read_manifest()
{
    file=$1; shift
    comment=$1; shift                         # append to this file any * lines
    ${vvv} &&                        echo "        manifest ${file}"
    ${vvv} && [ -n "${comment}" ] && echo "        commentF ${comment}"
    
    failed=false
    depends_failed=false
    files_failed=false
    local mm_others=
    local mm_javas=
    local mm_class=
    local mm_all=
    local mm_mans=

    #
    #  Read the manifest - passive, no actions.
    #
    while read line
    do
        ${vvvv} && echo "     . . . . . . . . . . . . . . . . . ${line}"
        case "${line}" in
        ""|"#"*)  ;;                          # ignore empties and comments
    
        depends=*)
            depends=${line#depends=}
            ${vvv} && echo "      check depends ${depends} are there"
            if [ -n "${depends}" ]
            then
                for dep in ${depends}
                do
                    dep2=$( ls -d ${PWD}/${dep}*.jar 2>&- || ls -d ${tgt_class_dir}/${dep}*.jar )
                    ${vvv} && echo "      ${dep2} ?"
                    if [ ! -f "${dep2}" ]
                    then
                        depends_failed=true
                        echo "depends on lost JAR: ${dep2}"
                    else
                        ${vv} && echo "      found ${dep2}"
                    fi
                    export CLASSPATH=${CLASSPATH}:${dep2}
                done
            fi
            ;;

       "*"*)                                  # a comment for a JAR header
           if [ -n "${comment}" ]
           then
               echo " ${line}"  >> ${comment}
           else
               ${vvv} && echo "      ignored: ${line}"
           fi
           ;;

        *=*)                                  # variable=value
            var=${line%%=*}                   # split manually to avoid quotes
            val=${line#*=}
            eval ${var}='"'${val}'"'
            ${vvv} && eval echo '"   "' \$${var} \= \" \$${val} \"
            ;;
    
        -[fd]" "*)                            # test existance of a file or dir
            typ=${line% *}
            nam=${line#-? }
            if eval "test ! ${typ} ${nam}"
            then
                ## echo "not present: ${nam}"
                ## failed=true
                #
                #  Hack to ignore .class check as compilation will pick it up
                #  Really need to turn off the check when the JAR is found.
                #
                case "${nam}" in
                  *.class)
                     echo "  warning: not found: ${nam}, maybe in CLASSPATH"
                     ;;
                  *)
                     echo "Error: not present: $nam"
                     compile=false
                     failed=true
                     files_failed=true
                     ;;
                esac

            else
                ${vv} && echo "    found ${nam}"
            fi
            ;;
    
        -" "*)                                # delete this one
            deletes="${deletes} ${line#- }"
            ${vv} && echo "    not ${line}"
            ;;

#       */"*")                                # recursively add these
#           dir=${line%/"*"}
#           ${vv} && echo "    all of ${dir}"
#           others="${others} $(find ${dir} -print)"
#           ;;

        *)                                    # hopefully a filename
            #
            #  Preprocess the line to expand <varname> if it is found,
            #  where varname must be set earlier by "varname=..."
            #  This enables a simple list method for repetition.
            #  Note that files must be of same type, e.g., all *.java.
            #
            start='<'                         # start symbol and
            end='>'                           # end symbol deliniate ${var}
            line2=""
            case "${line}" in
              *${start}*${end}*)              # found <varname>
                  var="${line#*${start}}"
                  begin="${line%${start}*}"
                  var2="${var%${end}*}"       # var2=varname
                  end="${line#*${end}}"
                  eval list='"$'${var2}'"'    # could be multi-word
                  if [ -z "${list}" ]
                  then
                      echo "Error: list variable '${var2}' invoked but not set"
                      exit 1
                  fi
                  for i in ${list}
                  do
                      full=${begin}${i}${end}
                      line2="${line2} ${full}"
                  done
                  line="${line2}"             # all <varname>s are now from list
                  ;;
            esac

            case "${line}" in
            *.java)                           # a Java source file
                mm_javas="${mm_javas} ${line}"
                ${vvv} && echo "    java ${line}"
                ;;
            *.class)                          # a Java class file
                mm_class="${mm_class} ${line}"
                ${vvv} && echo "  + class ${line}"
                ;;
            *.man)                            # a manifest to build
                mm_mans="${mm_mans} ${line}"
                ${vvv} && echo "    man ${line}"
                ;;
            *)                                # some other file to copy
                mm_others="${mm_others} ${line}"
                ${vvv} && echo "    others ${line}"
                ;;
            esac
            mm_all="${mm_all} ${line}"
            ;;
        esac
    done < ${file}

    if ${failed}
    then
        echo "This directory is not complete."
        return 1
    fi

    m_mans=${mm_mans}
    m_javas=${mm_javas}
    m_class=${mm_class}
    m_others=${mm_others}
    m_all=${mm_all}
}



delete_files()
{
# set -x
    [ -z "$*" ] && return 0

    ${verbose} && echo "  removing files from ${pwd}"
    # be real careful with this step!!!
    for i in $*
    do
        case $i in
          /*|*..*)       echo "No Way! deletes=$i" ; exit 1 ;;
          */)            rm -r ${i} ;;
          *)             rm    ${i} ;;   
#         */)            ${vv} && echo "    rm -r ${i}";; # rm -r ${i} ;;
#         *)             ${vv} && echo "    rm    ${i}";; # rm    ${i} ;;   
        esac
    done
set +x
}


# this is a bad idea.  If java has a problem, it should be sorted out
# externally, in a script or symlink.
# javac_one=false
# 
# if [ -n "${javac}" ]
# then
#   case $(uname -n) in
#     hayek*)    javac=javac_guava; javac_one=true ;;
#     noddy*)    javac=javac ;;
#     *)         javac=javac ;;
#   esac
# fi
#        
# 
# test_javac()
# {
#     if ${javac_one}
#     then 
# 	for jjj in $*
# 	do
#             ${javac} $*
# 	done
#     else
#         ${javac} $*
#     fi
# }
