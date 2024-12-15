/* Copyright Dr Stephen Henson 1997. All rights reserved.
 * The conditions of use are identical to those of SSLeay except that my
 * name (in addition to Eric Young or Tim Hudson) must be mentioned
 * if this software is used in a product and the notice "This product
 * includes software written by Dr Stephen Henson (shenson@bigfoot.com)"
 * used.
 */
#include <stdio.h>
#include <stdlib.h>
#include <objects.h>
#include <asn1.h>
#include <asn1_mac.h>
#include <x509.h>
#include <pkcs7.h>
#include <err.h>
#include <stack.h>

#define ADD_OBJS
#include "pfx.h"


#define PKCS12 "1.2.840.113549.1.12."

void add_objs ()
{
NID_off_line = OBJ_create_and_add_object (PKCS12 "1.1",
		"OFFLINE", "Off-lineTransportMode");
NID_on_line = OBJ_create_and_add_object (PKCS12 "1.2",
		"ONLINE", "On-lineTransportMode");

NID_pbe_sha1_rc4_128=
	 OBJ_create_and_add_object (PKCS12 "5.1.1",
		 "PBESHA1RC4-128", "pbeWithSHA1AndRC4-128");
NID_pbe_sha1_rc4_40=
	 OBJ_create_and_add_object (PKCS12 "5.1.2",
		 "PBESHA1RC4-40", "pbeWithSHA1AndRC4-40");
NID_pbe_sha1_tripdes =
	 OBJ_create_and_add_object (PKCS12 "5.1.3",
		 "PBESHA1TDES-CBC", "pbeWithSHA1AndTripleDES-CBC");
NID_pbe_sha1_rc2_128=
	 OBJ_create_and_add_object (PKCS12 "5.1.4",
		 "PBESHA1RC2-128", "pbeWithSHA1AndRC2-128");
NID_pbe_sha1_rc2_40=
	 OBJ_create_and_add_object (PKCS12 "5.1.5",
		 "PBESHA1RC2-40", "pbeWithSHA1AndRC2-40");
NID_pbe_sha1_des =
	 OBJ_create_and_add_object ("1.2.840.113549.5.10",
		 "PBESHA1DES-CBC", "pbeWithSHA1AndDES-CBC");

NID_pkcs8_key_shrouding =
		 OBJ_create_and_add_object (PKCS12 "2.1",
			 "PKCS8KSHRD", "pkcs8KeyShrouding");

NID_CertAndCRLBagID =
		 OBJ_create_and_add_object (PKCS12 "3.2",
			 "CERTCRLBAGID", "certAndCRLBagID");
NID_x509CertCRLBag =
		 OBJ_create_and_add_object (PKCS12 "4.1",
			 "X509CERTCRLBAG", "x509CertCRLBag");
}
