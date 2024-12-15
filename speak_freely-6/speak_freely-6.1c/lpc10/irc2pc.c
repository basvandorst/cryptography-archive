/******************************************************************
*
*	IRC2PC Version 48
*
******************************************************************
*
*   Convert Reflection Coefficients to Predictor Coeficients
*
* Inputs:
*  RC     - Reflection coefficients
*  ORDER  - Number of RC's
*  GPRIME - Excitation modification gain
* Outputs:
*  PC     - Predictor coefficients
*  *G2PASS - Excitation modification sharpening factor
*/

#include "config.ch"
#include "lpcdefs.h"
#include <math.h>

irc2pc( rc, pc, gprime, g2pass, where )
float rc[MAXORD][11], pc[], gprime, *g2pass;
int where;
{
int i,j;
float temp[MAXORD];

*g2pass = 1.;

for(i=0;i<ORDER;i++)	
	*g2pass = *g2pass*( 1. - rc[i][where]*rc[i][where] );

*g2pass = gprime*sqrt(*g2pass);
pc[0] = rc[0][where];

for(i=1;i<ORDER;i++)	{
	for(j=0;j<i;j++)
		temp[j] = pc[j] - rc[i][where]*pc[i-j-1];
	
	for(j=0;j<i;j++)
		pc[j] = temp[j];
	
	pc[i] = rc[i][where];
}

}
