/*

	Sound interface for Speak Freely for Unix

	Designed and implemented in July of 1990 by John Walker

*/

#define BUFL	8000

#include "speakfree.h"

#if defined(BSD_like) || defined(Solaris) || defined(LINUX)

#ifdef Solaris
#include <sys/filio.h>
#else
#include <sys/dir.h>
#include <sys/file.h>
#endif

/*#include <math.h>*/
#include <errno.h>

#include <sys/ioctl.h>
#ifdef sun
#include <sys/stropts.h>
#ifdef Solaris
#include <sys/audioio.h>
#else
#include <sun/audioio.h>
#endif
#else /* FreeBSD / Linux */
#ifdef LINUX
#include <linux/soundcard.h>
#else
#include <machine/soundcard.h>
#endif
#define AUDIO_MIN_GAIN 0
#define AUDIO_MAX_GAIN 255
static int abuf_size;
#endif

#define SoundFile	"/dev/audio"
#ifdef sun
#define AUDIO_CTLDEV	"/dev/audioctl"
#else
#define AUDIO_CTLDEV	"/dev/mixer"
#endif

#define MAX_GAIN	100

struct sound_buf {
    struct sound_buf *snext;	      /* Next sound buffer */
    int sblen;			      /* Length of this sound buffer */
    unsigned char sbtext[2];	      /* Actual sampled sound */
};

/*  Local variables  */

static int audiof = -1; 	      /* Audio device file descriptor */
static int Audio_fd;		      /* Audio control port */
#ifdef sun
static audio_info_t Audio_info;       /* Current configuration info */
#endif
struct sound_buf *sbchain = NULL,     /* Sound buffer chain links */
		 *sbtail = NULL;
static int sbtotal = 0; 	      /* Total sample bytes in memory */
static int playing = FALSE;	      /* Replay in progress ? */
/* static int playqsize;  */		    /* Output queue size */
static int playlen = 0; 	      /* Length left to play */
static unsigned char *playbuf= NULL;  /* Current play pointer */
static int squelch = 0; 	      /* Squelch value */

/* Convert local gain into device parameters */

static unsigned scale_gain(g)
  unsigned g;
{
    return (AUDIO_MIN_GAIN + (unsigned)
	((int) ((((double) (AUDIO_MAX_GAIN - AUDIO_MIN_GAIN)) *
	((double) g / (double) MAX_GAIN)) + 0.5)));
}

#ifdef Solaris

/*  SETAUBUFSIZE  --  Preset size of internal /dev/audio buffer segments
		      Must be called before soundinit()  */

static int aubufsize = 2048;	      /* Default */

void setaubufsize(size)
  int size;
{
    aubufsize = size;
}
#endif


/*  SOUNDINIT  --  Open the sound peripheral and initialise for
		   access.  Return TRUE if successful, FALSE
		   otherwise.  */

int soundinit(iomode)
  int iomode;
{
    int attempts = 3;

    assert(audiof == -1);
    while (attempts-- > 0) {
	if ((audiof = open(SoundFile, iomode)) >= 0) {

	    if ((Audio_fd = open(AUDIO_CTLDEV, O_RDWR)) < 0) {
		perror(AUDIO_CTLDEV);
		return FALSE;
	    }
    /*fcntl(audiof, F_SETFL, O_NDELAY);*/
#ifndef sun
#ifndef AUDIO_BLOCKING
	    if (ioctl(audiof, SNDCTL_DSP_NONBLOCK, NULL) < 0) {
		perror("SNDCTL_DSP_NONBLOCK");
		return FALSE;
	    }
	    if (ioctl(audiof, SNDCTL_DSP_GETBLKSIZE, &abuf_size) < 0) {
		perror("SNDCTL_DSP_GETBLKSIZE");
		return FALSE;
	    }
#endif
#else
	    AUDIO_INITINFO(&Audio_info);
	    if (iomode != O_RDONLY) {
		Audio_info.play.sample_rate = 8000;
		Audio_info.play.channels = 1;
		Audio_info.play.precision = 8;
		Audio_info.play.encoding = AUDIO_ENCODING_ULAW;
	    }
	    if (iomode != O_WRONLY) {
		Audio_info.record.sample_rate = 8000;
		Audio_info.record.channels = 1;
		Audio_info.record.precision = 8;
		Audio_info.record.encoding = AUDIO_ENCODING_ULAW;
#ifdef Solaris
		Audio_info.record.buffer_size = aubufsize;
#endif
	    }
	    ioctl(Audio_fd, AUDIO_SETINFO, &Audio_info);
#endif
	    return TRUE;
	}
	if (errno != EINTR) {
	    break;
	}
fprintf(stderr, "Audio open: retrying EINTR attempt %d\n", attempts);
    }
    return FALSE;
}

/*  SOUNDTERM  --  Close the sound device.  */

