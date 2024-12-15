// luc.cpp - written and placed in the public domain by Wei Dai

#include "misc.h"
#include "asn.h"
#include "nbtheory.h"

#include "luc.h"

LUCFunction::LUCFunction(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    n.BERDecode(seq);
    e.BERDecode(seq);
}

void LUCFunction::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    n.DEREncode(seq);
    e.DEREncode(seq);
}

Integer LUCFunction::ApplyFunction(const Integer &x) const
{
	return Lucas(e, x, n);
}

// *****************************************************************************
// private key operations:

InvertableLUCFunction::InvertableLUCFunction(const Integer &nIn, const Integer &eIn,
                             const Integer &pIn, const Integer &qIn, const Integer &uIn)
    : LUCFunction(nIn, eIn), p(pIn), q(qIn), u(uIn)
{
    assert(p*q==n);
    assert(u*q%p==1);
}

// generate a random private key
InvertableLUCFunction::InvertableLUCFunction(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart)
{
	assert(keybits >= 16);
    // generate 2 random primes of suitable size
	if (keybits%2==0)
	{
		const Integer minP = Integer(182) << (keybits/2-8);
		const Integer maxP = Integer::Power2(keybits/2)-1;
		p.Randomize(rng, minP, maxP, Integer::PRIME);
		q.Randomize(rng, minP, maxP, Integer::PRIME);
	}
	else
	{
		const Integer minP = Integer::Power2((keybits-1)/2);
		const Integer maxP = Integer(181) << ((keybits+1)/2-8);
		p.Randomize(rng, minP, maxP, Integer::PRIME);
		q.Randomize(rng, minP, maxP, Integer::PRIME);
	}

    // pre-calculate some other data for faster speed
    const Integer lcm = LCM(LCM(p-1, q-1), LCM(p+1, q+1));
	// make sure e starts odd
    for (e = eStart+(1-eStart%2); GCD(e, lcm)!=1; ++e, ++e);
    u = EuclideanMultiplicativeInverse(q, p);
    n = p * q;
	assert(n.BitCount() == keybits);
}

InvertableLUCFunction::InvertableLUCFunction(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);

    Integer version(seq);
    if (!!version)  // make sure version is 0
        BERDecodeError();

    n.BERDecode(seq);
    e.BERDecode(seq);
    p.BERDecode(seq);
    q.BERDecode(seq);
    u.BERDecode(seq);
}

void InvertableLUCFunction::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);

    const byte version[] = {INTEGER, 1, 0};
    seq.Put(version, sizeof(version));
    n.DEREncode(seq);
    e.DEREncode(seq);
    p.DEREncode(seq);
    q.DEREncode(seq);
    u.DEREncode(seq);
}

Integer InvertableLUCFunction::CalculateInverse(const Integer &x) const
{
	 return InverseLucas(e, x, q, p, u);
}

// ********************************************************

LUCELG_CryptoPublicKey::LUCELG_CryptoPublicKey(const Integer &p, const Integer &g, const Integer &y)
    : p(p), g(g), y(y), modulusLen(p.ByteCount())
{
}

LUCELG_CryptoPublicKey::LUCELG_CryptoPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    modulusLen=p.ByteCount();
    g.BERDecode(seq);
    y.BERDecode(seq);
}

void LUCELG_CryptoPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
}

void LUCELG_CryptoPublicKey::Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
{
    assert(plainTextLength <= MaxPlainTextLength());

	SecByteBlock block(modulusLen-1);
	rng.GetBlock(block, modulusLen-2-plainTextLength);
	memcpy(block+modulusLen-2-plainTextLength, plainText, plainTextLength);
	block[modulusLen-2] = plainTextLength;

    Integer m(block, modulusLen-1);
    Integer a,b;
    RawEncrypt(Integer(rng, ExponentBitLength()), m, a, b);

    a.Encode(cipherText, modulusLen);
    b.Encode(cipherText+modulusLen, modulusLen);
}

void LUCELG_CryptoPublicKey::RawEncrypt(const Integer &k, const Integer &m, Integer &a, Integer &b) const
{
    a = Lucas(k, g, p);
    b = m * Lucas(k, y, p) % p;
}

unsigned int LUCELG_CryptoPublicKey::ExponentBitLength() const
{
	// use 2*p.BitCount() because we're in GF(p^2)
	return 2*DiscreteLogWorkFactor(2*p.BitCount());
}

// *************************************************************

LUCELG_CryptoPrivateKey::LUCELG_CryptoPrivateKey(const Integer &p, const Integer &g, const Integer &y, const Integer &x)
    : LUCELG_CryptoPublicKey(p, g, y), x(x)
{
}

LUCELG_CryptoPrivateKey::LUCELG_CryptoPrivateKey(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(-1, rng, pbits);
    p = pg.Prime();
    modulusLen=p.ByteCount();
    g = pg.Generator();
    x.Randomize(rng, ExponentBitLength());
	y = Lucas(x, g, p);
}

LUCELG_CryptoPrivateKey::LUCELG_CryptoPrivateKey(RandomNumberGenerator &rng, const Integer &pIn, const Integer &gIn)
{
    p = pIn;
    modulusLen=p.ByteCount();
    g = gIn;
    x.Randomize(rng, ExponentBitLength());
	y = Lucas(x, g, p);
}

LUCELG_CryptoPrivateKey::LUCELG_CryptoPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    modulusLen=p.ByteCount();
    g.BERDecode(seq);
    y.BERDecode(seq);
    x.BERDecode(seq);
}

void LUCELG_CryptoPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
    x.DEREncode(seq);
}

