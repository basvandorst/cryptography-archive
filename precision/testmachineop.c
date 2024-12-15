#include <stdio.h>
#include "machineop.h"

#define randnum(x)	(rand() & 0xefff)
#define urand16(x)	((randnum(x) << 1) | (randnum(x) >> 14))
#define urand32(x)	((randnum(x) << 17) | (randnum(x) << 2) | (randnum(x) >> 13))

typedef unsigned short u16;
typedef unsigned long u32;

main()
{
    u32 p;
    u16 u, v, w, q, q0, q1, r, r0, r1;
    int i;
    long ip, iq, ir, iu, iv;

    srand(0);
    for (i = 0; i < 32767; i++) {
       u = urand16(0);
       v = urand16(0);
       p = umul16(u, v);
       ip = u * v;
       if (p != ip) {
	  printf("umul16: %u * %u = %lu (%lu)\n", u, v, p, ip);
       }
       q = urand16(0);
       r = urand16(0);
       w = urand16(0);
       if (w == 0) continue;
       p = q * w + r;
       iq = p / w;
       ir = p % w;
       if (iq != q || ir != r) continue;
       if (iq > 65535 || ir > 65535) continue;
       q0 = udivmod16(p, w, &r0);
       if (r0 != ir || q0 != iq) {
	  printf("udivmod16: %lu / %u = %u + %u %(%u + %u)\n", p, w, q0, r0, iq, ir);
       }
       r1 = umoddiv16(p, w, &q1);
       if (r1 != ir || q1 != iq) {
	  printf("umoddiv16: %lu / %u = %u + %u %(%u + %u)\n", p, w, q1, r1, q0, r0);
       }
    }
    return 0;
}
