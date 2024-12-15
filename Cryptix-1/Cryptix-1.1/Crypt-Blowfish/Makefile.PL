#! /usr/local/bin/perl

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile being created.
WriteMakefile(
	'NAME' => 'Crypt::Blowfish',
	'DISTNAME' => 'Crypt-Blowfish',
	'VERSION' => '1.00',
	'OBJECT' => 'Blowfish.o blowfish.o',
	'dist'   => {COMPRESS=>'gzip', SUFFIX=>'gz'}
);

