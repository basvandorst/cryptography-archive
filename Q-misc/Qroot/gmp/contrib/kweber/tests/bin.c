#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"

mp_limb bin(mp_srcptr u, mp_size_t ul, mp_srcptr v, mp_size_t vl)
{   mp_ptr u0 = (mp_ptr)alloca(sizeof(mp_limb)*ul);
    mp_ptr v0 = (mp_ptr)alloca(sizeof(mp_limb)*vl);
    MPN_COPY(u0,u,ul);
    MPN_COPY(v0,v,vl);
    ul = mpn_bingcd(u0, u0, ul, v0, vl);
    return u0[0];
}
