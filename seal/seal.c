/* seal.c - SEAL encryption algorithm */

#undef SEAL_DEBUG

#define ALG_OK 0
#define ALG_NOTOK 1

struct SealKey {
	unsigned long t[520]; /* 512 rounded up to a multiple of 5 + 5*/
	unsigned long s[265]; /* 256 rounded up to a multiple of 5 + 5*/
	unsigned long r[20];  /* 16 rounded up to multiple of 5 */
};

#define ROT2(x) (((x) >> 2) | ((x) << 30))
#define ROT9(x) (((x) >> 9) | ((x) << 23))
#define ROT8(x) (((x) >> 8) | ((x) << 24))
#define ROT16(x) (((x) >> 16) | ((x) << 16))
#define ROT24(x) (((x) >> 24) | ((x) << 8))
#define ROT27(x) (((x) >> 27) | ((x) << 5))

#define WORD(cp)  ((cp[0] << 24)|(cp[1] << 16)|(cp[2] << 8)|(cp[3]))

#define F1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))
#define F2(x, y, z) ((x)^(y)^(z))
#define F3(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define F4(x, y, z) ((x)^(y)^(z))

int g(in, i, h)
unsigned char *in;
int i;
unsigned long *h;
{
unsigned long h0;
unsigned long h1;
unsigned long h2;
unsigned long h3;
unsigned long h4;
unsigned long a;
unsigned long b;
unsigned long c;
unsigned long d;
unsigned long e;
unsigned char *kp;
unsigned long w[80];
unsigned long temp;

	kp = in;
	h0 = WORD(kp); kp += 4;
	h1 = WORD(kp); kp += 4;
	h2 = WORD(kp); kp += 4;
	h3 = WORD(kp); kp += 4;
	h4 = WORD(kp); kp += 4;

	w[0] = i;
	for (i=1;i<16;i++)
		w[i] = 0;
	for (i=16;i<80;i++)
		w[i] = w[i-3]^w[i-8]^w[i-14]^w[i-16];

	a = h0;
	b = h1;
	c = h2;
	d = h3;
	e = h4;

	for (i=0;i<20;i++)
	{
		temp = ROT27(a) + F1(b, c, d) + e + w[i] + 0x5a827999;
		e = d;
		d = c;
		c = ROT2(b);
		b = a;
		a = temp;
	}
	for (i=20;i<40;i++)
	{
		temp = ROT27(a) + F2(b, c, d) + e + w[i] + 0x6ed9eba1;
		e = d;
		d = c;
		c = ROT2(b);
		b = a;
		a = temp;
	}
	for (i=40;i<60;i++)
	{
		temp = ROT27(a) + F3(b, c, d) + e + w[i] + 0x8f1bbcdc;
		e = d;
		d = c;
		c = ROT2(b);
		b = a;
		a = temp;
	}
	for (i=60;i<80;i++)
	{
		temp = ROT27(a) + F4(b, c, d) + e + w[i] + 0xca62c1d6;
		e = d;
		d = c;
		c = ROT2(b);
		b = a;
		a = temp;
	}
	h[0] = h0+a;
	h[1] = h1+b;
	h[2] = h2+c;
	h[3] = h3+d;
	h[4] = h4+e;

	return (ALG_OK);
}	

unsigned long gamma(a, i)
unsigned char *a;
int i;
{
unsigned long h[5];

	(void) g(a, i/5, h);
	return h[i % 5];
}
	
int  seal_init(key, ks)
unsigned char *key;
struct SealKey **ks;
{
struct SealKey *result;
int i;
unsigned long h[5];

	result = (struct SealKey *) calloc(1, sizeof(*result));
	if (result == (struct SealKey *) 0)
		return (ALG_NOTOK);

