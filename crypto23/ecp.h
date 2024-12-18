#ifndef ECP_H
#define ECP_H

#include "modarith.h"
#include "eprecomp.h"
#include "smartptr.h"

struct ECPPoint
{
	ECPPoint() : identity(true) {}
	ECPPoint(const Integer &x, const Integer &y)
		: identity(false), x(x), y(y) {}

	bool operator==(const ECPPoint &t) const
		{return (identity && t.identity) || (!identity && !t.identity && x==t.x && y==t.y);}
	bool operator< (const ECPPoint &t) const
		{return identity ? !t.identity : (t.identity && (x<t.x || (x<=t.x && y<t.y)));}

	bool identity;
	Integer x, y;
};

class ECP : public AbstractGroup<ECPPoint>
{
public:
	typedef ModularArithmetic Field;
	typedef Integer FieldElement;

	typedef ECPPoint Point;

	ECP(const Integer &modulus, const FieldElement &a, const FieldElement &b)
		: fieldPtr(new Field(modulus)), field(*fieldPtr), a(a), b(b) {}
	ECP(const MontgomeryRepresentation &mr, const FieldElement &a, const FieldElement &b)
		: field(mr), a(a), b(b) {}
	ECP(const ECP &ecp)
		: fieldPtr(new Field(ecp.field.GetModulus())), field(*fieldPtr), a(ecp.a), b(ecp.b) {}

	bool Equal(const Point &P, const Point &Q) const;
	Point Zero() const {return Point();}
	Point Inverse(const Point &P) const;
	Point Add(const Point &P, const Point &Q) const;
	Point Double(const Point &P) const;
	Point Multiply(const Integer &k, const Point &P) const;
	Point CascadeMultiply(const Integer &k1, const Point &P, const Integer &k2, const Point &Q) const;

//	Integer Cardinality() const;	TODO: implement this

	// compute order of a point, given the cardinality of this curve
	Integer Order(const Point &P, const Integer &cardinality) const;

	const Field & GetField() const {return field;}

	const FieldElement & GetA() const {return a;}
	const FieldElement & GetB() const {return b;}

private:
	member_ptr<Field> fieldPtr;
	const Field &field;
	FieldElement a, b;
};

template <class T> class EcPrecomputation;

class EcPrecomputation<ECP>
{
public:
	EcPrecomputation();
	EcPrecomputation(const EcPrecomputation<ECP> &ecp);
	EcPrecomputation(const ECP &ecIn, const ECP::Point &base, unsigned int maxExpBits, unsigned int storage);
	~EcPrecomputation();

	void Precompute(const ECP::Point &base, unsigned int maxExpBits, unsigned int storage);
	void Load(BufferedTransformation &storedPrecomputation);
	void Save(BufferedTransformation &storedPrecomputation) const;

	ECP::Point Multiply(const Integer &exponent) const;
	ECP::Point CascadeMultiply(const Integer &exponent, EcPrecomputation<ECP> pc2, const Integer &exponent2) const;

private:
	member_ptr<MontgomeryRepresentation> mr;
	member_ptr<ECP> ec;
	member_ptr< ExponentiationPrecomputation<ECP> > ep;
};

#endif
