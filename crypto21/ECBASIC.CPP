// ecbasic.cpp - written and placed in the public domain by Wei Dai

#include "misc.h"
#include "asn.h"
#include "nbtheory.h"

#include "ecbasic.h"

boolean EC2N::Equal(const EC2N::Point &P, const EC2N::Point &Q) const
{
	if (P.identity && Q.identity)
		return TRUE;

	if (P.identity && !Q.identity)
		return FALSE;

	if (!P.identity && Q.identity)
		return FALSE;

	return (field.Equal(P.x,Q.x) && field.Equal(P.y,Q.y));
}

EC2N::Point EC2N::Inverse(const EC2N::Point &P) const
{
    if (P.identity)
        return P;
    else
        return Point(P.x, field.Add(P.x, P.y));
}

EC2N::Point EC2N::Add(const EC2N::Point &P, const EC2N::Point &Q) const
{
	if (P.identity) return Q;
	if (Q.identity) return P;
	if (Equal(P, Q)) return Double(P);
	if (field.Equal(P.x, Q.x) && field.Equal(P.y, field.Add(Q.x, Q.y))) return Identity();

	Field::Element t = field.Divide(field.Add(P.y, Q.y), field.Add(P.x, Q.x));
	EC2N::Point R;
	R.identity = FALSE;
	R.x = field.Square(t);
	field.Accumulate(R.x, t);
	field.Accumulate(R.x, Q.x);
	field.Accumulate(R.x, a);
	R.y = field.Multiply(t, R.x);
	field.Accumulate(R.x, P.x);
	field.Accumulate(R.y, R.x);
	field.Accumulate(R.y, P.y);
	return R;
}

EC2N::Point EC2N::Double(const EC2N::Point &P) const
{
	if (P.identity) return P;
	if (field.Equal(P.x, field.Identity())) return Identity();

	FieldElement t = P.x;
	field.Accumulate(t, field.Divide(P.y, P.x));
	EC2N::Point R(field.Square(t), field.Square(P.x));
	field.Accumulate(R.x, t);
	field.Accumulate(R.x, a);
	field.Accumulate(R.y, field.Multiply(t, R.x));
	field.Accumulate(R.y, R.x);
	return R;
}

EC2N::Point EC2N::Multiply(const Integer &k, const EC2N::Point &P) const
{
	return GeneralizedMultiplication(*this, P, k);
}

EC2N::Point EC2N::CascadeMultiply(const Integer &k1, const Point &P, const Integer &k2, const Point &Q) const
{
	return CascadeMultiplication(*this, P, k1, Q, k2);
}

Integer EC2N::Order(const EC2N::Point &P, const Integer &cardinality) const
{
	Point Q;
	Integer p = cardinality, n = cardinality;

	assert(100<=primeTableSize);

	for (unsigned int i=0; i<100; i++)
		if (p%primeTable[i]==0)
		{
			do
			{
				n /= primeTable[i];
				p /= primeTable[i];
			}
			while (p%primeTable[i]==0);

			Q = Multiply(n, P);
			while (!Q.identity)
			{
				Q = Multiply(primeTable[i], Q);
				n *= primeTable[i];
				assert(n<=cardinality);
			}
		}

	if (p==1)
		return n;

	assert(IsPrime(p));
	p = n/p;
	Q = Multiply(p, P);
	if (Q.identity)
		return p;
	else
		return n;
}

// ******************************************************************

boolean ECP::Equal(const ECP::Point &P, const ECP::Point &Q) const
{
	if (P.identity && Q.identity)
		return TRUE;

	if (P.identity && !Q.identity)
		return FALSE;

	if (!P.identity && Q.identity)
		return FALSE;

	return (field.Equal(P.x,Q.x) && field.Equal(P.y,Q.y));
}

ECP::Point ECP::Inverse(const ECP::Point &P) const
{
	if (P.identity)
        return P;
    else
        return Point(P.x, field.Inverse(P.y));
}

