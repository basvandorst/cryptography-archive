#ifndef ECBASIC_H
#define ECBASIC_H

#include "nbtheory.h"
#include "gf2n.h"

class EC2N
{
public:

	typedef GF2N Field;
	typedef Field::Element FieldElement;

	struct Point
	{
		Point() : identity(TRUE) {}
		Point(const Field::Element &x, const Field::Element &y)
			: identity(FALSE), x(x), y(y) {}

		boolean identity;
		Field::Element x, y;
	};

	typedef Point Element;

	EC2N(const Field &field, const Field::Element &a, const Field::Element &b)
		: field(field), a(a), b(b) {}

	boolean Equal(const Point &P, const Point &Q) const;
	Point Identity() const {return Point();}
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
	Field field;
	FieldElement a, b;
};

class ECP
{
public:

	struct Point
	{
		Point() : identity(TRUE) {}
		Point(const Integer &x, const Integer &y)
			: identity(FALSE), x(x), y(y) {}

		boolean identity;
		Integer x, y;
	};

	typedef ModularArithmetics Field;
	typedef Integer FieldElement;
	typedef Point Element;

	ECP(const Integer &modulus, const FieldElement &a, const FieldElement &b)
		: fieldPtr(new Field(modulus)), field(*fieldPtr), a(a), b(b) {}
	ECP(const MontgomeryRepresentation &mr, const FieldElement &a, const FieldElement &b)
		: field(mr), a(a), b(b) {}
	ECP(const ECP &ecp)
		: fieldPtr(new Field(ecp.field.Modulus())), field(*fieldPtr), a(ecp.a), b(ecp.b) {}

	boolean Equal(const Point &P, const Point &Q) const;
	Point Identity() const {return Point();}
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
	auto_ptr<Field> fieldPtr;
	const Field &field;
	FieldElement a, b;
};

template <class T> class ECPrecomputation;

class ECPrecomputation<EC2N>
{
public:
	ECPrecomputation() {}

	ECPrecomputation(const ECPrecomputation<EC2N> &ecp)
        : ec(new EC2N(*ecp.ec)),
          ep(new ExponentiationPrecomputation<EC2N>(*ec, *ecp.ep))
    {
    }

	ECPrecomputation(const EC2N &ecIn, const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage)
        : ec(new EC2N(ecIn)), ep(NULL)
    {
        Precompute(base, maxExpBits, storage);
    }

	void Precompute(const EC2N::Point &base, unsigned int maxExpBits, unsigned int storage);
	void Load(BufferedTransformation &storedPrecomputation);
	void Save(BufferedTransformation &storedPrecomputation) const;

	EC2N::Point Multiply(const Integer &exponent) const;
	EC2N::Point CascadeMultiply(const Integer &exponent, ECPrecomputation<EC2N> pc2, const Integer &exponent2) const;

private:
	auto_ptr<EC2N> ec;
	auto_ptr< ExponentiationPrecomputation<EC2N> > ep;
};

class ECPrecomputation<ECP>
{
public:
	ECPrecomputation() {}

	ECPrecomputation(const ECPrecomputation<ECP> &ecp)
		: mr(new MontgomeryRepresentation(*ecp.mr)),
		  ec(new ECP(*mr, ecp.ec->GetA(), ecp.ec->GetB())),
		  ep(new ExponentiationPrecomputation<ECP>(*ec, *ecp.ep))
	{
	}

	ECPrecomputation(const ECP &ecIn, const ECP::Point &base, unsigned int maxExpBits, unsigned int storage)
		: mr(new MontgomeryRepresentation(ecIn.GetField().Modulus())),
		  ec(new ECP(*mr, mr->ConvertIn(ecIn.GetA()), mr->ConvertIn(ecIn.GetB()))), 
		  ep(NULL)
	{
		Precompute(base, maxExpBits, storage);
	}

	void Precompute(const ECP::Point &base, unsigned int maxExpBits, unsigned int storage);
	void Load(BufferedTransformation &storedPrecomputation);
	void Save(BufferedTransformation &storedPrecomputation) const;

	ECP::Point Multiply(const Integer &exponent) const;
	ECP::Point CascadeMultiply(const Integer &exponent, ECPrecomputation<ECP> pc2, const Integer &exponent2) const;

private:
	auto_ptr<MontgomeryRepresentation> mr;
	auto_ptr<ECP> ec;
	auto_ptr< ExponentiationPrecomputation<ECP> > ep;
};

#endif
