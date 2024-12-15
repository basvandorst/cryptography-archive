/* misc.c - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"
void ferr(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(123);
}

int write_string(FILE * f, unsigned char *string)
{
	unsigned int length;
	unsigned int i;
	unsigned char clength;

	length = strlen(string);
	if (length > 127)
		return 0;
	clength = (unsigned char) length;

	fputc(length, f);
	for (i = 0; i < length; i++)
		fputc(*(string + i), f);
	return 1;
}

int read_string(FILE * f, unsigned char *buf)
{
	int i;
	int length;
	int j;
	length = fgetc(f);
	if (length < 0 || length > 127)
		return 0;
	for (i = 0; i < length; i++) {
		j = fgetc(f);
		if (j >= 0)
			*(buf + i) = j;
		else
			return 0;
	}
	*(buf + i) = '\0';
	return 1;
}

void write_number(FILE * f, MP_INT * x)
{
	unsigned char buf[MAXC];
	int i, j;
	unsigned int length;

	/* encode into buf */
	mptcs(buf, MAXC, x);

	/* find first non-zero in buf */
	for (i = 0; i < MAXC && !buf[i]; i++);

	if (i == MAXC) {
		/* number is 0 */
		i--;
	}
	length = MAXC - i;

	fputc((unsigned char) (length & 0xff), f);
	fputc((unsigned char) (length >> 8), f);
	for (j = i; j < MAXC; j++)
		fputc(buf[j], f);
}

int read_number(FILE * f, MP_INT * x)
{
	unsigned char buf[MAXC];
	int i, j;
	unsigned int length;

	length = fgetc(f);
	length |= (fgetc(f) << 8);
	if (length < 0 || length > MAXC)
		return 0;
	for (i = 0; i < length; i++) {
		j = fgetc(f);
		if (j < 0)
			return 0;
		buf[i] = (unsigned char) j;
	}
	cstmp(x, buf, length);
	return 1;
}

#define cleanup \
zap(mbuf,8); zap(nbuf,8), zap(buf,8); zap(sbuf,8); zap(bigbuf,4096); \
zap(iv,24)

int scramble(FILE * in, FILE * out, unsigned char *key, unsigned char *iiv,
			 int decrypt)