	for (i=0;i<510;i+=5)
		g(key, i/5, &(result->t[i]));
	/* horrible special case for the end */
	g(key, 510/5, h);
	for (i=510;i<512;i++)
		result->t[i] = h[i-510];
	/* 0x1000 mod 5 is +1, so have horrible special case for the start */
	g(key, (-1+0x1000)/5, h);
	for (i=0;i<4;i++)
		result->s[i] = h[i+1];
	for (i=4;i<254;i+=5)
		g(key, (i+0x1000)/5, &(result->s[i]));
	/* horrible special case for the end */
	g(key, (254+0x1000)/5, h);
	for (i=254;i<256;i++)
		result->s[i] = h[i-254];
	/* 0x2000 mod 5 is +2, so have horrible special case at the start */
	g(key, (-2+0x2000)/5, h);
	for (i=0;i<3;i++)
		result->r[i] = h[i+2];
	for (i=3;i<13;i+=5)
		g(key, (i+0x2000)/5, &(result->r[i]));
	/* horrible special case for the end */
	g(key, (13+0x2000)/5, h);
	for (i=13;i<16;i++)
		result->r[i] = h[i-13];
#ifdef SEAL_DEBUG
	printf("T:\n");
	for (i=0;i<8;i++)
		printf("%08x ", result->t[i]);
	printf("\n");
	for (i=0;i<8;i++)
                printf("%08x ", result->t[i+8]);
	printf("\n");
	printf("...\n");
	for (i=0;i<8;i++)
                printf("%08x ", result->t[i+504]);
	printf("\n");
	printf("S:\n");
	for (i=0;i<8;i++)
                printf("%08x ", result->s[i]);
	printf("\n");
	for (i=0;i<8;i++)
                printf("%08x ", result->s[i+8]);
	printf("\n");
	printf("...\n");
	for (i=0;i<8;i++)
                printf("%08x ", result->s[i+248]);
	printf("\n");
	printf("R:\n");
	for (i=0;i<8;i++)
                printf("%08x ", result->r[i]);
	printf("\n");
	for (i=0;i<8;i++)
                printf("%08x ", result->r[i+8]);
	printf("\n");
#endif /* SEAL_DEBUG */
	*ks = result;
	return (ALG_OK);
}

int seal(key, in, out)
struct SealKey *key;
unsigned long in;
unsigned long *out;
{
int i;
int j;
int l;
unsigned long a;
unsigned long b;
unsigned long c;
unsigned long d;
unsigned short p;
unsigned short q;
unsigned long n1;
unsigned long n2;
unsigned long n3;
unsigned long n4;
unsigned long *wp;

	wp = out;

	for (l=0;l<4;l++)
	{
		a = in ^ key->r[4*l];
		b = ROT8(in) ^ key->r[4*l+1];
		c = ROT16(in) ^ key->r[4*l+2];
		d = ROT24(in) ^ key->r[4*l+3];
	
		for (j=0;j<2;j++)
		{
			p = a & 0x7fc;
			b += key->t[p/4];
			a = ROT9(a);
	
			p = b & 0x7fc;
			c += key->t[p/4];
			b = ROT9(b);
	
			p = c & 0x7fc;
			d += key->t[p/4];
			c = ROT9(c);
	
			p = d & 0x7fc;
			a += key->t[p/4];
			d = ROT9(d);
	
		}
		n1 = d;
		n2 = b;
		n3 = a;
		n4 = c;
	
		p = a & 0x7fc;
		b += key->t[p/4];
		a = ROT9(a);
	
		p = b & 0x7fc;
		c += key->t[p/4];
		b = ROT9(b);
	
		p = c & 0x7fc;
		d += key->t[p/4];
		c = ROT9(c);
	
		p = d & 0x7fc;
		a += key->t[p/4];
		d = ROT9(d);
		
		for (i=0;i<64;i++)
		{
			p = a & 0x7fc;
			b += key->t[p/4];
			a = ROT9(a);
			b ^= a;
	
			q = b & 0x7fc;
			c ^= key->t[q/4];
			b = ROT9(b);
			c += b;
	
			p = (p+c) & 0x7fc;
			d += key->t[p/4];
			c = ROT9(c);
			d ^= c;
	
			q = (q+d) & 0x7fc;
			a ^= key->t[q/4];
			d = ROT9(d);
			a += d;
	
			p = (p+a) & 0x7fc;
			b ^= key->t[p/4];
			a = ROT9(a);
	
			q = (q+b) & 0x7fc;
			c += key->t[q/4];
			b = ROT9(b);
	
			p = (p+c) & 0x7fc;
			d ^= key->t[p/4];
			c = ROT9(c);
	
			q = (q+d) & 0x7fc;
			a += key->t[q/4];
			d = ROT9(d);
	
			*wp = b + key->s[4*i];
			wp++;
			*wp = c ^ key->s[4*i+1];
			wp++;
			*wp = d + key->s[4*i+2];
			wp++;
			*wp = a ^ key->s[4*i+3];
			wp++;
	
			if (i & 1)
			{
				a += n3;
				c += n4;
			}
			else
			{
				a += n1;
				c += n2;
			}
				
		}
	}
	return (ALG_OK);
}

/* end of file */