ECP::Point ECP::Add(const ECP::Point &P, const ECP::Point &Q) const
{
	if (P.identity) return Q;
	if (Q.identity) return P;
	if (Equal(P, Q)) return Double(P);
	if (field.Equal(P.x, Q.x) && field.Equal(P.y, field.Inverse(Q.y))) return Identity();

	ECP::Point R(Q.x, Q.y);
	field.Reduce(R.x, P.x);
	field.Reduce(R.y, P.y);
	GFP::Element t = field.Divide(R.y, R.x);
	R.x = field.Square(t);
	field.Reduce(R.x, P.x);
	field.Reduce(R.x, Q.x);
	R.y = field.Subtract(P.x, R.x);
	R.y = field.Multiply(t, R.y);
	field.Reduce(R.y, P.y);
	return R;
}

ECP::Point ECP::Double(const ECP::Point &P) const
{
	if (P.identity) return P;

	GFP::Element t = field.Square(P.x);
	ECP::Point R(t, P.y);
	field.Accumulate(R.x, R.x);
	field.Accumulate(R.y, R.y);
	field.Accumulate(t, R.x);
	field.Accumulate(t, a);
	t = field.Divide(t, R.y);
	R.x = field.Square(t);
	R.y = field.Double(P.x);
	field.Reduce(R.x, R.y);
	R.y = field.Subtract(P.x, R.x);
	R.y = field.Multiply(t, R.y);
	field.Reduce(R.y, P.y);
	return R;
}

ECP::Point ECP::Multiply(const Integer &k, const ECP::Point &P) const
{
//	return GeneralizedMultiplication(*this, P, k);
	MontgomeryRepresentation mr(field.Modulus());
	ECP ecpmr(mr, mr.ConvertIn(a), mr.ConvertIn(b));
	Point R(mr.ConvertIn(P.x), mr.ConvertIn(P.y));
	R = GeneralizedMultiplication(ecpmr, R, k);
	return ECP::Point(mr.ConvertOut(R.x), mr.ConvertOut(R.y));
}

ECP::Point ECP::CascadeMultiply(const Integer &k1, const Point &P, const Integer &k2, const Point &Q) const
{
//	return CascadeMultiplication(*this, P, k1, Q, k2);
	MontgomeryRepresentation mr(field.Modulus());
	ECP ecpmr(mr, mr.ConvertIn(a), mr.ConvertIn(b));
	Point R1(mr.ConvertIn(P.x), mr.ConvertIn(P.y));
	Point R2(mr.ConvertIn(Q.x), mr.ConvertIn(Q.y));
	R1 = CascadeMultiplication(ecpmr, R1, k1, R2, k2);
	return ECP::Point(mr.ConvertOut(R1.x), mr.ConvertOut(R1.y));
}

Integer ECP::Order(const ECP::Point &P, const Integer &cardinality) const
{
	Point Q;
	Integer p = cardinality, n = cardinality;

	assert(100<=primeTableSize);

	for (unsigned int i=0; i<100; i++)
		if (p%primeTable[i]==0)
		{
			do
			{
				n /= primeTable[i];
				p /= primeTable[i];
			}
			while (p%primeTable[i]==0);

			Q = Multiply(n, P);
			while (!Q.identity)
			{
				Q = Multiply(primeTable[i], Q);
				n *= primeTable[i];
				assert(n<=cardinality);
			}
		}

	if (p==1)
		return n;

	assert(IsPrime(p));
	p = n/p;
	Q = Multiply(p, P);
	if (Q.identity)
		return p;
	else
		return n;
}

// ********************************************************
/*
ECPrecomputation<EC2N>::ECPrecomputation<EC2N>()

ECPrecomputation<EC2N>::ECPrecomputation<EC2N>(const EC2N &ecIn, const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage)

ECPrecomputation<EC2N>::ECPrecomputation<EC2N>(const ECPrecomputation<EC2N> &ecp)

ECPrecomputation<EC2N>::~ECPrecomputation<EC2N>()
*/

void ECPrecomputation<EC2N>::Precompute(const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage)
{
	if (!ep.get() || ep->storage < storage)
		ep.reset(new ExponentiationPrecomputation<EC2N>(*ec, base, maxExpBits, storage));
}

