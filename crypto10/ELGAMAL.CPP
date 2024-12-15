#include "nbtheory.h"
#include "asn.h"
#include "elgamal.h"

ElGamalPublicKey::ElGamalPublicKey(const bignum &p, const bignum &g, const bignum &y)
    : p(p), g(g), y(y), modulusLen(p.ByteCount())
{
}

ElGamalPublicKey::ElGamalPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    modulusLen=p.ByteCount();
    g.BERDecode(seq);
    y.BERDecode(seq);
}

void ElGamalPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
}

void ElGamalPublicKey::Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
{
    assert(plainTextLength <= MaxPlainTextLength());

    // find a random k relatively prime to p-1
    bignum k;
    do
        k.Randomize(rng, 3, p-2, ODD);
    while (Gcd(k, p-1) != 1);

    bignum m(plainText, plainTextLength);
    m = (m << 8) + plainTextLength;
    bignum a,b;

    RawEncrypt(k, m, a, b);
    a.Encode(cipherText, modulusLen);
    b.Encode(cipherText+modulusLen, modulusLen);
}

boolean ElGamalPublicKey::Verify(const byte *message, unsigned int messageLen, const byte *signature)
{
    assert(messageLen <= MaxMessageLength());

    bignum m(message, messageLen);
    bignum a(signature, modulusLen);
    bignum b(signature+modulusLen, modulusLen);
    return RawVerify(m, a, b);
}

boolean ElGamalPublicKey::RawVerify(const bignum &m, const bignum &a, const bignum &b) const
{
    bignum ya = a_exp_b_mod_c(y, a, p);
    bignum ab = a_exp_b_mod_c(a, b, p);
    return a_times_b_mod_c(ya, ab, p) == a_exp_b_mod_c(g, m, p);
}

void ElGamalPublicKey::RawEncrypt(const bignum &k, const bignum &m, bignum &a, bignum &b) const
{
    a = a_exp_b_mod_c(g, k, p);
    b = a_times_b_mod_c(a_exp_b_mod_c(y, k, p), m, p);
}

// *************************************************************

ElGamalPrivateKey::ElGamalPrivateKey(const bignum &p, const bignum &g, const bignum &y, const bignum &x)
    : ElGamalPublicKey(p, g, y), x(x)
{
}

ElGamalPrivateKey::ElGamalPrivateKey(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(rng, pbits);
    p = pg.Prime();
    modulusLen=p.ByteCount();
    g = pg.Generator();
    x.Randomize(rng, 2, p-2);
    y = a_exp_b_mod_c(g, x, p);
}

ElGamalPrivateKey::ElGamalPrivateKey(RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits)
{
    PrimeAndGenerator pg(rng, pbits, qbits);
    p = pg.Prime();
    modulusLen=p.ByteCount();
    g = pg.Generator();
    x.Randomize(rng, 2, p-2);
    y = a_exp_b_mod_c(g, x, p);
}

ElGamalPrivateKey::ElGamalPrivateKey(RandomNumberGenerator &rng, const bignum &pIn, const bignum &gIn)
{
    p = pIn;
    modulusLen=p.ByteCount();
    g = gIn;
    x.Randomize(rng, 2, p-2);
    y = a_exp_b_mod_c(g, x, p);
}

ElGamalPrivateKey::ElGamalPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    modulusLen=p.ByteCount();
    g.BERDecode(seq);
    y.BERDecode(seq);
    x.BERDecode(seq);
}

void ElGamalPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
    x.DEREncode(seq);
}

unsigned int ElGamalPrivateKey::Decrypt(const byte *cipherText, byte *plainText)
{
    bignum a(cipherText, modulusLen);
    bignum b(cipherText+modulusLen, modulusLen);
    bignum m;

    RawDecrypt(a, b, m);
    m.Encode(plainText, 1);
    unsigned int plainTextLength = plainText[0];
    if (plainTextLength > MaxPlainTextLength())
        return 0;
    m >>= 8;
    m.Encode(plainText, plainTextLength);
    return plainTextLength;
}

void ElGamalPrivateKey::Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature)
{
    // find a random k relatively prime to p-1
    bignum k;
    do
        k.Randomize(rng, 3, p-2, ODD);
    while (Gcd(k, p-1) != 1);

    bignum m(message, messageLen);
    bignum a;
    bignum b;

    RawSign(k, m, a, b);
    a.Encode(signature, modulusLen);
    b.Encode(signature+modulusLen, modulusLen);
}

void ElGamalPrivateKey::RawSign(const bignum &k, const bignum &m, bignum &a, bignum &b) const
{
    a = a_exp_b_mod_c(g, k, p);
    bignum kInv = Inverse(k, p-1);
    b = a_times_b_mod_c(kInv, (m - x*a) % (p-1), p-1);
}

void ElGamalPrivateKey::RawDecrypt(const bignum &a, const bignum &b, bignum &m) const
{
    m = a_times_b_mod_c(b, Inverse(a_exp_b_mod_c(a, x, p), p), p);
}

