#! perl

#
# Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

#
# This file automates the creation of C stubs that
# are understandable by Watcom C
#

#
# For those of you without Perl, here is a description:
#
# The first line of the .c file MUST be '#include "local.h"'
# Each function declaration of the form
#     __declspec(dllexport) stack_item *FUNCTION_DECL
# will be changed to 
#     stack_item * __export FUNCTION_DECL
#

print "#include \"config.h\"\n";

while (<>) {
	if( /^__declspec\(dllexport\) stack_item \*/ ) {
		s/^__declspec\(dllexport\) stack_item \*/stack_item \* __export /;
	}
	print;
}

