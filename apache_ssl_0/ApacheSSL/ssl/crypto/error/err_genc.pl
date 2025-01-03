#!/usr/local/bin/perl

($#ARGV == 1) || die "usage: $0 <header file> <output C file>\n";
open(IN,"<$ARGV[0]") || die "unable to open $ARGV[0]:$!\n";

$Func=0;
$Reas=0;
$fuction{'FOPEN'}='fopen';
while (<IN>)
	{
	if (/(\S+)\s*\(\);/)
		{
		$t=$1;
		$t =~ s/\*//;
		($upper=$t) =~ tr/a-z/A-Z/;
		$fuction{$upper}=$t;
		}
	next unless (/^#define\s+(\S+)\s/);

	$o=$1;
	if ($o =~ /^([^_]+)_F_(.*)/)
		{
		$type=$1;
		$Func++;
		$n=$2;
		$n=$fuction{$n} if (defined($fuction{$n}));
		$out{$1."_str_functs"}.=
			sprintf("{ERR_PACK(0,%s,0),\t\"$n\"},\n",$o);
		}
	elsif ($o =~ /^([^_]+)_R_(.*)/)
		{
		$type=$1;
		$Reas++;
		$r=$2;
		$r =~ tr/A-Z_/a-z /;
		$pkg{$1."_str_reasons"}=$1;
		$out{$1."_str_reasons"}.=sprintf("{%-40s,\"$r\"},\n",$o);
		}
	}
close(IN);

&header($type,$ARGV[0]);

foreach (sort keys %out)
	{
	print "static ERR_STRING_DATA ${_}[]=\n\t{\n";
	print $out{$_};
	print "{0,NULL},\n";
	print "\t};\n\n";
	}

print "void ERR_load_${type}_strings()\n";
print "\t{\n";
print "\tstatic int init=1;\n\n";
print "\tif (init)\n";
print "\t\t{\n";
print "\t\tinit=0;\n";
print "\t\tERR_load_strings(ERR_LIB_$type,${type}_str_functs);\n"
	if $Func;
print "\t\tERR_load_strings(ERR_LIB_$type,${type}_str_reasons);\n"
	if $Reas;
print "\t\t}\n";
print "\t}\n";

sub header
	{
	($type,$header)=@_;

	($lc=$type) =~ tr/A-Z/a-z/;
	$header =~ s/^.*\///;

	print <<"EOF";
/* lib/$lc/${lc}_err.c */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SS
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIA
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "crypto.h"
#include "$header"

/* BEGIN ERROR CODES */
EOF
	}
