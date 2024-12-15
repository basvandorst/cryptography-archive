// nbtheory.h - written and placed in the public domain by Wei Dai

#ifndef NBTHEORY_H
#define NBTHEORY_H

#include "integer.h"

// export a table of small primes

extern const unsigned maxPrimeTableSize;
extern unsigned primeTableSize;
extern word16 primeTable[];

// build up the table to maxPrimeTableSize
void BuildPrimeTable();

// ************ primality testing ****************

// generate a provable prime
Integer ProvablePrime(RandomNumberGenerator &rng, unsigned int bits);

boolean IsSmallPrime(const Integer &p);

// returns TRUE if p is not divisible by small primes
boolean SmallDivisorsTest(const Integer &p);

// These is no reason to use these two, use the ones below instead
boolean IsFermatProbablePrime(const Integer &n, const Integer &b);
boolean IsLucasProbablePrime(const Integer &n);

boolean IsStrongProbablePrime(const Integer &n, const Integer &b);
boolean IsStrongLucasProbablePrime(const Integer &n);

// Rabin-Miller primality test, i.e. repeating the strong probable prime test 
// for several rounds with random bases
boolean RabinMillerTest(RandomNumberGenerator &rng, const Integer &w, unsigned int rounds);

// small divisors test + strong probable prime test + strong Lucas probable prime test
// should be good enough for all practical purposes
// but feel free to change this to suit your level of paranoia
inline boolean IsPrime(const Integer &p)
{
    return (IsSmallPrime(p) || (SmallDivisorsTest(p) 
		    && IsStrongProbablePrime(p, 3) && IsStrongLucasProbablePrime(p)));
}

// use a fast sieve to find the next probable prime after p
// returns TRUE iff successful
boolean NextPrime(Integer &p, const Integer &max, boolean blumInt=FALSE);

class PrimeAndGenerator
{
public:
	// generate a random prime p of the form 2*q+delta, where q is also prime
	// warning: this is slow!
    PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits);
	// generate a random prime p of the form 2*r*q+delta, where q is also prime
    PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits);

    const Integer& Prime() const {return p;}
    const Integer& SubPrime() const {return q;}
    const Integer& Generator() const {return g;}

private:
    Integer p, q, g;
};

// ********** other number theoretic functions ************

// use Chinese Remainder Theorem to calculate x given x mod p and x mod q
Integer CRT(const Integer &xp, const Integer &p, const Integer &xq, const Integer &q);
// use this one if u = inverse of p mod q has been precalculated
Integer CRT(const Integer &xp, const Integer &p, const Integer &xq, const Integer &q, const Integer &u);

// if b is prime, then Jacobi(a, b) returns 0 if a%b==0, 1 if a is quadratic residue mod b, -1 otherwise
// check a number theory book for what Jacobi symbol means when b is not prime
int Jacobi(const Integer &a, const Integer &b);

// calculates the Lucas function V_e(p, 1) mod n
Integer Lucas(const Integer &e, const Integer &p, const Integer &n);
// calculates x such that m==Lucas(e, x, p*q), p q primes
Integer InverseLucas(const Integer &e, const Integer &m, const Integer &p, const Integer &q);
// use this one if u=inverse of p mod q has been precalculated
Integer InverseLucas(const Integer &e, const Integer &m, const Integer &p, const Integer &q, const Integer &u);

inline Integer ModularExponentiation(const Integer &a, const Integer &e, const Integer &m)
    {return a_exp_b_mod_c(a, e, m);}
// returns x such that x*x%p == a, p prime
Integer ModularSquareRoot(const Integer &a, const Integer &p);
// returns x such that a==ModularExponentiation(x, e, p*q), p q primes,
// and e relatively prime to (p-1)*(q-1)
Integer ModularRoot(const Integer &a, const Integer &e, const Integer &p, const Integer &q);
// use this one if dp=d%(p-1), dq=d%(q-1), (d is inverse of e mod (p-1)*(q-1))
// and u=inverse of p mod q have been precalculated
Integer ModularRoot(const Integer &a, const Integer &dp, const Integer &dq, const Integer &p, const Integer &q, const Integer &u);

