// ec2n.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "ec2n.h"
#include "asn.h"
#include "nbtheory.h"	// for primeTable

#include "algebra.cpp"
#include "eprecomp.cpp"

bool EC2N::Equal(const Point &P, const Point &Q) const
{
	if (P.identity && Q.identity)
		return true;

	if (P.identity && !Q.identity)
		return false;

	if (!P.identity && Q.identity)
		return false;

	return (field.Equal(P.x,Q.x) && field.Equal(P.y,Q.y));
}

EC2N::Point EC2N::Inverse(const Point &P) const
{
	if (P.identity)
		return P;
	else
		return Point(P.x, field.Add(P.x, P.y));
}

EC2N::Point EC2N::Add(const Point &P, const Point &Q) const
{
	if (P.identity) return Q;
	if (Q.identity) return P;
	if (Equal(P, Q)) return Double(P);
	if (field.Equal(P.x, Q.x) && field.Equal(P.y, field.Add(Q.x, Q.y))) return Zero();

	Field::Element t = field.Divide(field.Add(P.y, Q.y), field.Add(P.x, Q.x));
	Point R(field.Square(t), P.y);
	field.Accumulate(R.x, t);
	field.Accumulate(R.x, Q.x);
	field.Accumulate(R.x, a);
	field.Accumulate(R.y, field.Multiply(t, R.x));
	field.Accumulate(R.x, P.x);
	field.Accumulate(R.y, R.x);
	return R;
}

EC2N::Point EC2N::Double(const Point &P) const
{
	if (P.identity) return P;
	if (!field.IsUnit(P.x)) return Zero();

	FieldElement t = field.Divide(P.y, P.x);
	field.Accumulate(t, P.x);
	Point R(field.Square(t), field.Square(P.x));
	field.Accumulate(R.x, t);
	field.Accumulate(R.x, a);
	field.Accumulate(R.y, field.Multiply(t, R.x));
	field.Accumulate(R.y, R.x);
	return R;
}

Integer EC2N::Order(const Point &P, const Integer &cardinality) const
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

EcPrecomputation<EC2N>::EcPrecomputation()
{
}

EcPrecomputation<EC2N>::EcPrecomputation(const EcPrecomputation<EC2N> &ecp)
	: ec(new EC2N(*ecp.ec))
	, ep(new ExponentiationPrecomputation<EC2N>(*ec, *ecp.ep))
{
}

EcPrecomputation<EC2N>::EcPrecomputation(const EC2N &ecIn, const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage)
	: ec(new EC2N(ecIn)), ep(NULL)
{
	Precompute(base, maxExpBits, storage);
}

EcPrecomputation<EC2N>::~EcPrecomputation()
{
}

void EcPrecomputation<EC2N>::Precompute(const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage)
{
	if (!ep.get() || ep->storage < storage)
		ep.reset(new ExponentiationPrecomputation<EC2N>(*ec, base, maxExpBits, storage));
}

void EcPrecomputation<EC2N>::Load(BufferedTransformation &bt)
{
	ep.reset(new ExponentiationPrecomputation<EC2N>(*ec));
	BERSequenceDecoder seq(bt);
	ep->storage = (unsigned int)(Integer(seq).ConvertToLong());
	ep->exponentBase.BERDecode(seq);
	ep->g.resize(ep->storage);

	unsigned int size = ec->GetField().MaxElementByteLength();
	SecByteBlock buffer(size);
	for (unsigned i=0; i<ep->storage; i++)
	{
		ep->g[i].identity = false;
		seq.Get(buffer, size);
		ep->g[i].x.Decode(buffer, size);
		seq.Get(buffer, size);
		ep->g[i].y.Decode(buffer, size);
	}
}

void EcPrecomputation<EC2N>::Save(BufferedTransformation &bt) const
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

EC2N::Point EcPrecomputation<EC2N>::Multiply(const Integer &exponent) const
{
	assert(ep.get());
	return ep->Exponentiate(exponent);
}

EC2N::Point EcPrecomputation<EC2N>::CascadeMultiply(const Integer &exponent, 
							EcPrecomputation<EC2N> pc2, const Integer &exponent2) const
{
	assert(ep.get());
	return ep->CascadeExponentiate(exponent, *pc2.ep, exponent2);
}
