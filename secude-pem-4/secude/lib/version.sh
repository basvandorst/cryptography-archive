: run this script through /bin/sh

: this hacks past a bug in make...
exec 3<&- 4<&- 5<&- 6<&- 7<&-

OFS="$IFS" IFS=:

if [ "x$1" = x ]; then echo 'usage: version.sh module' 1>&2; exit 1; fi

for A in rprompt hostname uname who
do
    for D in $PATH
    do
	if [ ! -f $D/$A ]; then
	    continue
	fi
	case $A in
	    rprompt)    LOCAL=`$A %h`
			;;
	    hostname)   LOCAL=`$A`
			;;
	    uname)	LOCAL=`$A -n`
			;;
	    who)	LOCAL=`$A am i | sed -e 's%^\(.*\)!.*$%\1%'`
			;;
	esac
	break
    done
    if [ "x$LOCAL" != x ]; then
	break
    fi
done

IFS=

if [ ! -r version.major ]; then echo 4 > version.major; fi
if [ ! -r version.minor ]; then echo 3 > version.minor; fi
if [ ! -r version.local ]; then echo 0 > version.local; fi
echo `cat version.major` `cat version.minor` `cat version.local` $1 $2 > version
rm -f version.c version.local

awk -f version.awk < version
echo '('$LOCAL') of '`date`'";'

rm -f version
