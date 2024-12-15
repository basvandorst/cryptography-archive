/*
 * $Id: pgpTrstPkt.h,v 1.10 1997/06/16 22:48:54 lloyd Exp $
 */

#ifndef Included_pgpTrstPkt_h
#define Included_pgpTrstPkt_h

#include "pgpBase.h"

/*
 * Key trust byte:
 * 8 +--+
 *   |  | BUCKSTOP bit - this is an axiomatic key
 * 7 +--+
 *   |  | 
 * 6 +--+
 *   |  | Disabled - don't use at user request
 * 5 +--+
 *   |  | Revoked - don't use at issuer request
 * 4 +--+
 *   |  | 
 * 3 +--+
 *   |  | Introducer trust.  6 levels are used:
 * 2 +  + 000 - undefined, need to ask user
 *   |  | 001 - Unknown, consider to be zero
 * 1 +  + 010 - Not trusted
 *   |  | 011 - unused
 * 0 +--+ 100 - unused
 *        101 - Marginal - partially trusted 
 *        110 - Complete - completely trusted
 *        111 - Ultimate - ultimately trusted
 */

#define PGP_KEYTRUSTF_BUCKSTOP	0x80u
#define PGP_KEYTRUSTF_EXPIRED   0x40u
#define PGP_KEYTRUSTF_DISABLED	0x20u
#define PGP_KEYTRUSTF_REVOKED	0x10u

#include "pgpKeys.h"


/*
 * Name trust byte:
 * 8 +--+
 *   |  | WARNONLY bit - yes, it's okay to use even though not fully trusted
 * 7 +--+
 *   |  | unused
 * 6 +--+
 *   |  | unused
 * 5 +--+
 *   |  | unused
 * 4 +--+
 *   |  | unused
 * 3 +--+
 *   |  | unused
 * 2 +--+
 *   |  | Confidence in name's validity.
 * 1 +  + 00 - Not known
 *   |  | 01 - Not trusted
 * 0 +--+ 10 - Partially trusted
 *        11 - Completely trusted
 *
 * New trust packets are followed by up to two more bytes, which are the
 * certainty PGP has of the name's validity and the confidence the user
 * has assigned to this name as an introducer, respectively.  The byte
 * b represents probability of 10^(-b/40) that the name is incorrect,
 * up to 252.  255 means "total confidence", 253 means confidence is
 * undefined, and 254 is reserved for now.
 *
 * @@@ Reserve 0 for "undefined/I dunno", 1 for "untrusted" and offset
 *     everything that way.  It might be more natural...
 *
 * The following pattern repeats each 40:
 *
 *  0 = 1.00000   9 = 1.67880  18 = 2.81838  27 = 4.73151  36 = 7.94328  
 *  1 = 1.05925  10 = 1.77828  19 = 2.98538  28 = 5.01187  37 = 8.41395  
 *  2 = 1.12202  11 = 1.88365  20 = 3.16228  29 = 5.30884  38 = 8.91251  
 *  3 = 1.18850  12 = 1.99526  21 = 3.34965  30 = 5.62341  39 = 9.44061  
 *  4 = 1.25893  13 = 2.11349  22 = 3.54813  31 = 5.95662  40 = 10.0000  
 *  5 = 1.33352  14 = 2.23872  23 = 3.75837  32 = 6.30957  41 = 10.5925
 *  6 = 1.41254  15 = 2.37137  24 = 3.98107  33 = 6.68344  42 = 11.2202
 *  7 = 1.49624  16 = 2.51189  25 = 4.21697  34 = 7.07946  43 = 11.8850
 *  8 = 1.58489  17 = 2.66073  26 = 4.46684  35 = 7.49894  44 = 12.5893
 *
 * As you can see, f(12) ~= 2, f(19) ~= 3, * and f(24) ~= 4.
 * The converse of this is that f(40-12) = f(28) = 10/f(12) ~= 10/2 = 5
 * and similarly f(21) ~= 10/3 = 3.33 and f(16) ~= 10/4 = 2.5.
 * The system was chosen because all of those nice integer ratios
 * work out.
 *
 * This is very close to 2^(-b/12), the difference being the 1024/1000
 * mismatch that programmers are used to ignoring.  Thus, each 12 steps
 * is an octave and each single step is a semitone.
 *
 * From the API, these values are translated to 16-bit values, which are
 * all the programmer sees.  The 16-bit values are shifted by 6 bits,
 * giving a minimum representable error of abut 2.5*10^-26.
 */
#define PGP_NAMETRUSTF_WARNONLY	0x80u


/* The following bytes apply to validity and confidence bytes on names */
#define PGP_NEWTRUST_MAX           252
#define PGP_NEWTRUST_UNDEFINED     253
#define PGP_NEWTRUST_INFINITE      255

/*
 * Signature trust byte:
 * 8 +--+
 *   |  | unused
 * 7 +--+
 *   |  | Checked - Signature is good
 * 6 +--+
 *   |  | Tried - Signature verification attempted
 * 5 +--+
 *   |  | unused
 * 4 +--+
 *   |  | unused
 * 3 +--+
 *   |  | Introducer trust, copied from key that makes it
 * 2 +  +
 *   |  |
 * 1 +  +
 *   |  |
 * 0 +--+
 */

/*
 * The low 3 bits of a signature's trust are generally the same as
 * the corresponding key's trust (kPGPKeyTrust_Undefined if none), but
 * the following signature pseudo-trusts used in some printing contexts:
 */
#define PGP_SIGTRUST_NOKEY	0x8u	/* No corresponding key */
#define PGP_SIGTRUST_RETIRED    0x9u    /* Retired/sig revoked */
#define PGP_SIGTRUST_REVOKED    0xAu    /* Signing key revoked */
#define PGP_SIGTRUST_UNTRIED	0xBu	/* Signature has not been tried */
#define PGP_SIGTRUST_INVALID	0xCu	/* Invalid signature or something */
#define PGP_SIGTRUST_BAD	0xDu	/* BAD SIGNATURE */

/*
 * There are four possible signature states worth noting, actually:
 * - No verification attempted (includes key not found)
 * - Verification impossible (format problem or key length)
 * - Verified bad (format correct, but signature is no good)
 * - Verified good
 *
 * In practice, the middle two can be merged, and the last marked by
 * the PGP_SIGTRUSTF_CHECKED bit for compatibility with 2.x.  The first two
 * can be distinguidhed through the use of the PGP_SIGTRUSTF_TRIED bit.
 * If clear, the signature's verification has never been attempted.
 *
 * At some future point, we should make CHECKED=1 and TRIED=0 mean that
 * a signature was bad, but postpone on that for a while; the state is
 * CHECKED=0, TRIED=1 for now.  If we see CHECKED set, that implies
 * TRIED.
 */
#define PGP_SIGTRUSTF_TRIED	0x20u	/* Visited by sig check (-kc) */
#define PGP_SIGTRUSTF_CHECKED	0x40u	/* Checked out good */
#define PGP_SIGTRUSTF_CHECKED_TRIED (PGP_SIGTRUSTF_CHECKED | \
							PGP_SIGTRUSTF_TRIED)

PGP_BEGIN_C_DECLARATIONS

extern char const  *keyTrustTable[];
extern char const  *uidValidityTable[];

PGP_END_C_DECLARATIONS

#endif /* Included_pgpTrstPkt_h */