void soundterm()
{
	if (audiof >= 0) {
	    if (close(audiof) < 0) {
		perror("closing audio device");
	    }
	    if (close(Audio_fd) < 0) {
		perror("closing audio control device");
	    }
	    audiof = -1;
	}
}

/*  SOUNDPLAY  --  Begin playing a sound.  */

void soundplay(len, buf)
  int len;
  unsigned char *buf;
{
    int ios;

    assert(audiof != -1);
    while (TRUE) {
	ios = write(audiof, buf, len);
	if (ios == -1) {
	    sf_usleep(100000);
	} else {
	    if (ios < len) {
		buf += ios;
		len -= ios;
	    } else {
		break;
	    }
	}
    }
}

/*  SOUNDPLAYVOL  --  Set playback volume from 0 (silence) to 100 (full on). */

void soundplayvol(value)
  int value;
{
#ifdef sun
    AUDIO_INITINFO(&Audio_info);
    Audio_info.play.gain = scale_gain(value);
    if (ioctl(Audio_fd, AUDIO_SETINFO, &Audio_info) < 0) {
	perror("Set play volume");
    }
#else
   int arg;

   arg	 = (value << 8) | value;

   if (ioctl(Audio_fd, MIXER_WRITE(SOUND_MIXER_PCM), &arg) < 0)
	perror("SOUND_MIXER_PCM");
#endif
}

/*  SOUNDRECGAIN  --  Set recording gain from 0 (minimum) to 100 (maximum).  */

void soundrecgain(value)
  int value;
{
#ifdef sun
    AUDIO_INITINFO(&Audio_info);
    Audio_info.record.gain = scale_gain(value);
    if (ioctl(Audio_fd, AUDIO_SETINFO, &Audio_info) < 0) {
	perror("Set record gain");
    }
#else
    int arg;

    arg   = (value << 8) | value;

    if (ioctl(Audio_fd, SOUND_MIXER_WRITE_RECLEV, &arg) < 0)
	perror("SOUND_MIXER_WRITE_RECLEV");
#endif
}

/*  SOUNDDEST  --  Set destination for generated sound.  If "where"
		   is 0, sound goes to the built-in speaker; if
		   1, to the audio output jack. */

void sounddest(where)
  int where;
{
#ifdef sun
    AUDIO_INITINFO(&Audio_info);
    Audio_info.play.port = (where == 0 ? AUDIO_SPEAKER : AUDIO_HEADPHONE);
    if (ioctl(Audio_fd, AUDIO_SETINFO, &Audio_info) < 0) {
	perror("Set output port");
    }
#endif
}

/*  SOUNDGRAB  --  Return audio information in the record queue.  */

int soundgrab(buf, len)
    char *buf;
    int len;
{
    long read_size;
    int c;

	read_size = len;
#ifndef sun
#ifndef AUDIO_BLOCKING
	if (read_size > abuf_size) {
	    read_size = abuf_size;
	}
#endif
#endif
	while (TRUE) {
	    c = read(audiof, buf, read_size);
	    if (c < 0) {
		if (errno == EINTR) {
		    continue;
		} else if (errno == EAGAIN) {
		    c = 0;
		}
	    }
	    break;
	}
	if (c < 0) {
	    perror("soundgrab");
	}
	return c;
}

/*  SOUNDFLUSH	--  Flush any queued sound.  */

void soundflush()
{
    char sb[BUFL];
#ifndef sun
    int c;
#endif

#ifndef sun
#ifndef AUDIO_BLOCKING
    while (TRUE) {
	c = read(audiof, sb, BUFL < abuf_size ? BUFL : abuf_size);
	if (c < 0 && errno == EAGAIN)
	    c = 0;
	if (c < 0)
	    perror("soundflush");
	if (c <= 0)
	    break;
    }
#endif
#else
    if (ioctl(audiof, I_FLUSH, FLUSHR)) {
	perror("soundflush");
    }
#endif
}
#endif

#ifdef sgi

/*

     Silicon Graphics audio drivers adapted from the stand-alone
	  Speak Freely for SGI designed and implemented by:

			    Paul Schurman
			    Espoo, Finland

			     16 July 1995

*/

/*#include <math.h>*/
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <dmedia/audio.h>

#include "ulaw2linear.h"

/*  Local variables  */

static ALport audioport;	      /* Audio I/O port */
static long origParams[] = {
    AL_INPUT_RATE, 0,
    AL_OUTPUT_RATE, 0,
    AL_LEFT_SPEAKER_GAIN, 0,
    AL_RIGHT_SPEAKER_GAIN, 0,
    AL_LEFT_INPUT_ATTEN, 0,
    AL_RIGHT_INPUT_ATTEN, 0
};
static long origMute[] = {
    AL_SPEAKER_MUTE_CTL, 0
};

/*  SOUNDINIT  --  Open the sound peripheral and initialise for
		   access.  Return TRUE if successful, FALSE
		   otherwise.  */

