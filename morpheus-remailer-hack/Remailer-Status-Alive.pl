#!/usr/bin/perl

# next remailer is fine, continue with message

while (<>) {
	s/[ \t\r]*$// ;
        last if /^$/ ;
        if (/^Remailer-Status-Alive:/) {
                chop ;
                s/^.*: // ;
                $spoolid = $_ ;
        }
}

open (SPOOL, "spool/$spoolid");

open( OUTPUT, "| slocal.pl " . $addressee );

#open (OUTPUT, ">/tmp/yak");

select( OUTPUT ) ;

while (<SPOOL>) 
{
print;
}

close(OUTPUT);

unlink ( "spool/$spoolid");
