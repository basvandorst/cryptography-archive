/* Crypto stream generator
 * Reads 16 bytes from stdin. The first 8 are the DES key, the second 8
 * are the initialization vector. The IV is
 * fed to DES in the encrypt mode (no IP, IP-1). The ciphertext
 * is then written to stdout, most significant byte first. The IV
 * register is then incremented by one and the process is repeated,
 * ad infinitum.
 */
#include <stdio.h>
main(argc,argv)
int argc;
char *argv[];
{
	char ks[16][8];	/* Key schedule */
	char iv[8];	/* Initial vector */
	char out[8];	/* Output buffer */
	int i,j;
	

	desinit(1);
	fread(iv,1,8,stdin);
	setkey(ks,iv);
	fread(iv,1,8,stdin);
	while(1){
		memcpy(out,iv,8);
		endes(ks,out);
		for(i=0;i<8;i++)
			putc(out[i],stdout);
		/* Increment register */
		iv[7]++;
		for(i=7;i != 0;i--){
			if(iv[i] == 0)
				iv[i-1]++;	/* Propagate carry */
		}
	}
}
int
htoi(c)
char c;
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


