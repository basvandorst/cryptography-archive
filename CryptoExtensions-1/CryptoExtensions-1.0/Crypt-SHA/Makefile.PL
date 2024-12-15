#! /usr/local/bin/perl

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile being created.
WriteMakefile(
	'NAME' => 'Crypt::SHA',
	'DISTNAME' => 'Crypt-SHA',
	'VERSION' => '1.6',
	'OBJECT' => 'SHA.o sha_func.o',
	'dist'   => {COMPRESS=>'gzip', SUFFIX=>'gz'}
);
