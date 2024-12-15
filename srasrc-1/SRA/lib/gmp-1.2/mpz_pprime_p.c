/* mpz_probab_prime_p --
 * An implementation of the probabilistic primality test found in Knuth's
 * Seminumerical Algorithms book.  If the function mpz_probab_prime_p()
 * returns 0 then n is not prime.  If it returns 1, then n is 'probably'
 * prime.  The probability of a false positive is (1/4)**reps, where
 * reps is the number of internal passes of the probabilistic algorithm.
 * Knuth indicates that 25 passes are reasonable.
 *
 * Contributed by John Amanatides, Nov 13, 1991.
 */

#include <gmp.h>

static int
possibly_prime (n, n_minus_1, x, y, q, k)
     MP_INT *n, *n_minus_1, *x, *y, *q;
     int k;
{
  int i;

  /* find random x s.t. 1 < x < n */
  do
    {
      mpz_random (x, mpz_size (n));
      mpz_mmod (x, x, n);
    }
  while (mpz_cmp_ui (x, 1) <= 0);

  mpz_powm (y, x, q, n);

  if (mpz_cmp_ui (y, 1) == 0 || mpz_cmp (y, n_minus_1) == 0)
    return 1;

  for (i = 1; i < k; i++)
    {
      mpz_powm_ui (y, y, 2, n);
      if (mpz_cmp (y, n_minus_1) == 0)
	return 1;
      if (mpz_cmp_ui (y, 1) == 0)
	return 0;
    }
  return 0;
}

int
mpz_probab_prime_p (m, reps)
     MP_INT *m;
     int reps;
{
  MP_INT n, n_minus_1, x, y, q;
  int i, k, is_prime;

  mpz_init (&n);
  /* Take the absolute value of M, to handle positive and negative primes.  */
  mpz_abs (&n, m);

  if (mpz_cmp_ui (&n, 1) <= 0)
    return 0;			/* smallest prime is 2 */
  if ((mpz_get_ui (&n) & 1) == 0)
    return 0;			/* even */
	
  mpz_init (&n_minus_1);
  mpz_sub_ui (&n_minus_1, &n, 1);
  mpz_init (&x);
  mpz_init (&y);

  /* find q and k, s.t.  n = 1 + 2**k * q */
  mpz_init_set (&q, &n_minus_1);
  k = 0;
  while ((mpz_get_ui (&q) & 1) == 0)
    {
      k++;
      mpz_div_2exp (&q, &q, 1);
    }

  is_prime = 1;
  for (i = 0; i < reps && is_prime; i++)
    is_prime &= possibly_prime (&n, &n_minus_1, &x, &y, &q, k);
	
  mpz_clear (&n_minus_1);
  mpz_clear (&n);
  mpz_clear (&x);
  mpz_clear (&y);
  mpz_clear (&q);
  return is_prime;
}
