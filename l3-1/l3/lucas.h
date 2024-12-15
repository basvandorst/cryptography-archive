/* lucas.h - Mark Henderson (markh@wimsey.bc.ca) */


#define init(g) { g = (MP_INT *)malloc(sizeof(MP_INT));  mpz_init(g); } 
#define clear(g) { mpz_clear(g); free(g); }

void cstmp(MP_INT *, unsigned char *, unsigned int);
void mptcs(unsigned char *,unsigned int,MP_INT *);
void modinv(MP_INT *, MP_INT *, MP_INT *);

#ifndef STAND
int genkey(MP_INT *, MP_INT *,  MP_INT *, unsigned int);
void encrypt_session_key(MP_INT *, unsigned char *, MP_INT *, MP_INT *,int);
void decrypt_session_key(unsigned char *, MP_INT *, MP_INT *,
    MP_INT *, MP_INT *,int);
void sign(MP_INT *, unsigned char *, MP_INT *, MP_INT *, MP_INT *,int);
int vsign(MP_INT *, unsigned char *, MP_INT *, MP_INT *,int);
#endif

void private_from_primes(MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void private_from_message(MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void v_pq_n(MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void v_pq(MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void v_p1_n(MP_INT *,MP_INT *, MP_INT *, MP_INT *);
#define luc(res,message,key,modulus)  v_p1_n(res,key,message,modulus)
#define rsa(res,message,key,modulus)  mpz_powm(res,message,key,modulus)
void halfluc(MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void halfrsa(MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *);

/* the following provide room for 4096 bit numbers */
#define MAXMPSIZE 128
#define MAXC (MAXMPSIZE * sizeof(unsigned long))

#define RSA 1
#define LUC 2
#define RL  3

#define PALG LUC
