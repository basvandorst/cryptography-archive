#include <wtypes.h>
#include <winbase.h>
#include <windef.h>
#include <winnt.h>
#include <winuser.h>
#include <process.h>
/* #include "libcrypt.h" */

volatile unsigned long count, ocount, randbuf;
volatile int dontstop;
char outbuf[1024], *bufp;

static void counter() {
	while (dontstop)
		count++;
	_endthread();
}


static unsigned long roulette() {
	unsigned long thread;

	count = 0;
	dontstop= 1;
	while ((thread = _beginthread((void *)counter, 1024, NULL)) < 0)
		;

	Sleep(16);
	dontstop = 0;
	Sleep(1);

	count ^= (count>>3) ^ (count>>6) ^ (ocount);
	count &= 0x7;
	ocount = count;
	randbuf = (randbuf<<3) ^ count;
	return randbuf;
}


unsigned long raw_truerand() {
	int ii;

	roulette();
	roulette();
	roulette();
	roulette();
	roulette();
	roulette();
	roulette();
	roulette();
	roulette();
	roulette();
	return roulette();
}

#ifdef RAND_DEBUG
int WINAPI WinMain(HINSTANCE hins, HINSTANCE hprevins, LPSTR cmdline, int cmdshow)
{
	int i, j;
	unsigned char randbuf[1024];
	FILE *fp;

#ifdef nodef
	bufp = outbuf;
	memset(outbuf, 0, 1024);
	for (i=0; i<25; i++)
		bufp += sprintf(bufp, "%08lx\n", raw_truerand());
	MessageBox(NULL, outbuf, "TEST", MB_ABORTRETRYIGNORE);
	return 1;
#endif

	fp = fopen("/users/lacy/newrand.out","wb");
	
	for (i=0; i<1024; i++) {
		for (j=0; j<1024; j++) {
			randbuf[j] = (unsigned char)(truebyte() & 0xff);
		}
		fwrite(randbuf, 1, 1024, fp);
	}
	fclose(fp);

	return 1;
}
#endif
