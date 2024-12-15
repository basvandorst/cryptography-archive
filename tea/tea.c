/*
The Tiny Encryption Algorithm, or TEA, is a Feistel cipher invented by David
Wheeler. It is intended for use in applications where code size is at a
premium, or where it is necessary for someone to remember the algorithm and
code it on an arbitrary machine at a later time.

Since its round function is relatively weak, with nonlinearity coming only from
the carry propagation, TEA has 64 rounds. However, its simplicity means that it
runs more quickly in software than many other algorithms with fewer, more
complex, rounds.
*/

void code(long* v, long* k)
{
unsigned long y=v[0],z=v[1],sum=0,             /* set up */
              delta=0x9e3779b9, n=32 ;         /* key schedule constant*/

while (n-->0)
{                                              /* basic cycle start*/
  sum += delta ;
  y += (z<<4)+k[0] ^ z+sum ^ (z>>5)+k[1] ;
  z += (y<<4)+k[2] ^ y+sum ^ (y>>5)+k[3] ;     /* end cycle */
}
v[0]=y ;
v[1]=z ;
}