// Greatest Common Divisor, using the Euclidean algorithm
// works with Integer and PolynomialMod2
template <class EDElement> EDElement GCD(const EDElement &a, const EDElement &n);
// Least Common Multiple
template <class EDElement> EDElement LCM(const EDElement &a, const EDElement &n);
// modular multiplicative inverse, using the extended Euclidean algorithm
template <class EDElement> EDElement EuclideanMultiplicativeInverse(const EDElement &a, const EDElement &n);

// returns log base 2 of estimated number of operations to calculate discrete log or factor a number
unsigned int DiscreteLogWorkFactor(unsigned int bitlength);
unsigned int FactoringWorkFactor(unsigned int bitlength);

// exponentiation using a binary sliding window
template <class Group, class Element> Element GeneralizedMultiplication(const Group &group, const Element &base, const Integer &exponent);
template <class Field, class Element> Element GeneralizedExponentiation(const Field &field, const Element &base, const Integer &exponent);
template <class Group, class Element> Element CascadeMultiplication(const Group &group, const Element &x, const Integer &e1, const Element &y, const Integer &e2);
template <class Field, class Element> Element CascadeExponentiation(const Field &field, const Element &x, const Integer &e1, const Element &y, const Integer &e2);

// cascade exponentiation using vector addition chains
#ifdef BETTER_TEMPLATE_IMPLEMENTATION_AVAILABLE
template <class Group, class Element> Element GeneralCascadeMultiplication(const Group &group, const Element *const bases[], const Integer *const exponents[], unsigned int m);
template <class Field, class Element> Element GeneralCascadeExponentiation(const Field &field, const Element *const bases[], const Integer *const exponents[], unsigned int m);
#else
// I think the above is the right way to use templates, but this is needed
// to work around bugs in MSVC's template implementation
template <class Group, class Element> Element GeneralCascadeMultiplication(const Group &group, Element *const bases[], const Integer *const exponents[], unsigned int m);
template <class Field, class Element> Element GeneralCascadeExponentiation(const Field &field, Element *const bases[], const Integer *const exponents[], unsigned int m);
#endif

// template class for doing precomputed exponentiation in any group
template <class Group> class ExponentiationPrecomputation;
// template class wrapper that converts a field class into a class for its multiplicative group
template <class Field> class MultiplicativeGroup;

// *************** template implmentations ****************

#define iplus1  ( i==2 ? 0 : i+1 )	// used by Euclid algorithms
#define iminus1 ( i==0 ? 2 : i-1 )	// used by Euclid algorithms

template <class EDElement> EDElement GCD(const EDElement &a, const EDElement &n)
{
    EDElement g[3]={n, a};
	int i;

    for (i=1; !!g[i]; i = iplus1)
        g[iplus1] = g[iminus1] % g[i];

    return g[iminus1];
}

template <class EDElement> EDElement EuclideanMultiplicativeInverse(const EDElement &a, const EDElement &n)
{
    EDElement g[3]={n, a};
    EDElement v[3]={EDElement::ZERO, 1};
    EDElement y;
	int i;

    for (i=1; !!g[i]; i = iplus1)
    {
        // y = g[iminus1] / g[i];
        // g[iplus1] = g[iminus1] % g[i];
        Divide(g[iplus1], y, g[iminus1], g[i]);
        v[iplus1] = v[iminus1] - (v[i] * y);
    }

	if (g[iminus1] == 1)
	    return v[iminus1] % n;
	else
		return EDElement::ZERO;	// no inverse
}

#undef iplus1
#undef iminus1

template <class EDElement> EDElement LCM(const EDElement &a, const EDElement &n)
{
    return a*(n/GCD(a,n));
}

template <class Field> class MultiplicativeGroup
{
public:
	typedef Field::Element Element;

	MultiplicativeGroup(const Field &field)
		: field(field) {}

	Element Identity() const
		{return field.MultiplicativeIdentity();}

	Element Add(const Element &a, const Element &b) const
		{return field.Multiply(a, b);}

	Element Inverse(const Element &a) const
		{return field.MultiplicativeInverse(a);}

	Element Subtract(const Element &a, const Element &b) const
		{return field.Divide(a, b);}

	Element Double(const Element &a) const
		{return field.Square(a);}

	const Field &field;
};

/*  // multiplication using double and add
template <class Group, class Element> Element GeneralizedMultiplication(const Group &group, const Element &base, const Integer &exponent)
{
	Element result = group.Identity();
	unsigned i = exponent.BitCount();
	while (i--)
	{
		result = group.Double(result);
		if (exponent[i])
			result = group.Add(result, base);
	}
	return result;
}
*/

