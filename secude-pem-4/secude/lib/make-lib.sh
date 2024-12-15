: run this script through /bin/sh

L= O= S= Q= SHD= MAJ= MIN= LD=ld

while [ $# -gt 0 ]
do
    A="$1"
    case $A in
	-major) MAJ="$2"
		shift
		;;
	-minor) MIN="$2"
		shift
		;;
	-options) SL_OPTIONS="$2"
		shift
		;;
	-appendix) SL_APPENDIX="$2"
		shift
		;;
    esac
    shift
done


if [ "x$MAJ" = x -o "x$MIN" = x ]; then
	echo "Missing major or minor number for library" 1>&2
	exit 1
fi

LSO="`echo libsecude | sed 's%.a$%%'`"$SL_APPENDIX.$MAJ.$MIN
echo $LD -o "$LSO" $SL_OPTIONS *.o
$LD -o "$LSO" $SL_OPTIONS *.o
rm -f libsecude$SL_APPENDIX
ln -s "$LSO" libsecude$SL_APPENDIX

exit 0
