/* snefrufile.c, snefrufile.h: Snefru stream hashing library
Daniel J. Bernstein, brnstnd@nyu.edu.
Depends on snefru512.h.
No environment requirements.
930708: snefrufile 0.95. Public domain.
930708: Added snefrufile_addn.
930708: Moved wm, wl, ctr out to static globals.
930601: Baseline, snefrufile 0.8. Public domain.
No known patent problems.

Documentation in snefrufile.3.
*/

#include "snefru512.h"
#include "snefrufile.h"

static snefru512_word tr[16] =
/* backwards, for compatibility with Merkle's implementation */
 {
  66051, 67438087, 134810123, 202182159, 269554195, 336926231,
  404298267, 471670303, 539042339, 606414375, 673786411, 741158447,
  808530483, 875902519, 943274555, 1010646591
 } ;

void snefrufile_clear(s)
snefrufile *s;
{
 int i;

 snefru512_setup(); /* XXX: called multiple times unnecessarily */

 for (i = 0;i < 64;++i) s->c[i] = 0;
 for (i = 0;i < 8;++i) s->len[i] = 0;
 s->n = 32;
}

static unsigned char l[64];
static unsigned char m[32];
static snefru512_word *wm = &m[0];
static snefru512_word *wl = &l[0];
static unsigned char *ctr = &tr[0];

void snefrufile_addn(s,t,len)
snefrufile *s;
register unsigned char *t;
int len;
{
 register int n;
 register int i;
 unsigned char newlen;

 i = len;
 while (i >= 32)
  {
   if (!++s->len[1]) if (!++s->len[2]) if (!++s->len[3]) if (!++s->len[4])
   if (!++s->len[5]) if (!++s->len[6]) if (!++s->len[7]) ; /* too bad */
   i -= 32;
  }
 newlen = i << 3;
 if ((s->len[0] += newlen) < newlen)
   if (!++s->len[1]) if (!++s->len[2]) if (!++s->len[3]) if (!++s->len[4])
   if (!++s->len[5]) if (!++s->len[6]) if (!++s->len[7]) ; /* too bad */
 n = s->n;
 while (len--)
  {
   s->c[n++] = *t++;
   if (n == 64)
    {
     for (i = 0;i < 64;++i) l[ctr[i]] = s->c[i];
     snefru512(wm,wl,8);
     for (i = 0;i < 32;++i) s->c[i] = m[ctr[i]];
     n = 32;
    }
  }
 s->n = n;
}

void snefrufile_add(s,c)
snefrufile *s;
unsigned char c;
{
 int n;
 register int i;

 n = s->n;
 s->c[n++] = c;
 if (!(s->len[0] += 8))
   if (!++s->len[1]) if (!++s->len[2]) if (!++s->len[3]) if (!++s->len[4])
   if (!++s->len[5]) if (!++s->len[6]) if (!++s->len[7]) ; /* too bad */
 if (n == 64)
  {
   for (i = 0;i < 64;++i) l[ctr[i]] = s->c[i];
   snefru512(wm,wl,8);
   for (i = 0;i < 32;++i) s->c[i] = m[ctr[i]];
   n = 32;
  }
 s->n = n;
}

void snefrufile_hash(s,h)
snefrufile *s;
unsigned char *h;
{
 int len[8];
 int i;

 for (i = 0;i < 8;++i) len[i] = s->len[i];
 while (s->n != 32) snefrufile_add(s,(unsigned char) 0);
 while (s->n != 56) snefrufile_add(s,(unsigned char) 0);
 for (i = 0;i < 8;++i) snefrufile_add(s,len[7 - i]);
 for (i = 0;i < 32;++i) h[i] = s->c[i];
}
