#! /usr/local/bin/perl

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile being created.
WriteMakefile(
	'NAME' => 'Cryptix',
	'VERSION' => '1.1',
	'DIR' => [
		'Crypt-Systemics',
		'Crypt-Blowfish',
		'Crypt-DES',
		'Crypt-IDEA',
		'Crypt-MD5',
		'Crypt-SHA',
		'Math-Systemics',
		'Math-BigInteger',
		'Math-PRSG',
		'Math-TrulyRandom',
		'Stream'
	],
	'dist'   => {COMPRESS=>'gzip', SUFFIX=>'gz'},
);
