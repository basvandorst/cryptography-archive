#!/usr/bin/perl

# Check the status of the next remailer in the chain 
# obviously I'm not much of a perl hacker - morpheus@entropy.linet.org

$spooldir = "spool";
$mid = $ENV{'MESSAGE_ID'};

@msg = <STDIN>;

open(SPOOL, ">$spooldir/$mid") || open (SPOOL, ">failed.$$") || die "Cannot open spool file." ;

while (@msg) {
	$_ = shift(@msg);
	s/^Request-Safe-Remailing-To:/Request-Remailing-To:/;
	print SPOOL $_;
	s/[ \t\r]*$// ;
        last if /^$/ ;
        if (/^Request-Remailing-To:/) {
                chop ;
                s/^.*:// ;
                $addressee = $_ ;
		s/.*//;
        }
}

$retaddr = " " . $ENV{'LOGNAME'} . "@" . $ENV{'VISIBLE_NAME'};

#open( OUTPUT, ">foo" ) || die "Cannot open 'foo'." ;
open( OUTPUT, "| /usr/lib/sendmail " . $addressee ) ;
select( OUTPUT ) ;

print "To:" . $addressee . "\n" ;
print "From:" . $retaddr . "\n" ;
print "Remailer-Status-Check: " . $mid . "\n";

print "\n";

print "Remailer status requested by " . $ENV{'USERNAME'} . "\n\n";

close(OUTPUT);

print SPOOL "\n";
while(@msg) {
print SPOOL shift(@msg);
}

close(SPOOL);
