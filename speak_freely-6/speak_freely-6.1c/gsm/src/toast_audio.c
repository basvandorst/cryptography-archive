/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /home/kbs/jutta/src/gsm/gsm-1.0/src/RCS/toast_audio.c,v 1.1 1992/10/28 00:15:50 jutta Exp $ */

#include	"toast.h"

/*  toast_audio -- functions to manipulate SunOS audio files.
 *
 *  This is reverse engineered from our present soundfiles
 *  and in no way portable, durable or aesthetically pleasing.
 */

extern FILE	* in, 	  * out;
extern char	* inname;
extern char	* progname;

extern int	(*output) P((gsm_signal *)),
		(*input ) P((gsm_signal *));

extern int	alaw_input   P((gsm_signal *)),
		ulaw_input   P((gsm_signal *)),
		linear_input P((gsm_signal *));

extern int	ulaw_output P((gsm_signal *));

static int put_u32 P2((f, u), FILE * f, unsigned long u)
{
	/*  Write a 32-bit unsigned value msb first. 
	 */
	if (  putc( (char)((u>>24) & 0x0FF), f) == EOF
	   || putc( (char)((u>>16) & 0x0FF), f) == EOF
	   || putc( (char)((u>> 8) & 0x0FF), f) == EOF
	   || putc( (char)( u      & 0x0FF), f) == EOF) return -1;

	return 0;
}

static int get_u32 P2((f, up), FILE * f, unsigned long * up)
{
	/*  Read a 32-bit unsigned value msb first.
	 */
	int		i;
	unsigned long 	u;

	if (  (i = getc(f)) == EOF
	   || ((u = (unsigned char)i), (i = getc(f)) == EOF)
	   || ((u = (u<<8)|(unsigned char)i), (i = getc(f)) == EOF)
	   || ((u = (u<<8)|(unsigned char)i), (i = getc(f)) == EOF)) return -1;

	*up = u | (unsigned char)i;
	return 0;
}

int audio_init_input P0()
{
	unsigned long	len, enc;	/* unsigned 32 bits	*/

	if (  fgetc(in) != '.' 
	   || fgetc(in) != 's'
	   || fgetc(in) != 'n'
	   || fgetc(in) != 'd'
	   || get_u32( in, &len )
	   || get_u32( in, &enc )	/* skip this */
	   || get_u32( in, &enc )
	   || fseek(in, len-4*4, 1) < 0) return 0;

	switch (enc) {
	case 1:	input = ulaw_input;  	break;
	case 2: input = alaw_input;  	break;
	case 3: input = linear_input; 	break;
	default:
		fprintf(stderr,
		"%s: warning: file format #%lu for %s not implemented.\n",
			progname, enc, inname);
		input = ulaw_input;
		break;
	}
	return 0;
}

int audio_init_output P0()
{
	if (  fputs(".snd", out) == EOF
	   || put_u32(out, 32)
	   || put_u32(out, ~0)
	   || put_u32(out, 1)
	   || put_u32(out, 8000)
	   || put_u32(out, 1)
	   || put_u32(out, 0)
	   || put_u32(out, 0)) return -1;

	return 0;
}

