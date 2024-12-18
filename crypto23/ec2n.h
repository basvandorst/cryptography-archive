#ifndef EC2N_H
#define EC2N_H

#include "gf2n.h"
#include "eprecomp.h"
#include "smartptr.h"

struct EC2NPoint
{
	EC2NPoint() : identity(true) {}
	EC2NPoint(const PolynomialMod2 &x, const PolynomialMod2 &y)
		: identity(false), x(x), y(y) {}

	bool operator==(const EC2NPoint &t) const
		{return (identity && t.identity) || (!identity && !t.identity && x==t.x && y==t.y);}
	bool operator< (const EC2NPoint &t) const
		{return identity ? !t.identity : (t.identity && (x<t.x || (x<=t.x && y<t.y)));}

	bool identity;
	PolynomialMod2 x, y;
};

class EC2N : public AbstractGroup<EC2NPoint>
{
public:
	typedef GF2N Field;
	typedef Field::Element FieldElement;

	typedef EC2NPoint Point;

	EC2N(const Field &field, const Field::Element &a, const Field::Element &b)
		: field(field), a(a), b(b) {}

	bool Equal(const Point &P, const Point &Q) const;
	Point Zero() const {return Point();}
	Point Inverse(const Point &P) const;
	Point Add(const Point &P, const Point &Q) const;
	Point Double(const Point &P) const;

	Point Multiply(const Integer &k, const Point &P) const
		{return IntMultiply(P, k);}
	Point CascadeMultiply(const Integer &k1, const Point &P, const Integer &k2, const Point &Q) const
		{return CascadeIntMultiply(P, k1, Q, k2);}

//	Integer Cardinality() const;	TODO: implement this

	// compute order of a point, given the cardinality of this curve
	Integer Order(const Point &P, const Integer &cardinality) const;

	const Field & GetField() const {return field;}

	const FieldElement & GetA() const {return a;}
	const FieldElement & GetB() const {return b;}

private:
	Field field;
	FieldElement a, b;
};

template <class T> class EcPrecomputation;

class EcPrecomputation<EC2N>
{
public:
	EcPrecomputation();
	EcPrecomputation(const EcPrecomputation<EC2N> &ecp);
	EcPrecomputation(const EC2N &ecIn, const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage);
	~EcPrecomputation();

	void Precompute(const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage);
	void Load(BufferedTransformation &storedPrecomputation);
	void Save(BufferedTransformation &storedPrecomputation) const;

	EC2N::Point Multiply(const Integer &exponent) const;
	EC2N::Point CascadeMultiply(const Integer &exponent, EcPrecomputation<EC2N> pc2, const Integer &exponent2) const;

private:
	member_ptr<EC2N> ec;
	member_ptr< ExponentiationPrecomputation<EC2N> > ep;
};

#endif
