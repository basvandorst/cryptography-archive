#!/bin/sh
set -e
#
# SecuDE Release 4.3 (GMD) 
#

helptext="\nSecuDE-4.3  \t\t\t\t\t\t                (c)1993 GMD Darmstadt \n
\ncreate_TestTree \t Create a test certification tree and test users \n
\nusage: \n
\ncreate_TestTree [-v] [-p] [-D] [-z] [-q] \n
\nwith: \n
\n-v \t	  verbose
\n-p \t	  use PEM RFC 1424 certification procedures,
\n   \t   otherwise use KM utilities for certification
\n-D \t	  enter certificates into Directory (X.500 or .af-db),
\n-z \t   Enable checking of malloc/free behaviour
\n-q \t   create PSEs with separate key pairs for SIGNATURE/ENCRYPTION purposes,
\n   \t   otherwise create 'one key pair' PSEs\n
\nThis shell-script creates, for test purposes, the following tree of
\ncertification authorities and users: \n
\n \t\t                          rootca
\n \t                         /     \t\t\t       \ 
\n \t            org1         \t\t\t            org2
\n      \t             /   \     \t\t\t              /        \ 
\n               org1usr1   org1usr2  \t org2usr1  org2usr2 \n
\n
\nIt creates the directory TestTree under the home directory and the following directories
\nunder TestTree: \n

\nrootca \t  CA directory of Root-CA <C=DE, O=Root-CA>, CA-PSE is .capse, PIN = test
\norg1 \t    CA directory of org1-CA <C=DE, O=ORG-1> under Root-CA, CA-PSE is .capse, PIN = test
\norg2 \t    CA directory of org2-CA <C=DE, O=ORG-2> under Root-CA, CA-PSE is .capse, PIN = test
\norg1usr1   PSE of user <C=DE, O=ORG-1, CN=User-1> under org1-CA, PIN = test
\norg1usr2   PSE of user <C=DE, O=ORG-1, CN=User-2> under org1-CA, PIN = test
\norg2usr1   PSE of user <C=DE, O=ORG-2, CN=User-1> under org2-CA, PIN = test
\norg2usr2   PSE of user <C=DE, O=ORG-2, CN=User-2> under org2-CA, PIN = test\n
\ncreate_test_tree needs about 100 sec on a Sun SPARC10-41\n"

q=

for par do case $par in
        -P)
                p=pem;;
        -D)
		D=$par;;
        -v)
		v=$par;;
	-q)
		q=$par;;
	-z)
		t=$par;;
        -h)
                echo $helptext
                exit;;
esac done

cd
mkdir TestTree
cd TestTree
HOME=`pwd`
export HOME

CAPIN=test
USERPIN=test
export USERPIN CAPIN

echo ""
echo "******* Create Root-CA *******:"
echo "Create prototype CA <C=DE, O=Root-CA>"
echo "CA directory is rootca"
cacreate $v $q $t -c rootca "C=DE, O=Root-CA"
echo "done."
if [ "$p" != "pem" ]
then
	getpkroot -c rootca $v > PKRoot
fi



echo ""
echo "******* Create ORG-1 CA under Root-CA *******:"
echo "Create prototype CA <C=DE, O=ORG-1>"
echo "CA directory is org1"
cacreate $v $q $t $D-c org1 "C=DE, O=ORG-1"
echo "done."
if [ "$p" = "pem" ]
then
	echo "Certification Request" > certreq
	echo "Create PEM certification request of <C=DE, O=ORG-1>"
	pem mic-clear $v $t -C -i certreq -o org1.pem.proto -c org1
	echo "done."
	echo "Create PEM certification reply by <C=DE, O=Root-CA>"
	pem certify $v $t -i org1.pem.proto -o org1.pem.cert -c rootca
	echo "Install PEM certification reply for <C=DE, O=ORG-1>"
	pem $v $t $D -i org1.pem.cert -o certreply -c org1 -u yes
	cmp certreq certreply
	rm certreq certreply
