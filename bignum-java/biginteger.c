/*
 * @(#)biginteger.c	1.1 95/08/30  
 *
 * Copyright (c) 1994 Sun Microsystems, Inc. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies. Please refer to the file "copyright.html"
 * for further important copyright and licensing information.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

/*
 * This is the "back end" for the big integer package. The Java code
 * implements 6 functions in C, these are add, sub, mulitiply, divide,
 * modulus, inverse modulus, and exponentiation with modulus.
 *
 * There are two critical function toNative and fromNative. toNative
 * take a big integer and returns a representation that is native to
 * the big number library, fromNative does the reverse. In the latter
 * case a new BigInteger object is created that exactly holds the number.
 */

#include <oobj.h>
#include <interpreter.h>
#include "CClassHeaders/java_misc_BigInteger.h"
#include "global.h"
#include "rsaref.h"
#include "nn.h"
#include "digit.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))


int
isnegative(struct Hjava_misc_BigInteger *n) {
    long *d = unhand(unhand(n)->data)->body;
    return (((*d & 0x80000000) != 0) ? 1 : 0);
}

void
negate(NN_DIGIT nn[], int nDigits) {
    NN_DIGIT 	a[MAX_NN_DIGITS];	/* the number "1" 	    */
    int		i; 			/* a temporary 		    */

    NN_ASSIGN_DIGIT(a, 1, nDigits);

    for (i = 0; i < nDigits; i++)
	nn[i] = ~nn[i];
    NN_Add(nn, nn, a, nDigits);
}

int 
toNative(struct Hjava_misc_BigInteger *Hnum, NN_DIGIT nn[], int md) {
    Classjava_misc_BigInteger *num = unhand(Hnum);
    int	len = obj_length(num->data);
    NN_DIGIT *dd = (NN_DIGIT *)(unhand((unhand(Hnum)->data))->body);
    int i;

    for (i = 0; i < md; i++) nn[i] = 0;

    if (len > md) {
	printf("MAXIMUM DIGITS EXCEEDED.");
	abort(); /* should throw an error */
    }

    for (i = 0; i < len; i++) {
	nn[len - i - 1] = *(dd+i); /* invert the ordering */
    }
    return (NN_Digits(nn, md));
}

static ClassClass *vpiclass = NULL;

Hjava_misc_BigInteger *
fromNative(NN_DIGIT num[], int numDigits) {
    Hjava_misc_BigInteger   *res;
    NN_DIGIT 		    *dd;
    long 		    bits = numDigits * 32;
    int			    i, xlen;
 
    if (vpiclass == NULL) {
	vpiclass = FindClass(0, "java/misc/BigInteger", TRUE);
	if (vpiclass == NULL) {
	    SignalError(0, JAVAPKG "NoClassDefFoundError", 
					"java.misc.BigInteger");
	    return (NULL);
	}
    }

    /* BigInteger res = new BigInteger(bits); */
    if (numDigits == 0) bits = 32;
    res = (Hjava_misc_BigInteger *)execute_java_constructor(0, 
			"java/misc/BigInteger", vpiclass, 
			"(I)", bits);
    xlen = obj_length(unhand(res)->data);
    dd = (NN_DIGIT *)(unhand(unhand(res)->data)->body);
    *dd = 0;
    for (i = 0; i < numDigits; i++) {
	*(dd + i) = num[numDigits - i - 1];
    }
    return (res);
}

/*
 * Add 'a' to 'this' and return in 'res'
 * Covering the corner cases...
 *	 this +  aValue	= this   + aValue;	0
 *	 this + -aValue	= this   - aValue;	1
 *	-this +  aValue	= aValue - this;	2
 *	-this + -aValue	= - (this + aValue);	3
 */
struct Hjava_misc_BigInteger *
java_misc_BigInteger_nativeAdd(
	struct Hjava_misc_BigInteger *this,	/* This vpi */
	struct Hjava_misc_BigInteger *aValue)	/* The operand */
{
	NN_DIGIT	a[MAX_NN_DIGITS], b[MAX_NN_DIGITS];
	int		aDigits, bDigits;
	int		xx;

	memset((char *)a, 0L, sizeof(a));
	memset((char *)b, 0L, sizeof(a));
	aDigits = toNative(this, a, MAX_NN_DIGITS);
	bDigits = toNative(aValue, b, MAX_NN_DIGITS);
	xx = isnegative(this) * 2 + isnegative(aValue);

