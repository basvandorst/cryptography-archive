// ecp.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "ecp.h"
#include "asn.h"
#include "nbtheory.h"

#include "algebra.cpp"
#include "eprecomp.cpp"

bool ECP::Equal(const Point &P, const Point &Q) const
{
	if (P.identity && Q.identity)
		return true;

	if (P.identity && !Q.identity)
		return false;

	if (!P.identity && Q.identity)
		return false;

	return (field.Equal(P.x,Q.x) && field.Equal(P.y,Q.y));
}

ECP::Point ECP::Inverse(const Point &P) const
{
	if (P.identity)
		return P;
	else
		return Point(P.x, field.Inverse(P.y));
}

ECP::Point ECP::Add(const Point &P, const Point &Q) const
{
	if (P.identity) return Q;
	if (Q.identity) return P;
	if (Equal(P, Q)) return Double(P);
	if (field.Equal(P.x, Q.x) && field.Equal(P.y, field.Inverse(Q.y))) return Zero();

	Point R(Q.x, Q.y);
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

ECP::Point ECP::Double(const Point &P) const
{
	if (P.identity) return P;

	GFP::Element t = field.Square(P.x);
	Point R(t, P.y);
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

ECP::Point ECP::Multiply(const Integer &k, const Point &P) const
{
//	return GeneralizedMultiplication(*this, P, k);
	MontgomeryRepresentation mr(field.GetModulus());
	ECP ecpmr(mr, mr.ConvertIn(a), mr.ConvertIn(b));
	Point R(mr.ConvertIn(P.x), mr.ConvertIn(P.y));
	R = ecpmr.IntMultiply(R, k);
	return Point(mr.ConvertOut(R.x), mr.ConvertOut(R.y));
}

ECP::Point ECP::CascadeMultiply(const Integer &k1, const Point &P, const Integer &k2, const Point &Q) const
{
//	return CascadeMultiplication(*this, P, k1, Q, k2);
	MontgomeryRepresentation mr(field.GetModulus());
	ECP ecpmr(mr, mr.ConvertIn(a), mr.ConvertIn(b));
	Point R1(mr.ConvertIn(P.x), mr.ConvertIn(P.y));
	Point R2(mr.ConvertIn(Q.x), mr.ConvertIn(Q.y));
	R1 = ecpmr.CascadeIntMultiply(R1, k1, R2, k2);
	return Point(mr.ConvertOut(R1.x), mr.ConvertOut(R1.y));
}

Integer ECP::Order(const Point &P, const Integer &cardinality) const
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

EcPrecomputation<ECP>::EcPrecomputation()
{
}

EcPrecomputation<ECP>::EcPrecomputation(const EcPrecomputation<ECP> &ecp)
	: mr(new MontgomeryRepresentation(*ecp.mr))
	, ec(new ECP(*mr, ecp.ec->GetA(), ecp.ec->GetB()))
	, ep(new ExponentiationPrecomputation<ECP>(*ec, *ecp.ep))
{
}

EcPrecomputation<ECP>::EcPrecomputation(const ECP &ecIn, const ECP::Point &base, unsigned int maxExpBits, unsigned int storage)
	: mr(new MontgomeryRepresentation(ecIn.GetField().GetModulus()))
	, ec(new ECP(*mr, mr->ConvertIn(ecIn.GetA()), mr->ConvertIn(ecIn.GetB())))
	, ep(NULL)
{
	Precompute(base, maxExpBits, storage);
}

EcPrecomputation<ECP>::~EcPrecomputation()
{
}

void EcPrecomputation<ECP>::Precompute(const ECP::Point &base, unsigned int maxExpBits, unsigned int storage)
{
	ECP::Point newBase(mr->ConvertIn(base.x), mr->ConvertIn(base.y));
	ep.reset(new ExponentiationPrecomputation<ECP>(*ec, newBase, maxExpBits, storage));
}

void EcPrecomputation<ECP>::Load(BufferedTransformation &bt)
{
	ep.reset(new ExponentiationPrecomputation<ECP>(*ec));
	BERSequenceDecoder seq(bt);
	ep->storage = (unsigned int)(Integer(seq).ConvertToLong());
	ep->exponentBase.BERDecode(seq);
	ep->g.resize(ep->storage);

	for (unsigned i=0; i<ep->storage; i++)
	{
		ep->g[i].identity = false;
		ep->g[i].x.BERDecode(seq);
		ep->g[i].y.BERDecode(seq);
	}
}

void EcPrecomputation<ECP>::Save(BufferedTransformation &bt) const
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

ECP::Point EcPrecomputation<ECP>::Multiply(const Integer &exponent) const
{
	assert(ep.get());
	ECP::Point result = ep->Exponentiate(exponent);
	return ECP::Point(mr->ConvertOut(result.x), mr->ConvertOut(result.y));
}

ECP::Point EcPrecomputation<ECP>::CascadeMultiply(const Integer &exponent, 
							EcPrecomputation<ECP> pc2, const Integer &exponent2) const
{
	assert(ep.get());
	ECP::Point result = ep->CascadeExponentiate(exponent, *pc2.ep, exponent2);
	return ECP::Point(mr->ConvertOut(result.x), mr->ConvertOut(result.y));
}
