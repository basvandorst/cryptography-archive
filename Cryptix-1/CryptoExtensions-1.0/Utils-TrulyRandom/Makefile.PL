#! /usr/local/bin/perl

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile being created.
WriteMakefile(
	'NAME' => 'Utils::TrulyRandom',
	'DISTNAME' => 'Utils-TrulyRandom',
	'VERSION' => '1.0',
	'OBJECT' => 'TrulyRandom.o truerand.o',
	'dist'   => {COMPRESS=>'gzip', SUFFIX=>'gz'}
);
