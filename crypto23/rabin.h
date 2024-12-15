#ifndef RABIN_H
#define RABIN_H

#include "oaep.h"
#include "integer.h"

class SHA;

class RabinFunction : virtual public TrapdoorFunction
{
public:
	RabinFunction(const Integer &n, const Integer &r, const Integer &s);
	RabinFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer ApplyFunction(const Integer &x) const;
	Integer MaxPreimage() const {return n-1;}
	Integer MaxImage() const {return n-1;}

protected:
	RabinFunction() {}	// to be used only by InvertableRabinFunction
	Integer n, r, s;	// these are only modified in constructors
};

class InvertableRabinFunction : public RabinFunction, public InvertableTrapdoorFunction
{
public:
	InvertableRabinFunction(const Integer &n, const Integer &r, const Integer &s,
							const Integer &p, const Integer &q, const Integer &u);
	// generate a random private key
	InvertableRabinFunction(RandomNumberGenerator &rng, unsigned int keybits);
	InvertableRabinFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer CalculateInverse(const Integer &x) const;

protected:
	Integer p, q, u;
};

class RabinPrivateKey : public DecryptorTemplate<OAEP<SHA>, InvertableRabinFunction>,
						public SignerTemplate<PSSR<SHA>, InvertableRabinFunction>
{
public:
	RabinPrivateKey(const Integer &n, const Integer &r, const Integer &s,
					const Integer &p, const Integer &q, const Integer &u)
		: PublicKeyBaseTemplate<InvertableRabinFunction>(
			InvertableRabinFunction(n, r, s, p, q, u)) {}

	RabinPrivateKey(RandomNumberGenerator &rng, unsigned int keybits)
		: PublicKeyBaseTemplate<InvertableRabinFunction>(
			InvertableRabinFunction(rng, keybits)) {}

	RabinPrivateKey(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<InvertableRabinFunction>(bt) {}
};

class RabinPublicKey : public EncryptorTemplate<OAEP<SHA>, RabinFunction>,
					   public VerifierTemplate<PSSR<SHA>, RabinFunction>
{
public:
	RabinPublicKey(const Integer &n, const Integer &r, const Integer &s)
		: PublicKeyBaseTemplate<RabinFunction>(RabinFunction(n, r, s)) {}

	RabinPublicKey(const RabinPrivateKey &priv)
		: PublicKeyBaseTemplate<RabinFunction>(priv.GetTrapdoorFunction()) {}

	RabinPublicKey(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<RabinFunction>(bt) {}
};

#endif
