#include "pdefs.h"
#include "precision.h"

precision pfastmul(u, v, minsize)
   precision u, v;
   posit minsize;
{
   precision w0 = pUndef, w1 = pUndef, w2 = pUndef, w = pUndef;
   precision u0, v0, u1, v1;
   register posit usize, vsize, n;

   pparm(u);
   pparm(v);
   usize = u->size;
   vsize = v->size;
   if (usize > vsize) {
      n = vsize / 2;
   } else {
      n = usize / 2;
   }
   if (n < minsize) {
      pset(&w, pmul(u, v));
   } else {
      u0 = palloc(n);
      u0->sign = u->sign;
      (void) memcpy(u0->value, u->value, n * sizeof(digit));

      v0 = palloc(n);
      v0->sign = v->sign;
      (void) memcpy(v0->value, v->value, n * sizeof(digit));

      u1 = palloc(usize - n);
      u1->sign = false;
      (void) memcpy(u1->value, u->value + n, (usize - n) * sizeof(digit));

      v1 = palloc(vsize - n);
      v1->sign = false;
      (void) memcpy(v1->value, v->value + n, (vsize - n) * sizeof(digit));

      pset(&w1, pfastmul(psub(u1, u0), psub(v0, v1), minsize));

      pset(&w0, pfastmul(u0, v0, minsize));
      pdestroy(u0);
      pdestroy(v0);

      pset(&w2, pfastmul(u1, v1,  minsize));
      pdestroy(u1);
      pdestroy(v1);

      pset(&w1, padd(w1, w0));
      pset(&w1, padd(w1, w2));

      w = palloc(usize + vsize);
      w->sign = false;
      (void) memset(w->value, 0, (usize + vsize) * sizeof(digit));
      (void) memcpy(w->value, w0->value, w0->size * sizeof(digit));
      (void) memcpy(w->value + n + n, w2->value, w2->size * sizeof(digit));
      pnorm(w);
      pdestroy(w0);
      pdestroy(w2);

      pset(&w, padd(w, pshift(w1, n)));
      pdestroy(w1);
   }
   pdestroy(u);
   pdestroy(v);
   return presult(w);
}
