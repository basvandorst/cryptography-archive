#!/bin/sh
# testjar.sh written automagically by testjar.sh
# ${jopt:=}     # local java options
: ${java:=java} # local JVM

good=false
bad=false
# exit with one of 4 codes, depending on results
trap 'if ${good}; then if ${bad}; then exit 4; else exit 0; fi; else if ${bad}; then exit 1; else exit 3; fi; fi' 0

PWD=`pwd`       # some shells have no PWD

# good tests for ./SPT_0-0-9.jar - bad tests commented out
CLASSPATH=:./IJCE_0-0-11.jar:./SPT_0-0-9.jar:${PWD}; export CLASSPATH
if ${java} ${jopt} cryptix.provider.Install; then good=true; else exit 1; fi
if ${java} ${jopt} cryptix.test.TestBR; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.Test3LFSR; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestBase64Stream; then good=true; else bad=true; fi
# end ./SPT_0-0-9.jar

# good tests for ./ALG_0-0-9.jar - bad tests commented out
CLASSPATH=:./SPT_0-0-9.jar:./IJCE_0-0-11.jar:./ALG_0-0-9.jar:${PWD}; export CLASSPATH
if ${java} ${jopt} cryptix.test.TestIJCE; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestDES; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestDES_EDE3; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestBlowfish; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestCAST5; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestIDEA; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestLOKI91; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestRC2; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestRC4; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestSAFER; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestSquare; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestSPEED; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestHMAC; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestMD2; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestMD4; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestRIPEMD128; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestRIPEMD160; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestHAVAL; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestSHA1; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestSHA0; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.test.TestMD5; then good=true; else bad=true; fi
# end ./ALG_0-0-9.jar

# good tests for ./RSA_0-0-9.jar - bad tests commented out
CLASSPATH=:./SPT_0-0-9.jar:./IJCE_0-0-11.jar:./ALG_0-0-9.jar:./RSA_0-0-9.jar:${PWD}; export CLASSPATH
if ${java} ${jopt} cryptix.test.TestRSA; then good=true; else bad=true; fi
# end ./RSA_0-0-9.jar

# good tests for ./ELGAMAL_0-0-2.jar - bad tests commented out
CLASSPATH=:./SPT_0-0-9.jar:./IJCE_0-0-11.jar:./ALG_0-0-9.jar:./ELGAMAL_0-0-2.jar:${PWD}; export CLASSPATH
if ${java} ${jopt} cryptix.test.TestElGamal; then good=true; else bad=true; fi
# end ./ELGAMAL_0-0-2.jar

# good tests for ./OLD_0-0-4.jar - bad tests commented out
CLASSPATH=:./SPT_0-0-9.jar:./IJCE_0-0-11.jar:./ALG_0-0-9.jar:./OLD_0-0-4.jar:${PWD}; export CLASSPATH
if ${java} ${jopt} cryptix.security.Blowfish; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.IDEA; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.SPEED; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.DES; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.MD5; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.SHA; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.SHA0; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.HashMD5; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.HashSHA0; then good=true; else bad=true; fi
if ${java} ${jopt} cryptix.security.HashSHA; then good=true; else bad=true; fi
# end ./OLD_0-0-4.jar

# good tests for ./TOOLS_X-Y-Z.jar - bad tests commented out
CLASSPATH=:./SPT_0-0-9.jar:./IJCE_0-0-11.jar:./ALG_0-0-9.jar:./TOOLS_X-Y-Z.jar:${PWD}; export CLASSPATH
if ${java} ${jopt} cryptix.tools.UnixCrypt; then good=true; else bad=true; fi
# end ./TOOLS_X-Y-Z.jar

# end testjar.sh
