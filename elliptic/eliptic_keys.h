/******    eliptic_keys.h   *****/
/*  higher level structures to manipulate elliptic curve public keys  */

#define MAX_NAME_SIZE	64
#define MAX_PHRASE_SIZE	256

typedef struct {
	POINT	p;
	POINT	q;
	CURVE	crv;
	char	name[MAX_NAME_SIZE];
	char	address[MAX_NAME_SIZE];
}  PUBKEY;