// a faster version of the above using a binary sliding window
template <class Group, class Element> Element GeneralizedMultiplication(const Group &group, const Element &base, const Integer &exponent)
{
	unsigned expLen = exponent.BitCount();
	if (expLen==0)
		return group.Identity();

	unsigned powerTableSize = (expLen <= 17 ? 1 : (expLen <= 24 ? 2 : (expLen <= 70 ? 4 : (expLen <= 197 ? 8 : (expLen <= 539 ? 16 : (expLen <= 1434 ? 32 : 64))))));
	vector<Element> powerTable(powerTableSize);

	powerTable[0] = base;
	if (powerTableSize > 1)
	{
		Element temp = group.Double(base);
		for (unsigned i=1; i<powerTableSize; i++)
			powerTable[i] = group.Add(temp, powerTable[i-1]);
	}

	Element result;
	unsigned power = 0, prevPosition = expLen-1;
	boolean firstTime = TRUE;

	for (int i = expLen-1; i>=0; i--)
	{
        power = 2*power + exponent[i];

		if (i==0 || power >= powerTableSize)
		{
			unsigned squaresBefore = prevPosition-i;
			unsigned squaresAfter = 0;
			prevPosition = i;
			while (power && power%2 == 0)
			{
				power /= 2;
				squaresBefore--;
				squaresAfter++;
			}
			if (firstTime)
			{
				result = powerTable[(power-1)/2];
				firstTime = FALSE;
			}
			else
			{
				while (squaresBefore--)
					result = group.Double(result);
				if (power)
					result = group.Add(powerTable[(power-1)/2], result);
			}
			while (squaresAfter--)
				result = group.Double(result);
			power = 0;
		}
	}
	return result;
}

/*
template <class Group, class Element> Element CascadeMultiplication(const Group &group, const Element &x, const Integer &e1, const Element &y, const Integer &e2)
{
	const Element xy = group.Add(x, y);
	Element result = group.Identity();
	unsigned i = max(e1.BitCount(), e2.BitCount());
	while (i--)
	{
		result = group.Double(result);
		switch (2*e1[i] + e2[i])
		{
		case 0:
			break;
		case 1:
			result = group.Add(result, y);
			break;
		case 2:
			result = group.Add(result, x);
			break;
		case 3:
			result = group.Add(result, xy);
			break;
		default:
			assert(FALSE);
		}
	}
	return result;
}
*/

// a faster version of the above using a binary sliding window
template <class Group, class Element> Element CascadeMultiplication(const Group &group, const Element &x, const Integer &e1, const Element &y, const Integer &e2)
{
	const unsigned expLen = max(e1.BitCount(), e2.BitCount());
	if (expLen==0)
		return group.Identity();

	const unsigned w = (expLen <= 46 ? 1 : (expLen <= 260 ? 2 : 3));
	const unsigned tableSize = 1<<w;
	vector<Element> powerTable(tableSize << w);

	powerTable[1] = x;
	powerTable[tableSize] = y;
	if (w==1)
		powerTable[3] = group.Add(x,y);
	else
	{
		powerTable[2] = group.Double(x);
		powerTable[2*tableSize] = group.Double(y);

		unsigned i, j;

		for (i=3; i<tableSize; i+=2)
			powerTable[i] = group.Add(powerTable[i-2], powerTable[2]);
		for (i=1; i<tableSize; i+=2)
			for (j=i+tableSize; j<(tableSize<<w); j+=tableSize)
				powerTable[j] = group.Add(powerTable[j-tableSize], y);

		for (i=3*tableSize; i<(tableSize<<w); i+=2*tableSize)
			powerTable[i] = group.Add(powerTable[i-2*tableSize], powerTable[2*tableSize]);
		for (i=tableSize; i<(tableSize<<w); i+=2*tableSize)
			for (j=i+2; j<i+tableSize; j+=2)
				powerTable[j] = group.Add(powerTable[j-1], x);
	}

	Element result;
	unsigned power1 = 0, power2 = 0, prevPosition = expLen-1;
	boolean firstTime = TRUE;

	for (int i = expLen-1; i>=0; i--)
	{
        power1 = 2*power1 + e1[i];
		power2 = 2*power2 + e2[i];

		if (i==0 || 2*power1 >= tableSize || 2*power2 >= tableSize)
		{
			unsigned squaresBefore = prevPosition-i;
			unsigned squaresAfter = 0;
			prevPosition = i;
			while ((power1 || power2) && power1%2 == 0 && power2%2==0)
			{
				power1 /= 2;
				power2 /= 2;
				squaresBefore--;
				squaresAfter++;
			}
			if (firstTime)
			{
				result = powerTable[(power2<<w) + power1];
				firstTime = FALSE;
			}
			else
			{
				while (squaresBefore--)
					result = group.Double(result);
				if (power1 || power2)
					result = group.Add(powerTable[(power2<<w) + power1], result);
			}
			while (squaresAfter--)
				result = group.Double(result);
			power1 = power2 = 0;
		}
	}
	return result;
}

