/* Copyright Dr Stephen Henson 1997. All rights reserved.
 * The conditions of use are identical to those of SSLeay except that my
 * name (in addition to Eric Young or Tim Hudson) must be mentioned
 * if this software is used in a product and the notice "This product
 * includes software written by Dr Stephen Henson (shenson@bigfoot.com)"
 * used.
 */
#include <stdio.h>
#include <stdlib.h>
#include <bio.h>
#include <objects.h>
#include <asn1.h>
#include <asn1_mac.h>
#include <x509.h>
#include <pkcs7.h>
#include <err.h>
#include <crypto.h>
#include <sha.h>
#include <rc2.h>
#include <stack.h>
#include <evp.h>
#include <rsa.h>
#include <pem.h>
#include "pfx.h"


int main(argc, argv)
int argc;
char **argv;
{
    char *infile=NULL, *outfile=NULL, *name=NULL, *keyname = NULL;	
    char *certfile=NULL;
    BIO *in=NULL, *out = NULL, *inkey = NULL, *certsin = NULL;
    BIO *bio_err;
    char **args;
    PFX *p = NULL;
    char pstr[50] ;
    int export_cert = 0;
    int noout = 0;
    int nokeys = 0;
    int nocerts = 0;
    int allcerts = 0;
    int chain = 0;
    int badarg = 0;
    STACK *certs;
    X509 *cert;
    EVP_CIPHER *enc = EVP_des_ede3_cbc();

    bio_err = BIO_new_fp (stderr, BIO_NOCLOSE);

    args = argv + 1;

    while (*args) {
	if (*args[0] == '-') {
		if (!strcmp (*args, "-print_certs")) allcerts = 1;
		else if (!strcmp (*args, "-nokeys")) nokeys = 1;
		else if (!strcmp (*args, "-nocerts")) nocerts = 1;
		else if (!strcmp (*args, "-noout")) noout = 1;
		else if (!strcmp (*args, "-chain")) chain = 1;
		else if (!strcmp (*args, "-export")) export_cert = 1;
		else if (!strcmp (*args, "-inkey")) {
		    if (args[1]) {
			args++;	
			keyname = *args;
		    } else badarg = 1;
		} else if (!strcmp (*args, "-certfile")) {
		    if (args[1]) {
			args++;	
			certfile = *args;
		    } else badarg = 1;
		} else if (!strcmp (*args, "-in")) {
		    if (args[1]) {
			args++;	
			infile = *args;
		    } else badarg = 1;
		} else if (!strcmp (*args, "-name")) {
		    if (args[1]) {
			args++;	
			name = *args;
		    } else badarg = 1;
		} else if (!strcmp (*args, "-out")) {
		    if (args[1]) {
			args++;	
			outfile = *args;
		    } else badarg = 1;
		} else if (!strcmp (*args, "-des")) enc=EVP_des_cbc();
		else if (!strcmp (*args, "-idea")) enc=EVP_idea_cbc();
		else if (!strcmp (*args, "-des3")) enc = EVP_des_ede3_cbc();
		else if (!strcmp (*args, "-nodes")) enc=NULL;
		else badarg = 1;
	} else badarg = 1;
	args++;
    }

    if (export_cert && !name) {
	BIO_printf (bio_err, "Export requires a name\n");
	exit (1);
    }

    if (badarg) {
	BIO_printf (bio_err, "PFX version 0.11. Copyright Dr. Stephen Henson.\n");
	BIO_printf (bio_err, "Usage:\npfx [options]\n");
	BIO_printf (bio_err, "where options are\n");
	BIO_printf (bio_err, "-export       output PFX file (needs -name)\n");
	BIO_printf (bio_err, "-name         certificate nickname for export\n");
	BIO_printf (bio_err, "-chain        add certificate chain\n");
	BIO_printf (bio_err, "-inkey file   private key if not infile\n");
	BIO_printf (bio_err, "-certfile f   add all certs in f\n");
	BIO_printf (bio_err, "-in  infile   input filename\n");
	BIO_printf (bio_err, "-out outfile  output filename\n");
	BIO_printf (bio_err, "-noout        don't output anything, just verify.\n");
	BIO_printf (bio_err, "-nocerts      don't output certificates.\n");
	BIO_printf (bio_err, "-nokeys       don't output private keys.\n");
	BIO_printf (bio_err, "-print_certs  output all certificates.\n");
	BIO_printf (bio_err, "-des          encrypt private keys with DES\n");
	BIO_printf (bio_err, "-des3         encrypt private keys with triple DES (default)\n");
	BIO_printf (bio_err, "-idea         encrypt private keys with idea\n");
	BIO_printf (bio_err, "-nodes        don't encrypt private keys\n");
    	exit (1);
    }

    if (noout) {
	nokeys = 1;
	nocerts = 1;
	allcerts = 0;
    }

    add_objs ();
    ERR_load_crypto_strings();
    SSLeay_add_all_algorithms();
    in = BIO_new (BIO_s_file());
    out = BIO_new (BIO_s_file());

    if (!infile) BIO_set_fp (in, stdin, BIO_NOCLOSE);
    else {
	if (!keyname) keyname = infile;
        if (BIO_read_filename (in, infile) <= 0) {
	    perror (infile);
	    exit (1);
	}
    }

   if (certfile) {
    	certsin = BIO_new (BIO_s_file());
        if (BIO_read_filename (certsin, certfile) <= 0) {
	    perror (certfile);
	    exit (1);
	}
    }

    if (keyname) {
    	inkey = BIO_new (BIO_s_file());
        if (BIO_read_filename (inkey, keyname) <= 0) {
	    perror (keyname);
	    exit (1);
	}
     }

    if (!outfile) BIO_set_fp (out, stdout, BIO_NOCLOSE);
    else {
        if (BIO_write_filename (out, outfile) <= 0) {
	    perror (outfile);
	    exit (1);
	}
    }

if (export_cert) {
	RSA *key;
	STACK *cstack;
	cert = PEM_read_bio_X509(in, NULL, NULL);
	if (!cert) {
		ERR_print_errors(bio_err);
		exit (1);
	}

	if (chain) {
		int err;
		err  = pfx_get_chain(cert, &cstack);
		if (err) {
			BIO_printf (bio_err, "Error getting chain: %s\n", 
					X509_verify_cert_error_string(err));	
			exit (1);
		}
	} else {
	/* If not chaining just add one certificate */	
		cstack = sk_new (NULL);
		sk_push (cstack, (char *)cert);
	}

	/* Add any more certificates asked for */
	if (certsin) {
	    STACK *sk;
	    X509_INFO *xi;
	    sk = PEM_X509_INFO_read_bio (certsin, NULL, NULL);
	    if (sk != NULL) {
		while (sk_num(sk)) {
		    xi = (X509_INFO *)sk_shift(sk);
		    if (xi->x509) {
		        sk_push (cstack, (char *)xi->x509);
		        xi->x509 = NULL;
		    }
		    X509_INFO_free (xi);
		}
	    }
 	}

	key = PEM_read_bio_RSAPrivateKey(inkey ? inkey : in, NULL, NULL);
	if (!key) {
		BIO_printf (bio_err, "Error loading private key\n");
		ERR_print_errors(bio_err);
		exit (1);
	}

	pfx_init (&p);

	pfx_add_cert_chain (p, cstack);
	pfx_add_private (p, key, cert, name, NID_sha1);

	if(EVP_read_pw_string (pstr, 50, "Enter Export Password:", 1)) {
	    BIO_printf (bio_err, "Can't read Password\n");
	    exit (1);
        }

	pfx_encrypt_pkeys (p, pstr, NID_pbe_sha1_tripdes);
	pfx_encrypt_safe (p, pstr, NID_pbe_sha1_rc2_40);


	pfx_safe_pack (p);
	pfx_set_mac (pstr, p, EVP_sha1());

	ASN1_i2d_bio (i2d_PFX, out, (char *)p);

	exit (0);
}

    p = (PFX *) ASN1_d2i_bio ((char *(*)())PFX_new, (char *(*)())d2i_PFX, in,
									 NULL);

    if (!p) {
	ERR_print_errors(bio_err);
	exit (1);
    }

    if(EVP_read_pw_string (pstr, 50, "Enter Import Password:", 0)) {
	BIO_printf (bio_err, "Can't read Password\n");
	exit (1);
    }

    if (pfx_verify_mac (pstr, p)) {
      BIO_printf (bio_err, "Mac verify errror: invalid password?\n");
      exit (1);
     } else BIO_printf (bio_err, "MAC verified OK\n");

    if(!pfx_safe_unpack(p)) {
	ERR_print_errors(bio_err);
	exit (1);
    }

    if (!pfx_decrypt_safe (p, pstr)) {
	ERR_print_errors(bio_err);
	exit (1);
    } else BIO_printf (bio_err, "Decrypted Authsafe OK\n");

    if (pfx_decrypt_pkeys (p, pstr)) {
	BIO_printf (bio_err, "Error decrypting private keys\n");
	exit (1);
    } else BIO_printf (bio_err, "Decrypted Private Keys OK\n");

    certs = pfx_get_certs (p);

    if (allcerts) {
	int i;
	for (i = 0; i < sk_num (certs); i++) {
	    cert = (X509 *) sk_value (certs, i);
	    dump_cert_text (out, cert);
	    PEM_write_bio_X509 (out, cert);
	}
    }

    /* If certs already printed or none wanted zero certs */
    if (allcerts || nocerts) {
	sk_free (certs);
	certs = NULL;
    }
    if (!nokeys) dump_pkeys (p, out, enc, certs);
    return (0);
}

