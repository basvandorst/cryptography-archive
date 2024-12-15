#! /usr/local/bin/perl

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile being created.
WriteMakefile(
	'NAME' => 'Math::BigInteger',
	'DISTNAME' => 'Math-BigInteger',
	'VERSION' => '1.0',
	'OBJECT' => 'BigInteger.o bn.o bn_mul.o bn_regs.o',
	'dist'   => {COMPRESS=>'gzip', SUFFIX=>'gz'}
);