else 
	echo "Install PKRoot in PSE of <C=DE, O=ORG-1>"
	instpkroot -c org1 $v < PKRoot
	echo "done."
	echo "Create prototype certificate of <C=DE, O=ORG-1>"
	getkey -c org1 -s $v > org1.sign.proto
	echo "done."
	echo "Sign certificate of <C=DE, O=ORG-1> by <C=DE, O=Root-CA>"
	certify -c rootca $v < org1.sign.proto > org1.sign.cert
	echo "done."
	echo "Install certificate in PSE of <C=DE, O=ORG-1>"
	instcert -c org1 $v -H < org1.sign.cert
	echo "done."
	getfcpath -c org1 $v > org1.FCPath
fi
if [ "$q" = "-q" ] 
then
	echo "Same for encryption certificate of <C=DE, O=ORG-1>"
	getkey -c org1 -e $v > org1.encr.proto
	certify -c rootca $v < org1.encr.proto > org1.encr.cert
	instcert $D -c org1 $v -He < org1.encr.cert
	echo "done."
fi
echo "PSE of CA <C=DE, O=ORG-1> O.K."
rm -f *.proto *.cert



echo ""
echo "******* Create ORG-2 CA under Root-CA *******:"
echo "Create prototype CA <C=DE, O=ORG-2>"
echo "CA directory is org2"
cacreate $v $q $t -c org2 "C=DE, O=ORG-2"
echo "done."
if [ "$p" = "pem" ]
then
	echo "Certification Request" > certreq
	echo "Create PEM certification request of <C=DE, O=ORG-2>"
	pem mic-clear $v $t -C -i certreq -o org2.pem.proto -c org2
	echo "done."
	echo "Create PEM certification reply by <C=DE, O=Root-CA>"
	pem certify $v $t -i org2.pem.proto -o org2.pem.cert -c rootca
	echo "Install PEM certification reply for <C=DE, O=ORG-2>"
	pem $v $t $D -i org2.pem.cert -o certreply -c org2 -u yes
	cmp certreq certreply
	rm certreq certreply
else 
	echo "Install PKRoot in PSE of <C=DE, O=ORG-2>"
	instpkroot -c org2 $v < PKRoot
	echo "done."
	echo "Create prototype certificate of <C=DE, O=ORG-2>"
	getkey -c org2 -s $v > org2.sign.proto
	echo "done."
	echo "Sign certificate of <C=DE, O=ORG-2> by <C=DE, O=Root-CA>"
	certify -c rootca $v < org2.sign.proto > org2.sign.cert
	echo "done."
	echo "Install certificate in PSE of <C=DE, O=ORG-2>"
	instcert $D -c org2 $v -H < org2.sign.cert
	echo "done."
	getfcpath -c org2 $v > org2.FCPath
fi
if [ "$q" = "-q" ] 
then
	echo "Same for encryption certificate of <C=DE, O=ORG-2>"
	getkey -c org2 -e $v > org2.encr.proto
	certify -c rootca $v < org2.encr.proto > org2.encr.cert
	instcert $D -c org2 $v -He < org2.encr.cert
	echo "done."
fi
echo "PSE of CA <C=DE, O=ORG-2> O.K."
rm -f *.proto *.cert



echo ""
echo "******* Create User-1 of ORG-1 *******:"
echo "Create prototype PSE for  <C=DE, O=ORG-1, CN=User-1>"
echo "PSE name is org1usr1"
psecreate -p org1usr1 $v $q $t "C=DE, O=ORG-1, CN=User-1"
echo "done."
if [ "$p" = "pem" ]
then
	echo "Certification Request" > certreq
	echo "Create PEM certification request of <C=DE, O=ORG-1, CN=User-1>"
	pem mic-clear $v $t -C -i certreq -o org1usr1.pem.proto -p org1usr1
	echo "done."
	echo "Create PEM certification reply by <C=DE, O=ORG-1>"
	pem certify $v $t -i org1usr1.pem.proto -o org1usr1.pem.cert -c org1
	echo "Install PEM certification reply for <C=DE, O=ORG-1, CN=User-1>"
	pem $v $t $D -i org1usr1.pem.cert -o certreply -p org1usr1 -u yes
	cmp certreq certreply
	rm certreq certreply
