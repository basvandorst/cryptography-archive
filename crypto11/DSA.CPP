#include "misc.h"
#include "queue.h"
#include "asn.h"
#include "dsa.h"
#include "nbtheory.h"
#include "shs.h"
#include <assert.h>

boolean GenerateDSAPrimes(byte *seed, unsigned int g, int &counter,
                          bignum &p, unsigned int L, bignum &q)
{
    assert(L >= MIN_DSA_PRIME_LENGTH && L <= MAX_DSA_PRIME_LENGTH);

    SHS shs;
    byte U[SHS::DIGESTSIZE];
    byte temp[SHS::DIGESTSIZE];
    byte W[(((MAX_DSA_PRIME_LENGTH-1)/160)+1) * SHS::DIGESTSIZE];
    const int n = (L-1) / 160;
    const int b = (L-1) % 160;
    bignum X;

    shs.CalculateDigest(U, seed, g/8);

    for (int i=g/8-1, carry=TRUE; i>=0 && carry; i--)
        carry=!++seed[i];

    shs.CalculateDigest(temp, seed, g/8);
    xorbuf(U, temp, SHS::DIGESTSIZE);

    U[0] |= 0x80;
    U[SHS::DIGESTSIZE-1] |= 1;
    q.Decode(U, SHS::DIGESTSIZE);

    if (!IsPrime(q))
        return FALSE;

    for (counter = 0; counter < 4096; counter++)
    {
        for (int k=0; k<=n; k++)
        {
            for (int i=g/8-1, carry=TRUE; i>=0 && carry; i--)
            	carry=!++seed[i];
            shs.CalculateDigest(W+(n-k)*SHS::DIGESTSIZE, seed, g/8);
        }
        W[SHS::DIGESTSIZE - 1 - b/8] |= 0x80;
        X.Decode(W + SHS::DIGESTSIZE - 1 - b/8, L/8);
        p = X-((X % (2*q))-1);

        if (p[L-1] && IsPrime(p))
            return TRUE;
    }
    return FALSE;
}

DSAPublicKey::DSAPublicKey(const bignum &p, const bignum &q,
                           const bignum &g, const bignum &y)
    : p(p), q(q), g(g), y(y)
{
}

boolean DSAPublicKey::Verify(const byte *message, unsigned int messageLen, const byte *signature)
{
    assert(messageLen <= MaxMessageLength());

    bignum h(message, messageLen);
    bignum r(signature, 20);
    bignum s(signature+20, 20);

    return RawVerify(h, r, s);
}


boolean DSAPublicKey::RawVerify(const bignum &h, const bignum &r, const bignum &s) const
{
    bignum w = Inverse(s, q);
    bignum u1 = (h * w) % q;
    bignum u2 = (r * w) % q;
    bignum gu1 = a_exp_b_mod_c(g, u1, p);
    bignum yu2 = a_exp_b_mod_c(y, u2, p);
    bignum v = a_times_b_mod_c(gu1, yu2, p) % q;
    return (v==r);
}

DSAPublicKey::DSAPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    q.BERDecode(seq);
    g.BERDecode(seq);
    y.BERDecode(seq);
}

void DSAPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    q.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
}

// ***********************************************************

DSAPrivateKey::DSAPrivateKey(const bignum &p, const bignum &q,
                             const bignum &g, const bignum &y, const bignum &x)
    : DSAPublicKey(p, q, g, y), x(x)
{
}

DSAPrivateKey::DSAPrivateKey(RandomNumberGenerator &rng, int keybits)
{
    byte seed[SHS::DIGESTSIZE];
    bignum h;
    int c;

    do
    {
        rng.GetBlock(seed, SHS::DIGESTSIZE);
    } while (!GenerateDSAPrimes(seed, SHS::DIGESTSIZE*8, c, p, keybits, q));

    do
    {
        h.Randomize(rng, 2, p-2);
        g = a_exp_b_mod_c(h, (p-1)/q, p);
    } while (g <= 1);

    x.Randomize(rng, 2, q-2);
    y = a_exp_b_mod_c(g, x, p);
}

DSAPrivateKey::DSAPrivateKey(RandomNumberGenerator &rng, const bignum &pIn, const bignum &qIn, const bignum &gIn)
{
    p = pIn;
    q = qIn;
    g = gIn;

    x.Randomize(rng, 2, q-2);
    y = a_exp_b_mod_c(g, x, p);
}

DSAPrivateKey::DSAPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    q.BERDecode(seq);
    g.BERDecode(seq);
    y.BERDecode(seq);
    x.BERDecode(seq);
}

void DSAPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    q.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
}

void DSAPrivateKey::Sign(RandomNumberGenerator &rng, const byte *message, unsigned int messageLen, byte *signature)
{
    assert(messageLen <= MaxMessageLength());

    bignum k(rng, 2, q-2);
    bignum h(message, messageLen);
    bignum r, s;

    RawSign(k, h, r, s);
    r.Encode(signature, 20);
    s.Encode(signature+20, 20);
}

void DSAPrivateKey::RawSign(const bignum &k, const bignum &h, bignum &r, bignum &s) const
{
    r = a_exp_b_mod_c(g, k, p) % q;
    bignum kInv = Inverse(k, q);
    s = (kInv * (x*r + h)) % q;
}