/* key is 72 bytes, iv is 24 bytes , 72 + 24 == 96 == LKEYLEN */
{
	unsigned char buf[8], nbuf[8];
	unsigned char mbuf[8], sbuf[8];
	unsigned char iv[24];
	BFkey_type     *bfkey ;
	unsigned char bigbuf[4096];
	unsigned char *t, *s;
	int count;
	int i, b, j, k;
	int must_pad;
	int ret_val = 1;
	clear_tran();

#ifdef DEBUG
	fprintf(stderr, "scramble key\n ");
	for (i=0; i<36; i++)
		fprintf(stderr, "%02x", key[i]);
	fprintf(stderr, "\n ");
	for (i=36; i<72; i++)
		fprintf(stderr, "%02x", key[i]);
	fprintf(stderr, "\n");
#endif

	des4key(&key[0], decrypt);	/* bytes 0-31 */
	initkey_idea(&key[32], decrypt);	/* bytes 32-47 */
	/* bytes 48 - 55 are used for tran */
	memcpy(iv, iiv, 24);  /* don't damage input iv */

	if (!(bfkey = make_bfkey(&key[56], (short)16))) {
		fprintf(stderr, "fatal key setup error\n");
		exit(1);
	}

	if (!decrypt) {	/* encrypt */
		must_pad = 1;	/* assume the worst */
		while (((i = fread(&bigbuf[0], 1, 4096, in)) > 0) || must_pad) {
			memcpy(mbuf, iv, 8);
			if (i < 4096) {
				for (k = i; k < i + (8 - (i % 8)); k++)
					bigbuf[k] = 0;
				bigbuf[i + (8 - (i % 8)) - 1] = i % 8;
				i += 8 - (i % 8);
				must_pad = 0;	/* we've already padded, so now we don't need to */
			}
			for (j = 0; j < i; j += 8) {
				for (t = &bigbuf[j], s = mbuf, count = 8; count; count--)
					*(t++) ^= *(s++);
				Ddes(&bigbuf[j], &bigbuf[j]);
				memcpy(mbuf, &bigbuf[j], 8);
			}
			memcpy(iv, mbuf, 8);
			tran1(&key[48], bigbuf, i);
			memcpy(mbuf, &iv[8], 8);
			for (j = 0; j < i; j += 8) {
				for (t = &bigbuf[j], s = mbuf, count = 8; count; count--)
					*(t++) ^= *(s++);
				idea_ecb((word16 *) & bigbuf[j], (word16 *) & bigbuf[j]);
				crypt_8bytes(&bigbuf[j],&bigbuf[j],bfkey,(short)0);
				memcpy(mbuf, &bigbuf[j], 8);
			}
			memcpy(&iv[8], mbuf, 8);
			tran2(&key[52], bigbuf, i);
			memcpy(mbuf, &iv[16], 8);
			for (j = 0; j < i; j += 8) {
				for (t = &bigbuf[j], s = mbuf, count = 8; count; count--)
					*(t++) ^= *(s++);
				des4(&bigbuf[j], &bigbuf[j]);
				idea_ecb((word16 *) & bigbuf[j], (word16 *) & bigbuf[j]);
				crypt_8bytes(&bigbuf[j],&bigbuf[j],bfkey,(short)0);
				Ddes(&bigbuf[j], &bigbuf[j]);
				memcpy(mbuf, &bigbuf[j], 8);
			}
			memcpy(&iv[16], mbuf, 8);
			fwrite(&bigbuf[0], 1, i, out);
		}
	}
	else {	/* decrypt */
		while ((i = fread(bigbuf, 1, 4096, in)) > 0) {
			if (i < 8 || i % 8) {
				cleanup;
				return 0;
			}
			memcpy(buf, &bigbuf[0], 8);
			memcpy(mbuf, &iv[16], 8);
			for (j = 8; j < i; j += 8) {
				memcpy(nbuf, &bigbuf[j], 8);
				memcpy(sbuf, buf, 8);
				Ddes(buf,buf);
				crypt_8bytes(buf,buf,bfkey,(short)1);
				idea_ecb((word16 *)buf,(word16 *)buf);
				des4(buf, buf);
				for (t = buf, s = mbuf, count = 8; count; count--)
					*(t++) ^= *(s++);
				memcpy(mbuf, sbuf, 8);
				memcpy(&bigbuf[j - 8], buf, 8);;
				memcpy(buf, nbuf, 8);
			}
			memcpy(sbuf, buf, 8);
			Ddes(buf,buf);
			crypt_8bytes(buf,buf,bfkey,(short)1);
			idea_ecb((word16 *)buf,(word16 *)buf);
			des4(buf, buf);
			for (t = buf, s = mbuf, count = 8; count; count--)
				*(t++) ^= *(s++);
			memcpy(&bigbuf[j - 8], buf, 8);
			memcpy(&iv[16], sbuf, 8);
			tran2(&key[52], bigbuf, i);
			memcpy(mbuf, &iv[8], 8);
			memcpy(buf, &bigbuf[0], 8);
			for (j = 8; j < i; j += 8) {
				memcpy(nbuf, &bigbuf[j], 8);
				memcpy(sbuf, buf, 8);
				crypt_8bytes(buf,buf,bfkey,(short)1);
				idea_ecb((word16 *) buf, (word16 *) buf);
				for (t = buf, s = mbuf, count = 8; count; count--)
					*(t++) ^= *(s++);
				memcpy(mbuf, sbuf, 8);
				memcpy(&bigbuf[j - 8], buf, 8);;
				memcpy(buf, nbuf, 8);
			}
			memcpy(sbuf, buf, 8);
			crypt_8bytes(buf,buf,bfkey,(short)1);
			idea_ecb((word16 *) buf, (word16 *) buf);
			for (t = buf, s = mbuf, count = 8; count; count--)
				*(t++) ^= *(s++);
			memcpy(&bigbuf[j - 8], buf, 8);
			memcpy(&iv[8], sbuf, 8);
			tran1(&key[48], bigbuf, i);
			memcpy(mbuf, iv, 8);
			memcpy(buf, &bigbuf[0], 8);
			for (j = 8; j < i; j += 8) {
				memcpy(nbuf, &bigbuf[j], 8);
				memcpy(sbuf, buf, 8);
				Ddes(buf, buf);
				for (t = buf, s = mbuf, count = 8; count; count--)
					*(t++) ^= *(s++);
				memcpy(mbuf, sbuf, 8);
				memcpy(&bigbuf[j - 8], buf, 8);
				memcpy(buf, nbuf, 8);
			}
			memcpy(sbuf, buf, 8);
			Ddes(buf, buf);
			for (t = buf, s = mbuf, count = 8; count; count--)
				*(t++) ^= *(s++);
			memcpy(&bigbuf[j - 8], buf, 8);
			memcpy(iv, sbuf, 8);
			if ((b = getc(in)) != EOF) {
				ungetc(b, in);
				fwrite(&bigbuf[0], 1, i, out);
			}
			else {
				/* this is the last bigbuf - so we need to strip off padding */
				b = (int) bigbuf[i - 1];
				if (b < 0 || b > 7) { /* something is wrong */
					ret_val = 0;
					b = 8; 
				}
				i = i - 8 + b;
				/* at this point i might be 0 */
				if (i)
					fwrite(&bigbuf[0], 1, i, out);
			}
			memcpy(buf, nbuf, 8);
		}
	}
	close_idea();
	close_des();
	clear_tran();
	destroy_bfkey(bfkey);
	cleanup;
	return ret_val;
}