	switch (xx) {
	    case 0: /* Just add a and b */
		if (NN_Add(a, a, b, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    a[aDigits] = 1;
	    	    return (fromNative(a, aDigits+1));
		} else {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			return (fromNative(a, aDigits+1));
		    } else {
		    	return (fromNative(a, aDigits));
	  	    }
		}
	    case 1: /* this - aValue */
		negate(b, bDigits);
		bDigits = NN_Digits(b, MAX_NN_DIGITS);
		if (NN_Sub(a, a, b, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits - 1] & 0x80000000) == 0) {
		        a[aDigits] = 0xffffffff;	/* borrowed bit */
		        return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		} else { /* should be positive */
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		}
	    case 2: /* aValue - this */
		negate(a, aDigits);
		aDigits = NN_Digits(a, MAX_NN_DIGITS);
		if (NN_Sub(a, b, a, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits - 1] & 0x80000000) == 0) {
		        a[aDigits] = 0xffffffff;	/* borrowed bit */
		        return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		} else { /* should be positive */
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		}
	    case 3: /* - (this + AValue) */
		negate(a, aDigits);
		aDigits = NN_Digits(a, MAX_NN_DIGITS);
		negate(b, bDigits);
		bDigits = NN_Digits(b, MAX_NN_DIGITS);
		if (NN_Add(a, a, b, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    a[aDigits] = 1;
		    negate(a, aDigits+1);
	    	    return (fromNative(a, NN_Digits(a, MAX_NN_DIGITS)));
		} else {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			negate(a, aDigits+1);
		    } else {
		        negate(a, aDigits);
		    }
		    return (fromNative(a, NN_Digits(a, MAX_NN_DIGITS)));
		}
		
	}
	return (NULL); /* make cc happy, not reached */
}

/*
 * Subtract 'a' from 'this' and return in 'res'
 * Covering the corner cases...
 *	 this -  aValue	= this   - aValue;	0
 *	 this - -aValue	= this   + aValue;	1
 *	-this -  aValue	= -(aValue + this);	2
 *	-this - -aValue	= aValue - this;	3
 */
struct Hjava_misc_BigInteger *
java_misc_BigInteger_nativeSub(
	struct Hjava_misc_BigInteger *this,	/* This vpi */
	struct Hjava_misc_BigInteger *aValue)	/* The operand */
{
	NN_DIGIT	a[MAX_NN_DIGITS], b[MAX_NN_DIGITS];
	int		aDigits, bDigits;
	int		xx;

	memset((char *)a, 0L, sizeof(a));
	memset((char *)b, 0L, sizeof(a));
	aDigits = toNative(this, a, MAX_NN_DIGITS);
	bDigits = toNative(aValue, b, MAX_NN_DIGITS);
	xx = isnegative(this) * 2 + isnegative(aValue);

	switch (xx) {
	    case 0: /* (+a) - (+b) => (+a) - (+b) */
		if (NN_Sub(a, a, b, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits - 1] & 0x80000000) == 0) {
		        a[aDigits] = 0xffffffff;	/* borrowed bit */
		        return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		} else { /* should be positive */
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		}
	    case 1: /* Add a - (-b) => (+a) + (+b) */
		negate(b, bDigits);
		bDigits = NN_Digits(b, MAX_NN_DIGITS);
		if (NN_Add(a, a, b, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    a[aDigits] = 1;
	    	    return (fromNative(a, aDigits+1));
		} else {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			return (fromNative(a, aDigits+1));
		    } else {
		    	return (fromNative(a, aDigits));
	  	    }
		}
	    case 2: /* (-a) - (+b) => -1((+a) + (+b)) */
		negate(a, aDigits);
		aDigits = NN_Digits(a, MAX_NN_DIGITS);
		if (NN_Add(a, a, b, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    a[aDigits] = 1;
		    negate(a, aDigits+1);
	    	    return (fromNative(a, NN_Digits(a, MAX_NN_DIGITS)));
		} else {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			negate(a, aDigits+1);
		    } else {
		        negate(a, aDigits);
		    }
		    return (fromNative(a, NN_Digits(a, MAX_NN_DIGITS)));
		}
	    case 3: /* (-a) - (-b) => (+b) - (+a) */
		negate(a, aDigits);
		aDigits = NN_Digits(a, MAX_NN_DIGITS);
		negate(b, bDigits);
		bDigits = NN_Digits(b, MAX_NN_DIGITS);
		if (NN_Sub(a, b, a, max(aDigits, bDigits)) != 0) {
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits - 1] & 0x80000000) == 0) {
		        a[aDigits] = 0xffffffff;	/* borrowed bit */
		        return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		} else { /* should be positive */
		    aDigits = NN_Digits(a, MAX_NN_DIGITS);
		    if ((a[aDigits-1] & 0x80000000) != 0) {
			return (fromNative(a, aDigits+1));
		    } else {
		        return (fromNative(a, aDigits));
		    }
		}
		
	}
	return (NULL); /* make cc happy, not reached */
}