int dump_cert_text (out, x)
BIO *out;
X509 *x;
{
	char buf[256];
	X509_NAME_oneline(X509_get_subject_name(x),buf,256);
	BIO_puts(out,"subject=");
	BIO_puts(out,buf);

	X509_NAME_oneline(X509_get_issuer_name(x),buf,256);
	BIO_puts(out,"\nissuer= ");
	BIO_puts(out,buf);
	BIO_puts(out,"\n");
        return 0;
}

int dump_pkeys (pfx, op, enc, certs)
PFX *pfx;
BIO *op;
EVP_CIPHER *enc;
STACK *certs;
{
int i, j;
BAGITEM *tmpbag;
ESPVK *tmpsbag;
unsigned char *p;
RSA *rsa;
X509 *cert;

/* Get all private keys */
for (i = 0; i < sk_num (pfx->safe_cont->bagitms); i++) {
    tmpbag = (BAGITEM *) sk_value (pfx->safe_cont->bagitms, i) ;
    for (j = 0 ; j < sk_num (tmpbag->espvks); j++) {
        tmpsbag = (ESPVK *) sk_value (tmpbag->espvks, j);
	/* Find corresponding certificate and output */
	if (certs) {
	    cert = pfx_get_cert_by_thumbprint (tmpsbag->supdata->prints, certs);
	    if (cert) {
	   	dump_cert_text (op, cert);
	    	PEM_write_bio_X509 (op, cert);
	    }
	}
        if (tmpsbag->pkey && OBJ_obj2nid (tmpsbag->pkey->pkeyalg->algorithm) 
							 == NID_rsaEncryption) {
	    p = (unsigned char *) tmpsbag->pkey->pkey->data;
            rsa = d2i_RSAPrivateKey (NULL, &p, tmpsbag->pkey->pkey->length);
	    PEM_write_bio_RSAPrivateKey (op, rsa, enc, NULL, 0, NULL);
	    RSA_free (rsa);
	}
  }
} 
return 0;
}
