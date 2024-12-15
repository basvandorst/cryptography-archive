/*+********************************************************
 * File name   :  GAMMA.H
 * Title       :  Incomplete gamma functions for Chi-square
 * Project     :  COS 497
 * Date created:  10 October, 1994
 * Revision    :  Rev 0.0
 * Author      :  C. C. Stevens
 * History     :  Rev 0.0  16-06-1994   start of coding
 *             :
 *
 * Description : Incomplete gamma function
 *             : Tests based on:
 *               Numerical Recipes in C
 *
 *-*******************************************************/

#define	ITMAX	100					// max. nr of iterations
#define	EPS		3.0e-7				// relative accuracy
#define	FPMIN	1.0e-30				// smallest nr near zero

// Returns the incomplete gamma function Q(a, x) == 1 - P(a, x)
float	gammaq(float a, float x);

// Returns the incomplete gamma function P(a, x) evaluated by its
// series representation as gamser
int	gser(float *gamser, float a, float x, float *gln);

// Returns the incomplete gamma function Q(a, x) evaluated by its
// continued fraction as gammacf
int	gcf(float *gammacf, float a, float x, float *gln);

// Returns ln(tau(xx)) for xx > 0
float	gammaln(float xx);
