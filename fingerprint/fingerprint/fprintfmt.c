/* fprintfile.c, fprintfmt.c, fprintfile.h: fingerprint library
Daniel J. Bernstein, brnstnd@nyu.edu.
Depends on snefrufile.h, md5file.h, crc32file.h.
No environment requirements.
930708: fprintfile 0.95. Public domain.
930708: Changed slash to colon in base64sane.
930622: Split off fprintfmt.c.
930601: Baseline, fprintfile 0.8. Public domain.
No known patent problems.

Documentation in fprintfile.3.
*/

#include "fprintfile.h"

static char base64sane[]
= "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:.";

unsigned int fingerprintfile_fmt(s,h)
char *s;
unsigned char *h;
{
 if (s)
  {
   int i;
   unsigned long x;

   for (i = 0;i < 19;++i)
    {
     x = h[3 * i] + 256 * (h[3 * i + 1] + 256 * (long) h[3 * i + 2]);
     s[(12 * i) % 76] = base64sane[x & 63]; x /= 64;
     s[(12 * i + 41) % 76] = base64sane[x & 63]; x /= 64;
     s[(12 * i + 6) % 76] = base64sane[x & 63]; x /= 64;
     s[(12 * i + 47) % 76] = base64sane[x & 63];
    }
  }
 return 76;
}

static int unbase64(c)
char c;
{
 int i;

 /* XXX: efficiency? */

 for (i = 0;i < 64;++i)
   if (base64sane[i] == c)
     return i;
 return -1;
}

unsigned int fingerprintfile_scan(s,h)
char *s;
unsigned char h[57];
{
 int i;
 unsigned long x;

 for (i = 0;i < 76;++i)
   if (unbase64(s[i]) == -1) /* XXX: great, calling it twice! */
     return 0;
 for (i = 0;i < 19;++i)
  {
   x = unbase64(s[(12 * i + 47) % 76]); x *= 64;
   x += unbase64(s[(12 * i + 6) % 76]); x *= 64;
   x += unbase64(s[(12 * i + 41) % 76]); x *= 64;
   x += unbase64(s[(12 * i) % 76]);
   h[3 * i] = x & 255; x = x / 256;
   h[3 * i + 1] = x & 255; x = x / 256;
   h[3 * i + 2] = x;
  }
 return 76;
}
