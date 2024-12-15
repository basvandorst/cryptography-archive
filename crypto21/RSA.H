#ifndef RSA_H
#define RSA_H

#include "pkcspad.h"

class RSAFunction : virtual public TrapdoorFunction
{
public:
    RSAFunction(const Integer &n, const Integer &e) : n(n), e(e) {}
    RSAFunction(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    Integer ApplyFunction(const Integer &x) const;
	Integer MaxPreimage() const {return n-1;}
	Integer MaxImage() const {return n-1;}

protected:
    RSAFunction() {}	// to be used only by InvertableRSAFunction
    Integer n, e;	// these are only modified in constructors
};

class InvertableRSAFunction : public RSAFunction, public InvertableTrapdoorFunction
{
public:
    InvertableRSAFunction(const Integer &n, const Integer &e, const Integer &d,
						  const Integer &p, const Integer &q, const Integer &dp, const Integer &dq, const Integer &u);
    // generate a random private key
    InvertableRSAFunction(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart=17);
    InvertableRSAFunction(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    Integer CalculateInverse(const Integer &x) const;

protected:
    Integer d, p, q, dp, dq, u;
};

class RSAPrivateKey : public DecryptorTemplate<PKCS_EncryptionPaddingScheme, InvertableRSAFunction>,
					  public SignerTemplate<PKCS_SignaturePaddingScheme, InvertableRSAFunction>
{
public:
    RSAPrivateKey(const Integer &n, const Integer &e, const Integer &d,
				  const Integer &p, const Integer &q, const Integer &dp, const Integer &dq, const Integer &u)
		: PublicKeyBaseTemplate<InvertableRSAFunction>(
			InvertableRSAFunction(n, e, d, p, q, dp, dq, u)) {}

    RSAPrivateKey(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart=17)
		: PublicKeyBaseTemplate<InvertableRSAFunction>(
			InvertableRSAFunction(rng, keybits, eStart)) {}

    RSAPrivateKey(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<InvertableRSAFunction>(bt) {}
};

class RSAPublicKey : public EncryptorTemplate<PKCS_EncryptionPaddingScheme, RSAFunction>,
					 public VerifierTemplate<PKCS_SignaturePaddingScheme, RSAFunction>
{
public:
    RSAPublicKey(const Integer &n, const Integer &e)
		: PublicKeyBaseTemplate<RSAFunction>(RSAFunction(n, e)) {}

	RSAPublicKey(const RSAPrivateKey &priv)
		: PublicKeyBaseTemplate<RSAFunction>(priv.GetTrapdoorFunction()) {}

    RSAPublicKey(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<RSAFunction>(bt) {}
};

#endif
