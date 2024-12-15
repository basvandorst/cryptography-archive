/* 
 * simple insecure hash function   -- 64 bits of output
 * the two data paramaters to hashstr are seed values 
 * assumes 32 bit integers
 */

char *linsert(x, i)
char *x;
unsigned int i;
{
    *x++ = i & 0xff; i>>=8;
    while (i) {
        *x++ = i & 0xff; i>>=8;
    }
    return x;
}


void scramble(s)
char *s;
{
    char s0,s1,s2,s3;
    s0 = *s; s1 = s[1]; s2 = s[2]; s3 = s[3];
    s[0] = (s[7] + s[4]) & 0xff;
    s[1] = (s[5] + s3) & 0xff;
    s[2] = (~(s2 + s[4])) & 0xff;
    s[3] = (s[5] + s0) & 0xff;
    s[4] = (s[6] + s3) & 0xff;
    s[5] = (~(s[6] + s1)) & 0xff;
    s[6] = (s0 + s[7]) & 0xff;
    s[7] = (s2 + s1) & 0xff;
}
    
/*
input parameters
str - input -- string    
len - length of input string -- int
data0, data1 -- seed values

returns:
null terminated char *
 hex value of hash
*/
char *hashstr(str, len, data0, data1)
char *str;
int len;
unsigned int data0,data1;
{
    char qq[8];
    char *ret = (char *)malloc(20);
    char *l;
    int z,i;
    memset(&qq[0],0,8);
    
    l = &qq[0];
    l = linsert(l,data0);
    l = linsert(l,data1);

    for (l=str; len > 0; l+=8,len-=8) { 
        for (z = 0; z < ((len < 8) ? len : 8); z++) 
            qq[z] ^= *(l+z);
	scramble(qq);
    }	    


    for (i=0;i<8;i++) {
        int h,l;
        h = (qq[i] & 0xf0) >> 4;
        l = (qq[i] & 0x0f);
        if (h < 10)
            *(ret+i+i) = h + '0';
        else
            *(ret+i+i) = h - 10 + 'A';
        if (l < 10)
            *(ret+i+i+1) = l + '0';
        else
            *(ret+i+i+1) = l - 10 + 'A';
    }
    *(ret+16)=0;
    return ret;
}

