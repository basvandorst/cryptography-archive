#!/usr/local/bin/perl

sub obj_cmp
	{
	local(@a,@b,$_,$r);

	$A=$obj_len{$obj{$nid{$a}}};
	$B=$obj_len{$obj{$nid{$b}}};

	$r=($A-$B);
	return($r) if $r != 0;

	$A=$obj_der{$obj{$nid{$a}}};
	$B=$obj_der{$obj{$nid{$b}}};

	return($A cmp $B);
	}

sub expand_obj
	{
	local(*v)=@_;
	local($k,$d);
	local($i);

	do	{
		$i=0;
		foreach $k (keys %v)
			{
			if ($v{$k} =~ s/(OBJ_[^,]+),/$v{\1},/)
				{ $i++; }
			}
		} while($i);
	foreach $k (keys %v)
		{
		@a=split(/,/,$v{$k});
		$objn{$k}=$#a+1;
		}
	}

while (<>)
	{
	next unless /^\#define\s+(\S+)\s+(.*)$/;
	$v=$1;
	$d=$2;
	if ($v =~ /^SN_(.*)$/)
		{ $sn{$1}=$d; }
	elsif ($v =~ /^LN_(.*)$/)
		{ $ln{$1}=$d; }
	elsif ($v =~ /^NID_(.*)$/)
		{ $nid{$d}=$1; }
	elsif ($v =~ /^OBJ_(.*)$/)
		{
		$obj{$1}=$v;
		$objd{$v}=$d;
		}
	}

%ob=&expand_obj(*objd);

@a=sort { $a <=> $b } keys %nid;
$n=$a[$#a]+1;

@lvalues=();
$lvalues=0;

for ($i=0; $i<$n; $i++)
	{
	if (!defined($nid{$i}))
		{
		push(@out,"{NULL,NULL,NID_undef,0,NULL},\n");
		}
	else
		{
		$sn=defined($sn{$nid{$i}})?"$sn{$nid{$i}}":"NULL";
		$ln=defined($ln{$nid{$i}})?"$ln{$nid{$i}}":"NULL";
		$sn=$ln if ($sn eq "NULL");
		$ln=$sn if ($ln eq "NULL");
		$out ="{";
		$out.=$sn;
		$out.=",".$ln;
		$out.=",NID_$nid{$i},";
		if (defined($obj{$nid{$i}}))
			{
			$v=$objd{$obj{$nid{$i}}};
			$v =~ s/L//g;
			$v =~ s/,/ /g;
			$r=&der_it($v);
			$z="";
			$length=0;
			foreach (unpack("C*",$r))
				{
				$z.=sprintf("0x%02X,",$_);
				$length++;
				}
			$obj_der{$obj{$nid{$i}}}=$z;
			$obj_len{$obj{$nid{$i}}}=$length;

			push(@lvalues,sprintf("%-45s/* [%3d] %s */\n",
				$z,$lvalues,$obj{$nid{$i}}));
			$out.="$length,&(lvalues[$lvalues])";
			$lvalues+=$length;
			}
		else
			{
			$out.="0,NULL";
			}
		$out.="},\n";
		push(@out,$out);
		}
	}

@a=grep(defined($sn{$nid{$_}}),0 .. $n);
foreach (sort { $sn{$nid{$a}} cmp $sn{$nid{$b}} } @a)
	{
	push(@sn,sprintf("&(nid_objs[%2d]),/* $sn{$nid{$_}} */\n",$_));
	}

@a=grep(defined($ln{$nid{$_}}),0 .. $n);
foreach (sort { $ln{$nid{$a}} cmp $ln{$nid{$b}} } @a)
	{
	push(@ln,sprintf("&(nid_objs[%2d]),/* $ln{$nid{$_}} */\n",$_));
	}

@a=grep(defined($obj{$nid{$_}}),0 .. $n);
foreach (sort obj_cmp @a)
	{
	$m=$obj{$nid{$_}};
	$v=$objd{$m};
	$v =~ s/L//g;
	$v =~ s/,/ /g;
	push(@ob,sprintf("&(nid_objs[%2d]),/* %-32s %s */\n",$_,$m,$v));
	}

print <<"EOF";
/* lib/obj/obj_dat.h */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
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
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
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

/* THIS FILE IS GENERATED FROM Objects.h by obj_dat.pl via the
 * following command:
 * perl obj_dat.pl < objects.h > obj_dat.h
 */

EOF
printf "#define NUM_NID %d\n",$n;
printf "#define NUM_SN %d\n",$#sn+1;
printf "#define NUM_LN %d\n",$#ln+1;
printf "#define NUM_OBJ %d\n\n",$#ob+1;

printf "static unsigned char lvalues[%d]={\n",$lvalues+1;
print @lvalues;
print "};\n\n";

printf "static ASN1_OBJECT nid_objs[NUM_NID]={\n";
foreach (@out)
	{
	if (length($_) > 75)
		{
		$out="";
		foreach (split(/,/))
			{
			$t=$out.$_.",";
			if (length($t) > 70)
				{
				print "$out\n";
				$t="\t$_,";
				}
			$out=$t;
			}
		chop $out;
		print "$out";
		}
	else
		{ print $_; }
	}
print  "};\n\n";

printf "static ASN1_OBJECT *sn_objs[NUM_SN]={\n";
print  @sn;
print  "};\n\n";

printf "static ASN1_OBJECT *ln_objs[NUM_LN]={\n";
print  @ln;
print  "};\n\n";

printf "static ASN1_OBJECT *obj_objs[NUM_OBJ]={\n";
print  @ob;
print  "};\n\n";

sub der_it
	{
	local($v)=@_;
	local(@a,$i,$ret,@r);

	@a=split(/\s+/,$v);
	$ret.=pack("C*",$a[0]*40+$a[1]);
	shift @a;
	shift @a;
	while ($_=shift(@a))
		{
		@r=();
		$t=0;
		while ($_ >= 128)
			{
			$x=$_%128;
			$_/=128;
			push(@r,((($t++)?0x80:0)|$x));
			}
		push(@r,((($t++)?0x80:0)|$_));
		$ret.=pack("C*",reverse(@r));
		}
	return($ret);
	}
