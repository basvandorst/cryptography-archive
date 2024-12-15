/* lucas.h - Mark Henderson (markh@wimsey.bc.ca) */


#define init(g) { g = (MP_INT *)malloc(sizeof(MP_INT));  mpz_init(g); } 
#define clear(g) { mpz_clear(g); free(g); }

void cstmp(MP_INT *, unsigned char *, unsigned int);
void mptcs(unsigned char *,unsigned int,MP_INT *);
int genkey(MP_INT *, MP_INT *,  MP_INT *, unsigned int);
void private_from_primes(MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void v_pq_n(MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void v_p1_n(MP_INT *,MP_INT *, MP_INT *, MP_INT *);
#define luc(res,message,key,modulus)  v_p1_n(res,key,message,modulus)
void halfluc(MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *, MP_INT *);
void encrypt_session_key(MP_INT *, unsigned char *, MP_INT *, MP_INT *);
void decrypt_session_key(unsigned char *, MP_INT *, MP_INT *,
    MP_INT *, MP_INT *);
void sign(MP_INT *, unsigned char *, MP_INT *, MP_INT *, MP_INT *);
int vsign(MP_INT *, unsigned char *, MP_INT *, MP_INT *);

/* the following provide room for 2048 bit numbers */
#define MAXMPSIZE 64
#define MAXC (MAXMPSIZE * 4)