/*
 * Multi precision multiplication
 *
 * Multiply aValue and this and return res.
 */
struct Hjava_misc_BigInteger *
java_misc_BigInteger_nativeMul(
	struct Hjava_misc_BigInteger *this, 
	struct Hjava_misc_BigInteger *aValue)
{
	NN_DIGIT    r[MAX_NN_DIGITS * 2], a[MAX_NN_DIGITS], b[MAX_NN_DIGITS];
	int	    aDigits, bDigits;
	int	    negateResult = 0;
	int	    i;


	/* clear the result */
	memset((char *)r, 0L, sizeof(r));
	memset((char *)a, 0L, sizeof(a));
	memset((char *)b, 0L, sizeof(a));

	/* convert this and aValue to native */
	aDigits = toNative(this, a, MAX_NN_DIGITS);
	bDigits = toNative(aValue, b, MAX_NN_DIGITS);

	/* Check for negative numbers, negate if necessary, track state */
	if (isnegative(this)) {
	    negate(a, aDigits);
	    negateResult = ~negateResult; /* flip state */
	}
	if (isnegative(aValue)) {
	    negate(b, bDigits);
	    negateResult = ~negateResult; /* flip state */
	}

	/* a and b are now both "positive" numbers , recompute length */
	aDigits = NN_Digits(a, MAX_NN_DIGITS);
	bDigits = NN_Digits(b, MAX_NN_DIGITS);
	
	NN_Mult(r, a, b, max(aDigits, bDigits));

	/*
	 * Since they were both positive the result is by definition
	 * positive. If it appears negative (sign bit true) then we
 	 * "extend" it by a word of zeros to protect that bit. This
	 * is done by feeding # of digits + 1 to the fromNative function.
	 */
	if ((r[NN_Digits(r, MAX_NN_DIGITS * 2)-1] & 0x80000000) != 0) {
	    if (negateResult == 0) {
		return (fromNative(r, 1+NN_Digits(r, MAX_NN_DIGITS*2)));
	    } else {
		negate(r, 1+NN_Digits(r, MAX_NN_DIGITS * 2));
		return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS*2)));
	    }
	} 

	/*
	 * If the result was positive, we check to see if we really need
	 * to negate it, and if so do that now before converting it into
	 * a VPI to be returned to the user.
	 */
	if (negateResult != 0) {
	    negate(r, NN_Digits(r, MAX_NN_DIGITS * 2));
	}
	return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS * 2)));
}

/*
 * Multi precision division
 *
 * Divide this by  aValue return the result.
 */
struct Hjava_misc_BigInteger *
java_misc_BigInteger_nativeDiv(
	struct Hjava_misc_BigInteger *this, 
	struct Hjava_misc_BigInteger *aValue)
{
	NN_DIGIT    r1[MAX_NN_DIGITS * 2], a[MAX_NN_DIGITS * 2], 
		    b[MAX_NN_DIGITS], r2[MAX_NN_DIGITS];
	int	    aDigits, bDigits;
	int	    negateResult = 0;
	int	    i;

	/* clear both the division result */
	memset((char *)r1, 0L, sizeof(r1));
	memset((char *)a, 0L, sizeof(a));
	memset((char *)b, 0L, sizeof(a));

	/* Convert this and aValue to native form */
	aDigits = toNative(this, a, MAX_NN_DIGITS * 2);
	bDigits = toNative(aValue, b, MAX_NN_DIGITS);

	/* Note the signs, negate if necessary, maintain state */
	if (isnegative(this)) {
	    negate(a, aDigits);
	    negateResult = ~negateResult; /* flip state */
	}
	if (isnegative(aValue)) {
	    negate(b, bDigits);
	    negateResult = ~negateResult; /* flip state */
	}

	/* a and b are now both "positive" numbers, recompute length */
	aDigits = NN_Digits(a, MAX_NN_DIGITS);
	bDigits = NN_Digits(b, MAX_NN_DIGITS);
	
