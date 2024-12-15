

/* C - program blockcipher IDEA */

#include <stdio.h>
#include <fcntl.h>

#define maxim 65537
#define fuyi  65536
#define one   65535
#define round 8


void idea_cip();
void idea_key();
void idea_de_key();

unsigned idea_inv();
unsigned idea_mul();


/* encryption algorithm */
void idea_cip(IN,OUT,Z)
unsigned IN[5];
unsigned OUT[5];
unsigned Z[7][10];
{
        unsigned int x1, x2, x3, x4;
        unsigned int r, kk, t1, t2, a;
        x1=IN[1]; x2=IN[2]; x3=IN[3]; x4=IN[4];
        for (r=1; r<=8; r++)
        {       x1 = idea_mul (x1,Z[1][r]);
                x4 = idea_mul (x4,Z[4][r]);
                x2 = (x2+Z[2][r]) & one;
                x3 = (x3+Z[3][r]) & one;
                kk = idea_mul(Z[5][r], (x1^x3));
                t1 = idea_mul(Z[6][r], (kk+(x2^x4)) & one);
                t2 = (kk+t1) & one;
                x1 = x1^t1; x4 = x4^t2;
                a  = x2^t2; x2 = x3^t1;
                x3 = a;
        }
        OUT[1] = idea_mul (x1,Z[1][round+1]);
        OUT[4] = idea_mul (x4,Z[4][round+1]);
        OUT[2] = (x3+Z[2][round+1]) & one;
        OUT[3] = (x2+Z[3][round+1]) & one;
}

/* multiplication modulo 2^16+1 of 16-bit integers with the zero subblock
   corresponding to 2^16  */
unsigned idea_mul(a,b)
unsigned a;
unsigned b;
{
        long int p;
        long unsigned q;
        if (a==0) p = maxim-b;
        else if (b==0) p = maxim-a;
        else
        {       q=a*b;
                p=(q & one) - (q>>16);
                if (p<=0) p = p+maxim;
        }
        return (unsigned) (p & one);
}

/* compute the inverse of xin by the Euclidean god algorithm */
unsigned idea_inv(xin)
unsigned xin;
{
        long n1, n2, q, r, b1, b2, t;
        unsigned k;

        if (xin==0) b2 = 0;
        else
        {      n1=maxim;
               n2=xin;
               b2=1; b1=0;
               do
               {       r=(n1 % n2);
                       q=(n1-r)/n2;
                       if (r==0) { if (b2<0) b2=maxim+b2; }
                       else
                       {       n1=n2; n2=r;
                               t=b2; b2=b1-q*b2;
                               b1=t;
                       }
               } while (r != 0);
        }
        k = (unsigned)b2;
        if (idea_mul(k,xin) != 1)
        {       printf("error!!! x %6d * x~1 ",xin,k);
                printf("%6d = %6d \n", idea_mul (k,xin),maxim);
        }
        return k;
}

/* generate subkeys Z's */
void idea_key(uskey,Z)
short unsigned uskey[9];
unsigned Z[7][10];
{
        short unsigned S[54];
        int   i,j,r;

        for (i=1; i<9; i++) S[i-1] = uskey[i];
        for (i=8; i<54; i++)
        {       if ((i+2)%8 == 0)       /* for S[14], S[22], ....*/
                        S[i] = (S[i-7]<<9) ^ (S[i-14]>>7);
                else if ((i+1)%8 == 0)  /* for S[15], S[23], ....*/
                        S[i] = (S[i-15]<<9) ^ (S[i-14]>>7);
                else
                        S[i] = (S[i-7]<<9) ^ (S[i-6]>>7);
        }

        for (r=1; r<=round+1; r++)
                for (j=1; j<7; j++)
                        Z[j][r] = S[6*(r-1) + j-1];
}

/* compute decryption subkeys DK's */
void idea_de_key (Z,DK)
unsigned Z[7][10];
unsigned DK[7][10];
{
        int j;
        for (j=1; j<=round+1; j++)
        {       DK[1][round-j+2] = idea_inv(Z[1][j]);
                DK[4][round-j+2] = idea_inv(Z[4][j]);
                if (j==1 || j==round+1)
                {       DK[2][round-j+2] = (fuyi-Z[2][j]) & one;
                        DK[3][round-j+2] = (fuyi-Z[3][j]) & one;
                }
                else
                {       DK[2][round-j+2] = (fuyi-Z[3][j]) & one;
                        DK[3][round-j+2] = (fuyi-Z[2][j]) & one;
                }
        }
        for (j=1; j<=round; j++)
        {       DK[5][round+1-j] = Z[5][j];
                DK[6][round+1-j] = Z[6][j];
        }
        DK[5][9] = 0;
        DK[6][9] = 0;
}
