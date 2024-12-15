/* pfx.c */
int main(int argc, char **argv);
int dump_cert_text(BIO *out, X509 *x);
int dump_pkeys(PFX *pfx, BIO *op, EVP_CIPHER *enc, STACK *certs);
/* pfx_add.c */
int pfx_add_cert_chain(PFX *pfx, STACK *chain);
int pfx_add_private(PFX *pfx, RSA *rsa, X509 *cert, char *name, int md_type);
/* pfx_asfe.c */
int i2d_PFX_AUTHSAFE(AUTHSAFE *a, unsigned char **pp);
AUTHSAFE *d2i_PFX_AUTHSAFE(AUTHSAFE **a, unsigned char **pp, long length);
AUTHSAFE *PFX_AUTHSAFE_new(void);
void PFX_AUTHSAFE_free(AUTHSAFE *a);
/* pfx_bitm.c */
int i2d_BAGITEM(BAGITEM *a, unsigned char **pp);
BAGITEM *d2i_BAGITEM(BAGITEM **a, unsigned char **pp, long length);
BAGITEM *BAGITEM_new(void);
void BAGITEM_free(BAGITEM *a);
/* pfx_ccrl.c */
int i2d_CERTCRL(CERTCRL *a, unsigned char **pp);
CERTCRL *CERTCRL_new(void);
CERTCRL *d2i_CERTCRL(CERTCRL **a, unsigned char **pp, long length);
void CERTCRL_free(CERTCRL *a);
/* pfx_chn.c */
int pfx_get_chain(X509 *cert, STACK **chain);
/* pfx_crpt.c */
int pfx_crypt_init(int pbe_nid, unsigned char *pass, int passlen, unsigned char *salt, int saltlen, int iter, EVP_CIPHER_CTX *ctx, int en_de);
/* pfx_crts.c */
int i2d_CERTS(CERTS *a, unsigned char **pp);
CERTS *CERTS_new(void);
CERTS *d2i_CERTS(CERTS **a, unsigned char **pp, long length);
void CERTS_free(CERTS *a);
/* pfx_decr.c */
int pfx_verify_mac(char *pass, PFX *p);
int pfx_decrypt_pkeys(PFX *pfx, char *pass);
STACK *pfx_decrypt_safe(PFX *pfx, char *pass);
/* pfx_encr.c */
int pfx_set_mac(char *pass, PFX *p, EVP_MD *md_type);
int pfx_encrypt_pkeys(PFX *pfx, char *pass, int pbe_nid);
int pfx_encrypt_safe(PFX *pfx, char *pass, int pbe_nid);
/* pfx_espk.c */
int i2d_ESPVK(ESPVK *a, unsigned char **pp);
ESPVK *d2i_ESPVK(ESPVK **a, unsigned char **pp, long length);
ESPVK *ESPVK_new(void);
void ESPVK_free(ESPVK *a);
/* pfx_hash.c */
void pfx_P_HASH(unsigned char *secret, int secretlen, unsigned char *seed, int seedlen, unsigned char *key, int count, EVP_MD *md_type);
/* pfx_init.c */
void pfx_init(PFX **p);
X509_ALGOR *pbe_set(int alg, int iter, unsigned char *salt, int saltlen);
/* pfx_key.c */
void pfx_keyiv_gen(unsigned char *pass, int passlen, unsigned char *salt, int saltlen, int iter, unsigned char *iv, int ivlen, unsigned char *key, int keylen, EVP_MD *md_type);
void pfx_pkcs5(unsigned char *pass, int passlen, unsigned char *salt, int saltlen, int iter, unsigned char *md, EVP_MD *md_type);
/* pfx_lib.c */
int i2d_PFX(PFX *a, unsigned char **pp);
PFX *d2i_PFX(PFX **a, unsigned char **pp, long length);
PFX *PFX_new(void);
void PFX_free(PFX *a);
/* pfx_mac.c */
int i2d_MAC_DATA(MAC_DATA *a, unsigned char **pp);
MAC_DATA *MAC_DATA_new(void);
MAC_DATA *d2i_MAC_DATA(MAC_DATA **a, unsigned char **pp, long length);
void MAC_DATA_free(MAC_DATA *a);
/* pfx_objs.c */
void add_objs(void);
/* pfx_pass.c */
unsigned char *vpasswd(unsigned char *pass, unsigned char *salt, int saltlen, int *oplen);
void PKCS5_KEY_GEN(unsigned char *pass, int passlen, unsigned char *salt, int saltlen, int iter, unsigned char *md);
/* pfx_pbe.c */
int i2d_PBEPARAM(PBEPARAM *a, unsigned char **pp);
PBEPARAM *PBEPARAM_new(void);
PBEPARAM *d2i_PBEPARAM(PBEPARAM **a, unsigned char **pp, long length);
void PBEPARAM_free(PBEPARAM *a);
/* pfx_pk.c */
AUTHSAFE *pfx_safe_unpack(PFX *pfx);
int pfx_safe_pack(PFX *pfx);
/* pfx_priv.c */
int i2d_PRIV_KEY_INFO(PRIV_KEY_INFO *a, unsigned char **pp);
PRIV_KEY_INFO *PRIV_KEY_INFO_new(void);
PRIV_KEY_INFO *d2i_PRIV_KEY_INFO(PRIV_KEY_INFO **a, unsigned char **pp, long length);
void PRIV_KEY_INFO_free(PRIV_KEY_INFO *a);
/* pfx_sbag.c */
int i2d_SAFEBAG(SAFEBAG *a, unsigned char **pp);
SAFEBAG *SAFEBAG_new(void);
SAFEBAG *d2i_SAFEBAG(SAFEBAG **a, unsigned char **pp, long length);
void SAFEBAG_free(SAFEBAG *a);
/* pfx_supd.c */
int i2d_SUP_DATA(SUP_DATA *a, unsigned char **pp);
SUP_DATA *SUP_DATA_new(void);
SUP_DATA *d2i_SUP_DATA(SUP_DATA **a, unsigned char **pp, long length);
void SUP_DATA_free(SUP_DATA *a);
/* pfx_utl.c */
STACK *pfx_get_certs(PFX *p);
X509 *pfx_get_cert_by_thumbprint(STACK *thprints, STACK *certs);