else 
	echo "Install PKRoot in PSE of <C=DE, O=ORG-1, CN=User-1>"
	instpkroot -p org1usr1 $v < PKRoot
	echo "done."
	echo "Install FCPath in PSE of <C=DE, O=ORG-1, CN=User-1>"
	instfcpath -p org1usr1 $v < org1.FCPath
	echo "done."
	echo "Create prototype certificate of <C=DE, O=ORG-1, CN=User-1>"
	getkey -p org1usr1 -s $v > org1usr1.sign.proto
	echo "done."
	echo "Sign certificate of <C=DE, O=ORG-1, CN=User-1> by <C=DE, O=ORG-1>"
	certify -c org1 $v < org1usr1.sign.proto > org1usr1.sign.cert
	echo "done."
	echo "Install certificate in PSE of <C=DE, O=ORG-1, CN=User-1>"
	instcert $D -p org1usr1 $v -H < org1usr1.sign.cert
	echo "done."
fi
if [ "$q" = "-q" ] 
then
	echo "Same for encryption certificate of <C=DE, O=ORG-1, CN=User-1>"
	getkey -p org1usr1 -e $v > org1usr1.encr.proto
	certify -c org1 $v < org1usr1.encr.proto > org1usr1.encr.cert
	instcert $D -p org1usr1 $v -He < org1usr1.encr.cert
	echo "done."
fi
echo "PSE of user <C=DE, O=ORG-1, CN=User-1> O.K."
rm -f *.proto *.cert



echo ""
echo "******* Create User-2 of ORG-1 *******:"
echo "Create prototype PSE for  <C=DE, O=ORG-1, CN=User-2>"
echo "PSE name is org1usr2"
psecreate -p org1usr2 $v $q $t "C=DE, O=ORG-1, CN=User-2"
echo "done."
if [ "$p" = "pem" ]
then
	echo "Certification Request" > certreq
	echo "Create PEM certification request of <C=DE, O=ORG-1, CN=User-2>"
	pem mic-clear $v $t -C -i certreq -o org1usr2.pem.proto -p org1usr2
	echo "done."
	echo "Create PEM certification reply by <C=DE, O=ORG-1>"
	pem certify $v $t -i org1usr2.pem.proto -o org1usr2.pem.cert -c org1
	echo "Install PEM certification reply for <C=DE, O=ORG-1, CN=User-2>"
	pem $v $t $D -i org1usr2.pem.cert -o certreply -p org1usr2 -u yes
	cmp certreq certreply
	rm certreq certreply
else 
	echo "Install PKRoot in PSE of <C=DE, O=ORG-1, CN=User-2>"
	instpkroot -p org1usr2 $v < PKRoot
	echo "done."
	echo "Install FCPath in PSE of <C=DE, O=ORG-1, CN=User-2>"
	instfcpath -p org1usr2 $v < org1.FCPath
	echo "done."
	echo "Create prototype certificate of <C=DE, O=ORG-1, CN=User-2>"
	getkey -p org1usr2 -s $v > org1usr2.sign.proto
	echo "done."
	echo "Sign certificate of <C=DE, O=ORG-1, CN=User-2> by <C=DE, O=ORG-1>"
	certify -c org1 $v < org1usr2.sign.proto > org1usr2.sign.cert
	echo "done."
	echo "Install certificate in PSE of <C=DE, O=ORG-1, CN=User-2>"
	instcert $D -p org1usr2 $v -H < org1usr2.sign.cert
	echo "done."
fi
if [ "$q" = "-q" ] 
then
	echo "Same for encryption certificate of <C=DE, O=ORG-1, CN=User-2>"
	getkey -p org1usr2 -e $v > org1usr2.encr.proto
	certify -c org1 $v < org1usr2.encr.proto > org1usr2.encr.cert
	instcert $D -p org1usr2 $v -He < org1usr2.encr.cert
	echo "done."
fi
echo "PSE of user <C=DE, O=ORG-1, CN=User-2> O.K."
rm -f *.proto *.cert



echo ""
echo "******* Create User-1 of ORG-2 *******:"
echo "Create prototype PSE for  <C=DE, O=ORG-2, CN=User-1>"
echo "PSE name is org2usr1"
psecreate -p org2usr1 $v $q $t "C=DE, O=ORG-2, CN=User-1"
echo "done."
if [ "$p" = "pem" ]
then
	echo "Certification Request" > certreq
	echo "Create PEM certification request of <C=DE, O=ORG-2, CN=User-1>"
	pem mic-clear $v $t -C -i certreq -o org2usr1.pem.proto -p org2usr1
	echo "done."
	echo "Create PEM certification reply by <C=DE, O=ORG-2>"
	pem certify $v $t -i org2usr1.pem.proto -o org2usr1.pem.cert -c org2
	echo "Install PEM certification reply for <C=DE, O=ORG-2, CN=User-1>"
	pem $v $t $D -i org2usr1.pem.cert -o certreply -p org2usr1 -u yes
	cmp certreq certreply
	rm certreq certreply
