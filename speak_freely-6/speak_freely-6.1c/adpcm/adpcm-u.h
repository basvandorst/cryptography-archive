/*
** adpcm.h - include file for adpcm coder.
**
** Version 1.0, 7-Jul-92.
*/

struct adpcm_state {
    short       valprev;        /* Previous output value */
    char        index;          /* Index into stepsize table */
};

#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

void adpcm_coder_u ARGS((unsigned char [], char [], int, struct adpcm_state *));
void adpcm_decoder_u ARGS((char [], unsigned char [], int, struct adpcm_state *));
