package PGP::CmdLine;

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

use strict;


sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	STDOUT->autoflush(1);

	$self;
}

sub getPassphrase
{
	print shift, ": ";
	system("stty -echo");
	my $passphrase = <STDIN>;
	chop($passphrase);
	print "\n";
	system("stty echo");
	$passphrase;
}

sub getNewPassphrase
{
	for (;;)
	{
		my $pass1 = getPassphrase("Please enter your passphrase");
		my $pass2 = getPassphrase("Please re-enter your passphrase");
		return $pass1 if ($pass1 eq $pass2);
		print "Passphrases do not match - please retry.\n";
	}
}

1;
