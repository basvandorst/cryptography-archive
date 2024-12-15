#!/bin/sh
#
# Make secret and public RSA key files.
#
# This code is written to minimise the time the secret information
# is stored anywhere other than in the secret file. To this end:
#
#	- The secret and public files are opened and truncated before
#	  any key generation takes place.
#	- The umask is set to 077 after creation of the secret and public files
#	  so that the temporary file is not accidentally accessible.
#	- The temporary file from which the secret and public files
#	  are derived is made in the current directory rather than
#	  in /tmp.
#	- The temporary is removed once it has been opened for copying to
#	  the secret and public files, but before the copies.
#	- The key information is never present on the command line.
#
# Author: Cameron Simpson <cameron@cse.unsw.edu.au>, April 1993
#

cmd=`basename "$0"`
len=10
prob=20
secret=${RSASECRET-$HOME/.rsasecret}
public=${RSASECRET-$HOME/.rsapublic}
usage="Usage: $cmd [-l length] [-P prob] [-s secret] [-p public]
	-l length	Specify length of base primes used to make keys.
			Default: $len.
	-P prob		Specify probability that base primes are not really
			prime is to be 1 in 2^prob. Default: prob=$prob.
	-s secret	Specify file to hold secret keys.
			Default: $secret
	-p public	Specify file to hold public keys.
			Default: $public"

badopts=
while :
do
    case $1 in
	-l)	len=$2; shift ;;
	-P)	prob=$2; shift ;;
	-s)	secret=$2; shift ;;
	-p)	public=$2; shift ;;
	--)	shift; break ;;
	-*)	echo "$cmd: unrecognised option \"$1\"" >&2
		badopts=1
		shift
		;;
	*)	break ;;
    esac
done

case $# in
    0)	;;
    *)	echo "$cmd: extra arguments: $*" >&2
	badopts=1
	;;
esac

if [ $badopts ]
then
    echo "$usage" >&2
    exit 2
fi

# open output files
exec 3>"$secret" 4>"$public"
[ -w "$secret" ] || { echo "$cmd: secret file \"$secret\" is not writable"; exit 1; }
[ -w "$public" ] || { echo "$cmd: public file \"$public\" is not writable"; exit 1; }

umask 077	# so temporary isn't accidentally public
tmp=.rsatmp$$
trap 'rm -f "$tmp"; exit 1' 1 2 3 15
exec >"$tmp"
chmod 600 "$tmp"
[ -r "$tmp" ] || { echo "$cmd: temporary file \"$tmp\" is not readable"; exit 1; }

len1=`expr $len + 1`

chmod 600 "$secret" || echo "$cmd: warning: can't chmod 0600 secret file" >&2

( genprim $len $prob	# make base prime 1
  echo '#'
  genprim $len1 $prob	# make base prime 2
) | genrsa >"$tmp"

exec 5<"$tmp" 6<"$tmp"
rm -f "$tmp"

sed 2,3d <&5 >&3; exec 3>&- 5>&-	# make public file
sed 4,5d <&6 >&4; exec 4>&- 6>&-	# make secret file

exit 0
