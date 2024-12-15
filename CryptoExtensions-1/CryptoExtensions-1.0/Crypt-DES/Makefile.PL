#! /usr/local/bin/perl

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile being created.
WriteMakefile(
	'NAME' => 'Crypt::DES',
	'DISTNAME' => 'Crypt-DES',
	'VERSION' => '1.0',
	'OBJECT' => 'DES.o des.o',
	'dist'   => {COMPRESS=>'gzip', SUFFIX=>'gz'}
);

