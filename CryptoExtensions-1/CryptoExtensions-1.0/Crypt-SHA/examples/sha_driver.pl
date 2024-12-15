#!/usr/local/bin/perl -w

require 'getopts.pl';
use Crypt::SHA;

sub do_test
{
    my ($label, $str, $expect) = @_;
    my ($c, @tmp);
    my $sha = new SHA;
    $sha->add($str);
    print "$label:\nEXPECT:   $expect\n";
    print "RESULT 1: " . $sha->hexdigest() . "\n";
# comment out the following line if you run out of memory
    $run_big_test = 1;
    if (defined($run_big_test)) {
	$sha->reset();
	@tmp = split(//, $str);
	foreach $c (@tmp) {
	    $sha->add($c);
	}
	print "RESULT 2: " . $sha->hexdigest() . "\n";
    }
}

&Getopts('s:x');

$sha = new SHA;

if (defined($opt_s)) {
    $sha->add($opt_s);
    print("SHA(\"$opt_s\") = " . $sha->hexdigest() . "\n");
} elsif ($opt_x) {
    print "If the following results don't match, check that you have\ncorrectly set \"LITTLE_ENDIAN\" in sha_func.c, and that\n\"USE_MODIFIED_SHA\" is undefined. (I have no test cases\nfor the modified SHA algorithm.)\n";
    do_test("test1",
	"abc",
	"0164b8a9 14cd2a5e 74c4f7ff 082c4d97 f1edf880");
    do_test("test2",
	"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	"d2516ee1 acfa5baf 33dfc1c4 71e43844 9ef134c8");
    do_test("test3",
	"a" x 1000000,
	"3232affa 48628a26 653b5aaa 44541fd9 0d690603");
} else {
    if ($#ARGV >= 0) {
	foreach $ARGV (@ARGV) {
	    die "Can't open file '$ARGV' ($!)\n" unless open(ARGV, $ARGV);
	    $sha->reset();
	    $sha->addfile(ARGV);
	    print "SHA($ARGV) = " . $sha->hexdigest() . "\n";
	    close(ARGV);
	}
    } else {
	$sha->reset();
	$sha->addfile(STDIN);
	print "SHA(STDIN) = " . $sha->hexdigest() . "\n";
    }
}

exit 0;