int soundinit(iomode)
  int iomode;
{
    ALconfig audioconfig; 
    int err;
    static long params[] = {
	AL_INPUT_RATE, 8000,
	AL_OUTPUT_RATE, 8000
    };

    ALseterrorhandler(0);

    /* Get a new audioconfig and set parameters. */
  
    audioconfig = ALnewconfig();   
    ALsetsampfmt(audioconfig, AL_SAMPFMT_TWOSCOMP);
    ALsetwidth(audioconfig, AL_SAMPLE_16);
    ALsetqueuesize(audioconfig, 8000L);  
    ALsetchannels(audioconfig, AL_MONO);   

    /* Save original of all global modes we change. */ 

    ALgetparams(AL_DEFAULT_DEVICE, origMute, 2);
    ALgetparams(AL_DEFAULT_DEVICE, origParams, 12);

    /* Set input and output data rates to 8000 samples/second. */

    ALsetparams(AL_DEFAULT_DEVICE, params, 4);

    /* Open the audioport. */

    audioport = ALopenport((iomode & O_WRONLY) ? "Speaker" : "Mike",
			   (iomode & O_WRONLY) ? "w" : "r", audioconfig);
    if (audioport == (ALport) 0) {	
       err = oserror();      
       if (err == AL_BAD_NO_PORTS) {
	  fprintf(stderr, " System is out of audio ports\n"); 
       } else if (err == AL_BAD_DEVICE_ACCESS) { 
	  fprintf(stderr, " Couldn't access audio device\n"); 
       } else if (err == AL_BAD_OUT_OF_MEM) { 
	  fprintf(stderr, " Out of memory\n"); 
       } 
       return FALSE;
    }  

    if (ALfreeconfig(audioconfig) != 0) {
       err = oserror();
       if (err == AL_BAD_CONFIG) {
	  fprintf(stderr, " Config not valid");
	  return FALSE;
       }
    } 

    /* Initialized the audioport okay. */

    return TRUE;
}

/*  SOUNDTERM  --  Close the sound device.  */

void soundterm()
{
    ALsetparams(AL_DEFAULT_DEVICE, origParams, 12);
    ALsetparams(AL_DEFAULT_DEVICE, origMute, 2);
    ALcloseport(audioport);
}

/*  SOUNDPLAY  --  Begin playing a sound.  */

void soundplay(len, buf)
  int len;
  unsigned char *buf;
{
    int i;
    short abuf[BUFL];

    for (i = 0; i < len; i++) {
	abuf[i] = audio_u2s(buf[i]);
    }
    ALwritesamps(audioport, abuf, len);
}

/*  SOUNDPLAYVOL  --  Set playback volume from 0 (silence) to 100 (full on). */

void soundplayvol(value)
  int value;
{
    long par[] = {
	AL_LEFT_SPEAKER_GAIN, 0,
	AL_RIGHT_SPEAKER_GAIN, 0
    };

    par[1] = par[3] = (value * 255L) / 100;
    ALsetparams(AL_DEFAULT_DEVICE, par, 4);
}

/*  SOUNDRECGAIN  --  Set recording gain from 0 (minimum) to 100 (maximum).  */

void soundrecgain(value)
  int value;
{
    long par[] = {
	AL_LEFT_INPUT_ATTEN, 0,
	AL_RIGHT_INPUT_ATTEN, 0
    };

    par[1] = par[3] = ((100 - value) * 255L) / 100;
    ALsetparams(AL_DEFAULT_DEVICE, par, 4);
}

/*  SOUNDDEST  --  Set destination for generated sound.  If "where"
		   is 0, sound goes to the built-in speaker; if
		   1, to the audio output jack. */

void sounddest(where)
  int where;
{
    /* Since we can't mute independently, and this is used only
       for ring, always unmute the speaker and headphones. */

    static long par[] = {
	AL_SPEAKER_MUTE_CTL, AL_SPEAKER_MUTE_OFF
    };

    ALsetparams(AL_DEFAULT_DEVICE, par, 2);
}

/*  SOUNDGRAB  --  Return audio information in the record queue.  */

int soundgrab(buf, len)
    char *buf;
    int len;
{
    int i;
    short sb[BUFL];
/*  long filled = ALgetfilled(audioport);

    if (len > filled) {
	len = filled;
    }
len=488;
*/
/*len=1600;*/
    if (len > 0) {
       ALreadsamps(audioport, sb, len);

      
       for (i = 0; i < len; i++) {
	  buf[i] = audio_s2u(sb[i]);
       }
    }
    return len;
}

/*  SOUNDFLUSH	--  Flush any queued sound.  */

void soundflush()
{
    short sb[BUFL];

    while (TRUE) {
	long l = ALgetfilled(audioport);

	if (l < 400) {
	    break;
	} else {
	    if (l > BUFL) {
		l = BUFL;
	    }
	    ALreadsamps(audioport, sb, l);
	}
    }
}
#endif
