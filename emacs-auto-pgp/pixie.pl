# Passphrase server.
# Deliberately very simple so you can see what's going on.
# This one needs to be left running in a terminal window.
# An X version would be nice.

# This file is COPYRIGHT - see notice above or file COPYRIGHT
# in this directory.

require 'sys/socket.ph';

$0 =~ m|[^/]+$|; $name= $&;

# Check for spurious arguments
@ARGV && die "$name: usage: $name\n";

# Change into the right directory, creating it if necessary.
if (!chdir($config_wrapperdir)) {
    warn "$name: $config_wrapperdir: $! -- now creating it\n";
    mkdir($config_wrapperdir,0700) || die "$name: creating $config_wrapperdir: $!\n";
    chdir($config_wrapperdir) || die "$name: changing to $config_wrapperdir: $!\n";
}

# Make sure it's not accessible by others
@s= stat('.');
@s || die "stat $config_wrapperdir: $!\n";
($s[2] & 0777) == 0700 || die "$name: $config_wrapperdir must be mode 700 (rwx------)";

unlink($config_socketname);
socket(S,&PF_UNIX, &SOCK_STREAM, 0) || die "creating socket: $!\n";
bind(S,pack('S',&AF_UNIX).$config_socketname) || die "bind to $config_socketname: $!\n";
listen(S,5) || die "listening: $!\n";

system('stty','-echo') && die "stty -echo gave $?\n";

print STDERR "OK, type in your passphrase: ";

$passphrase= <STDIN>;

system('stty','echo') && die "stty echo gave $?\n";

print STDERR <<'END';

Thanks. If you think you got it wrong, just kill me and try again.

Achtung! Ensure this program dies when you log out, if not sooner!
END

for(;;) {
    defined($t=accept(NS,S)) || die "accept: $!\n";
    print NS $passphrase;
    $_=`date`; chop;
    print("$_: $name supplied passphrase\n") ||
        die "Failed to write log message to stdout: $!\n";
    close(NS);
}

exit 0;
