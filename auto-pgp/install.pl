# -* perl -*- script for installation
# args: actual-filename notional-thingumname type

# This file is COPYRIGHT - see the file COPYRIGHT
# in this directory.

require 'EDITME';

$actual= $ARGV[0];
eval '$destn= $config_i_'.$ARGV[1];
eval '$user= $config_'.$ARGV[2].'user';
eval '$group= $config_'.$ARGV[2].'group';
eval '$mode= $config_'.$ARGV[2].'mode';

if ($ARGV[2] eq 'man') {
    eval '$_= $config_l_'.$ARGV[1];
    $destn= "$config_i_mandir/$_.$config_i_manext";
}

$it= <<END
    cp $actual $destn && \\
    (chown $user $destn; \\
     chgrp $group $destn; \\
     true) && \\
    chmod $mode $destn
END
    ;

print $it;

exec $it;