#ifdef BETTER_TEMPLATE_IMPLEMENTATION_AVAILABLE
template <class Group, class Element> Element GeneralCascadeMultiplication(const Group &group, const Element *const bases[], const Integer *const exponents[], unsigned int m)
#else
template <class Group, class Element> Element GeneralCascadeMultiplication(const Group &group, Element *const bases[], const Integer *const exponents[], unsigned int m)
#endif
{
	assert(m>0);

	if (m==1)
		return GeneralizedMultiplication(group, *bases[0], (const Integer &)*exponents[0]);

	if (m==2)
		return CascadeMultiplication(group, *bases[0], (const Integer &)*exponents[0],
		                                    *bases[1], (const Integer &)*exponents[1]);

	vector<Element> b(m);
	vector<Integer> t(m);
	Integer q, r;
	unsigned int i, max=0, next=1;

	for (i=0; i<m; i++)
	{
		b[i] = *bases[i];
		t[i] = *exponents[i];
	}

	if (t[next] > t[max])
		swap(next, max);

	for (i=2; i<m; i++)
		if (t[i] > t[next])
		{
			if (t[i] > t[max])
			{
				next = max;
				max = i;
			}
			else
				next = i;
		}

	while (!!t[next])
	{
		Divide(r, q, t[max], t[next]);
		t[max] = r;
		if (q==Integer::ONE)
			b[next] = group.Add(b[max], b[next]);
		else
			b[next] = group.Add(GeneralizedMultiplication(group, b[max], (const Integer &)q), b[next]);
		max = next;
		next = next ? next-1 : m-1;
		for (i=0; i<m; i++)
			if (i != max && i!= next && t[i] > t[next])
				next = i;
	}

	return GeneralizedMultiplication(group, b[max], (const Integer &)t[max]);
}

template <class Field, class Element> Element GeneralizedExponentiation(const Field &field, const Element &base, const Integer &exponent)
{
    MultiplicativeGroup<Field> mg(field);
    return GeneralizedMultiplication(mg, base, exponent);
}

template <class Field, class Element> Element CascadeExponentiation(const Field &field, const Element &x, const Integer &e1, const Element &y, const Integer &e2)
{
    MultiplicativeGroup<Field> mg(field);
    return CascadeMultiplication(mg, x, e1, y, e2);
}

#ifdef BETTER_TEMPLATE_IMPLEMENTATION_AVAILABLE
template <class Field, class Element> Element GeneralCascadeExponentiation(const Field &field, const Element *const bases[], const Integer *const exponents[], unsigned int m)
#else
template <class Field, class Element> Element GeneralCascadeExponentiation(const Field &field, Element *const bases[], const Integer *const exponents[], unsigned int m)
#endif
{
    MultiplicativeGroup<Field> mg(field);
    return GeneralCascadeMultiplication(mg, bases, exponents, m);
}

// Please do not directly use the following class.  It should be
// considered a private class for the library.  The following
// classes are public and use ExponentiationPrecomputation internally.
//
// ModExpPrecomputation;
// ECPrecomputation<EC2N>;
// ECPrecomputation<ECP>;

