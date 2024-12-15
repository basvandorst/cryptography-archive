
###############################################################################
##                                                                           ##
##    Copyright (c) 1995, 1996, 1997, 1998 by Steffen Beyer.                 ##
##    All rights reserved.                                                   ##
##                                                                           ##
##    This piece of software is "Non-Profit-Ware" ("NP-ware").               ##
##                                                                           ##
##    You may use, copy, modify and redistribute it under                    ##
##    the terms of the "Non-Profit-License" (NPL).                           ##
##                                                                           ##
##    Please refer to the file "NONPROFIT" in this distribution              ##
##    for details!                                                           ##
##                                                                           ##
###############################################################################

use ExtUtils::MakeMaker;

WriteMakefile(
    'NAME'              => 'Bit::Vector',
    'VERSION_FROM'      => 'Vector.pm',
    'OBJECT'            => '$(O_FILES)',
    'LIBS'              => [''],   # e.g., '-lm'
    'DEFINE'            => '',     # e.g., '-DHAVE_SOMETHING'
    'INC'               => '',     # e.g., '-I/usr/include/other'
    'EXE_FILES'         => [ qw(modify upgrade_BV50) ],
    'dist'              => { COMPRESS => "gzip -9", SUFFIX => "gz" }
);