void ECPrecomputation<EC2N>::Load(BufferedTransformation &bt)
{
	ep.reset(new ExponentiationPrecomputation<EC2N>(*ec));
	BERSequenceDecoder seq(bt);
	ep->storage = (unsigned int)(Integer(seq).ConvertToLong());
	ep->exponentBase.BERDecode(seq);
	ep->g.reserve(ep->storage);

	unsigned int size = ec->GetField().MaxElementByteLength();
	SecByteBlock buffer(size);
	for (unsigned i=0; i<ep->storage; i++)
	{
		ep->g[i].identity = FALSE;
		seq.Get(buffer, size);
		ep->g[i].x.Decode(buffer, size);
		seq.Get(buffer, size);
		ep->g[i].y.Decode(buffer, size);
	}
}

void ECPrecomputation<EC2N>::Save(BufferedTransformation &bt) const
{
	assert(ep.get());
	DERSequenceEncoder seq(bt);
	Integer(ep->storage).DEREncode(seq);
	ep->exponentBase.DEREncode(seq);

	unsigned int size = ec->GetField().MaxElementByteLength();
	SecByteBlock buffer(size);
	for (unsigned i=0; i<ep->storage; i++)
	{
		ep->g[i].x.Encode(buffer, size);
		seq.Put(buffer, size);
		ep->g[i].y.Encode(buffer, size);
		seq.Put(buffer, size);
	}
}

EC2N::Point ECPrecomputation<EC2N>::Multiply(const Integer &exponent) const
{
	assert(ep.get());
	return ep->Exponentiate(exponent);
}

EC2N::Point ECPrecomputation<EC2N>::CascadeMultiply(const Integer &exponent, 
		                    ECPrecomputation<EC2N> pc2, const Integer &exponent2) const
{
	assert(ep.get());
	return ep->CascadeExponentiate(exponent, *pc2.ep, exponent2);
}

// ********************************************************

/*
ECPrecomputation<ECP>::ECPrecomputation<ECP>()

ECPrecomputation<ECP>::ECPrecomputation<ECP>(const ECP &ecIn, const ECP::Point &base, unsigned int maxExpBits, unsigned int storage)

ECPrecomputation<ECP>::ECPrecomputation<ECP>(const ECPrecomputation<ECP> &ecp)

ECPrecomputation<ECP>::~ECPrecomputation<ECP>()
*/

void ECPrecomputation<ECP>::Precompute(const ECP::Point &base, unsigned int maxExpBits, unsigned int storage)
{
	ECP::Point newBase(mr->ConvertIn(base.x), mr->ConvertIn(base.y));
	ep.reset(new ExponentiationPrecomputation<ECP>(*ec, newBase, maxExpBits, storage));
}

void ECPrecomputation<ECP>::Load(BufferedTransformation &bt)
{
	ep.reset(new ExponentiationPrecomputation<ECP>(*ec));
	BERSequenceDecoder seq(bt);
	ep->storage = (unsigned int)(Integer(seq).ConvertToLong());
	ep->exponentBase.BERDecode(seq);
	ep->g.reserve(ep->storage);

	for (unsigned i=0; i<ep->storage; i++)
	{
		ep->g[i].identity = FALSE;
		ep->g[i].x.BERDecode(seq);
		ep->g[i].y.BERDecode(seq);
	}
}

void ECPrecomputation<ECP>::Save(BufferedTransformation &bt) const
{
	assert(ep.get());
	DERSequenceEncoder seq(bt);
	Integer(ep->storage).DEREncode(seq);
	ep->exponentBase.DEREncode(seq);

	for (unsigned i=0; i<ep->storage; i++)
	{
		ep->g[i].x.DEREncode(seq);
		ep->g[i].y.DEREncode(seq);
	}
}

ECP::Point ECPrecomputation<ECP>::Multiply(const Integer &exponent) const
{
	assert(ep.get());
	ECP::Point result = ep->Exponentiate(exponent);
	return ECP::Point(mr->ConvertOut(result.x), mr->ConvertOut(result.y));
}

ECP::Point ECPrecomputation<ECP>::CascadeMultiply(const Integer &exponent, 
		                    ECPrecomputation<ECP> pc2, const Integer &exponent2) const
{
	assert(ep.get());
	ECP::Point result = ep->CascadeExponentiate(exponent, *pc2.ep, exponent2);
	return ECP::Point(mr->ConvertOut(result.x), mr->ConvertOut(result.y));
}
