# Usage:
#  pgp-auto ... pgp options ...
# Supplies passphrase (from pixie) to PGP.

# This file is COPYRIGHT - see notice above or file COPYRIGHT
# in this directory.

require 'sys/socket.ph';

$path= "$config_wrapperdir/$config_socketname";

unless (defined($ENV{'PGPPASS'})) {
    socket(S, &AF_UNIX, &SOCK_STREAM, 0)
        || die "$0: create unix-domain socket: $!\n";
    connect(S,pack('S',&AF_UNIX).$path)
        || die "$0: connect to pass-socket for passphrase: $!\n";
    $ENV{'PGPPASSFD'}= fileno(S);
}

exec $config_x_normalpgp,@ARGV;
die "$0: exec $config_x_normalpgp: $!\n";