	NN_Div(r1, r2, a, aDigits, b, bDigits);
	
	/*
	 * Since a and b are positive, r1 must be positive. We force
	 * that here.
	 */
	if ((r1[NN_Digits(r1, MAX_NN_DIGITS * 2)-1] & 0x80000000) != 0) {
	    if (negateResult == 0) {
		return (fromNative(r1, 1+NN_Digits(r1, MAX_NN_DIGITS*2)));
	    } else {
		negate(r1, 1+NN_Digits(r1, MAX_NN_DIGITS * 2));
		return (fromNative(r1, NN_Digits(r1, MAX_NN_DIGITS*2)));
	    }
	}
	
	if (negateResult != 0) {
	    negate(r1, NN_Digits(r1, MAX_NN_DIGITS * 2));
	}
	return (fromNative(r1, NN_Digits(r1, MAX_NN_DIGITS * 2)));
}

/*
 * Multi precision Modulus operation
 *
 * Computes r = this mod a
 *	From Guy Steele's Common Lisp section on numbers;
 *	mod  13  4 =  1		#0	0 0 
 *	mod -13  4 =  3		#1	0 1
 *	mod  13 -4 = -3		#2	1 0 
 *	mod -13 -4 = -1		#3	1 1
 */
struct Hjava_misc_BigInteger *
java_misc_BigInteger_nativeMod(
	struct Hjava_misc_BigInteger *this, 
	struct Hjava_misc_BigInteger *aValue)
{
	NN_DIGIT    a[MAX_NN_DIGITS * 2], 
		    b[MAX_NN_DIGITS], r[MAX_NN_DIGITS];
	int	    aDigits, bDigits, rDigits;
	int	    i, version;
	int	    shouldBeNegative = 0;

	memset((char *)r, 0L, sizeof(r));
	memset((char *)a, 0L, sizeof(a));
	memset((char *)b, 0L, sizeof(a));

	aDigits = toNative(this, a, MAX_NN_DIGITS * 2);
	bDigits = toNative(aValue, b, MAX_NN_DIGITS);
	
	if (isnegative(aValue)) {
	    /* Reflect case #2 -> #1 or #3 -> #0 */
	    negate(a, aDigits);
	    aDigits = NN_Digits(a, MAX_NN_DIGITS*2);
	    negate(b, bDigits);
	    bDigits = NN_Digits(b, MAX_NN_DIGITS);
	    shouldBeNegative = 1;
	}
	
	NN_Mod(r, a, aDigits, b, bDigits);
	/*
	 * Make sure the result is correctly negative or positive
 	 */
	rDigits = NN_Digits(r, MAX_NN_DIGITS);
	if ((r[rDigits - 1] & 0x80000000) != 0) {
	    if (shouldBeNegative != 0) {
		return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS)));
	    } else {
		return (fromNative(r, 1+NN_Digits(r, MAX_NN_DIGITS)));
	    }
	}
	/*
	 */
	if (shouldBeNegative != 0) {
	    negate(r, NN_Digits(r, MAX_NN_DIGITS));
	}
	return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS)));
}

/*
 * Multi precision Remainder operation
 *
 * Computes r = this rem a
 *	From Guy Steele's Common Lisp section on numbers;
 *	rem  13  4 =  1
 *	rem -13  4 = -1
 *	rem  13 -4 =  1
 *	rem -13 -4 = -1
 */

struct Hjava_misc_BigInteger *
java_misc_BigInteger_nativeRem(
	struct Hjava_misc_BigInteger *this, 
	struct Hjava_misc_BigInteger *aValue)
{
	NN_DIGIT    a[MAX_NN_DIGITS * 2], 
		    b[MAX_NN_DIGITS], r[MAX_NN_DIGITS];
	int	    aDigits, bDigits;
	int	    i;

	memset((char *)r, 0L, sizeof(r));
	memset((char *)a, 0L, sizeof(a));
	memset((char *)b, 0L, sizeof(a));

	aDigits = toNative(this, a, MAX_NN_DIGITS * 2);
	bDigits = toNative(aValue, b, MAX_NN_DIGITS);
	if (isnegative(this)) {
	    negate(a, aDigits);
	}
	if (isnegative(aValue)) {
	    negate(b, bDigits);
	}
	/* a and b are now both "positive" numbers */
	aDigits = NN_Digits(a, MAX_NN_DIGITS);
	bDigits = NN_Digits(b, MAX_NN_DIGITS);
	