else 
	echo "Install PKRoot in PSE of <C=DE, O=ORG-2, CN=User-1>"
	instpkroot -p org2usr1 $v < PKRoot
	echo "done."
	echo "Install FCPath in PSE of <C=DE, O=ORG-2, CN=User-1>"
	instfcpath -p org2usr1 $v < org2.FCPath
	echo "done."
	echo "Create prototype certificate of <C=DE, O=ORG-2, CN=User-1>"
	getkey -p org2usr1 -s $v > org2usr1.sign.proto
	echo "done."
	echo "Sign certificate of <C=DE, O=ORG-2, CN=User-1> by <C=DE, O=ORG-2>"
	certify -c org2 $v < org2usr1.sign.proto > org2usr1.sign.cert
	echo "done."
	echo "Install certificate in PSE of <C=DE, O=ORG-2, CN=User-1>"
	instcert $D -p org2usr1 $v -H < org2usr1.sign.cert
	echo "done."
fi
if [ "$q" = "-q" ] 
then
	echo "Same for encryption certificate of <C=DE, O=ORG-2, CN=User-1>"
	getkey -p org2usr1 -e $v > org2usr1.encr.proto
	certify -c org2 $v <org2usr1.encr.proto > org2usr1.encr.cert
	instcert $D -p org2usr1 $v -He < org2usr1.encr.cert
	echo "done."
fi
echo "PSE of user <C=DE, O=ORG-2, CN=User-1> O.K."
rm -f *.proto *.cert



echo ""
echo "******* Create User-2 of ORG-2 *******:"
echo "Create prototype PSE for  <C=DE, O=ORG-2, CN=User-2>"
echo "PSE name is org2usr2"
psecreate -p org2usr2 $v $q $t "C=DE, O=ORG-2, CN=User-2"
echo "done."
if [ "$p" = "pem" ]
then
	echo "Certification Request" > certreq
	echo "Create PEM certification request of <C=DE, O=ORG-2, CN=User-2>"
	pem mic-clear $v $t -C -i certreq -o org2usr2.pem.proto -p org2usr2
	echo "done."
	echo "Create PEM certification reply by <C=DE, O=ORG-2>"
	pem certify $v $t -i org2usr2.pem.proto -o org2usr2.pem.cert -c org2
	echo "Install PEM certification reply for <C=DE, O=ORG-2, CN=User-2>"
	pem $v $t $D -i org2usr2.pem.cert -o certreply -p org2usr2 -u yes
	cmp certreq certreply
	rm certreq certreply
else 
	echo "Install PKRoot in PSE of <C=DE, O=ORG-2, CN=User-2>"
	instpkroot -p org2usr2 $v < PKRoot
	echo "done."
	echo "Install FCPath in PSE of <C=DE, O=ORG-2, CN=User-2>"
	instfcpath -p org2usr2 $v < org2.FCPath
	echo "done."
	echo "Create prototype certificate of <C=DE, O=ORG-2, CN=User-2>"
	getkey -p org2usr2 -s $v > org2usr2.sign.proto
	echo "done."
	echo "Sign certificate of <C=DE, O=ORG-2, CN=User-2> by <C=DE, O=ORG-2>"
	certify -c org2 $v < org2usr2.sign.proto > org2usr2.sign.cert
	echo "done."
	echo "Install certificate in PSE of <C=DE, O=ORG-2, CN=User-2>"
	instcert $D -p org2usr2 $v -H < org2usr2.sign.cert
	echo "done."
fi
if [ "$q" = "-q" ] 
then
	echo "Same for encryption certificate of <C=DE, O=ORG-2, CN=User-2>"
	getkey -p org2usr2 -e $v > org2usr2.encr.proto
	certify -c org2 $v < org2usr2.encr.proto > org2usr2.encr.cert
	instcert $D -p org2usr2 $v -He < org2usr2.encr.cert
	echo "done."
fi
echo "PSE of user <C=DE, O=ORG-2, CN=User-2> O.K."
rm -f *.proto *.cert *FCPath PKRoot
