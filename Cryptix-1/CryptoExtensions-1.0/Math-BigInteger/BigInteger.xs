/*
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
*/


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "bn.h"



typedef BIGNUM *	BigInteger;

MODULE = Math::BigInteger		PACKAGE = Math::BigInteger



BigInteger
new(packname = "Math::BigInteger")
	char *	packname
    CODE:
	{
		RETVAL = bn_new();
		if (RETVAL == 0)
			croak("Could not allocate a new BigInteger");
	}
    OUTPUT:
		RETVAL

BigInteger
restore(packname = "Math::BigInteger", buf)
	char *	packname
	char *	buf = NO_INIT
	STRLEN	buf_len = NO_INIT
    CODE:
	{
		buf = (char *) SvPV(ST(1), buf_len);
		RETVAL = bn_bin2bn(buf_len, buf, (BigInteger)0);
		if (RETVAL == 0)
			croak("Could not allocate a new BigInteger");
	}
    OUTPUT:
		RETVAL

BigInteger
clone(context)
	BigInteger	context
    CODE:
	{
		RETVAL = bn_new();
		if (RETVAL == 0)
			croak("Could not allocate a new BigInteger");
		bn_copy(RETVAL, context);
	}
    OUTPUT:
		RETVAL

void
DESTROY(context)
	BigInteger	context
    CODE:
	{
		bn_free(context);
	}

void
copy(a, b)
	BigInteger	a
	BigInteger	b
    CODE:
	{
		bn_copy(a, b);
	}

char *
save(context)
	BigInteger	context
    CODE:
	{
		char buf[1024];

		int len = bn_bn2bin(context, buf);
		if (len > sizeof(buf))
			croak("Internal overflow (Math::BigInteger::save)");
	    ST(0) = sv_2mortal(newSVpv(buf, len));
	}

void
inc(context)
	BigInteger	context
    CODE:
	{
		static int first_time = 1;
		static BigInteger one;
		int ret;
		if (first_time)
		{
			one = bn_new();
			bn_one(one);
			first_time = 0;
		}

	    ret = bn_add(context, context, one);
		if (ret == 0)
			croak("bn_add failed");
	}

void
dec(context)
	BigInteger	context
    CODE:
	{
		static int first_time = 1;
		static BigInteger one;
		int ret;
		if (first_time)
		{
			one = bn_new();
			bn_one(one);
			first_time = 0;
		}

	    ret = bn_sub(context, context, one);
		if (ret == 0)
			croak("bn_sub failed");
	}

#
#	r can be a or b
#
void
add(r, a, b)
	BigInteger	r
	BigInteger	a
	BigInteger	b
    CODE:
	{
	    int ret = bn_add(r, a, b);
		if (ret == 0)
			croak("bn_add failed");
	}

#
#	r can be a or b
#
void
sub(r, a, b)
	BigInteger	r
	BigInteger	a
	BigInteger	b
    CODE:
	{
	    int ret = bn_sub(r, a, b);
		if (ret == 0)
			croak("bn_sub failed");
	}

void
mod(rem, m, d)
	BigInteger	rem
	BigInteger	m
	BigInteger	d
    CODE:
	{
	    int ret = bn_mod(rem, m, d);
		if (ret == 0)
			croak("bn_mod failed");
	}

void
div(dv, rem, m, d)
	BigInteger	dv
	BigInteger	rem
	BigInteger	m
	BigInteger	d
    CODE:
	{
	    int ret = bn_div(dv, rem, m, d);
		if (ret == 0)
			croak("bn_div failed");
	}

void
mul(r, a, b)
	BigInteger	r
	BigInteger	a
	BigInteger	b
    CODE:
	{
	    int ret = bn_mul(r, a, b);
		if (ret == 0)
			croak("bn_mul failed");
	}

int
ucmp(a, b)
	BigInteger	a
	BigInteger	b
	CODE:
	{
		RETVAL = bn_Ucmp(a, b);
	}
	OUTPUT:
		RETVAL

int
cmp(a, b)
	BigInteger	a
	BigInteger	b
	CODE:
	{
		RETVAL = bn_cmp(a, b);
	}
	OUTPUT:
		RETVAL

void
lshift(r, a, n)
	BigInteger	r
	BigInteger	a
	int		n
	CODE:
	{
		int ret = bn_lshift(r, a, n);
		if (ret == 0)
			croak("bn_lshift failed");
	}

void
lshift1(r, a)
	BigInteger	r
	BigInteger	a
	CODE:
	{
		int ret = bn_lshift1(r, a);
		if (ret == 0)
			croak("bn_lshift1 failed");
	}

void
rshift(r, a, n)
	BigInteger	r
	BigInteger	a
	int		n
	CODE:
	{
		int ret = bn_rshift(r, a, n);
		if (ret == 0)
			croak("bn_rshift failed");
	}

void
rshift1(r, a)
	BigInteger	r
	BigInteger	a
	CODE:
	{
		int ret = bn_rshift1(r, a);
		if (ret == 0)
			croak("bn_rshift1 failed");
	}

void
mod_exp(r, a, p, m)
	BigInteger	r
	BigInteger	a
	BigInteger	p
	BigInteger	m
	CODE:
	{
		int ret = bn_mod_exp(r, a, p, m);
		if (ret == 0)
			croak("bn_mod_exp failed");
	}

void
modmul_recip(r, x, y, m, i, nb)
	BigInteger	r
	BigInteger	x
	BigInteger	y
	BigInteger	m
	BigInteger	i
	int		nb
	CODE:
	{
		int ret = bn_modmul_recip(r, x, y, m, i, nb);
		if (ret == 0)
			croak("bn_modmul_recip failed");
	}

void
mul_mod(r, a, b, m)
	BigInteger	r
	BigInteger	a
	BigInteger	b
	BigInteger	m
	CODE:
	{
		int ret = bn_mul_mod(r, a, b, m);
		if (ret == 0)
			croak("bn_mul_mod failed");
	}

void
reciprical(r, m)
	BigInteger	r
	BigInteger	m
	CODE:
	{
		int ret = bn_reciprical(r, m);
		if (ret == 0)
			croak("bn_reciprical failed");
	}

void
gcd(r, a, b)
	BigInteger	r
	BigInteger	a
	BigInteger	b
	CODE:
	{
		int ret = bn_gcd(r, a, b);
		if (ret == 0)
			croak("bn_gcd failed");
	}

void
inverse_modn(r, a, b)
	BigInteger	r
	BigInteger	a
	BigInteger	b
	CODE:
	{
		int ret = bn_inverse_modn(r, a, b);
		if (ret == 0)
			croak("bn_inverse_modn failed");
	}

int
num_bits(a)
	BigInteger a
	CODE:
	{
		RETVAL = bn_num_bits(a);
	}
	OUTPUT:
		RETVAL
