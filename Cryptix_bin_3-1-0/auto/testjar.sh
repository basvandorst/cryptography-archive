#!/bin/sh
# testjar.sh written automagically by testjar.sh
# ${jopt:=}     # local java options
: ${java:=java} # local JVM

good=false
bad=false
# exit with one of 4 codes, depending on results
trap 'if ${good}; then if ${bad}; then exit 4; else exit 0; fi; else if ${bad}; then exit 1; else exit 3; fi; fi' 0

PWD=`pwd`       # some shells have no PWD

OK_exit()
{
  if [ "$#" -lt 1 ]; then
    echo "internal error: Usage: OK_exit <class> [<exit code>, def. 10]" 1>&2
    return 1
  elif [ "$#" = 1 ]; then
#
# Default exit code is 10
#
    valid_exit=10
  else
    valid_exit="$2"
  fi

  ${java} ${jopt} $1

  if [ "$?" = "${valid_exit}" ]; then
    return 0
  else
    return -1
  fi
}

# good tests for ./SPT_0-1-0.jar - bad tests commented out
CLASSPATH=:./IJCE_0-1-0.jar:./SPT_0-1-0.jar:${PWD}; export CLASSPATH
if OK_exit cryptix.provider.Install 10; then good=true; else exit 1; fi
if OK_exit cryptix.test.TestBR 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.Test3LFSR 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestBase64Stream 10; then good=true; else bad=true; fi
# end ./SPT_0-1-0.jar

# good tests for ./ALG_0-1-0.jar - bad tests commented out
CLASSPATH=:./SPT_0-1-0.jar:./IJCE_0-1-0.jar:./ALG_0-1-0.jar:${PWD}; export CLASSPATH
if OK_exit cryptix.test.TestIJCE 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestDES 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestDES_EDE3 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestBlowfish 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestCAST5 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestIDEA 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestLOKI91 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestRC2 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestRC4 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestSAFER 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestSquare 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestSPEED 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestHMAC 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestMD2 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestMD4 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestRIPEMD128 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestRIPEMD160 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestHAVAL 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestSHA1 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestSHA0 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestMD5 10; then good=true; else bad=true; fi
# end ./ALG_0-1-0.jar

# good tests for ./RSA_0-1-0.jar - bad tests commented out
CLASSPATH=:./SPT_0-1-0.jar:./IJCE_0-1-0.jar:./ALG_0-1-0.jar:./RSA_0-1-0.jar:${PWD}; export CLASSPATH
if OK_exit cryptix.test.TestRSA 10; then good=true; else bad=true; fi
# end ./RSA_0-1-0.jar

# good tests for ./ELGAMAL_0-1-0.jar - bad tests commented out
CLASSPATH=:./SPT_0-1-0.jar:./IJCE_0-1-0.jar:./ALG_0-1-0.jar:./ELGAMAL_0-1-0.jar:${PWD}; export CLASSPATH
if OK_exit cryptix.test.TestElGamal 10; then good=true; else bad=true; fi
# end ./ELGAMAL_0-1-0.jar

# good tests for ./OLD_0-1-4.jar - bad tests commented out
CLASSPATH=:./SPT_0-1-0.jar:./IJCE_0-1-0.jar:./ALG_0-1-0.jar:./OLD_0-1-4.jar:${PWD}; export CLASSPATH
if OK_exit cryptix.security.Blowfish 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.IDEA 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.SPEED 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.DES 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.MD5 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.SHA 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.SHA0 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.HashMD5 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.HashSHA0 0; then good=true; else bad=true; fi
if OK_exit cryptix.security.HashSHA 0; then good=true; else bad=true; fi
# end ./OLD_0-1-4.jar

# good tests for ./TOOLS_0-1-0.jar - bad tests commented out
CLASSPATH=:./SPT_0-1-0.jar:./IJCE_0-1-0.jar:./ALG_0-1-0.jar:./TOOLS_0-1-0.jar:${PWD}; export CLASSPATH
if OK_exit cryptix.test.TestUnixCrypt 10; then good=true; else bad=true; fi
if OK_exit cryptix.test.TestScar 10; then good=true; else bad=true; fi
# end ./TOOLS_0-1-0.jar

# good tests for ./PGP26_0-1-0.jar - bad tests commented out
CLASSPATH=:./OLD_0-1-4.jar:./RSA_0-1-0.jar:./SPT_0-1-0.jar:./PGP26_0-1-0.jar:${PWD}; export CLASSPATH
if OK_exit cryptix.test.TestPGP 10; then good=true; else bad=true; fi
# end ./PGP26_0-1-0.jar

# end testjar.sh