unsigned int LUCELG_CryptoPrivateKey::Decrypt(const byte *cipherText, byte *plainText)
{
    Integer a(cipherText, modulusLen);
    Integer b(cipherText+modulusLen, modulusLen);
    Integer m;

    RawDecrypt(a, b, m);
    m.Encode(plainText, 1);
    unsigned int plainTextLength = plainText[0];
    if (plainTextLength > MaxPlainTextLength())
        return 0;
    m >>= 8;
    m.Encode(plainText, plainTextLength);
    return plainTextLength;
}

void LUCELG_CryptoPrivateKey::RawDecrypt(const Integer &a, const Integer &b, Integer &m) const
{
	m = b * EuclideanMultiplicativeInverse(Lucas(x, a, p), p) % p;
}

// ******************************************************************

LUCELG_SigPublicKey::LUCELG_SigPublicKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y)
    : p(p), q(q), g(g), y(y), pLen(p.ByteCount()), qLen(q.ByteCount())
{
}

LUCELG_SigPublicKey::LUCELG_SigPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    q.BERDecode(seq);
    g.BERDecode(seq);
    y.BERDecode(seq);
    pLen = p.ByteCount();
    qLen = q.ByteCount();
}

void LUCELG_SigPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    q.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
}

boolean LUCELG_SigPublicKey::Verify(const byte *message, unsigned int messageLen, const byte *signature)
{
    assert(messageLen <= MaxMessageLength());

    Integer m(message, messageLen);
    Integer r(signature, pLen);
    Integer s(signature+pLen, qLen);
    return RawVerify(m, r, s);
}

boolean LUCELG_SigPublicKey::RawVerify(const Integer &m, const Integer &r, const Integer &s) const
{
	Integer Vsg=Lucas(s, g, p);
	Integer Vry=Lucas((r+m)%q, y, p);
	return (Vsg*Vsg + Vry*Vry + r*r) % p == (Vsg * Vry * r + 4) % p;
}

// *************************************************************

LUCELG_SigPrivateKey::LUCELG_SigPrivateKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x)
    : LUCELG_SigPublicKey(p, q, g, y), x(x)
{
}

LUCELG_SigPrivateKey::LUCELG_SigPrivateKey(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(-1, rng, pbits);
    p = pg.Prime();
	q = pg.SubPrime();
    g = pg.Generator();
    x.Randomize(rng, 2, q-2, Integer::ANY);
	y = Lucas(x, g, p);
    pLen = p.ByteCount();
	qLen = q.ByteCount();
}

LUCELG_SigPrivateKey::LUCELG_SigPrivateKey(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits)
{
    PrimeAndGenerator pg(-1, rng, pbits, qbits);
    p = pg.Prime();
	q = pg.SubPrime();
    g = pg.Generator();
    x.Randomize(rng, 2, q-2, Integer::ANY);
	y = Lucas(x, g, p);
    pLen = p.ByteCount();
	qLen = q.ByteCount();
}

LUCELG_SigPrivateKey::LUCELG_SigPrivateKey(RandomNumberGenerator &rng, const Integer &pIn, const Integer &qIn, const Integer &gIn)
{
    p = pIn;
    q = qIn;
    g = gIn;
    x.Randomize(rng, 2, q-2, Integer::ANY);
	y = Lucas(x, g, p);
    pLen = p.ByteCount();
	qLen = q.ByteCount();
}

LUCELG_SigPrivateKey::LUCELG_SigPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    q.BERDecode(seq);
    g.BERDecode(seq);
    y.BERDecode(seq);
    x.BERDecode(seq);
    pLen = p.ByteCount();
	qLen = q.ByteCount();
}

void LUCELG_SigPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    q.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
    x.DEREncode(seq);
}

void LUCELG_SigPrivateKey::Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature)
{
    assert(messageLen <= MaxMessageLength());

    Integer m(message, messageLen);
    Integer r;
    Integer s;

    RawSign(rng, m, r, s);
    r.Encode(signature, pLen);
    s.Encode(signature+pLen, qLen);
}

void LUCELG_SigPrivateKey::RawSign(RandomNumberGenerator &rng, const Integer &m, Integer &r, Integer &s) const
{
    Integer k(rng, 2, q-2, Integer::ANY);
    r = Lucas(k, g, p);
    s = (k + x*(r+m)) % q;
}

// ********************************************************

LUCDIF::LUCDIF(const Integer &p, const Integer &g)
    : p(p), g(g)
{
}

LUCDIF::LUCDIF(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(-1, rng, pbits);
    p = pg.Prime();
    g = pg.Generator();
}

LUCDIF::LUCDIF(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits)
{
    PrimeAndGenerator pg(-1, rng, pbits, qbits);
    p = pg.Prime();
    g = pg.Generator();
}

LUCDIF::LUCDIF(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    g.BERDecode(seq);
}

void LUCDIF::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
}

void LUCDIF::Setup(RandomNumberGenerator &rng, byte *publicValue)
{
    x.Randomize(rng, ExponentBitLength());
	Integer y = Lucas(x, g, p);
    y.Encode(publicValue, PublicValueLength());
}

void LUCDIF::Agree(const byte *otherPublicValue, byte *out) const
{
    Integer otherPub(otherPublicValue, PublicValueLength());
    Integer agreedKey = Lucas(x, otherPub, p);
    agreedKey.Encode(out, AgreedKeyLength());
}

unsigned int LUCDIF::ExponentBitLength() const
{
	return 2*DiscreteLogWorkFactor(2*p.BitCount());
}
