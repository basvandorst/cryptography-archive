Crypto++: a C++ Class Library of Cryptographic Primitives
Version 2.1   5/10/1996

This library includes:

MD5, SHA, HAVAL, Tiger, RIPE-MD160, MD5-MAC, HMAC, XOR-MAC, DES, IDEA,
WAKE, 3-WAY, TEA, SAFER, Blowfish, SHARK, GOST, Diamond2, Diamond2 Lite,
Sapphire, RC5, Luby-Rackoff, MDC, various encryption modes (CFB, CBC,
OFB, counter), DH, DSA, ElGamal, LUC, LUCDIF, LUCELG, Rabin, RSA, 
BlumGoldwasser, elliptic curve cryptosystems, BBS, gzip compression, 
Shamir's secret sharing scheme, Rabin's information dispersal scheme, 
and zero-knowledge prover and verifier for graph isomorphism.  There 
are also various miscellanous modules such as base 64 coding and 32-bit 
CRC.

You are welcome to use it for any purpose without paying me, but see
license.txt for the fine print.

Crypto++ has been compiled and tested with Borland C++ 4.5 and MSVC 4.0, 
on MS-DOS and Windows NT.  Unfortunately the latest G++ currently available
(2.7.2) cannot support the extensive use of templates in Crypto++ 2.1.
If you wish to use Crypto++ with G++, I recommend that you obtain a copy
of Crypto++ 2.0, which has been successfully compiled with G++ 2.7.2 and
tested on a variety of UNIX machines.  Crypto++ 2.0 has also been ported
to the Mac using CodeWarrior.  A porting note for CodeWarrior is available 
on the Crypto++ home page (see below).

To compile and test Crypto++, add all of the .cpp files into a console
application project and make the test driver.  To use the library in
your application, you can either add individual Crypto++ modules into
your application project, or compile Crypto++ into a library and link
with the library.

Finally, a note on object ownership:  If a constructor for A takes 
a pointer to an object B (except primitive types such as int and char),
then A owns B and will delete B at A's destruction.  If a constructor 
for A takes a reference to an object B, then the caller retains ownership 
of B and should not destroy it until A no longer needs it.

Good luck, and feel free to e-mail me at weidai@eskimo.com if you have
any problems.  Also, check http://www.eskimo.com/~weidai/cryptlib.html
for updates and new versions.

Wei Dai

History

1.0 - First public release.  Withdrawn at the request of RSA DSI.
    - Has a big bug in the RSA key generation code.

1.1 - Removed RSA, RC4, RC5
    - Disabled calls to RSAREF's non-public functions
    - Minor bugs fixed

2.0 - a completely new, faster multiprecision integer class
    - added MD5-MAC, HAVAL, 3-WAY, TEA, SAFER, LUC, Rabin, BlumGoldwasser,
      elliptic curve algorithms
    - added the Lucas strong probable primality test
    - ElGamal encryption and signature schemes modified to avoid weaknesses
    - Diamond changed to Diamond2 because of key schedule weakness
    - fixed bug in WAKE key setup
    - SHS class renamed to SHA
    - lots of miscellaneous optimizations

2.1 - added Tiger, HMAC, GOST, RIPE-MD160, LUCELG, LUCDIF, XOR-MAC,
      OAEP, PSSR, SHARK
    - added precomputation to DH, ElGamal, DSA, and elliptic curve algorithms
    - optimizations in elliptic curves over GF(p)
	- changed Rabin to use OAEP and PSSR
    - changed many classes to allow copy constructors to work correctly
    - improved exception generation and handling
