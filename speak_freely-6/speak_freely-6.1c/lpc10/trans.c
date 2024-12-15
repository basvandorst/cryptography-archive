/*********************************************************************
*
*	TRANS Version 54
*
**********************************************************************
*
*   Handle Quantization and Input/Output of LPC parameters
*
* Input:
*  ORDER - Prediction order
* In/Outputs:
*  VOICE - Half frame voicing decisions
*  PITCH - Pitch index
*  RMS	 - Energy
*  RC	 - Reflection coefficients
*  EOF	 - End of file flag
*/

#include <stdio.h>
#include "config.ch"
#include "contrl.ch"
#include "lpcdefs.h"


trans(voice, pitch, rms, rc)
int voice[2], *pitch;
float *rms, rc[ORDER];
{
int i, ipitv, irms, irc[MAXORD], ibits[MAXNB];

/* Initialization */
memset(ibits, 0, MAXNB*sizeof(int));

/*     Quantize to 2400 bps, 600 bps, 800 bps or 1200 bps	*/

  encode(voice, pitch, rms, rc-1, &ipitv, &irms, irc-1);
  channel(0, &ipitv, &irms, irc-1, ibits-1);

/*  Decode parameters from bitstream	*/
  channel(1, &ipitv, &irms, irc-1, ibits-1);
  decode(ipitv, &irms, irc-1, voice, pitch, rms, rc-1);
}
