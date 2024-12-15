#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::CRC;

use strict;

my $junk=vec("1",0,1);$junk=""; # Hack


BEGIN
{
	my $poly = 0x864CFB;
	my $p = 0;
	my $q = 0;
	$PGP::CRC::crctable[$q++] = 0;
	$PGP::CRC::crctable[$q++] = $poly;
	my $i;
	for ($i = 1; $i < 128; $i++)
	{
		my $t = $PGP::CRC::crctable[++$p] << 1;
		if (($t & 0x1000000) == 0)
		{
			$PGP::CRC::crctable[$q++] = $t;
			$PGP::CRC::crctable[$q++] = $t ^ $poly;
		}
		else
		{	
			$PGP::CRC::crctable[$q++] = $t ^ $poly;
			$PGP::CRC::crctable[$q++] = $t;
		}
	}
}

sub checksum
{
	my $data = shift;

	my $accum = 0xB704CE;
	my $pos = 0;
	do {
		$accum = ($accum << 8) ^ $PGP::CRC::crctable[(($accum >> 16) ^ ord(substr($data, $pos++, 1))) & 0xFF];
	} while ($pos < length($data));
	$accum &= 0xFFFFFF;
	$accum;
}

1;