void mdfile(FILE * inFile, unsigned char *digest)
{
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	MD5Init(&mdContext);
	while ((bytes = fread(data, 1, 1024, inFile)) != 0)
		MD5Update(&mdContext, data, bytes);
	MD5Final(&mdContext);
	memcpy(digest, &mdContext.digest[0], 16);
}

void mdstring(char *s, unsigned int len, unsigned char *digest)
{
	MD5_CTX mdContext;
	int bytes;
	MD5Init(&mdContext);
	MD5Update(&mdContext, s, len);
	MD5Final(&mdContext);
	memcpy(digest, &mdContext.digest[0], 16);
}

/* this really shouldn't depend on MAXC, but it does -should fix, not hard */
void havalpkid(MP_INT *n, MP_INT *e, unsigned char *id, unsigned char *digest)
{
	unsigned char buf[MAXC];
	haval_state state;
#ifdef TEST
	int i;
#endif
	haval_start(&state);
	mptcs(buf, MAXC, n);
	haval_hash(&state,&buf[0],(unsigned int)MAXC);
	mptcs(buf, MAXC, e);
	haval_hash(&state,&buf[0],(unsigned int)MAXC);
	haval_hash(&state,id,(unsigned int)strlen(id));
	haval_end(&state,digest);
#ifdef TEST
	fprintf(stderr, "havalpkid returns digest:\n  ");
	for (i=0; i<HASHLEN; i++)
		fprintf(stderr, "%02x",digest[i]);
	fprintf(stderr, "\n");
#endif
}

/* this really shouldn't depend on MAXC, but it does -should fix, not hard */
void mdpkid(MP_INT * n, MP_INT * e, unsigned char *id, unsigned char *digest)
{
	unsigned char buf[MAXC];
	MD5_CTX mdContext;
	MD5Init(&mdContext);

	mptcs(buf, MAXC, n);
	MD5Update(&mdContext, buf, MAXC);
	mptcs(buf, MAXC, e);
	MD5Update(&mdContext, buf, MAXC);
	MD5Update(&mdContext, id, strlen(id));
	MD5Final(&mdContext);
	memcpy(digest, &mdContext.digest[0], 16);
}

/* 16 byte key - for private keys */
static void desbuf(int mode, unsigned char *buf, unsigned int len,
				   unsigned char *key, unsigned char *iv)
{
	int count;
	unsigned char miv[8], siv[8];
	unsigned char *s, *t;
	unsigned int block_cnt = (len + 8 - 1) / 8;

	des2key(key, mode);
	memcpy(miv, iv, 8);

	if (!mode) {	/* encrypt == EN0 */
		while (block_cnt--) {
			for (t = buf, s = miv, count = 8; count; count--)
				*(t++) ^= *(s++);
			Ddes(buf, buf);
			memcpy(miv, buf, 8);
			buf += 8;
		}
	}
	else {
		while (block_cnt--) {
			memcpy(siv, buf, 8);
			Ddes(buf, buf);
			for (t = buf, s = miv, count = 8; count; count--)
				*(t++) ^= *(s++);
			memcpy(miv, siv, 8);
			buf += 8;
		}
	}
	close_des();
}

void write_numberx(FILE * f, MP_INT * x, unsigned char *key, unsigned char *iv)
{
	unsigned char buf[MAXC];
	int i, j, k;
	unsigned int length;

	/* encode into buf */
	mptcs(buf, MAXC, x);

	/* find first non-zero in buf */
	for (i = 0; i < MAXC && !buf[i]; i++);

	if (i == MAXC) {
		/* number is 0 */
		i--;
	}
	length = MAXC - i;
	k = length % 8;

	i -= (8 - k);
	length += (8 - k);

	fputc((unsigned char) (length & 0xff), f);
	fputc((unsigned char) (length >> 8), f);
	desbuf(EN0, &buf[i], length, key, iv);
	for (j = i; j < MAXC; j++)
		fputc(buf[j], f);
}

