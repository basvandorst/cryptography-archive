#!/bin/csh -f -e
#
#  SecuDE Release 4.3 (GMD)
#
# inst_pse - installs a PSE which was previously generated with gen_pse

umask 0077

set cadir=/home/gmdca/.ca

cd
if ( "$1" != "" ) then
        set userpse=$1
        set dirr=`/usr/local/secude/bin/decode -r <"$userpse" | uncompress | tar tf - | line`
else
#        echo Error: file argument missing
#        exit
        set userpse=$cadir/genpse/$LOGNAME
        set dirr=`/usr/local/secude/bin/decode -r <"$userpse" | uncompress | tar tf - | line`
endif
set dir=`/usr/local/gnu/bin/basename $dirr`
echo $dir
if ( -f $dir || -d $dir ) then
#        echo $dir exists already in the home directory
#        echo -n Overwrite \? \(y/n\)': '
#        set answer=$<
#        if ( "$answer" != "y" ) then
#                echo $dir unchanged
#                exit
#        endif
#        rm -rf $dir
         mv $dir $dir.BAK
endif

/usr/local/secude/bin/decode -r <"$userpse" | uncompress | ( cd ~; tar xvf - )
/usr/local/secude/bin/psemaint write PKList $cadir/pklist 

echo PSE $dir installed in your home directory. 
#echo Now change the transport PIN to your personal PIN
#psemaint -p $dir challpin
