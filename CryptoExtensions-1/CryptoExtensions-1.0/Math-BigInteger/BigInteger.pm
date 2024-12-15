#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
# 
# This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
# as long as the following conditions are adheared to.
# 
# Copyright remains Systemics Ltd, and as such any Copyright notices in
# the code are not to be removed.  If this code is used in a product,
# Systemics should be given attribution as the author of the parts used.
# This can be in the form of a textual message at program startup or
# in documentation (online or textual) provided with the package.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#    This product includes software developed by Systemics Ltd (http://www.systemics.com/)   
# 
#    THIS SOFTWARE IS PROVIDED BY SYSTEMICS LTD ``AS IS'' AND
#    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
#    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#    SUCH DAMAGE.
# 
#    The licence and distribution terms for any publically available version or
#    derivative of this code cannot be changed.  i.e. this code cannot simply be
#    copied and put under another distribution licence
#    [including the GNU Public Licence.]
#

package Math::BigInteger;

use Exporter;
use DynaLoader;
@ISA = (Exporter, DynaLoader);

# Items to export into callers namespace by default
@EXPORT = qw();

# Other items we are prepared to export if requested
@EXPORT_OK = qw();

bootstrap Math::BigInteger;






package BigInteger;

use strict;
use integer;
use Carp;

sub usage
{
    my ($mess, $package, $filename, $line, $subr);
	($mess) = @_;
	($package, $filename, $line, $subr) = caller(1);
	$Carp::CarpLevel = 2;
	croak "Usage: $package\::$subr - $mess";
}



sub new
{
	usage("new BigInteger") unless (@_ == 1 || @_ == 2);

	my $type = shift; my $self = {}; bless $self, $type;
	my $init = shift;


	if (defined($init))
	{
		$self->{'bn'} = restore Math::BigInteger pack("N", $init);
	}
	else
	{
		$self->{'bn'} = new Math::BigInteger;
	}

	$self;
}

sub clone
{
	usage("clone BigInteger mpi-object") unless @_ == 2;

	my $type = shift; my $self = {}; bless $self, $type;

	$self->{'bn'} = Math::BigInteger::clone(shift->{'bn'});
	$self;
}

sub restore
{
	usage("restore BigInteger data") unless @_ == 2;

	my $type = shift; my $self = {}; bless $self, $type;

	my $data = shift;
	$self->{'bn'} = restore Math::BigInteger $data;
	$self;
}

#	NB = There is no restoreFromDataInput since we would not know how
#	many bytes to read.
sub restoreFromDataInput { confess("How many bytes should I read?"); }


sub save
{
	usage("save") unless @_ == 1;

	Math::BigInteger::save(shift->{'bn'});
}

sub saveAsInt
{
	usage("saveAsInt") unless @_ == 1;

	hex unpack("H*", Math::BigInteger::save(shift->{'bn'}));
}

sub saveToDataStream
{
	usage("saveToDataStream DataOutputStream") unless @_ == 2;

	my $self = shift;
	my $dos = shift;

	$dos->write(Math::BigInteger::save($self->{'bn'}));
}


sub inc
{
	usage("inc") unless @_ == 1;

	Math::BigInteger::inc(shift->{'bn'});
}

sub dec
{
	usage("dec") unless @_ == 1;

	Math::BigInteger::dec(shift->{'bn'});
}

sub add
{
	usage("add r a b") unless @_ == 3;

	my $r = shift;
	my $a = shift;
	my $b = shift;
	Math::BigInteger::add($r->{'bn'}, $a->{'bn'}, $b->{'bn'});
}

sub sub
{
	usage("sub r a b") unless @_ == 3;

	my $r = shift;
	my $a = shift;
	my $b = shift;
	Math::BigInteger::sub($r->{'bn'}, $a->{'bn'}, $b->{'bn'});
}

sub mod
{
	usage("mod rem m d") unless @_ == 3;

	my $rem = shift;
	my $m = shift;
	my $d = shift;
	Math::BigInteger::mod($rem->{'bn'}, $m->{'bn'}, $d->{'bn'});
}

