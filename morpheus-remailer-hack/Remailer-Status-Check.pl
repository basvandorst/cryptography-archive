#!/usr/bin/perl

# I'm alive!

while (<>) {
	s/[ \t\r]*$// ;
        last if /^$/ ;
        if (/^Remailer-Status-Check:/) {
                chop ;
                s/^.*:// ;
                $mid = $_ ;
        }
        if (/^From:/) {
                chop ;
                s/^.*:// ;
                $addressee = $_ ;
        }
}

$retaddr = " " . $ENV{'LOGNAME'} . "@" . $ENV{'VISIBLE_NAME'};

#open( OUTPUT, ">bar" ) || die "Cannot open 'bar'." ;
open( OUTPUT, "| /usr/lib/sendmail " . $addressee ) ;
select( OUTPUT ) ;

print "To:" . $addressee . "\n" ;
print "From:" . $retaddr . "\n" ;
print "Remailer-Status-Alive:" . $mid . "\n";

print "\n";

print "Remailer alive request by" . $addressee . " answered\n\n";

close(OUTPUT);
