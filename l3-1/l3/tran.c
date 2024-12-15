#include <stdio.h>
#include <fcntl.h>

static int flag2 = 0;
static int flag1 = 0;
extern void set_rnd_seed1();
extern void set_rnd_seed2();
extern long rnd1();
extern long rnd2();

long str2int();

#define BLOCKSIZE   4096

static int perm[BLOCKSIZE];

#define swap(A,B)  tmp = A; A = B; B = tmp;
void clear_tran()
{
	flag2 = flag1 = 0;
}

void tran1(key, buff, buffl)
unsigned char key[4];
char *buff;
int buffl;
{
	int i, pos;
	long keykey;
	char tmp;
	if (flag1 == 0) {
		flag1 = 1;
		keykey = 0;
		for (i = 0; i < 4; i++) {
			keykey <<= 8;
			keykey += key[i];
		}
		keykey += str2int(buff, buffl);
		set_rnd_seed1(keykey);
		keykey = 0;
	}
	for (i = 0; i < buffl; i++) {
		perm[i] = i;
	}
	while (buffl > 1) {
		pos = 1 + rnd1() % (buffl - 1);
		swap(buff[perm[0]], buff[perm[pos]]);
		perm[0] = perm[(pos == buffl - 2) ? buffl - 1 : buffl - 2];
		perm[pos] = perm[buffl - 1];
		buffl -= 2;
	}
}

void tran2(key, buff, buffl)
unsigned char key[4];
char *buff;
int buffl;
{
	int i, pos;
	long keykey;
	char tmp;
	if (flag2 == 0) {
		flag2 = 1;
		keykey = 0;
		for (i = 0; i < 4; i++) {
			keykey <<= 8;
			keykey += key[i];
		}
		keykey += str2int(buff, buffl);
		set_rnd_seed2(keykey);
		keykey = 0;
	}
	for (i = 0; i < buffl; i++) {
		perm[i] = i;
	}
	while (buffl > 1) {
		pos = 1 + rnd2() % (buffl - 1);
		swap(buff[perm[0]], buff[perm[pos]]);
		perm[0] = perm[(pos == buffl - 2) ? buffl - 1 : buffl - 2];
		perm[pos] = perm[buffl - 1];
		buffl -= 2;
	}
}

long str2int(str, len)
unsigned char *str;
int len;
{
	int i;
	long sum;
	sum = 0;
	for (i = 0; i < len; i++) {
		sum += *str++;
	}
	return (sum);
}
