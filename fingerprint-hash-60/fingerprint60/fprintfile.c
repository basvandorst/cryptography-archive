/* fprintfile.c, fprintfmt.c, fprintfile.h: fingerprint library
Daniel J. Bernstein, brnstnd@nyu.edu.
Depends on snefrufile.h, md5file.h, crc32file.h.
No environment requirements.
930708: fprintfile 0.95. Public domain.
930708: Added fingerprintfile_addn.
930622: Split off fprintfmt.c.
930601: Baseline, fprintfile 0.8. Public domain.
No known patent problems.

Documentation in fprintfile.3.
*/

#include "fprintfile.h"

void fingerprintfile_clear(f)
fingerprintfile *f;
{
 snefrufile_clear(&f->s);
 md5file_clear(&f->m);
 crc32file_clear(&f->c);
 f->len[0] = f->len[1] = f->len[2] = f->len[3] = f->len[4] = 0;
}

void fingerprintfile_addn(f,s,n)
fingerprintfile *f;
unsigned char *s;
int n;
{
 unsigned char newlen;

 snefrufile_addn(&f->s,s,n);
 md5file_addn(&f->m,s,n);
 crc32file_addn(&f->c,s,n);
 while (n >= 256)
  {
   if(!++f->len[1])if(!++f->len[2])if(!++f->len[3])++f->len[4];
   n -= 256;
  }
 newlen = n;
 if ((f->len[0] += newlen) < newlen)
   if(!++f->len[1])if(!++f->len[2])if(!++f->len[3])++f->len[4];
}

void fingerprintfile_add(f,c)
fingerprintfile *f;
unsigned char c;
{
 snefrufile_add(&f->s,c);
 md5file_add(&f->m,c);
 crc32file_add(&f->c,c);
 if(!++f->len[0])if(!++f->len[1])if(!++f->len[2])if(!++f->len[3])++f->len[4];
}

void fingerprintfile_hash(f,h)
fingerprintfile *f;
unsigned char h[57];
{
 snefrufile_hash(&f->s,h);
 md5file_hash(&f->m,h + 32);
 crc32file_hash(&f->c,h + 48);
 h[52] = f->len[0]; h[53] = f->len[1]; h[54] = f->len[2];
 h[55] = f->len[3]; h[56] = f->len[4];
}