sub mul
{
	usage("mul r a b") unless @_ == 3;

	my $r = shift;
	my $a = shift;
	my $b = shift;
	Math::BigInteger::mul($r->{'bn'}, $a->{'bn'}, $b->{'bn'});
}

sub div
{
	usage("div dv rem m d") unless @_ == 4;

	my $dv = shift;
	my $rem = shift;
	my $m = shift;
	my $d = shift;
	Math::BigInteger::div($dv->{'bn'}, $rem->{'bn'}, $m->{'bn'}, $d->{'bn'});
}

sub ucmp
{
	usage("ucmp a b") unless @_ == 2;

	my $a = shift;
	my $b = shift;
	Math::BigInteger::ucmp($a->{'bn'}, $b->{'bn'});
}

sub cmp
{
	usage("cmp a b") unless @_ == 2;

	my $a = shift;
	my $b = shift;
	Math::BigInteger::cmp($a->{'bn'}, $b->{'bn'});
}

sub lshift1
{
	usage("lshift r a") unless @_ == 2;

	my $r = shift;
	my $a = shift;
	Math::BigInteger::lshift1($r->{'bn'}, $a->{'bn'});
}

sub lshift
{
	usage("lshift r a n") unless @_ == 3;

	my $r = shift;
	my $a = shift;
	my $n = shift;
	Math::BigInteger::lshift($r->{'bn'}, $a->{'bn'}, $n->{'bn'});
}

sub rshift1
{
	usage("rshift1 r a") unless @_ == 2;

	my $r = shift;
	my $a = shift;
	Math::BigInteger::rshift1($r->{'bn'}, $a->{'bn'});
}

sub rshift
{
	usage("rshift r a n") unless @_ == 3;

	my $r = shift;
	my $a = shift;
	my $n = shift;
	Math::BigInteger::rshift($r->{'bn'}, $a->{'bn'}, $n->{'bn'});
}

sub mod_exp
{
	usage("mod_exp r a p m") unless @_ == 4;

	my $r = shift;
	my $a = shift;
	my $p = shift;
	my $m = shift;
	Math::BigInteger::mod_exp($r->{'bn'}, $a->{'bn'}, $p->{'bn'}, $m->{'bn'});
}

sub modmul_recip
{
	usage("modmul_recip r x y m i nb") unless @_ == 6;

	my $r = shift;
	my $x = shift;
	my $y = shift;
	my $m = shift;
	my $i = shift;
	my $nb = shift;
	Math::BigInteger::modmul_recip($r->{'bn'}, $x->{'bn'}, $y->{'bn'}, $m->{'bn'},
							$i->{'bn'}, $nb->{'bn'});
}

sub mul_mod
{
	usage("mul_mod r a b m") unless @_ == 4;

	my $r = shift;
	my $a = shift;
	my $b = shift;
	my $m = shift;
	Math::BigInteger::mul_mod($r->{'bn'}, $a->{'bn'}, $b->{'bn'}, $m->{'bn'});
}

sub reciprical
{
	usage("reciprical r m") unless @_ == 2;

	my $r = shift;
	my $m = shift;
	Math::BigInteger::reciprical($r->{'bn'}, $m->{'bn'});
}

sub inverse_modn
{
	usage("inverse_modn r a b") unless @_ == 3;

	my $r = shift;
	my $a = shift;
	my $b = shift;
	Math::BigInteger::inverse_modn($r->{'bn'}, $a->{'bn'}, $b->{'bn'});
}

sub gcd
{
	usage("gcd r a b") unless @_ == 3;

	my $r = shift;
	my $a = shift;
	my $b = shift;
	Math::BigInteger::gcd($r->{'bn'}, $a->{'bn'}, $b->{'bn'});
}

sub toString
{
	usage("toString") unless @_ == 1;
	my $self = shift;

	unpack("H*", $self->save());
}

sub display
{
	usage("display") unless @_ == 1;
	my $self = shift;

	print $self->toString(), "\n";
}

#
#	Return the number of bits in the number
#
sub bits
{
	Math::BigInteger::num_bits(shift->{'bn'});
}

1;
