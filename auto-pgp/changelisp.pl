# -* perl -*- script for editing lisp - as filter

# This file is COPYRIGHT - see the file COPYRIGHT
# in this directory.

require 'EDITME';

while(<>) { print; last if m/^/; }

%map= ('autopgp-normal-program',            $config_x_normalpgp,
       'autopgp-sign-program',              $config_x_pgpauto,
       'autopgp-decrypt-filter-program',    $config_x_decrypt);

for $f (keys %map) { $map{$f} =~ s/\W/\\$&/g; }

while(<>) {
    s/\(provide '.*\)/(provide '$config_lisp)/;
    s/\(defvar (\S+) ".*"/(defvar $1 "$map{$1}"/;
    print;
    last if m/^/;
}

while(<>) { print; }

exit 0;
