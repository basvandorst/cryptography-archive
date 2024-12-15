#!/usr/local/bin/perl

$sendmail = @ARGV[0];
$addressee = @ARGV[1];
$sleeptime = @ARGV[2];
$message = @ARGV[3];

open(MSG, "$message");
@mesg = <MSG>;
close(MSG);
unlink("$message");
$text = join("", @mesg);
undef @mesg;

sleep $sleeptime;
open(SENDMAIL, "|$sendmail $addressee");
print SENDMAIL "$text\n";
close(SENDMAIL);

