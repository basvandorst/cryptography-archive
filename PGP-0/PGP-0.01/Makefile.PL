#! /usr/local/bin/perl

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile being created.
WriteMakefile(
	'NAME' => 'PGP',
	'VERSION' => '0.01',
	'dist'   => {COMPRESS=>'gzip', SUFFIX=>'gz'},
);
