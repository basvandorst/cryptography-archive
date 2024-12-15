/**********************************************************************
*
*	ENERGY Version 50
*
**********************************************************************
*
* Compute RMS energy
*
* Inputs:
*  LEN    - Length of speech buffer
*  SPEECH - Speech buffer
* Output:
*  RMS    - Root Mean Square energy
*/

#include <math.h>

energy( len, speech, rms )
int len;
float speech[], *rms;
{
int  i;


*rms = 0;
for(i=1;i<=len;i++)
	*rms += speech[i]*speech[i];

*rms = sqrt( *rms / len );

}
