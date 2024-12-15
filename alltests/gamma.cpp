/*+********************************************************
 * File name   :  GAMMA.CPP
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
//#pragma option -v-

/*page*/
/************************INCLUDE FILES********************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gamma.h"

// Returns the incomplete gamma function Q(a, x) == 1 - P(a, x)
float	gammaq(float a, float x)
{
	float	gamser, gammacf, gln;

	if (x < 0.0 || a <= 0.0)
		return 0;

	if (x < (a+1.0))				// use series representation
	{
		gser(&gamser, a, x, &gln);
		return (1.0 - gamser);		// get complement
	}
	else							// use continued fraction
	{
		gcf(&gammacf, a, x, &gln);
		return gammacf;
	}
}

// Returns the incomplete gamma function P(a, x) evaluated by its
// series representation as gamser
int	gser(float *gamser, float a, float x, float *gln)
{
	int		n;
	float	sum, del, ap;

	*gln	= gammaln(a);
	if (x <= 0.0)
	{
		*gamser	= 0.0;
		return 0;
	}
	else
	{
		ap	= a;
		del	= sum	= 1.0/a;
		for (n = 1; n < ITMAX; n++)
		{
			++ap;
			del	*= x/ap;
			sum	+= del;
			if (fabs(del) < fabs(sum) * EPS)
			{
				*gamser	= sum * exp(-x+a*log(x)-(*gln));
				return 1;
			}
		}
	}
	return 0;
}

// Returns the incomplete gamma function Q(a, x) evaluated by its
// continued fraction as gammacf
int	gcf(float *gammacf, float a, float x, float *gln)
{
	int		i;
	float	an, b, c, d, del, h;

	*gln	= gammaln(a);
	b		= x+1.0-a;
	c		= 1.0/FPMIN;
	d		= 1.0/b;
	h		= d;

	for (i = 1; i < ITMAX; i++)
	{
		an	= -i*(i-a);
		b  += 2.0;
		d	= an*d+b;
		if (fabs(d) < FPMIN)
			d = FPMIN;
		c	= b+an/c;
		if (fabs(c) < FPMIN)
			c = FPMIN;
		d	= 1.0/d;
		del	= d*c;
		h  *= del;
		if (fabs(del - 1.0) < EPS)
			break;
	}
	*gammacf	= exp(-x+a*log(x)-(*gln))*h;
	if (i > ITMAX)
		return 0;
	else
		return 1;
}

// Returns ln(tau(xx)) for xx > 0
float	gammaln(float xx)
 {
	int		j;
	double	x, y, tmp, ser;
	static	double cof[6]	=
	{
		76.18009172947146, -86.50532032941677, 24.01409824083091,
		-1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5
	};
	y	= xx;
	x 	= xx;
	tmp	= x+5.5;
	tmp-= (x+0.5)*log(tmp);
	ser	= 1.000000000190015;
	for (j = 0; j < 6; j++)
		ser	+= cof[j]/++y;

	return (-tmp+log(2.5066282746310005*ser/x));
}