template <class Group> class ExponentiationPrecomputation
{
public:
	typedef Group::Element Element;

	ExponentiationPrecomputation(const Group &group) : group(group) {}

	ExponentiationPrecomputation(const Group &group, const Element &base, unsigned int maxExpBits, unsigned int storage)
		: group(group), storage(storage), exponentBase(Integer::Power2((maxExpBits+storage-1)/storage)), 
		  g(storage)
	{
		assert(storage <= maxExpBits);

		g[0] = base;
		for (unsigned i=1; i<storage; i++)
			g[i] = GeneralizedMultiplication(group, g[i-1], (const Integer &)exponentBase);
	}

	ExponentiationPrecomputation(const Group &group, const ExponentiationPrecomputation &pc)
		: group(group), storage(pc.storage), exponentBase(pc.exponentBase), g(pc.g)	{}

	Element Exponentiate(const Integer &exponent) const;
	Element CascadeExponentiate(const Integer &exponent, const ExponentiationPrecomputation<Group> &pc2, const Integer &exponent2) const;

	const Group &group;
	unsigned int storage;	// number of precalculated bases
	Integer exponentBase;	// what base to represent the exponent in
	vector<Element> g;		// precalculated bases
};

template <class Group> ExponentiationPrecomputation<Group>::Element ExponentiationPrecomputation<Group>::Exponentiate(const Integer &exponent) const
{
	vector<Integer> x(storage);				// array of segments of the exponent
	vector<const Integer *> xPtrs(storage);	// array of pointers to segments of the exponent
	vector<const Element *> gPtrs(storage);	// array of pointers to precalculated bases
	Integer temp, e = exponent;
	unsigned i;

	for (i=0; i+1<storage; i++)
	{
		Divide(x[i], temp, e, exponentBase);
		xPtrs[i] = &x[i];
		gPtrs[i] = &g[i];
		swap(temp, e);
	}
	x[i] = e;
	xPtrs[i] = &x[i];
	gPtrs[i] = &g[i];

#ifdef BETTER_TEMPLATE_IMPLEMENTATION_AVAILABLE
	return GeneralCascadeMultiplication(group, gPtrs.begin(), xPtrs, storage);
#else
	return GeneralCascadeMultiplication(group, (Element *const *)gPtrs.begin(), (const Integer *const *)xPtrs.begin(), storage);
#endif
}

template <class Group> ExponentiationPrecomputation<Group>::Element 
	ExponentiationPrecomputation<Group>::CascadeExponentiate(const Integer &exponent, 
		const ExponentiationPrecomputation<Group> &pc2, const Integer &exponent2) const
{
	vector<Integer> x(storage+pc2.storage);				// array of segments of the exponent
	vector<const Integer *> xPtrs(storage+pc2.storage);	// array of pointers to segments of the exponent
	vector<const Element *> gPtrs(storage+pc2.storage);	// array of pointers to precalculated bases
	Integer temp, e = exponent;
	unsigned i;

	for (i=0; i+1<storage; i++)
	{
		Divide(x[i], temp, e, exponentBase);
		xPtrs[i] = &x[i];
		gPtrs[i] = &g[i];
		swap(temp, e);
	}
	x[i] = e;
	xPtrs[i] = &x[i];
	gPtrs[i] = &g[i];

	e = exponent2;
	for (i=storage; i+1<storage+pc2.storage; i++)
	{
		Divide(x[i], temp, e, pc2.exponentBase);
		xPtrs[i] = &x[i];
		gPtrs[i] = &pc2.g[i-storage];
		swap(temp, e);
	}
	x[i] = e;
	xPtrs[i] = &x[i];
	gPtrs[i] = &pc2.g[i-storage];

#ifdef BETTER_TEMPLATE_IMPLEMENTATION_AVAILABLE
	return GeneralCascadeMultiplication(group, gPtrs.begin(), xPtrs, storage+pc2.storage);
#else
	return GeneralCascadeMultiplication(group, (Element *const *)gPtrs.begin(), (const Integer *const *)xPtrs.begin(), storage+pc2.storage);
#endif
}

class ModExpPrecomputation
{
public:
	ModExpPrecomputation() {}
	ModExpPrecomputation(const ModExpPrecomputation &mep);
	ModExpPrecomputation(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage);
	
	void Precompute(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage);
	void Load(const Integer &modulus, BufferedTransformation &storedPrecomputation);
	void Save(BufferedTransformation &storedPrecomputation) const;

	Integer Exponentiate(const Integer &exponent) const;
	Integer CascadeExponentiate(const Integer &exponent, ModExpPrecomputation pc2, const Integer &exponent2) const;

private:
	typedef MultiplicativeGroup<MontgomeryRepresentation> MR_MG;
	auto_ptr<MontgomeryRepresentation> mr;
	auto_ptr<MR_MG> mg;
	auto_ptr< ExponentiationPrecomputation<MR_MG> > ep;
};

#endif
