#ifndef __GMP__PLUS_H__
#define __GMP__PLUS_H__
#ifdef KLUDGE
#define mpn_rshift newmpn_rshift	/*  TEMPORARY KLUDGE!!!  */
#endif

#include "gmp.h"

mp_size_t mpn_bingcd(mp_ptr g, mp_ptr u, mp_size_t ul, mp_ptr v, mp_size_t vl);
mp_size_t mpn_accelgcd(mp_ptr g, mp_ptr u, mp_size_t ul, mp_ptr v,mp_size_t vl);
mp_size_t mpn_bmod(mp_ptr, mp_size_t, mp_srcptr, mp_size_t);
mp_size_t mpn_bmodgcd(mp_ptr, mp_ptr, mp_size_t, mp_ptr, mp_size_t);
void mpn_compl (mp_ptr res_ptr, mp_srcptr src_ptr, mp_size_t size);
mp_limb mpn_compl_rsh (register mp_ptr wp,
	    register mp_srcptr up, mp_size_t usize,
	    register unsigned int cnt);
mp_limb mpn_sub_n_rsh (mp_ptr res_ptr,
	       mp_srcptr s1_ptr, mp_srcptr s2_ptr, mp_size_t size,
	       unsigned int cnt);

int mpz_divides(const MP_INT *vz, const MP_INT *uz);

#endif /* ndef __GMP__PLUS_H__  */