	NN_Mod(r, a, aDigits, b, bDigits);
	/*
	 * At this point the result should be positive, if it
 	 * appears negative then that is simply an artifact of
	 * the way the number is represnted.
 	 */
	if ((r[NN_Digits(r, MAX_NN_DIGITS)-1] & 0x80000000) != 0) {
	    if (isnegative(this)) {
		negate(r, 1+NN_Digits(r, MAX_NN_DIGITS));
		return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS)));
	    } else {
		return (fromNative(r, 1+NN_Digits(r, MAX_NN_DIGITS)));
	    }
	}
	/*
	 * Else our result appears to be positive so
	 * we treat it that way.
	 */
	if (isnegative(this)) {
	    negate(r, NN_Digits(r, MAX_NN_DIGITS));
	}
	return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS)));
}

/*
 * Computes this ^ e mod m
 */
Hjava_misc_BigInteger *
java_misc_BigInteger_nativeExpMod(
	struct Hjava_misc_BigInteger *this,
	struct Hjava_misc_BigInteger *exponent,
	struct Hjava_misc_BigInteger *modulus)
{
	NN_DIGIT b[MAX_NN_DIGITS], c[MAX_NN_DIGITS], d[MAX_NN_DIGITS];
	NN_DIGIT r[MAX_NN_DIGITS];
	int	bDigits, cDigits, dDigits, rDigits;
	int	i;
	int	shouldBeNegative = 0;

	/* clear out result register */
	memset((char *)r, 0L, sizeof(r));
	memset((char *)b, 0L, sizeof(b));
	memset((char *)c, 0L, sizeof(c));
	memset((char *)d, 0L, sizeof(d));

	bDigits = toNative(this, b, MAX_NN_DIGITS);
	cDigits = toNative(exponent, c, MAX_NN_DIGITS);
	dDigits = toNative(modulus, d, MAX_NN_DIGITS);
	if (isnegative(modulus)) {
	    shouldBeNegative = 1;
	    negate(d, dDigits);
	    dDigits = NN_Digits(d, MAX_NN_DIGITS);
	}

	/* a = b^c mod d ... NN_ModExp(a, b, c, cDigits, d, dDigits) */
	/* a = result, b = this, c = exponent, d = modulus */
	NN_ModExp(r, b, c, cDigits, d, dDigits);
	rDigits = NN_Digits(r, MAX_NN_DIGITS);
	if ((r[rDigits-1] & 0x80000000) != 0) {	/* it IS negative */
	    if (shouldBeNegative != 0) {
	        return (fromNative(r, rDigits)); /* and should be */
	    } else {
	        return (fromNative(r, 1+rDigits)); /* it should not be */
	    }
	}
	if (shouldBeNegative == 0) {
	    return (fromNative(r, rDigits));
	} else {
	    negate(r, rDigits);
	    return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS)));
	}
}

/*
 * Computes result = 1/this mod m
 */
Hjava_misc_BigInteger *
java_misc_BigInteger_nativeModInv(
	struct Hjava_misc_BigInteger *this,
	struct Hjava_misc_BigInteger *modulus)
{
	NN_DIGIT	r[MAX_NN_DIGITS], a[MAX_NN_DIGITS], b[MAX_NN_DIGITS];
	int		aDigits, bDigits, rDigits;
	int	shouldBeNegative = 0;

	/* clear our result */
	memset((char *)r, 0L, sizeof(r));
	memset((char *)a, 0L, sizeof(a));
	memset((char *)b, 0L, sizeof(b));

	aDigits = toNative(this, a, MAX_NN_DIGITS);
	bDigits = toNative(modulus, b, MAX_NN_DIGITS);
	if (isnegative(modulus)) {
	    negate(b, bDigits);
	    bDigits = NN_Digits(b, MAX_NN_DIGITS);
	    shouldBeNegative = 1;
	}

	NN_ModInv(r, a, b, max(aDigits, bDigits));
	rDigits = NN_Digits(r, MAX_NN_DIGITS);
	if ((r[rDigits-1] & 0x80000000) != 0) {	/* it IS negative */
	    if (shouldBeNegative != 0) {
	        return (fromNative(r, rDigits)); /* and should be */
	    } else {
	        return (fromNative(r, 1+rDigits)); /* it should not be */
	    }
	}
	if (shouldBeNegative == 0) {
	    return (fromNative(r, rDigits));
	} else {
	    negate(r, rDigits);
	    return (fromNative(r, NN_Digits(r, MAX_NN_DIGITS)));
	}
}