int read_numberx(FILE * f, MP_INT * x, unsigned char *key, unsigned char *iv)
{
	unsigned char buf[MAXC];
	int i, j;
	unsigned int length;

	length = fgetc(f);
	length |= (fgetc(f) << 8);
	if (length < 0 || length > MAXC || length % 8)
		return 0;
	for (i = 0; i < length; i++) {
		j = fgetc(f);
		if (j < 0)
			return 0;
		buf[i] = (unsigned char) j;
	}
	desbuf(DE1, buf, length, key, iv);
	if (buf[0] != 0)
		return 0;
	cstmp(x, buf, length);
	return 1;
}

FILE *mfopen(unsigned char *file, unsigned char *mode)
{
	unsigned char *pc, *p, *po;
	unsigned char *s;
	unsigned char *path[256];
	unsigned char fbuf[1024];
	unsigned int npath;
	unsigned int i;
	FILE *ret;

	/* if filename contains a /, just try and open it, otherwise
     * work along L3PATH trying to open file
     */

	if (strchr(file, '/')) {
		ret = fopen(file, mode);
	}
	else {
		po = p = (unsigned char *) malloc(1024);
		npath = 0;
		ret = NULL;
		if (pc = (unsigned char *) getenv("L3PATH")) {
			strncpy(p, pc, 1024);
			p[1023] = '\0';
			for (s = p; *p && (npath < 256); p++) {
				if (*p == ':') {
					*p = '\0';
					if (s != p)
						path[npath] = s;
					else
						path[npath] = ".";
					s = p + 1;
					npath++;
				}
			}
			if (npath < 256) {
				/* need to deal with end */
				*p = '\0';
				if (s != p)
					path[npath] = s;
				else
					path[npath] = ".";
				npath++;
			}
		}
		else {
			npath = 1;
			path[0] = ".";
		}
		for (i = 0; i < npath; i++) {
			strcpy(fbuf, path[i]);
			strcat(fbuf, "/");
			strcat(fbuf, file);
			if (ret = fopen(fbuf, mode)) {
				break;
			}
		}
		free(po);
	}
	return ret;
}

void zap(void *ptr, unsigned int size)
{
	memset(ptr, 0, size);
}

int getalg(void)
{
	char *s;
	int i;
	s = getenv("L3ALG");
	if (!s)
		i = PALG;
	else {
		i = atoi(s);
		if (i == 0)
			i = PALG;
		if (i > 3)
			i = PALG;
	}
#ifdef DEBUG
	fprintf(stderr, "getalg returns %d\n", i);
#endif
	return i;
}

int readalg(FILE * f, int *r)
{
	int j;
	j = fgetc(f);
#ifdef DEBUG
	fprintf(stderr, "readalg -- read %d\n", j);
#endif
	if (j < 0)
		return 0;
	if (j == 0)
		return 0;
	if (j > 3)
		return 0;
	*r = j;
	return 1;
}

/* read a public key and verify signature if there is one
returns -2 if key corrupt
        -1 if self signed with an invalid signature
         0 if public key is not self signed.
         1 if public key is self signed with a valid signature
         also outputs messages to stderr as a side effect
*/


int readpk(FILE * f, MP_INT * n, MP_INT * e)
{
	int salg;
	unsigned char idstring[128];
	char hash[HASHLEN];
	MP_INT *c;

	if (!read_number(f, n) || !read_number(f, e)) {
		return -2;
	}

	if (!read_string(f, idstring)) {
		fprintf(stderr, "warning: public key is not self signed\n");
		return 0;
	}
	else {
		init(c);
		havalpkid(n, e, idstring, hash);
		if (!readalg(f, &salg) || !read_number(f, c) ||
			!vsign(c, hash, n, e, salg)) {
			fprintf(stderr,
				"public key is self signed with an INVALID signature\n");
			fprintf(stderr, "public key corruption detected\n");
			clear(c);
			return (-1);
		}
		fprintf(stderr,
		"key is bound to identifier \n\"%s\"\n with a valid signature\n",
				idstring);
	}
	clear(c);
	return 1;
}
